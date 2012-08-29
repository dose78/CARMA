#include "../header.h"

void inner_initialize(int N, int n, float *A, float *B, int depth) {
  if (depth >= 2) {
    int i, j;
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        A[i + N*j] = 2 * drand48() - 1;
      B[i + N*j] = 2 * drand48() - 1;
      }
    }
    return;
  }

  n = n/2;

  float *A11 = A;
  float *A12 = A + n*N;
  float *A21 = A + n;
  float *A22 = A + n*N + n;
  float *B11 = B;
  float *B12 = B + n*N;
  float *B21 = B + n;
  float *B22 = B + n*N + n;

  int next_depth = depth + 1;
  cilk_spawn inner_initialize(N, n, A11, B11, next_depth);
  cilk_spawn inner_initialize(N, n, A12, B21, next_depth);
  cilk_spawn inner_initialize(N, n, A11, B12, next_depth);
  cilk_spawn inner_initialize(N, n, A12, B22, next_depth);
  cilk_spawn inner_initialize(N, n, A21, B11, next_depth);
  cilk_spawn inner_initialize(N, n, A22, B21, next_depth);
  cilk_spawn inner_initialize(N, n, A21, B12, next_depth);
  inner_initialize(N, n, A22, B22, next_depth);
  cilk_sync;
}

void initialize(int m, int k, int n, float* A, float* B, float* C) {
  srand48(time(NULL));
  inner_initialize(n,n,A,B,0);
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
	    C[i + n*j] = 2 * drand48() - 1;
	  }
  }
}

void inner_multiply(int N, int n, float *A, float *B, float *C, int depth) {
  if (depth >= 2) { // 2 BFS levels
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, 1, A,N, B,N, 1, C,n);
    return;
  }

  n = n/2;

  float *Q1 = (float*) malloc(n * n * sizeof(float));
  float *Q2 = (float*) malloc(n * n * sizeof(float));
  float *Q3 = (float*) malloc(n * n * sizeof(float));
  float *Q4 = (float*) malloc(n * n * sizeof(float));
  float *Q5 = (float*) malloc(n * n * sizeof(float));
  float *Q6 = (float*) malloc(n * n * sizeof(float));
  float *Q7 = (float*) malloc(n * n * sizeof(float));
  float *Q8 = (float*) malloc(n * n * sizeof(float));

  memset(Q1, 0, sizeof(float) * n * n);
  memset(Q2, 0, sizeof(float) * n * n);
  memset(Q3, 0, sizeof(float) * n * n);
  memset(Q4, 0, sizeof(float) * n * n);
  memset(Q5, 0, sizeof(float) * n * n);
  memset(Q6, 0, sizeof(float) * n * n);
  memset(Q7, 0, sizeof(float) * n * n);
  memset(Q8, 0, sizeof(float) * n * n);

  float *A11 = A;
  float *A12 = A + n*N;
  float *A21 = A + n;
  float *A22 = A + n*N + n;
  float *B11 = B;
  float *B12 = B + n*N;
  float *B21 = B + n;
  float *B22 = B + n*N + n;

  int next_depth = depth + 1;

  cilk_spawn inner_multiply(N, n, A11, B11, Q1, next_depth);
  cilk_spawn inner_multiply(N, n, A12, B21, Q2, next_depth);
  cilk_spawn inner_multiply(N, n, A11, B12, Q3, next_depth);
  cilk_spawn inner_multiply(N, n, A12, B22, Q4, next_depth);
  cilk_spawn inner_multiply(N, n, A21, B11, Q5, next_depth);
  cilk_spawn inner_multiply(N, n, A22, B21, Q6, next_depth);
  cilk_spawn inner_multiply(N, n, A21, B12, Q7, next_depth);
  inner_multiply(N, n, A22, B22, Q8, next_depth);
  cilk_sync;

  int x, y;
  for (x = 0; x < n; x++) {
    for (y = 0; y < n; y++) {
      C[y+2*n*x] = Q1[y+n*x] + Q2[y+n*x];
      C[2*n*n + y+2*n*x] = Q3[y+n*x] + Q4[y+n*x];
      C[n + y+2*n*x] = Q5[y+n*x] + Q6[y+n*x];
      C[2*n*n + n + y+2*n*x] = Q7[y+n*x] + Q8[y+n*x];
    }
  }

  free(Q1);
  free(Q2);
  free(Q3);
  free(Q4);
  free(Q5);
  free(Q6);
  free(Q7);
  free(Q8);
}

void multiply(int m, int k, int n, float *A, float *B, float *C) {
  inner_multiply(n,n,A,B,C,0);
}
