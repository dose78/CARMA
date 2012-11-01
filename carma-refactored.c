#include "header.h"
#define MAX_DEPTH 5

void initialize(int m, int k, int n, double* A, double* B, double* C) {
  srand48(time(NULL));
  int i;
  for(i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
  for(i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
  for(i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

typedef struct {
  int M, K, m, k, n, CM;
  double *A, *B, *C;
} problem;

void base_case(problem p) {
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,p.m,p.n,p.k,1,p.A,p.M,p.B,p.K,0,p.C,p.CM);
  return;
}

problem* split(problem p) {
  int max = p.k;
  if (p.m > max) {
    max = p.m;
  }
  if (p.n > max) {
    max = p.n;
  }

  problem* problems = malloc(2 * sizeof(problem));

  if (max == p.n) {
    p.n = p.n/2;
    double *B1 = p.B;
    double *B2 = p.B + p.n * p.K;
    problem p1 = {p.M, p.K, p.m, p.k, p.n, p.CM, p.A, B1, p.C};
    problem p2 = {p.M, p.K, p.m, p.k, p.n, p.CM, p.A, B2, p.C + p.n * p.CM};
    problems[0] = p1;
    problems[1] = p2;

  } else if (max == p.m) {
    p.m = p.m/2;
    double *A1 = p.A;
    double *A2 = p.A + p.m;
    problem p1 = {p.M, p.K, p.m, p.k, p.n, p.CM, A1, p.B, p.C};
    problem p2 = {p.M, p.K, p.m, p.k, p.n, p.CM, A2, p.B, p.C + p.m};
    problems[0] = p1;
    problems[1] = p2;

  } else {
    p.k = p.k/2;
    double *A1 = p.A;
    double *A2 = p.A + p.k * p.M;
    double *B1 = p.B;
    double *B2 = p.B + p.k;
    double *Q1 = (double*) malloc(p.m * p.n * sizeof(double));
    problem p1 = {p.M, p.K, p.m, p.k, p.n, p.m, A1, B1, Q1};
    problem p2 = {p.M, p.K, p.m, p.k, p.n, p.CM, A2, B2, p.C};
    problems[0] = p1;
    problems[1] = p2;
  }

  return problems;
}

void merge(problem* problems) {
  problem p1 = problems[0];
  problem p2 = problems[1];
  if (p1.A != p2.A && p1.B != p2.B) { // merge required only if split k
    int x;
    for (x = 0; x < p1.n; x++) {
      cblas_daxpy(p1.m, 1, p1.C + p1.m * x, 1, p2.C + p2.CM * x, 1);
    }
    free(p1.C);
  }
  free(problems);
}

void inner_multiply(problem p, int depth) {
  if (depth >= MAX_DEPTH) {
    base_case(p);
    return;
  }

  problem* problems = split(p);
  problem p1 = problems[0];
  problem p2 = problems[1];

  cilk_spawn inner_multiply(p1, depth + 1);
  inner_multiply(p2, depth + 1);
  cilk_sync;

  merge(problems);
}

void multiply(int m, int k, int n, double *A, double *B, double *C) {
  problem p = {m, k, m, k, n, m, A, B, C};
  inner_multiply(p, 0);
}
