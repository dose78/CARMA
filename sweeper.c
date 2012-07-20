#include "header.h"

#define MIN_M 64
#define MAX_M 128
#define MIN_K 256*256
#define MAX_K 256*256
#define MIN_N 64
#define MAX_N 128

char* getName();
int timeMultiply(char* alg, int m, int k, int n, int threads);

int main(int argc, char **argv) {
  char* alg = getName();

  int threads;
  char* envVar = (strcmp(alg,"MKL") == 0) ? "MKL_NUM_THREADS" : "CILK_NWORKERS";
  char* threadsStr = getenv(envVar);
  if (threadsStr == NULL) {
    printf("You did not export %s. Quitting now...\n", envVar);
    exit(1);
  } else {
    threads = atoi(threadsStr);
  }

  int m, k, n;
  for (m = MIN_M; m <= MAX_M; m*=2) {
    for (k = MIN_K; k <= MAX_K; k*=2) {
      for (n = MIN_N; n <= MAX_N; n*=2) {
        timeMultiply(alg, m, k, n, threads);
      }
    }
  }
}
