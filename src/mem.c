/* #include "mem.h" */
#include <stdlib.h>
#include <stdint.h>
#include <concord/log.h>

void *malloc_checked(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    log_fatal("Failed to allocate %ld bytes", size);
    exit(1);
  }
  return ptr;
}

void *realloc_checked(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (new_ptr == NULL) {
    log_fatal("Failed to reallocate %ld bytes", size);
    exit(1);
  }
  return new_ptr;
}
