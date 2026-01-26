#include <vector>
using namespace std;

vector<int> GenerateData(long long  number_of_integers, int max_integer_value) {
  vector<int> data;
  srandom(static_cast<unsigned int>(time(0)));
  for (long long i = 0; i < number_of_integers; ++i) {
    int random_integer = random() % (max_integer_value + 1);
    data.push_back(random_integer);
  }
  return data;
}