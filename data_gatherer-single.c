#include "header.h"

void initialize(int m, int k, int n, float* A, float* B, float* C);
void multiply(int m, int k, int n, float *A, float *B, float *C);

#define NUM_SMALL_MATRICES_MAX 10000

void clearCache(double *F) {
  int i;
  double sum = 0;
  for (i = 0; i < 12500000; i++) {
    sum += F[i];
  }
  if (sum == 0.1) { // Prevent the compiler from optimizing this away
    printf("sum = %f\n", sum);
  }
}

int guess_num_matrices(int m, int k, int n) {
  float *A[NUM_SMALL_MATRICES_MAX], *B[NUM_SMALL_MATRICES_MAX], *C[NUM_SMALL_MATRICES_MAX];
  double *cacheClearer = (double*) malloc(100000000); //L3 cahce is less than 100MB
  int num_matrices, i, previous_trial = 0;
  struct timeval start, end;
  for(i = 0; i < 12500000; i++) cacheClearer[i] = 2 * drand48() - 1;

  for (num_matrices = 1; num_matrices < NUM_SMALL_MATRICES_MAX; num_matrices *= 2) {
    for (i = previous_trial; i < num_matrices; i++) {
      A[i] = (float*) malloc(m * k * sizeof(float));
      B[i] = (float*) malloc(k * n * sizeof(float));
      C[i] = (float*) malloc(m * n * sizeof(float));
      initialize(m, k, n, A[i], B[i], C[i]);
    }
    previous_trial = num_matrices;

    clearCache(cacheClearer); // clear cache
    gettimeofday(&start, NULL);
    for (i = 0; i < num_matrices; i++) {
      multiply(m, k, n, A[i], B[i], C[i]);
    }
    gettimeofday(&end, NULL);
    double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    // printf("%i matrices take %f seconds.\n", num_matrices, seconds);
    if (seconds > 0.2) {
      break;
    }
  }

  for (i = 0; i < num_matrices; i++) {
    free(A[i]);
    free(B[i]);
    free(C[i]);
  }

  return num_matrices;
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

  float *A[NUM_SMALL_MATRICES_MAX], *B[NUM_SMALL_MATRICES_MAX], *C[NUM_SMALL_MATRICES_MAX];

  // discover how many multiplies are needed
  int num_matrices = guess_num_matrices(m, k, n);
  // printf("Num matrices required: %d\n", num_matrices);

  for (i=0; i<num_matrices; i++) {
    A[i] = (float*) malloc(m * k * sizeof(float));
    B[i] = (float*) malloc(k * n * sizeof(float));
    C[i] = (float*) malloc(m * n * sizeof(float));

    initialize(m, k, n, A[i], B[i], C[i]);
  }

  // Time multiplication
  struct timeval start, end;
  multiply(m, k, n, A[0], B[0], C[0]); // warmup
  clearCache(cacheClearer); // clear cache
  gettimeofday(&start, NULL);
  for (i = 0; i < num_matrices; i++) {
    multiply(m, k, n, A[i], B[i], C[i]);
  }
  gettimeofday(&end, NULL);
  double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
  double Gflop_s = num_matrices * 2e-9 * m * k * n / seconds;

  if (seconds < 0.02) {
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

  for (i=0; i<num_matrices; i++) free(A[i]);
  for (i=0; i<num_matrices; i++) free(B[i]);
  for (i=0; i<num_matrices; i++) free(C[i]);
  free(cacheClearer);
  fclose(f);
  return 0;
}
