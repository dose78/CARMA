#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <mkl.h>
#include "multiply.h"

int main(int argc, char **argv) {
  srand(time(NULL));

  for(int n = 2; n <= 2048; n*=2) {
    int m = n;
  
    /* Allocate and fill 2 random matrices A, C */
    float *A = (float*) malloc(m * n * sizeof(float));
    float *B = (float*) malloc(m * n * sizeof(float));
    float *C = (float*) malloc(m * m * sizeof(float));
    for(int i = 0; i < m*n; i++) A[i] = 2 * drand48() - 1;
    for(int i = 0; i < m*n; i++) B[i] = 2 * drand48() - 1;
    for(int i = 0; i < m*m; i++) C[i] = 2 * drand48() - 1;
    
    /* measure Gflop/s rate; time a sufficiently long sequence of calls to eliminate noise */
    double Gflop_s, seconds = -1.0;
    for(int iterations = 1; seconds < 0.1; iterations *= 2) {
      /* warm-up */
      
      multiply(n, A, B, C);
      //cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,m,n, 1, A,m, B,m, 1, C,m);
      
      /* measure time */
      struct timeval start, end;
      gettimeofday(&start, NULL);
      for(int i = 0; i < iterations; i++) {
        multiply(n, A, B, C);
        //cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,m,n, 1, A,m, B,m, 1, C,m);
      }
      gettimeofday(&end, NULL);
      seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
      
      /* compute Gflop/s rate */
      Gflop_s = 2e-9 * iterations * m * m * n / seconds;
    }
    printf("%d by %d matrix \t %g Gflop/s\n", m, n, Gflop_s);
    
    
    /* Ensure that error does not exceed the theoretical error bound */
		
    /* Set initial C to 0 and do matrix multiply of A*B */
    memset(C, 0, sizeof(float) * m * m);
    multiply(n, A, B, C);
    //cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,m,n, 1, A,m, B,m, 1, C,m);
    
    /* Subtract A*B from C (this should be 0 to within machine roundoff) */
    //for(int i = 0; i < m*n; i++) A[i] = -A[i];
    //simple(m, m, n, A, B, C);
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,m,n, -1, A,m, B,m, 1, C,m);

    /* Subtract the maximum allowed roundoff from each element of C */
    for(int i = 0; i < m*n; i++) A[i] = fabs( A[i] );
    for(int i = 0; i < m*n; i++) B[i] = fabs( B[i] );
    for(int i = 0; i < m*m; i++) C[i] = fabs( C[i] );
    //for(int i = 0; i < m*n; i++) A[i] = -3.0*FLT_EPSILON*n*A[i];
    //simple(m, m, n, A, B, C);
    cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,m,n, -3.0*FLT_EPSILON*n, A,m, B,m, 1, C,m);

    /* If any element in C is still positive, something went wrong */
    for(int i = 0; i < m * m; i++) {
      if(C[i] > 0) {
        printf("FAILURE: error in matrix multiply exceeds an acceptable margin\n");
        return -1;
      }
    }

    /* release memory */
    free(C);
    free(B);
    free(A);
  }
  
  return 0;
}

