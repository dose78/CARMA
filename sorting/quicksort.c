#include "../header.h"

typedef struct {
  double *A;
  int length;
} Problem;

typedef struct {
  Problem* problems;
  int count;
} Problems;

typedef struct {
  double *A;
  int length;
} Result;

Result base_case(Problem p) {
  Result r = {p.A, p.length};
  return r;
}

Problems split(Problem p) {
  Problems problems;
  problems.problems = malloc(2 * sizeof(Problem));
  problems.count = 2;

  double *A = p.A;
  int length = p.length;
  // TODO: Quicksort

  return problems;
}

Result merge(Result* results) {
  Result r1 = results[0];
  Result r2 = results[1];
  Result r = {r1.A, r1.length + r2.length + 1};
  return r;
}

int should_run_base_case(Problem problem, int depth) {
  if (problem.length < 2) {
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

void sort(double *A, int length) {
  Problem problem = {A, length};
  solve(problem, 0);
}
