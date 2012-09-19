#include "header.h"

void initialize(int m, int k, int n, double* A, double* B, double* C);
void multiply(int m, int k, int n, double *A, double *B, double *C);

void clearCache(double *F) {
  int i;
  double sum = 0;
  for (i = 0; i < 12500000; i++) {
    sum += F[i];
  }
  if (sum == 0.1) { // purpose is to prevent the compiler from optimizing this away
    printf("sum = %f\n", sum);
  }
}

int main(int argc, char **argv) {
  srand48(time(NULL));
  double *cacheClearer = (double*) malloc(100000000); //L3 cahce is less than 100MB
  int i;
  for(i = 0; i < 12500000; i++) cacheClearer[i] = 2 * drand48() - 1;

  char* alg = argv[1];
  int m = atoi(argv[2]);
  int k = atoi(argv[3]);
  int n = atoi(argv[4]);
  int threads = atoi(argv[5]);

  FILE *f = fopen("data.csv","a");

  double *A = (double*) malloc(m * k * sizeof(double));
  double *B = (double*) malloc(k * n * sizeof(double));
  double *C = (double*) malloc(m * n * sizeof(double));

  initialize(m, k, n, A, B, C);

  // Time cache clearing
  struct timeval start, end;
  int j, iterations;
  gettimeofday(&start, NULL);
  clearCache(cacheClearer); // clear cache
  gettimeofday(&end, NULL);
  double cacheClearTime = ((end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec));

  // Time multiplication
  double Gflop_s, seconds = -1.0;
  for(iterations = 1; seconds < 0.1; iterations *= 2) {
    multiply(m, k, n, A, B, C); // warmup
    gettimeofday(&start, NULL);
    for(j = 0; j < iterations; j++) {
      clearCache(cacheClearer); // clear cache
      multiply(m, k, n, A, B, C);
    }
    gettimeofday(&end, NULL);
    seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec) - iterations * cacheClearTime;
    Gflop_s = 2e-9 * iterations * m * k * n / seconds;
  }

  fprintf(f,"%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflop_s);
  printf("%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflop_s);

  // check for correctness
  /*
  // memset(C, 0, sizeof(double) * m * n); //if commented, this tests C = A*B instead of C += A*B
  multiply(m, k, n, A, B, C);
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -1, A,m, B,k, 1, C,m);
  for(i = 0; i < m*k; i++) A[i] = fabs( A[i] );
  for(i = 0; i < k*n; i++) B[i] = fabs( B[i] );
  for(i = 0; i < m*n; i++) C[i] = fabs( C[i] );
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -3.0*DBL_EPSILON*n, A,m, B,k, 1, C,m);
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
  free(cacheClearer);
  fclose(f);
  return 0;
}
