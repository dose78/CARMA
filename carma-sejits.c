#include "header.h"
#define MAX_DEPTH 5
#define SPLIT_M 1
#define SPLIT_K 2
#define SPLIT_N 3

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
} Problem;

typedef struct {
  Problem* problems;
  int count;
} Problems;

typedef struct {
  int m, n, CM;
  double *C;
} Result;

int dim_to_split(int m, int k, int n) {
  if (n >= k && n >= m) return SPLIT_N;
  if (m >= k && m >= n) return SPLIT_M;
  return SPLIT_K;
}

Result base_case(Problem p) {
  cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,p.m,p.n,p.k,1,p.A,p.M,p.B,p.K,0,p.C,p.CM);
  Result r = {p.m, p.n, p.CM, p.C};
  return r;
}

Problems split(Problem p) {
  Problems problems;
  problems.problems = malloc(2 * sizeof(Problem));
  problems.count = 2;

  int split_dim = dim_to_split(p.m, p.k, p.n);
  if (split_dim == SPLIT_N) {
    p.n = p.n/2;
    double *B1 = p.B;
    double *B2 = p.B + p.n * p.K;
    Problem p1 = {p.M, p.K, p.m, p.k, p.n, p.CM, p.A, B1, p.C};
    Problem p2 = {p.M, p.K, p.m, p.k, p.n, p.CM, p.A, B2, p.C + p.n * p.CM};
    problems.problems[0] = p1;
    problems.problems[1] = p2;

  } else if (split_dim == SPLIT_M) {
    p.m = p.m/2;
    double *A1 = p.A;
    double *A2 = p.A + p.m;
    Problem p1 = {p.M, p.K, p.m, p.k, p.n, p.CM, A1, p.B, p.C};
    Problem p2 = {p.M, p.K, p.m, p.k, p.n, p.CM, A2, p.B, p.C + p.m};
    problems.problems[0] = p1;
    problems.problems[1] = p2;

  } else { // SPLIT_K
    p.k = p.k/2;
    double *A1 = p.A;
    double *A2 = p.A + p.k * p.M;
    double *B1 = p.B;
    double *B2 = p.B + p.k;
    double *Q1 = (double*) malloc(p.m * p.n * sizeof(double));
    Problem p1 = {p.M, p.K, p.m, p.k, p.n, p.CM, A2, B2, p.C};
    Problem p2 = {p.M, p.K, p.m, p.k, p.n, p.m, A1, B1, Q1};
    problems.problems[0] = p1;
    problems.problems[1] = p2;
  }

  return problems;
}

Result merge(Result* results) {
  Result r1 = results[0];
  Result r2 = results[1];

  if (r1.C + r1.n * r1.CM == r2.C) { // split n
    Result r = {r1.m, 2*r1.n, r1.CM, r1.C};
    return r;

  } else if (r1.C + r1.m == r2.C) { // split m
    Result r = {2*r1.m, r1.n, r1.CM, r1.C};
    return r;

  } else { // split k
    int x;
    for (x = 0; x < r1.n; x++) {
      cblas_daxpy(r2.m, 1, r2.C + r2.m * x, 1, r1.C + r1.CM * x, 1);
    }
    free(r2.C);
    Result r = {r1.m, r1.n, r1.CM, r1.C};
    return r;
  }
}

int should_run_base_case(Problem problem, int depth) {
  if (depth >= MAX_DEPTH) {
    return 1;
  } else {
    return 0;
  }
}

Result solve(Problem problem, int depth) {
  if (should_run_base_case(problem, depth)) {
    return base_case(problem);
  }

  Problems subproblems = split(problem);
  Result* results = malloc(subproblems.count * sizeof(Result));

  int i;
  for (i = 0; i < subproblems.count; i++) {
    results[i] = cilk_spawn solve(subproblems.problems[i], depth + 1);
  }
  cilk_sync;

  free(subproblems.problems);

  Result result = merge(results);
  free(results);
  return result;
}

void multiply(int m, int k, int n, double *A, double *B, double *C) {
  Problem problem = {m, k, m, k, n, m, A, B, C};
  solve(problem, 0);
}
