#define _GNU_SOURCE
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "include/conversion.h"
#include "include/vector.h"

const char *conversion_error_to_str(enum ConversionError err) {
  switch (err) {
  case CE_OK:
    return "OK";
  case CE_ATTEMPT_TO_CONVERT_FLOAT:
    return "ATTEMPT_TO_CONVERT_FLOAT";
  case CE_STR_CONVERSION_FAILED:
    return "STR_CONVERSION_FAILED";
  }
  return "N/A";
}

void convert_to_hex(long long from, struct vector_char *to) {
  char *hex_str = NULL;
  assert(asprintf(&hex_str, "0x%llX", from) != -1);
  vector_free_char(to);
  to->buf = hex_str;
  to->len = strlen(hex_str);
  to->cap = strlen(hex_str);
}

void convert_to_bin(long long from, struct vector_char *to) {
  bool has_a_one = false;

  if (from < 0) {
    vector_push_char(to, '-');
    from *= -1;
  }
  vector_push_char(to, '0');
  vector_push_char(to, 'b');

  long long mask = 1LL << 62;
  while (mask > 0) {
    if ((from & mask) > 0) {
      vector_push_char(to, '1');
      has_a_one = true;
    } else if (has_a_one) {
      vector_push_char(to, '0');
    }
    mask >>= 1;
  }

  if (!has_a_one) {
    vector_push_char(to, '0');
  }

  vector_push_char(to, '\0');
}

static inline bool is_valid_dec(char c) { return c >= '0' && c <= '9'; }

static inline bool is_valid_hex(char c) {
  return is_valid_dec(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
}

static inline bool is_valid_bin(char c) { return c == '0' || c == '1'; }

long long convert_from_string(char *str, enum ConversionError *error) {
  bool is_negative = false;
  if (*str == '-') {
    is_negative = true;
    str++;
  }

  int base;
  struct vector_char parsed;
  vector_init_char(&parsed);
  if (*str == '0') {
    str++;
    if (tolower(*str) == 'b') {
      str++;
      base = 2;
      while (1) {
        if (*str == '\0') {
          break;
        }

        if (!is_valid_bin(*str)) {
          *error = CE_STR_CONVERSION_FAILED;
          return 0;
        }
        vector_push_char(&parsed, *str);
        str++;
      }
      goto fin;
    } else if (tolower(*str) == 'x') {
      str++;
      base = 16;
      while (1) {
        if (*str == '\0') {
          break;
        }

        if (!is_valid_hex(*str)) {
          *error = CE_STR_CONVERSION_FAILED;
          return 0;
        }
        vector_push_char(&parsed, *str);
        str++;
      }
      goto fin;
    }
  }

  base = 10;
  while (1) {
    if (*str == '\0') {
      break;
    }

    if (!is_valid_dec(*str)) {
      if (*str == '.') {
        *error = CE_ATTEMPT_TO_CONVERT_FLOAT;
      } else {
        *error = CE_STR_CONVERSION_FAILED;
      }
      return 0;
    }
    vector_push_char(&parsed, *str);
    str++;
  }

fin:
  vector_push_char(&parsed, '\0');
  char *endptr = NULL;
  long long res = strtoll(parsed.buf, &endptr, base);
  if (endptr != NULL) {
    *error = CE_STR_CONVERSION_FAILED;
  }

  *error = CE_OK;

  vector_free_char(&parsed);

  return is_negative ? -1 * res : res;
}
