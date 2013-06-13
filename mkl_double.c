#include "header.h"

void multiply(int m, int k, int n, double *A, double *B, double *C, int max_depth) {
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1, A, m, B, k, 0, C, m);
}
