#include "../header.h"

void initialize(int m, int k, int n, float* A, float* B, float* C) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

void two_inner_multiply(int AN, int BN, int n, int next_depth, float *S1, float *S2, float *S3, float *T1, float *T2) {
  memset(T1, 0, sizeof(float) * n * n);
  memset(T2, 0, sizeof(float) * n * n);
  inner_multiply(AN, BN, n, S1, S2, T1, next_depth);
  inner_multiply(AN, BN, n, S1, S3, T2, next_depth);
}

void inner_multiply(int AN, int BN, int n, float *A, float *B, float *C, int depth) {
  if (depth >= 3) {
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, 1, A,AN, B,BN, 1, C,n);
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

  float *A11 = A;
  float *A12 = A + n*AN;
  float *A21 = A + n;
  float *A22 = A + n*AN + n;
  float *B11 = B;
  float *B12 = B + n*BN;
  float *B21 = B + n;
  float *B22 = B + n*BN + n;

  int next_depth = depth + 1;

  cilk_spawn two_inner_multiply(AN, BN, n, next_depth, A11, B11, B12, Q1, Q3);
  cilk_spawn two_inner_multiply(AN, BN, n, next_depth, A12, B21, B22, Q2, Q4);
  cilk_spawn two_inner_multiply(AN, BN, n, next_depth, A21, B12, B11, Q7, Q5);
  two_inner_multiply(AN, BN, n, next_depth, A22, B22, B21, Q8, Q6);
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
  inner_multiply(n,n,n,A,B,C,0);
}
