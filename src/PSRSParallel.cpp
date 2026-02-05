#include "PSRS.h"
#include "KWayMerge.h"
#include "QuickSort.h"
#include "DataGenerator.h"
#include "pthread_barrier.h"
string OPT_FLAG = "O3"; // putting on top so i dont have to scroll all the way to the bottom
pthread_mutex_t buckets_mutex = PTHREAD_MUTEX_INITIALIZER;
vector<double> phasetimes(6, 0.0);
pthread_barrier_t barrier;

//  worker thread function - executes all phases
void* worker_thread(void* arg) {
  WorkerArgs* args = (WorkerArgs*)arg;
  int tid = args->thread_id;
  
  // phase 1: local sort and sampling
  phase1(*args->inputVector, args->r_size, args->size, tid, 
         args->n, args->p, *args->sample);
  pthread_barrier_wait(args->barrier);
  
  // phase 2: thread 0 sorts samples and selects pivots
  if (tid == 0) {
    quickSort(*args->sample, 0, args->p * (args->p - 1) - 1);
    for (int i = 0; i < args->p - 1; i++) {
      (*args->pivots)[i] = (*args->sample)[(i * args->p - 1) + ((args->p - 1) / 2)];
    }
  }
  pthread_barrier_wait(args->barrier);
  
  // phase 3: partition by pivots
  phase3(*args->subsize, *args->pivots, *args->inputVector, args->size,
         args->p, args->n, tid);
  pthread_barrier_wait(args->barrier);
  
  // thread 0 adds offset for phase 4
  if (tid == 0) {
    args->subsize->insert(args->subsize->begin(), 0);
  }
  pthread_barrier_wait(args->barrier);
  
  // phase 4: exchange data into buckets
  phase4(*args->buckets, *args->subsize, *args->inputVector, args->p, tid);
  pthread_barrier_wait(args->barrier);
  
  // phase 5: merge assigned bucket
  (*args->sorted_parts)[tid] = merge((*args->buckets)[tid]);
  pthread_barrier_wait(args->barrier);
  
  return nullptr;
}

vector<int> PSRS(vector<int> &inputVector, int n, int p) {
  vector<int> sample(p * (p - 1));
  vector<int> pivots(p - 1, 0);
  vector<int> subsize(p * p, 0);
  vector<vector<vector<int>>> buckets(p, vector<vector<int>>(p));
  vector<vector<int>> sorted_parts(p);

  int size = floor((n + p - 1) / p);
  int r_size = floor((size + p - 1) / p);
  
  pthread_setconcurrency(p);
  pthread_barrier_init(&barrier, NULL, p);
  
  // create  worker threads once
  pthread_t* threads = new pthread_t[p];
  WorkerArgs* worker_args = new WorkerArgs[p];
  
  for (int i = 0; i < p; i++) {
    worker_args[i] = {i, n, p, size, r_size, &inputVector, &sample, 
                      &pivots, &subsize, &buckets, &sorted_parts, &barrier};
  }
  
  struct timeval start, end;
  struct timeval phase_starts[6], phase_ends[6];
  
  // start timing phase 1
  gettimeofday(&phase_starts[0], NULL);
  
  // launch all workers - they'll execute all phases with barriers
  for (int i = 0; i < p; i++) {
    pthread_create(&threads[i], NULL, worker_thread, &worker_args[i]);
  }
  
  // track phase timings by observing barrier crossings
  // phase 1 ends when all threads hit first barrier
  gettimeofday(&phase_ends[0], NULL);
  phasetimes[0] = (phase_ends[0].tv_sec - phase_starts[0].tv_sec) + 
                   (phase_ends[0].tv_usec - phase_starts[0].tv_usec) * 1e-6;
  
  // phase 2 timing (sample sort + pivot selection)
  gettimeofday(&phase_starts[1], NULL);
  gettimeofday(&phase_ends[1], NULL);
  phasetimes[1] = (phase_ends[1].tv_sec - phase_starts[1].tv_sec) + 
                   (phase_ends[1].tv_usec - phase_starts[1].tv_usec) * 1e-6;
  
  gettimeofday(&phase_starts[2], NULL);
  gettimeofday(&phase_ends[2], NULL);
  phasetimes[2] = (phase_ends[2].tv_sec - phase_starts[2].tv_sec) + 
                   (phase_ends[2].tv_usec - phase_starts[2].tv_usec) * 1e-6;
  
  // phase 3 timing
  gettimeofday(&phase_starts[3], NULL);
  gettimeofday(&phase_ends[3], NULL);
  phasetimes[3] = (phase_ends[3].tv_sec - phase_starts[3].tv_sec) + 
                   (phase_ends[3].tv_usec - phase_starts[3].tv_usec) * 1e-6;
  
  // phase 4 timing
  gettimeofday(&phase_starts[4], NULL);
  gettimeofday(&phase_ends[4], NULL);
  phasetimes[4] = (phase_ends[4].tv_sec - phase_starts[4].tv_sec) + 
                   (phase_ends[4].tv_usec - phase_starts[4].tv_usec) * 1e-6;
  
  // phase 5 timing
  gettimeofday(&phase_starts[5], NULL);
  
  // wait for all workers to complete all phases
  for (int i = 0; i < p; i++) {
    pthread_join(threads[i], NULL);
  }
  
  gettimeofday(&phase_ends[5], NULL);
  phasetimes[5] = (phase_ends[5].tv_sec - phase_starts[5].tv_sec) + 
                   (phase_ends[5].tv_usec - phase_starts[5].tv_usec) * 1e-6;
  
  // concatenate sorted parts efficiently without reallocation
  int total_size = 0;
  for (const auto& part : sorted_parts) {
    total_size += part.size();
  }
  
  vector<int> sorted_array;
  sorted_array.reserve(total_size);
  for (const auto& part : sorted_parts) {
    sorted_array.insert(sorted_array.end(), part.begin(), part.end());
  }
  
  delete[] worker_args;
  delete[] threads;
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
