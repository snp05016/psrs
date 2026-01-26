#ifndef PSRS_H
#define PSRS_H
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include "pthread_barrier.h"
using namespace std;
extern pthread_mutex_t buckets_mutex;
extern vector<double> phasetimes;
extern pthread_barrier_t barrier;
vector<int> PSRS(vector<int> &inputVector, int n, int p);
void Sublists(vector<int> &vec, int start, int end, vector<int> &subsize,
              int at, const vector<int> &pivots, int fp, int lp);
void phase1(vector<int> &inputVector, int r_size, int size, int processor_i,
            int n, int p, vector<int> &sample);
void phase3(vector<int> &subsize, const vector<int> &pivots, vector<int> &inputVector,
            int size, int p, int n, int processor_i);
void phase4(vector<vector<vector<int>>> &buckets, vector<int> &subsize,
                   vector<int> &inputVector, int p, int processor_i);


//thread args
struct phase1Args {
  vector<int> *inputVector;
  int r_size;
  int size;
  int processor_i;
  int n;
  int p;
  vector<int> *sample;
};

struct phase2PivotArgs {
  vector<int> *pivots;
  vector<int> *sample;
  int p;
  int thread_id;
  int pivots_per_thread;
};

struct phase3Args {
  vector<int> *subsize;
  const vector<int> *pivots;
  vector<int> *inputVector;
  int size;
  int p;
  int n;
  int processor_i;
};

struct phase4Args {
  vector<vector<vector<int>>> *buckets;
  vector<int> *subsize;
  vector<int> *inputVector;
  int p;
  int processor_i;
};

#endif // PSRS_H