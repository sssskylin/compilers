
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define TRANSFORMED 896

uint32_t inverse_55 = 0x586FB587;

int count_z_calls(const char* start, const char* end) {
  int count = 0;

  while (start < end) {
    if (*start == 'z') {
      const char* p = start + 1;

      while (p < end && isspace((unsigned char)*p)) p++;

      if (p < end && *p == '(') count++;
    }

    start++;
  }

  return count;
}

int32_t to_signed(uint32_t value) {
  int32_t result;
  memcpy(&result, &value, sizeof(result));
  return result;
}

int32_t restore_value(int32_t value, int index) {
  switch (index % 7) {
    case 0:
      value ^= 0x63;
      break;

    case 1:
      value ^= 48;
      break;

    case 2:
      if (value % 16 != 0) exit(1);
      value /= 16;
      break;

    case 3:
      if (value % 4 != 0) exit(1);
      value /= 4;
      break;

    case 4:
      value = to_signed((uint32_t)value * inverse_55);
      break;

    case 5:
      value -= 97;
      break;

    case 6:
      value -= 44;
      break;
  }

  return value;
}

int main(int argc, char** argv) {
  const char* filename = argc > 1 ? argv[1] : "legacy_solved.c";

  FILE* file = fopen(filename, "rb");

  if (!file) {
    perror("fopen");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char* source = malloc(file_size + 1);

  if (!source) {
    fclose(file);
    return 1;
  }

  fread(source, 1, file_size, file);
  source[file_size] = '\0';
  fclose(file);

  char* array = strstr(source, "a[1024]");

  if (!array) {
    fprintf(stderr, "Array not found\n");
    free(source);
    return 1;
  }

  char* open = strchr(array, '{');

  if (!open) {
    fprintf(stderr, "Initializer not found\n");
    free(source);
    return 1;
  }

  int targets[SIZE];
  int count = 0;
  int depth = 0;

  char* element_start = open + 1;
  char* p = element_start;

  while (*p && count < SIZE) {
    if (*p == '(')
      depth++;

    else if (*p == ')')
      depth--;

    else if (*p == ',' && depth == 0) {
      targets[count++] = count_z_calls(element_start, p);

      element_start = p + 1;
    }

    else if (*p == '}' && depth == 0) {
      targets[count++] = count_z_calls(element_start, p);
      break;
    }

    p++;
  }

  if (count != SIZE) {
    fprintf(stderr, "Expected %d elements, found %d\n", SIZE, count);
    free(source);
    return 1;
  }

  unsigned char result[SIZE];

  for (int i = 0; i < SIZE; i++) {
    int32_t value = targets[i];

    if (i < TRANSFORMED) value = restore_value(value, i);

    if (value < 0 || value > 255) {
      fprintf(stderr, "Invalid byte at position %d: %d\n", i, value);
      free(source);
      return 1;
    }

    result[i] = (unsigned char)value;
  }

  result[SIZE - 1] = '\0';

  printf("%s", result);

  free(source);
  return 0;
}
