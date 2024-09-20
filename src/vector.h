#ifndef __H_VECTOR
#define __H_VECTOR 1

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "mem.h"
#include "parser.h"

#define VECTOR_DEFAULT_CAPACITY 16

#define VECTOR_HEADER_DEF(__T, __NAME)                                         \
  struct vector_##__NAME {                                                     \
    __T *buf;                                                                  \
    size_t len;                                                                \
    size_t cap;                                                                \
  };                                                                           \
  void vector_free_##__NAME(struct vector_##__NAME *v);                        \
  void vector_init_##__NAME(struct vector_##__NAME *v);                        \
  void vector_push_##__NAME(struct vector_##__NAME *v, __T e);                 \
  __T vector_pop_##__NAME(struct vector_##__NAME *v)

#define VECTOR_INIT_DEF(__T, __NAME)                                           \
  void vector_init_##__NAME(struct vector_##__NAME *v) {                       \
    v->buf = malloc_checked(sizeof(__T) * VECTOR_DEFAULT_CAPACITY);            \
    v->cap = VECTOR_DEFAULT_CAPACITY;                                          \
    v->len = 0;                                                                \
  }

#define VECTOR_PUSH_DEF(__T, __NAME)                                           \
  void vector_push_##__NAME(struct vector_##__NAME *v, __T e) {                \
    if (v->cap <= v->len) {                                                    \
      v->cap += VECTOR_DEFAULT_CAPACITY;                                       \
      v->buf = realloc_checked(v->buf, v->cap * sizeof(__T));                  \
    }                                                                          \
    v->buf[v->len] = e;                                                        \
    v->len++;                                                                  \
  }

#define VECTOR_POP_DEF(__T, __NAME)                                            \
  __T vector_pop_##__NAME(struct vector_##__NAME *v) {                         \
    assert(v->len > 0);                                                        \
    v->len--;                                                                  \
    return v->buf[v->len];                                                     \
  }

#define VECTOR_FREE_DEF(__T, __NAME)                                           \
  void vector_free_##__NAME(struct vector_##__NAME *v) {                       \
    free(v->buf);                                                              \
    v->cap = 0;                                                                \
    v->len = 0;                                                                \
  }

#define VECTOR_FUNC_DEF(__T, __NAME)                                           \
  VECTOR_INIT_DEF(__T, __NAME)                                                 \
  VECTOR_FREE_DEF(__T, __NAME)                                                 \
  VECTOR_PUSH_DEF(__T, __NAME)                                                 \
  VECTOR_POP_DEF(__T, __NAME)

VECTOR_HEADER_DEF(TokenType, tokentype);
VECTOR_HEADER_DEF(char, char);
VECTOR_HEADER_DEF(Token, token);
VECTOR_HEADER_DEF(double, double);

#endif /* __H_VECTOR */
