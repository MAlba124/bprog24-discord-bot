#ifndef __H_CONVERSION
#define __H_CONVERSION 1

#include "vector.h"

enum ConversionError {
  CE_OK,
  CE_ATTEMPT_TO_CONVERT_FLOAT,
  CE_STR_CONVERSION_FAILED
};

const char *conversion_error_to_str(enum ConversionError err);

void convert_to_hex(long long from, struct vector_char *to);
void convert_to_bin(long long from, struct vector_char *to);
long long convert_from_string(char *str, enum ConversionError *error);

#endif /* __H_CONVERSION */
