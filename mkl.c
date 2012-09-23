#include "header.h"

void initialize(int m, int k, int n, double* A, double* B, double* C) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

void multiply(int m, int k, int n, double *A, double *B, double *C) {
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,m, B,k, 0, C,m);
}
