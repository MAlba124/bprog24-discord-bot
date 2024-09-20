#ifndef __H_MEM
#define __H_MEM 1

#include <stdint.h>

void *malloc_checked(size_t size);
void *realloc_checked(void *ptr, size_t size);

#endif /* __H_MEM */
