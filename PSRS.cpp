#include "PSRS.h"
#include "KWayMerge.cpp"
#include "QuickSort.cpp"
vector<int> PSRS(vector<int> &inputVector, int n,
                 int p) { // here the number of processors
                          // will be the number of threads
  vector<int> sample(
      p * (p - 1)); // initializing a sample vector of p(p-1) elements
  vector<int> pivots(p - 1, 0); // initializing a pivots array of p-1 elements
  int size = floor((n + p - 1) / p);
  // cout << "size: " << size << endl;
  int r_size = floor((size + p - 1) / p); // size of the regular sample
  for (int i = 0; i < p; i++) {
    // cout << "iteration: " << i << endl;
    int start = i * size;
    int end = (i + 1) * size - 1;
    if (end >= n) {
      end = n - 1;
    }
    // cout << "start: " << start << " end: " << end << endl;

    quickSort(inputVector, start, end);
    // cout << "partially sorted inp vec\n[";
    for (int q = 0; q < inputVector.size(); q++) {
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
        sample[(i * (p - 1)) + j - 1] = inputVector[start + (j * r_size)];
      } else {
        sample[(i * (p - 1)) + j - 1] = inputVector[end];
      }
    }
    // cout << "the element of the sample is [";
    // for (const int elem : sample)
    // cout << elem << " ";
    // cout << "]\n";
  }

  quickSort(sample, 0, p * (p - 1) - 1);
  // cout << "the element of the sorted sample is [";
  // for (const int elem : sample)
  // cout << elem << " ";
  // cout << "]\n";

  for (int i = 0; i < p - 1; i++) {
    pivots[i] = sample[(i * p - 1) + ((p - 1) / 2)];
  }
  // cout << "the element of the pivots is [";
  // for (const int elem : pivots)
  // cout << elem << " ";
  // cout << "]\n";

  vector<int> subsize(p * p, 0);
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

  // adjusting subsizes array
  // for (int i = 0; i < subsize.size(); i++) {
  //  subsize[i] -= 1;
  //}

  subsize.insert(subsize.begin(), 0);
  // for (const int elem : subsize)
  // cout << elem << " ";

  // cout << endl;
  vector<vector<vector<int>>> buckets(
      p); // initializing a vector of buckets that contain vectors
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
  for (const vector<vector<int>> bucket : buckets) {
    vector<int> iter_vec = merge(bucket);
    sorted_array.insert(sorted_array.end(), iter_vec.begin(), iter_vec.end());
  }
  return sorted_array;
}

void Sublists(vector<int> &vec, int start, int end, vector<int> &subsize,
              int at, vector<int> &pivots, int fp,
              int lp) { // here fp is the index of the first pivot and lp is the
                        // index of the last pivot
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
  cout << "helo" << endl;
  while (inputFile >> number) {
    toPutInto.push_back(number);
  }
  inputFile.close();
  return 0;
}
int main(int argc, char *argv[]) {
  string inputFileName = (argv[1]);  // getting the input file name
  string outputFileName = (argv[3]); // getting the output file name
  int num_processors = stoi(argv[2]);
  ofstream outputFile(outputFileName);

  if (!argv[1]) {
    cerr << "enter the first command line argv please --> number of processors";
  }
  struct timeval starttime, endtime;
  cout << num_processors << endl;
  vector<int> keys;
  ReadFile(inputFileName, keys);
  // vector<int> keys = {16, 2,  17, 24, 33, 28, 30, 1,  0,  27, 9,  25,
  //                     34, 23, 19, 18, 11, 7,  21, 13, 8,  35, 12, 29,
  //                     6,  3,  4,  14, 22, 15, 32, 10, 26, 31, 20, 5};
  gettimeofday(&starttime, NULL);

  vector<int> c = PSRS(keys, keys.size(), 3);

  gettimeofday(&endtime, NULL);

  long seconds = endtime.tv_sec - starttime.tv_sec;
  long ms = endtime.tv_sec - starttime.tv_usec;
  double elpsed = seconds + ms * 1e-6;
  if (!outputFile.is_open()) {
    cerr << "error opening output file" << endl;
    return 1;
  }
  outputFile << "------------ Input File Name: " << inputFileName
             << " ------------" << endl;
  outputFile << "time taken: " << elpsed << endl;

  // cout << "the elements of the sorted key are\n { " << endl;
  // for (const int elem : c)
  //   cout << elem << " ";
  // cout << " }";
}
