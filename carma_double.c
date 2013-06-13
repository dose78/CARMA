#include "header.h"
#define SPLIT_M 1
#define SPLIT_K 2
#define SPLIT_N 3

// Split largest dimension
int dim_to_split(int m, int k, int n) {
  if (n >= k && n >= m) return SPLIT_N;
  if (m >= k && m >= n) return SPLIT_M;
  return SPLIT_K;
}

void inner_multiply(int m, int k, int n, double *A, int LDA, double *B, int LDB, double *C, int LDC, int depth, int max_depth) {
  if (depth >= max_depth) {
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1, A, LDA, B, LDB, 0, C, LDC);
    return;
  }
  int next_depth = depth + 1;
  int dim = dim_to_split(m, k, n);
  if (dim == SPLIT_N) {
    cilk_spawn inner_multiply(m, k, n/2, A, LDA, B, LDB, C, LDC, next_depth, max_depth);
    inner_multiply(m, k, n/2, A, LDA, B + n/2*LDB, LDB, C + n/2*LDC, LDC, next_depth, max_depth);
    cilk_sync;

  } else if (dim == SPLIT_M) {
    cilk_spawn inner_multiply(m/2, k, n, A, LDA, B, LDB, C, LDC, next_depth, max_depth);
    inner_multiply(m/2, k, n, A + m/2, LDA, B, LDB, C + m/2, LDC, next_depth, max_depth);
    cilk_sync;

  } else { // SPLIT_K
    double *C2 = (double*) malloc(m * n * sizeof(double));
    cilk_spawn inner_multiply(m, k/2, n, A, LDA, B, LDB, C2, m, next_depth, max_depth);
    inner_multiply(m, k/2, n, A + k/2*LDA, LDA, B + k/2, LDB, C, LDC, next_depth, max_depth);
    cilk_sync;
    int x;
    for (x = 0; x < n; x++) {
      cblas_daxpy(m, 1, C2 + m*x, 1, C + LDC*x, 1);
    }
    free(C2);
  }
}

void multiply(int m, int k, int n, double *A, double *B, double *C, int max_depth) {
  inner_multiply(m, k, n, A, m, B, k, C, m, 0, max_depth);
}
