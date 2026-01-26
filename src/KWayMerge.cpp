#include "KWayMerge.h"
#include "PSRS.h"
vector<int>
merge(vector<vector<int>> poVector) { // poVector will be the vector of
                                      // partially ordered vectors
  vector<int> pointer_tracker(
      poVector.size(),
      0); // this tracks the index of the element that we are at.
  int last_updated_sorted_vec;
  int index = 0;
  std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                      std::greater<std::pair<int, int>>>
      merge_pq; // initialize a minheap using a pq
  vector<int> sorted_vec;
  for (const vector<int>& sortedVec : poVector) {
    if (!sortedVec.empty()) {
      merge_pq.push({(*sortedVec.begin()), index});
    }
    index++;
  }
  while (!merge_pq.empty()) {
    pair<int, int> smallest_element = merge_pq.top();
    //        cout << "the poped element is " << smallest_element.first  << "and
    //        the index popped from is" << smallest_element.second << endl;
    sorted_vec.push_back(smallest_element.first);
    last_updated_sorted_vec = smallest_element.second;
    pointer_tracker[last_updated_sorted_vec]++;
    merge_pq.pop();
    if ((unsigned long)(pointer_tracker[last_updated_sorted_vec]) !=
        ((unsigned long)poVector[last_updated_sorted_vec].size())) {
      merge_pq.push({(poVector[last_updated_sorted_vec]
                              [pointer_tracker[last_updated_sorted_vec]]),
                     last_updated_sorted_vec});
    }
  }
  return sorted_vec;
}

