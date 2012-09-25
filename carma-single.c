#include "header.h"
#define MAX_DEPTH 5

void initialize(int m, int k, int n, float* A, float* B, float* C) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

void inner_multiply(int M, int K, int m, int k, int n, float *A, float *B, float *C, int depth, int CM) {
  if (depth >= MAX_DEPTH) {
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,M, B,K, 0, C, CM);
    return;
  }

  int next_depth = depth + 1;

  int max = k;
  if (m > max) {
    max = m;
  }
  if (n > max) {
    max = n;
  }

  if (max == n) {
    n = n/2;
    float *B1 = B;
    float *B2 = B + n*K;
    cilk_spawn inner_multiply(M, K, m, k, n, A, B1, C, next_depth, CM);
    inner_multiply(M, K, m, k, n, A, B2, C + n*CM, next_depth, CM);
    cilk_sync;

  } else if (max == m) {
    m = m/2;
    float *A1 = A;
    float *A2 = A + m;
    cilk_spawn inner_multiply(M, K, m, k, n, A1, B, C, next_depth, CM);
    inner_multiply(M, K, m, k, n, A2, B, C + m, next_depth,CM);
    cilk_sync;

  } else {
    k = k/2;
    float *A1 = A;
    float *A2 = A + k*M;
    float *B1 = B;
    float *B2 = B + k;
    float *Q1 = (float*) malloc(m * n * sizeof(float));
    cilk_spawn inner_multiply(M, K, m, k, n, A1, B1, Q1, next_depth, m);
    inner_multiply(M, K, m, k, n, A2, B2, C, next_depth, CM);
    cilk_sync;
    int x;
    for (x = 0; x < n; x++) {
      cblas_saxpy(m, 1, Q1 + m*x, 1, C + CM*x, 1);
    }
    free(Q1);
  }
}

void multiply(int m, int k, int n, float *A, float *B, float *C) {
  inner_multiply(m,k,m,k,n,A,B,C,0,m);
}
