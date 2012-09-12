#include "header.h"

void initialize(int m, int k, int n, float* A, float* B, float* C);
void multiply(int m, int k, int n, float *A, float *B, float *C);

int main(int argc, char **argv) {
  int m = atoi(argv[1]);
  int k = atoi(argv[2]);
  int n = atoi(argv[3]);

  float *A = (float*) malloc(m * k * sizeof(float));
  float *B = (float*) malloc(k * n * sizeof(float));
  float *C = (float*) malloc(m * n * sizeof(float));

  initialize(m, k, n, A, B, C);

  multiply(m, k, n, A, B, C);

  free(A);
  free(B);
  free(C);
  return 0;
}
