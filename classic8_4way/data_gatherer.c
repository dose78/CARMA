#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <mkl.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "multiply.h"

int main(int argc, char **argv) {
  srand(time(NULL));
  
  int alg = atoi(argv[1]);
  int threads = atoi(argv[2]);
  int n = atoi(argv[3]);
  FILE *f = fopen(argv[4],"a");
  
  float *A = (float*) malloc(n * n * sizeof(float));
  float *B = (float*) malloc(n * n * sizeof(float));
  float *C = (float*) malloc(n * n * sizeof(float));
  
  int i;
  for(i = 0; i < n*n; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < n*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < n*n; i++) C[i] = 2 * drand48() - 1;
  
  double Gflop_s, iterations, seconds = -1.0;
  for(iterations = 1; seconds < 0.1; iterations *= 2) {
    struct timeval start, end;
    int j;
    if (alg == 1) {
      cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, 1, A,n, B,n, 1, C,n); //warmup
      gettimeofday(&start, NULL);
      for(j = 0; j < iterations; j++) {
        cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, 1, A,n, B,n, 1, C,n);
      }
      gettimeofday(&end, NULL);
    } else {
      multiply(n, A, B, C); //warmup
      gettimeofday(&start, NULL);
      for(j = 0; j < iterations; j++) {
        multiply(n, A, B, C);
      }
      gettimeofday(&end, NULL);
    }
    seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    Gflop_s = 2e-9 * iterations * n * n * n / seconds;
  }
  
  if (alg == 1) {
    fprintf(f,"MKL,%d,%d,%g\n", threads, n, Gflop_s);
    printf("MKL,%d,%d,%g\n", threads, n, Gflop_s);
  } else {
    fprintf(f,"classic8_4way,%d,%d,%g\n", threads, n, Gflop_s);
    printf("classic8_4way,%d,%d,%g\n", threads, n, Gflop_s);
  }
  
  // check for correctness
  /*
  if (alg == 2) {
    int i;
    memset(C, 0, sizeof(float) * n * n);
    multiply(n, A, B, C);
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, -1, A,n, B,n, 1, C,n);
    for(i = 0; i < n*n; i++) A[i] = fabs( A[i] );
    for(i = 0; i < n*n; i++) B[i] = fabs( B[i] );
    for(i = 0; i < n*n; i++) C[i] = fabs( C[i] );
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, -3.0*FLT_EPSILON*n, A,n, B,n, 1, C,n);
    for(i = 0; i < n * n; i++) {
      if(C[i] > 0) {
        printf("FAILURE: error in matrix multiply exceeds an acceptable margin\n");
        return -1;
      }
    }
  }
  //*/

  free(C);
  free(B);
  free(A);
  return 0;
}

