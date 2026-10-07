#include "vector_math.h"

static const char* LIB_VERSION = "v2.4.1-release";

const char* get_lib_version(void) { return LIB_VERSION; }

int dot_product(const int* a, const int* b, int n) {
  int sum = 0;
  for (int i = 0; i < n; i++) {
    sum += a[i] * b[i];
  }
  return sum;
}
