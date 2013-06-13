#include "header.h"

void multiply(int m, int k, int n, float *A, float *B, float *C, int max_depth) {
  cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1, A, m, B, k, 0, C, m);
}
