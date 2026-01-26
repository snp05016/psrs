#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <sys/time.h>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;
vector<double> phasetimes(6, 0.0); 
vector<int> PSRS(vector<int> &inputVector, int n, int p);
void Sublists(vector<int> &vec, int start, int end, vector<int> &subsize,
              int at, const vector<int> &pivots, int fp, int lp);
int ReadFile(string fileName, vector<int> &toPutInto);
void phase1(vector<int> &inputVector, int r_size, int size, int processor_i,
            int n, int p, vector<int> &sample);
void phase3(vector<int> &subsize, const vector<int> &pivots, vector<int> &inputVector,
            int size, int p, int n, int processor_i);
void phase4(vector<vector<vector<int>>> &buckets, vector<int> &subsize,
                   vector<int> &inputVector, int p, int processor_i);
