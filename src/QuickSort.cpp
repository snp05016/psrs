#include "PSRS.h"
#include "QuickSort.h"

int Partition(vector<int> &vec, int low, int high) { // hoare's partion
  int pivot = vec[low];
  int i = low - 1;
  int j = high + 1;
  while (true) {
    i++;
    while (vec[i] < pivot) {
      i++;
    }
    j--;
    while (vec[j] > pivot) {
      j--;
    }
    if (i >= j)
      return j;
    swap(vec[i], vec[j]);
  }
}

void quickSort(vector<int> &vec, int low, int high) {
  if (low < high) {
    int p_element = Partition(vec, low, high);
    quickSort(vec, low, p_element);
    quickSort(vec, p_element + 1, high);
  }
}
// int main() {
//
//   vector<int> arr = {16, 2,  17, 24, 33, 28, 30, 1,  0,  27, 9,  25,
//                      34, 23, 19, 18, 11, 7,  21, 13, 8,  35, 12, 29,
//                      6,  3,  4,  14, 22, 15, 32, 10, 26, 31, 20, 5};
//   quickSort(arr, 0, arr.size() - 1);
//   for (const int i : arr) {
//     cout << i << endl;
//   }
// }
