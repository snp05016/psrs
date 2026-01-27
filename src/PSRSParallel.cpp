#include "PSRS.h"
#include "KWayMerge.h"
#include "QuickSort.h"
#include "DataGenerator.h"
#include "pthread_barrier.h"
string OPT_FLAG = "O3"; // putting on top so i dont have to scroll all the way to the bottom
pthread_mutex_t buckets_mutex = PTHREAD_MUTEX_INITIALIZER;
vector<double> phasetimes(6, 0.0);
pthread_barrier_t barrier;

void* phase1_pthread(void* arg) {
  phase1Args* args = (phase1Args*)arg;
  phase1(*args->inputVector, args->r_size, args->size, args->processor_i,
         args->n, args->p, *args->sample);
  return nullptr;
}

void* phase2_pivot_pthread(void* arg) {
  phase2PivotArgs* args = (phase2PivotArgs*)arg;
  int start = args->thread_id * args->pivots_per_thread;
  int end = std::min(start + args->pivots_per_thread, args->p - 1);
  for (int i = start; i < end; i++) {
    (*args->pivots)[i] = (*args->sample)[(i * args->p - 1) + ((args->p - 1) / 2)];
  }
  return nullptr;
}

void* phase3_pthread(void* arg) {
  phase3Args* args = (phase3Args*)arg;
  phase3(*args->subsize, *args->pivots, *args->inputVector, args->size,
         args->p, args->n, args->processor_i);
  return nullptr;
}

void* phase4_pthread(void* arg) {
  phase4Args* args = (phase4Args*)arg;
  phase4(*args->buckets, *args->subsize, *args->inputVector, args->p,
         args->processor_i);
  return nullptr;
}

void* phase5_merge_pthread(void* arg) {
  phase5MergeArgs* args = (phase5MergeArgs*)arg;
  // Merge the assigned bucket (which belongs to processor_i)
  (*args->sorted_parts)[args->processor_i] = merge((*args->buckets)[args->processor_i]);
  return nullptr;
}

vector<int> PSRS(vector<int> &inputVector, int n, int p) {
  vector<int> sample(p * (p - 1));
  vector<int> pivots(p - 1, 0);

  int size = floor((n + p - 1) / p);
  int r_size = floor((size + p - 1) / p);
  
  // max speedup type shi 
  pthread_setconcurrency(p);
  
  // Initialize barrier for p threads
  pthread_barrier_init(&barrier, NULL, p);
  
  struct timeval start, end;
  
  // phase 1: sort small segs and select the samples 
  gettimeofday(&start, NULL);
  pthread_t* threads = new pthread_t[p];
  phase1Args* phase1_args = new phase1Args[p];
  
  for (int processor_i = 0; processor_i < p; processor_i++) {
    phase1_args[processor_i] = {&inputVector, r_size, size, processor_i, n, p, &sample};
    pthread_create(&threads[processor_i], NULL, phase1_pthread, &phase1_args[processor_i]);
  }
  
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&end, NULL);

  // calcualte phase 1 time
  long seconds = end.tv_sec - start.tv_sec;
  long ms = end.tv_usec - start.tv_usec;
  double elpsed = seconds + ms * 1e-6;
  phasetimes[0] = elpsed;

  delete[] phase1_args;
  
  // phase 2a: sort samples
  gettimeofday(&start, NULL);
  quickSort(sample, 0, p * (p - 1) - 1);
  gettimeofday(&end, NULL);
  // calcualte phase 2 sample sort time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[1] = elpsed;

  // phase 2b: select pivots
  gettimeofday(&start, NULL);
  int pivots_per_thread = (p - 1 + p - 1) / p;
  phase2PivotArgs* phase2_args = new phase2PivotArgs[p];
  
  for (int t = 0; t < p; t++) {
    phase2_args[t] = {&pivots, &sample, p, t, pivots_per_thread};
    pthread_create(&threads[t], NULL, phase2_pivot_pthread, &phase2_args[t]);
  }
  
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&end, NULL);
  // calcualte phase 2 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[2] = elpsed;

  delete[] phase2_args;

  // phase 3: Partition local segments by pivots
  gettimeofday(&start, NULL);
  vector<int> subsize(p * p, 0);
  phase3Args* phase3_args = new phase3Args[p];
  
  for (int i = 0; i < p; i++) {
    phase3_args[i] = {&subsize, &pivots, &inputVector, size, p, n, i};
    pthread_create(&threads[i], NULL, phase3_pthread, &phase3_args[i]);
  }
  
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&end, NULL);
  // calcualte phase 3 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[3] = elpsed;

  delete[] phase3_args;
  
  subsize.insert(subsize.begin(), 0);
  
  // phase 4: Exchange and merge
  gettimeofday(&start, NULL);
  // Pre-allocate buckets to avoid locking (p x p structure)
  vector<vector<vector<int>>> buckets(p, vector<vector<int>>(p));
  phase4Args* phase4_args = new phase4Args[p];
  
  for (int processor = 0; processor < p; processor++) {
    phase4_args[processor] = {&buckets, &subsize, &inputVector, p, processor};
    pthread_create(&threads[processor], NULL, phase4_pthread, &phase4_args[processor]);
  }
  
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&end, NULL);
  // calcualte phase 4 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[4] = elpsed;

  delete[] phase4_args;
  

  // phase 4(b)  --> merging the bucks 

  gettimeofday(&start, NULL);
  
  // Parallelize the merge phase: Each processor merges its own bucket
  vector<vector<int>> sorted_parts(p);
  phase5MergeArgs* phase5_args = new phase5MergeArgs[p];
  
  for (int processor = 0; processor < p; processor++) {
    phase5_args[processor] = {&buckets, &sorted_parts, processor};
    pthread_create(&threads[processor], NULL, phase5_merge_pthread, &phase5_args[processor]);
  }
  
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  
  // Final concatenation (sequential but fast)
  vector<int> sorted_array;
  
  // Pre-calculate total size to reserve memory
  int total_size = 0;
  for (const auto& part : sorted_parts) {
      total_size += part.size();
  }
  sorted_array.reserve(total_size);
  
  for (const auto& part : sorted_parts) {
      sorted_array.insert(sorted_array.end(), part.begin(), part.end());
  }

  gettimeofday(&end, NULL);
  // calcualte phase 4(b) time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[5] = elpsed;
  
  delete[] phase5_args;
  delete[] threads;

  // Destroy barrier
  pthread_barrier_destroy(&barrier);
  
  return sorted_array;
}
void phase1(vector<int> &inputVector, int r_size, int size, int processor_i,
            int n, int p, vector<int> &sample) {
  int start = processor_i * size;
  int end = (processor_i + 1) * size - 1;
  if (end >= n) {
    end = n - 1;
  }

  quickSort(inputVector, start, end);

  for (int j = 1; j < p; j++) {
    if (start + (j * r_size) <= end) {
      sample[(processor_i * (p - 1)) + j - 1] =
          inputVector[start + (j * r_size)];
    } else {
      sample[(processor_i * (p - 1)) + j - 1] = inputVector[end];
    }
  }
}
void phase3(vector<int> &subsize, const vector<int> &pivots,
            vector<int> &inputVector, int size, int p, int n, int processor_i) {
 
    int start = processor_i * size;
    int end = (processor_i + 1) * size - 1;
    if (processor_i == p - 1) {
      end = n - 1;
    }
    subsize[(processor_i * (p))] = start;
    subsize[(processor_i * (p)) + p - 1] = end + 1;
    Sublists(inputVector, start, end, subsize, processor_i * (p), pivots, 0, p - 2);
}

void phase4(vector<vector<vector<int>>> &buckets, vector<int> &subsize,
                   vector<int> &inputVector, int p, int processor_i) {
  for (int bucket = 0; bucket < p; bucket++) {
    int start = subsize[processor_i * p + bucket];
    int end = subsize[processor_i * p + bucket + 1];
    if (start < end) {
      // Direct access without lock - safe because each processor_i writes to a unique slot
      buckets[bucket][processor_i] = vector<int>(inputVector.begin() + start,
                                                 inputVector.begin() + end);
    }
  }
}


void Sublists(vector<int> &vec, int start, int end, vector<int> &subsize,
              int at, const vector<int> &pivots, int fp, int lp) {

  int mid = floor((fp + lp) / 2);
  int pv = pivots[mid];
  int lb = start;
  int ub = end;

  while (lb <= ub) {
    int center = floor((lb + ub) / 2);
    if (vec[center] > pv) {
      ub = center - 1;
    } else {
      lb = center + 1;
    }
  }
  subsize[at + mid] = lb;
  if (fp < mid) {
    Sublists(vec, start, lb - 1, subsize, at, pivots, fp, mid - 1);
  }
  if (mid < lp) {
    Sublists(vec, lb, end, subsize, at, pivots, mid + 1, lp);
  }
}

int main(int /*argc*/, char *argv[]) {
  int n = stoi(argv[1]);  // getting the input file name
  string outputFileName = (argv[3]); // getting the output file name
  int num_processors = stoi(argv[2]);
  ofstream outputFile(outputFileName);

  if (!argv[1]) {
    cerr << "enter the first command line argv please --> number of "
            "processors";
  }
  struct timeval starttime, endtime;
  // cout << num_processors << endl;
  vector<int> keys = GenerateData(n, 1000000);
  // int size_of_input_vector = keys.size();
  vector<int> copy_key = keys;
  // vector<int> keys = {16, 2,  17, 24, 33, 28, 30, 1,  0,  27, 9,  25,
  //                     34, 23, 19, 18, 11, 7,  21, 13, 8,  35, 12, 29,
  //                     6,  3,  4,  14, 22, 15, 32, 10, 26, 31, 20, 5};
  gettimeofday(&starttime, NULL);
  vector<int> c = PSRS(keys, keys.size(), num_processors);
  gettimeofday(&endtime, NULL);

  long seconds = endtime.tv_sec - starttime.tv_sec;
  long ms = endtime.tv_usec - starttime.tv_usec;
  double elpsed = seconds + ms * 1e-6;
  if (!outputFile.is_open()) {
    cerr << "error opening output file" << endl;
    return 1;
  }

  outputFile << OPT_FLAG << ", " << n << ", "  << num_processors << ", " << elpsed << ", ";
  for (double phase_time : phasetimes) {
    outputFile << phase_time << ", ";
  }
  sort(copy_key.begin(), copy_key.end());
  outputFile << (copy_key == c) << endl; // verifying the correctness of the sort
  outputFile.close();

  // cout << "the elements of the sorted key are\n { " << endl;
  // for (const int elem : c)
  //   cout << elem << " ";
  // cout << " }";
}
