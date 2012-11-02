#include "../header.h"

void sort(double *A, int length);

void initialize(double *A, int length) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < length; i++) A[i] = 2 * drand48() - 1;
}

int main(int argc, char **argv) {
  srand48(time(NULL));
  int length = atoi(argv[1]);

  double *A = (double*) malloc(length * sizeof(double));
  initialize(A, length);
  sort(A, length);

  // check for correctness
  int i;
  for(i = 0; i < length-1; i++) {
    if (A[i] > A[i+1] {
      printf("FAILURE: array not sorted\n");
    }
  }

  free(A);
  return 0;
}
