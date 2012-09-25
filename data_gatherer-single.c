#include "header.h"

void initialize(int m, int k, int n, float* A, float* B, float* C);
void multiply(int m, int k, int n, float *A, float *B, float *C);

void clearCache(float *F) {
  int i;
  double sum = 0;
  for (i = 0; i < 25000000; i++) {
    sum += F[i];
  }
  if (sum == 0.1) { // Prevent the compiler from optimizing this away
    printf("sum = %f\n", sum);
  }
}

int main(int argc, char **argv) {
  srand48(time(NULL));
  float *cacheClearer = (float*) malloc(100000000); //L3 cahce is less than 100MB
  int i;
  for(i = 0; i < 25000000; i++) cacheClearer[i] = 2 * drand48() - 1;

  char* alg = argv[1];
  int m = atoi(argv[2]);
  int k = atoi(argv[3]);
  int n = atoi(argv[4]);
  int threads = atoi(argv[5]);

  FILE *f = fopen("data.csv","a");

  float *A = (float*) malloc(m * k * sizeof(float));
  float *B = (float*) malloc(k * n * sizeof(float));
  float *C = (float*) malloc(m * n * sizeof(float));

  initialize(m, k, n, A, B, C);

  // Time multiplication
  struct timeval start, end;
  multiply(m, k, n, A, B, C); // warmup
  clearCache(cacheClearer); // clear cache
  gettimeofday(&start, NULL);
  multiply(m, k, n, A, B, C);
  gettimeofday(&end, NULL);
  double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
  double Gflop_s = 2e-9 * m * k * n / seconds;

  if (seconds < 0.01) {
    printf("WARNING: Matrix size may be too small to produce accurate timing data\n");
  }

  fprintf(f,"%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflop_s);
  printf("%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflop_s);

  // check for correctness
  /*
  // memset(C, 0, sizeof(float) * m * n); //if commented, this tests C = A*B instead of C += A*B
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
  free(cacheClearer);
  fclose(f);
  return 0;
}
