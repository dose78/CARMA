#include "../header.h"

char* getName() {
  return "skinny10";
}

void initialize(int m, int k, int n, float* A, float* B, float* C) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

void inner_multiply(int M, int K, int m, int k, int n, float *A, float *B, float *C, int depth, int CM) {
  if (depth >= 10 || m == k) {
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,M, B,K, 1, C, CM);
    return;
  }

  int next_depth = depth + 1;
  k = k/2;
  float *A1 = A;
  float *A2 = A + k*M;
  float *B1 = B;
  float *B2 = B + k;
  float *Q1 = (float*) malloc(m * n * sizeof(float));
  memset(Q1, 0, sizeof(float) * m * n);
  cilk_spawn inner_multiply(M, K, m, k, n, A1, B1, Q1, next_depth, m);
  inner_multiply(M, K, m, k, n, A2, B2, C, next_depth, CM);
  cilk_sync;
  int x, y;
  for (x = 0; x < n; x++) {
    for (y = 0; y < m; y++) {
      C[y + CM*x] += Q1[y + m*x];
    }
  }
  free(Q1);
}

void multiply(int m, int k, int n, float *A, float *B, float *C) {
  inner_multiply(m,k,m,k,n,A,B,C,0,m);
}
