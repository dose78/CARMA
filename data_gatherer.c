#include "header.h"

void initialize(int m, int k, int n, float* A, float* B, float* C);
void multiply(int m, int k, int n, float *A, float *B, float *C);

void timeMultiply(char* alg, int m, int k, int n, int threads) {
  FILE *f = fopen("data.csv","a");

  float *A = (float*) malloc(m * k * sizeof(float));
  float *B = (float*) malloc(k * n * sizeof(float));
  float *C = (float*) malloc(m * n * sizeof(float));

  initialize(m, k, n, A, B, C);

  double Gflop_s, iterations, seconds = -1.0;
  for(iterations = 1; seconds < 0.1; iterations *= 2) {
    struct timeval start, end;
    int j;
    multiply(m, k, n, A, B, C); // warmup
    gettimeofday(&start, NULL);
    for(j = 0; j < iterations; j++) {
      multiply(m, k, n, A, B, C);
    }
    gettimeofday(&end, NULL);
    seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    Gflop_s = 2e-9 * iterations * m * k * n / seconds;
  }

  fprintf(f,"%s,%d,%d,%d,%d,%g\n", alg, m, k, n, threads, Gflop_s);
  printf("%s,%d,%d,%d,%d,%g\n", alg, m, k, n, threads, Gflop_s);


  // check for correctness
  /*
  int i;
  memset(C, 0, sizeof(float) * m * n);
  multiply(m, k, n, A, B, C);
  cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -1, A,m, B,k, 1, C,m);
  for(i = 0; i < m*k; i++) A[i] = fabs( A[i] );
  for(i = 0; i < k*n; i++) B[i] = fabs( B[i] );
  for(i = 0; i < m*n; i++) C[i] = fabs( C[i] );
  cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -3.0*FLT_EPSILON*n, A,m, B,k, 1, C,m);
  for(i = 0; i < m*n; i++) {
    if(C[i] > 0) {
      printf("FAILURE: error in matrix multiply exceeds an acceptable margin\n");
      return -1;
    }
  }
  */

  free(A);
  free(B);
  free(C);
}

