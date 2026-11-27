#include <cstddef>
#include <cstdio>

struct Mixed {
  char a;
  int b;
  char c;
  double d;
  short e;
};

#pragma pack(push, 1)
struct MixedP {
  char a;
  int b;
  char c;
  double d;
  short e;
};
#pragma pack(pop)

int main() {
  printf("sizeof(Mixed) = %zu\n", sizeof(Mixed));
  printf("alignof(Mixed) = %zu\n", alignof(Mixed));
  printf("sizeof(MixedP) = %zu\n", sizeof(MixedP));
  printf("alignof(MixedP) = %zu\n", alignof(MixedP));

  size_t sum = sizeof(char) + sizeof(int) + sizeof(char)
             + sizeof(double) + sizeof(short);
  printf("sum of fields = %zu\n", sum);

  MixedP p{};
  char* pc = &p.c;

  return 0;
}