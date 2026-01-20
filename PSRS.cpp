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
  cout << "size: " << size << endl;
  int r_size = floor((size + p - 1) / p); // size of the regular sample
  for (int i = 0; i < p; i++) {
    cout << "iteration: " << i << endl;
    int start = i * size;
    int end = (i + 1) * size - 1;
    if (end >= n) {
      end = n - 1;
    }
    cout << "start: " << start << " end: " << end << endl;

    quickSort(inputVector, start, end);
    cout << "partially sorted inp vec\n[";
    for (int q = 0; q < inputVector.size(); q++) {
      if (q == start) {
        cout << "|";
      }
      cout << inputVector[q] << " ";
      if (q == end) {
        cout << "| ";
      }
    }
    cout << "]\n";
    for (int j = 1; j < p; j++) {
      if (start + (j * r_size) <= end) {
        sample[(i * (p - 1)) + j - 1] = inputVector[start + (j * r_size)];
      } else {
        sample[(i * (p - 1)) + j - 1] = inputVector[end];
      }
    }
    cout << "the element of the sample is [";
    for (const int elem : sample)
      cout << elem << " ";
    cout << "]\n";
  }

  quickSort(sample, 0, p * (p - 1) - 1);
  cout << "the element of the sorted sample is [";
  for (const int elem : sample)
    cout << elem << " ";
  cout << "]\n";

  for (int i = 0; i < p - 1; i++) {
    pivots[i] = sample[(i * p - 1) + ((p - 1) / 2)];
  }
  cout << "the element of the pivots is [";
  for (const int elem : pivots)
    cout << elem << " ";
  cout << "]\n";

  vector<int> subsize(p * (p - 1), 0);
  for (int i = 0; i < p - 1; i++) {
    int start = i * size;
    int end = (i + 1) * size - 1;
    if (end >= n) {
      end = n - 1;
    }
    subsize[i * (p + 1)] = start;
    subsize[i * (p + 1) + p] = end + 1;
    Sublists(inputVector, start, end, subsize, i * (p + 1), pivots, 1, p - 1);
  }
  for (const int elem : subsize)
    cout << "the element of the subsize is " << elem << endl;

  vector<vector<vector<int>>> buckets(
      p); // initializing a vector of buckets that contain vectors
          // of vectors of integers
  for (int processor = 0; processor < p; processor++) {
    for (int bucket = 0; bucket < p; bucket++) {
      int start = subsize[processor * (p + 1) + bucket];
      int end = subsize[processor * (p - 1) + bucket + 1];
      if (start < end) {
        buckets[bucket].push_back(vector<int>(inputVector.begin() + start,
                                              inputVector.begin() + end));
        for (const vector<int> elem : buckets[bucket]) {
          cout << "{";
          for (const int i : elem) {
            cout << i << " ";
          }
          cout << "}";
        }
      }
    }
  }
  vector<int> sorted_array;
  for (const vector<vector<int>> bucket : buckets) {
    vector<int> iter_vec = merge(bucket);
    sorted_array.insert(sorted_array.end(), iter_vec.begin(), iter_vec.end());
  }
  return sorted_array;
}

void Sublists(vector<int> vec, int start, int end, vector<int> subsize, int at,
              vector<int> pivots, int fp, int lp) {
  int mid = floor((fp + lp) / 2);
  int pv = pivots[mid];
  int lb = start;
  int ub = end;

  while (lb <= ub) {
    int center = floor((lb + ub) / 2);
    if (vec[center] > pv) {
      ub = center - 1;
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

int main() {
  vector<int> keys = {16, 2,  17, 24, 33, 28, 30, 1,  0,  27, 9,  25,
                      34, 23, 19, 18, 11, 7,  21, 13, 8,  35, 12, 29,
                      6,  3,  4,  14, 22, 15, 32, 10, 26, 31, 20, 5};
  vector<int> c = PSRS(keys, keys.size(), 3);
  cout << "the elements of the sorted key are " << endl;
  for (const int elem : c)
    cout << elem << endl;
}
