#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <thread>
#include <vector>
using namespace std;

vector<int> PSRS(vector<int> &inputVector, int n, int p);
void Sublists(vector<int> vec, int start, int end, vector<int> subsize, int at,
              vector<int> pivots, int fp, int lp);
