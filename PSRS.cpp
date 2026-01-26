#include "PSRS.h"
#include "KWayMerge.h"
#include "QuickSort.h"

void phase1(vector<int> &inputVector, int r_size, int size, int processor_i,
            int n, int p, vector<int> &sample) {
  int start = processor_i * size;
  int end = (processor_i + 1) * size - 1;
  if (end >= n) {
    end = n - 1;
  }
  // cout << "start: " << start << " end: " << end << endl;

  quickSort(inputVector, start, end);
  // cout << "partially sorted inp vec\n[";
  for (size_t q = 0; q < inputVector.size(); q++) {
    if (q == start) {
      // cout << "|";
    }
    // cout << inputVector[q] << " ";
    if (q == end) {
      // cout << "| ";
    }
  }
  // cout << "]\n";
  for (int j = 1; j < p; j++) {
    if (start + (j * r_size) <= end) {
      sample[(processor_i * (p - 1)) + j - 1] =
          inputVector[start + (j * r_size)];
    } else {
      sample[(processor_i * (p - 1)) + j - 1] = inputVector[end];
    }
  }
}
void phase2(vector<int> &sample, vector<int> &pivots, int p) {
  quickSort(sample, 0, p * (p - 1) - 1); // quicksorting the samples array
  for (int i = 0; i < p - 1; i++) {
    pivots[i] = sample[(i * p - 1) + ((p - 1) / 2)];
  }
}

void phase3(vector<int> &subsize, const vector<int> &pivots, vector<int> &inputVector,
            int size, int p, int n) {
  for (int i = 0; i < p; i++) {
    int start = i * size;
    int end = (i + 1) * size - 1;
    if (i == p - 1) {
      end = n - 1;
    }
    subsize[(i * (p))] = start;
    subsize[(i * (p)) + p - 1] = end + 1;
    // cout << "starting the sublists function with start: " << start
    //    << " and end: " << end << endl;
    Sublists(inputVector, start, end, subsize, i * (p), pivots, 0, p - 2);
  }
}

vector<int> phase4(vector<vector<vector<int>>> &buckets, vector<int> &subsize,
                   vector<int> &inputVector, int p) {

  // of vectors of integers
  for (int processor = 0; processor < p; processor++) {
    // cout << "processor " << processor << ":" << endl;
    for (int bucket = 0; bucket < p; bucket++) {
      int start = subsize[processor * p + bucket];
      int end = subsize[processor * p + bucket + 1];
      if (start < end) {
        buckets[bucket].push_back(vector<int>(inputVector.begin() + start,
                                              inputVector.begin() + end));
        // cout << "bucket " << bucket << endl;
        // for (const vector<int> elem : buckets[bucket]) {
        // cout << "{";
        // for (const int i : elem) {
        // cout << i << " ";
        //}
        // cout << "}\n";
      }
    }
  }
  //}
  vector<int> sorted_array;
  for (const vector<vector<int>> &bucket : buckets) {
    vector<int> iter_vec = merge(bucket);
    sorted_array.insert(sorted_array.end(), iter_vec.begin(), iter_vec.end());
  }
  return sorted_array;
}
vector<int> PSRS(vector<int> &inputVector, int n,
                 int p) { // here the number of processors
                          // will be the number of threads
  vector<int> sample(
      p * (p - 1)); // initializing a sample vector of p(p-1) elements
  vector<int> pivots(p - 1, 0); // initializing a pivots array of p-1 elements
  int size = floor((n + p - 1) / p);
  // cout << "size: " << size << endl;
  int r_size = floor((size + p - 1) / p); // size of the regular sample
  struct timeval start, end;
  gettimeofday(&start, NULL);
  // phase 1
  for (int processor_i = 0; processor_i < p; processor_i++) {
    phase1(inputVector, r_size, size, processor_i, n, p, sample);
  }
  gettimeofday(&end, NULL);

  // calcualte phase 1 time
  long seconds = end.tv_sec - start.tv_sec;
  long ms = end.tv_usec - start.tv_usec;
  double elpsed = seconds + ms * 1e-6;
  phasetimes[0] = elpsed;
  // cout << "the element of the sample is [";
  // for (const int elem : sample)
  // cout << elem << " ";
  // cout << "]\n";

  
  // clear start time for phase 2
  gettimeofday(&start, NULL);
  // phase 2
  phase2(sample, pivots, p);
  gettimeofday(&end, NULL);
  // calcualte phase 2 sample sort time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[1] = elpsed;

  gettimeofday(&start, NULL);
  for (int i = 0; i < p - 1; i++) {
    pivots[i] = sample[(i * p - 1) + ((p - 1) / 2)];
  }
  gettimeofday(&end, NULL);
  // calcualte phase 2 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[2] = elpsed;
  // cout << "the element of the pivots is [";
  // for (const int elem : pivots)
  // cout << elem << " ";
  // cout << "]\n";

  // phase 3
  gettimeofday(&start, NULL);
  vector<int> subsize(p * p, 0);
  phase3(subsize, pivots, inputVector, size, p, n);
  gettimeofday(&end, NULL);
  // calcualte phase 3 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[3] = elpsed;

  subsize.insert(subsize.begin(), 0);

  //phase 4
  gettimeofday(&start, NULL);
  vector<vector<vector<int>>> buckets(p);
  
  for (int processor = 0; processor < p; processor++) {
    for (int bucket = 0; bucket < p; bucket++) {
      int start = subsize[processor * p + bucket + 1];
      int end = subsize[processor * p + bucket + 1 + 1];
      if (start < end && end <= (int)inputVector.size()) {
        buckets[bucket].push_back(vector<int>(inputVector.begin() + start,
                                              inputVector.begin() + end));
      }
    }
  }
  gettimeofday(&end, NULL);
  // calcualte phase 4 time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[4] = elpsed;


  // phase 4(b)  --> merging the bucks 

  gettimeofday(&start, NULL);
  vector<int> sorted_array;
  for (const vector<vector<int>> &bucket : buckets) {
    vector<int> iter_vec = merge(bucket);
    sorted_array.insert(sorted_array.end(), iter_vec.begin(), iter_vec.end());
  }
  gettimeofday(&end, NULL);
  // calcualte phase 4(b) time
  seconds = end.tv_sec - start.tv_sec;
  ms = end.tv_usec - start.tv_usec;
  elpsed = seconds + ms * 1e-6;
  phasetimes[5] = elpsed;
  
  return sorted_array;
}

void Sublists(vector<int> &vec, int start, int end, vector<int> &subsize,
              int at, const vector<int> &pivots, int fp,
              int lp) { // here fp is the index of the first pivot and lp is
                        // the index of the last pivot
  int mid = floor((fp + lp) / 2);
  int pv = pivots[mid];
  int lb = start;
  int ub = end;
  // cout << "start: " << lb << " end: " << ub << " center pivot: " << pv <<
  // endl;
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
int ReadFile(string fileName, vector<int> &toPutInto) {
  ifstream inputFile(fileName);
  if (!inputFile.is_open()) {
    cerr << " error " << endl;
    return 1;
  }
  int number;
  // cout << "helo" << endl;
  while (inputFile >> number) {
    toPutInto.push_back(number);
  }
  inputFile.close();
  return 0;
}
int main(int /*argc*/, char *argv[]) {
  string inputFileName = (argv[1]);  // getting the input file name
  string outputFileName = (argv[3]); // getting the output file name
  int num_processors = stoi(argv[2]);
  ofstream outputFile(outputFileName);

  if (!argv[1]) {
    cerr << "enter the first command line argv please --> number of "
            "processors";
  }
  struct timeval starttime, endtime;
  // cout << num_processors << endl;
  vector<int> keys;
  ReadFile(inputFileName, keys);
  int size_of_input_vector = keys.size();
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
  outputFile << inputFileName << ", " << size_of_input_vector << ", " << num_processors << ", " << elpsed << endl;
  for (double phase_time : phasetimes) {
    outputFile << phase_time << ", ";
  }
  outputFile.close();

  // cout << "the elements of the sorted key are\n { " << endl;
  // for (const int elem : c)
  //   cout << elem << " ";
  // cout << " }";
}
