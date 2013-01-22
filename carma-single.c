#include "header.h"
#define MAX_DEPTH 5
#define SPLIT_M 1
#define SPLIT_K 2
#define SPLIT_N 3

// Split largest dimension
int dim_to_split(int m, int k, int n) {
  if (n >= k && n >= m) return SPLIT_N;
  if (m >= k && m >= n) return SPLIT_M;
  return SPLIT_K;
}

void inner_multiply(int M, int K, int m, int k, int n, float *A, float *B, float *C, int depth, int CM) {
  if (depth >= MAX_DEPTH) {
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,M, B,K, 0, C, CM);
    return;
  }
  int next_depth = depth + 1;
  int dim = dim_to_split(m, k, n);
  if (dim == SPLIT_N) {
    n = n/2;
    float *B1 = B;
    float *B2 = B + n*K;
    cilk_spawn inner_multiply(M, K, m, k, n, A, B1, C, next_depth, CM);
    inner_multiply(M, K, m, k, n, A, B2, C + n*CM, next_depth, CM);
    cilk_sync;

  } else if (dim == SPLIT_M) {
    m = m/2;
    float *A1 = A;
    float *A2 = A + m;
    cilk_spawn inner_multiply(M, K, m, k, n, A1, B, C, next_depth, CM);
    inner_multiply(M, K, m, k, n, A2, B, C + m, next_depth,CM);
    cilk_sync;

  } else { // SPLIT_K
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
