#include <cstdio>
#include <iostream>

#include "vector_math.h"

int main() {
  int v1[] = {1, 2, 3};
  int v2[] = {4, 5, 6};

  std::cout << "Library version: " << get_lib_version() << std::endl;
  std::cout << "Dot product result: " << dot_product(v1, v2, 3) << std::endl;

  return 0;
}
