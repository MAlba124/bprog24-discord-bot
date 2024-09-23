#include <stdio.h>
#include <assert.h>

#include "include/vector.h"

VECTOR_HEADER_DEF(double, double);
VECTOR_FUNC_DEF(double, double);

int main() {
  struct vector_double vec;
  vector_init_double(&vec);
  assert(vec.len == 0);
  vector_push_double(&vec, 1.0);
  assert(vec.len == 1);
  vector_pop_double(&vec);
  assert(vec.len == 0);

  vector_push_double(&vec, 1.0);
  vector_push_double(&vec, 1.0);
  assert(vec.len == 2);
  vector_pop_double(&vec);
  assert(vec.len == 1);
  vector_pop_double(&vec);
  assert(vec.len == 0);

  printf("All tests passed\n");

  return 0;
}
