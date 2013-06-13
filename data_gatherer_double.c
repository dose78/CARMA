#include "header.h"
#define NUM_SMALL_MATRICES_MAX 20000

void multiply(int m, int k, int n, double *A, double *B, double *C);

void initialize(int m, int k, int n, double* A, double* B, double* C) {
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

void clearCache(double *F) {
  int i;
  double sum = 0;
  for (i = 0; i < 12500000; i++) {
    sum += F[i];
  }
  if (sum == 0.12345) { // Prevent the compiler from optimizing this away
    printf("sum = %f\n", sum);
  }
}

void correctnessTest(int m, int k, int n) {
  double *A = (double*) malloc(m * k * sizeof(double));
  double *B = (double*) malloc(k * n * sizeof(double));
  double *C = (double*) malloc(m * n * sizeof(double));
  initialize(m, k, n, A, B, C);

  // memset(C, 0, sizeof(double) * m * n); // uncommented allows for C+=A*B (in addition to C=A*B)
  multiply(m, k, n, A, B, C);
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -1, A,m, B,k, 1, C,m);
  int i;
  for(i = 0; i < m*k; i++) A[i] = fabs( A[i] );
  for(i = 0; i < k*n; i++) B[i] = fabs( B[i] );
  for(i = 0; i < m*n; i++) C[i] = fabs( C[i] );
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -3.0*DBL_EPSILON*n, A,m, B,k, 1, C,m);
  for(i = 0; i < m*n; i++) {
    if(C[i] > 0) {
      printf("FAILURE: error in matrix multiply exceeds an acceptable margin\n");
      break;
    }
  }

  free(A);
  free(B);
  free(C);
}

int guess_num_matrices(int m, int k, int n) {
  double *A[NUM_SMALL_MATRICES_MAX], *B[NUM_SMALL_MATRICES_MAX], *C[NUM_SMALL_MATRICES_MAX];
  int num_matrices, i, previous_trial = 0;
  struct timeval start, end;
  double *cacheClearer = (double*) malloc(100000000); // L3 cache is less than 100MB
  for(i = 0; i < 12500000; i++) cacheClearer[i] = i;

  for (num_matrices = 1; num_matrices <= NUM_SMALL_MATRICES_MAX; num_matrices *= 2) {
    for (i = previous_trial; i < num_matrices; i++) {
      A[i] = (double*) malloc(m * k * sizeof(double));
      B[i] = (double*) malloc(k * n * sizeof(double));
      C[i] = (double*) malloc(m * n * sizeof(double));
      initialize(m, k, n, A[i], B[i], C[i]);
    }
    previous_trial = num_matrices;

    clearCache(cacheClearer);
    gettimeofday(&start, NULL);
    for (i = 0; i < num_matrices; i++) {
      multiply(m, k, n, A[i], B[i], C[i]);
    }
    gettimeofday(&end, NULL);
    double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    // printf("%i matrices take %f seconds.\n", num_matrices, seconds);
    if (seconds > 0.2) break;
  }

  int num_to_free = num_matrices;
  if (num_matrices > NUM_SMALL_MATRICES_MAX) {
    num_to_free = num_matrices/2;
    num_matrices = NUM_SMALL_MATRICES_MAX;
  }

  for (i = 0; i < num_to_free; i++) { free(A[i]); free(B[i]); free(C[i]); }
  free(cacheClearer);

  return num_matrices;
}

int main(int argc, char **argv) {
  srand48(time(NULL));
  char* alg = argv[1];
  int m = atoi(argv[2]);
  int k = atoi(argv[3]);
  int n = atoi(argv[4]);
  int threads = atoi(argv[5]);
  int num_iters = atoi(argv[6]);

  int i, iter, success = 0, num_failures = 0;
  double *Gflops = (double*) malloc(num_iters * sizeof(double));
  double *cacheClearer = (double*) malloc(100000000); //L3 cache is less than 100MB
  for(i = 0; i < 12500000; i++) cacheClearer[i] = i;

  while (success == 0) {
    // discover how many multiplies are needed
    int num_matrices = guess_num_matrices(m, k, n);
    // printf("Num matrices required: %d\n", num_matrices);

    double *A[num_matrices], *B[num_matrices], *C[num_matrices];
    for (i=0; i<num_matrices; i++) {
      A[i] = (double*) malloc(m * k * sizeof(double));
      B[i] = (double*) malloc(k * n * sizeof(double));
      C[i] = (double*) malloc(m * n * sizeof(double));
      initialize(m, k, n, A[i], B[i], C[i]);
    }

    success = 1;
    for (iter = 0; iter < num_iters; iter++) {
      struct timeval start, end;
      clearCache(cacheClearer);
      gettimeofday(&start, NULL);
      for (i = 0; i < num_matrices; i++) {
        multiply(m, k, n, A[i], B[i], C[i]);
      }
      gettimeofday(&end, NULL);
      double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
      Gflops[iter] = num_matrices * 2e-9 * m * k * n / seconds;
      printf("%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflops[iter]);

      if (seconds < 0.05 && num_failures < 10) {
        printf("WARNING: Matrix size may be too small to produce accurate timing data. Re-running...\n");
        num_failures++;
        success = 0;
        break;
      }
    }
    for (i=0; i<num_matrices; i++) free(A[i]);
    for (i=0; i<num_matrices; i++) free(B[i]);
    for (i=0; i<num_matrices; i++) free(C[i]);
  }

  FILE *f = fopen("data.csv","a");
  for (iter = 0; iter < num_iters; iter++) {
    fprintf(f,"%s,%d,%d,%d,%d,%f\n", alg, m, k, n, threads, Gflops[iter]);
  }
  fclose(f);

  free(Gflops);
  free(cacheClearer);

  // correctnessTest(m, k, n);
  return 0;
}
