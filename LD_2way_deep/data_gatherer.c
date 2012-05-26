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
  int m = atoi(argv[3]);
  int k = atoi(argv[4]);
  int n = atoi(argv[5]);
  FILE *f = fopen(argv[6],"a");
  
  float *A = (float*) malloc(m * k * sizeof(float));
  float *B = (float*) malloc(k * n * sizeof(float));
  float *C = (float*) malloc(m * n * sizeof(float));
  
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
  
  double Gflop_s, iterations, seconds = -1.0;
  for(iterations = 1; seconds < 0.1; iterations *= 2) {
    struct timeval start, end;
    int j;
    if (alg == 1) {
      cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,m, B,k, 1, C,m); //warmup
      gettimeofday(&start, NULL);
      for(j = 0; j < iterations; j++) {
        cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, 1, A,m, B,k, 1, C,m);
      }
      gettimeofday(&end, NULL);
      
    } else {
      multiply(m, k, n, A, B, C); //warmup
      gettimeofday(&start, NULL);
      for(j = 0; j < iterations; j++) {
        multiply(m, k, n, A, B, C);
      }
      gettimeofday(&end, NULL);
    }
    seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    Gflop_s = 2e-9 * iterations * m * k * n / seconds;
  }
  
  if (alg == 1) {
    fprintf(f,"MKL,%d,%d,%d,%d,%g\n", threads, m,k,n, Gflop_s);
    printf("MKL,%d,%d,%d,%d,%g\n", threads, m,k,n, Gflop_s);
  } else {
    fprintf(f,"LD_2way_deep,%d,%d,%d,%d,%g\n", threads, m,k,n, Gflop_s);
    printf("LD_2way_deep,%d,%d,%d,%d,%g\n", threads, m,k,n, Gflop_s);
  }
  
  // check for correctness
  /*
  if (alg == 2) {
    memset(C, 0, sizeof(float) * m * n);
    multiply(m, k, n, A, B, C);
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -1, A,m, B,k, 1, C,m);
    for(i = 0; i < m*k; i++) A[i] = fabs( A[i] );
    for(i = 0; i < k*n; i++) B[i] = fabs( B[i] );
    for(i = 0; i < m*n; i++) C[i] = fabs( C[i] );
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -3.0*FLT_EPSILON*n, A,m, B,k, 1, C,m);
    for(i = 0; i < m * n; i++) {
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

