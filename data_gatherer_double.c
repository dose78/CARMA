#include "header.h"
#define NUM_SMALL_MATRICES_MAX 20000
#define MAX_NUM_FAILURES 10

void multiply(int m, int k, int n, double *A, double *B, double *C, int max_depth);

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

void correctnessTest(int m, int k, int n, int max_depth) {
  double *A = (double*) malloc(m * k * sizeof(double));
  double *B = (double*) malloc(k * n * sizeof(double));
  double *C = (double*) malloc(m * n * sizeof(double));
  initialize(m, k, n, A, B, C);

  // memset(C, 0, sizeof(double) * m * n); // uncommented allows for C+=A*B (in addition to C=A*B)
  multiply(m, k, n, A, B, C, max_depth);
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
  printf("Correcness test passed\n");

  free(A);
  free(B);
  free(C);
}

int init_matrices(int m, int k, int n, double **A, double **B, double **C, int max_depth) {
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
      multiply(m, k, n, A[i], B[i], C[i], max_depth);
    }
    gettimeofday(&end, NULL);
    double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    // printf("%i matrices take %f seconds.\n", num_matrices, seconds);
    if (seconds > 0.2) break;
  }

  if (num_matrices > NUM_SMALL_MATRICES_MAX) {
    num_matrices = num_matrices/2;
  }

  free(cacheClearer);

  return num_matrices;
}

void trial(char* alg, int m, int k, int n, int threads, int max_depth, int num_iters, char* output) {
  int i, iter, success = 0, num_failures = 0;
  double *Gflops = (double*) malloc(num_iters * sizeof(double));
  double *cacheClearer = (double*) malloc(100000000); //L3 cache is less than 100MB
  for(i = 0; i < 12500000; i++) cacheClearer[i] = i;

  while (success == 0) {
    double *A[NUM_SMALL_MATRICES_MAX], *B[NUM_SMALL_MATRICES_MAX], *C[NUM_SMALL_MATRICES_MAX];
    // discover how many multiplies are needed and init them
    int num_matrices = init_matrices(m, k, n, A, B, C, max_depth);
    // printf("Num matrices required: %d\n", num_matrices);

    success = 1;
    for (iter = 0; iter < num_iters; iter++) {
      struct timeval start, end;
      clearCache(cacheClearer);
      gettimeofday(&start, NULL);
      for (i = 0; i < num_matrices; i++) {
        multiply(m, k, n, A[i], B[i], C[i], max_depth);
      }
      gettimeofday(&end, NULL);
      double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
      Gflops[iter] = num_matrices * 2e-9 * m * k * n / seconds;
      printf("%s,%d,%d,%d,%d,%d,%f", alg, m, k, n, max_depth, threads, Gflops[iter]);

      if (seconds < 0.05 && num_failures < MAX_NUM_FAILURES) {
        printf(" WARNING: Matrix size may be too small to produce accurate timing data. Re-running...\n");
        num_failures++;
        success = 0;
        break;
      }
      printf("\n");
    }
    if (num_failures == MAX_NUM_FAILURES) {
      printf("ERROR: data at %s,%d,%d,%d,%d,%d should be gathered again\n", alg, m, k, n, max_depth, threads);
    }
    for (i=0; i<num_matrices; i++) free(A[i]);
    for (i=0; i<num_matrices; i++) free(B[i]);
    for (i=0; i<num_matrices; i++) free(C[i]);
  }

  FILE *f = fopen(output,"a");
  for (iter = 0; iter < num_iters; iter++) {
    fprintf(f,"%s,%d,%d,%d,%d,%d,%f\n", alg, m, k, n, max_depth, threads, Gflops[iter]);
  }
  fclose(f);

  free(Gflops);
  free(cacheClearer);

  // correctnessTest(m, k, n, max_depth);
}

void runSweepLinear(char* alg, int min_m, int min_k, int min_n, int max_m, int max_k, int max_n, int threads, int max_depth, int num_iters, int sweep_constant, char* output) {
  int m, k, n;
  for (m = min_m; m <= max_m; m += sweep_constant) {
    for (k = min_k; k <= max_k; k += sweep_constant) {
      for (n = min_n; n <= max_n; n += sweep_constant) {
        trial(alg, m, k, n, threads, max_depth, num_iters, output);
      }
    }
  }
}

void runSweepExp(char* alg, int min_m, int min_k, int min_n, int max_m, int max_k, int max_n, int threads, int max_depth, int num_iters, int sweep_constant, char* output) {
  int m, k, n;
  for (m = min_m; m <= max_m; m *= sweep_constant) {
    for (k = min_k; k <= max_k; k *= sweep_constant) {
      for (n = min_n; n <= max_n; n *= sweep_constant) {
        trial(alg, m, k, n, threads, max_depth, num_iters, output);
      }
    }
  }
}

int main(int argc, char **argv) {
  srand48(time(NULL));
  char* alg = argv[1];
  int min_m = atoi(argv[2]);
  int min_k = atoi(argv[3]);
  int min_n = atoi(argv[4]);
  int max_m = atoi(argv[5]);
  int max_k = atoi(argv[6]);
  int max_n = atoi(argv[7]);
  int threads = atoi(argv[8]);
  int max_depth = atoi(argv[9]);
  int num_iters = atoi(argv[10]);
  char* pattern = argv[11];
  int sweep_constant = atoi(argv[12]);
  char* output = argv[13];

  if (strcmp(pattern, "linear") == 0) {
    runSweepLinear(alg, min_m, min_k, min_n, max_m, max_k, max_n, threads, max_depth, num_iters, sweep_constant, output);
  }
  else if (strcmp(pattern, "exp") == 0) {
    runSweepExp(alg, min_m, min_k, min_n, max_m, max_k, max_n, threads, max_depth, num_iters, sweep_constant, output);
  }

  return 0;
}
