#include <math.h>

#include "evaluator.h"
#include "vector.h"

const char *evaluator_result_to_str(EvaluatorResult er) {
  switch (er) {
  case ER_OK:
    return "OK";
  case ER_MULTIPLE_RESULTS:
    return "MULTIPLE_RESULTS";
  case ER_MISSING_OPERAND:
    return "MISSING_OPERAND";
  case ER_INVALID_OPERATOR:
    return "INVALID_OPERATOR";
  }
  return "N/A";
}

double evaluate(struct vector_token *tokens, EvaluatorResult *res) {
  struct vector_double vstack;
  vector_init_double(&vstack);

  for (size_t i = 0; i < tokens->len; i++) {
    switch (tokens->buf[i].type) {
    case TT_NUM:
      vector_push_double(&vstack, tokens->buf[i].num);
      break;
    case TT_ADD:
      if (vstack.len < 2) {
        break;
      }
      const double top = vector_pop_double(&vstack);
      vstack.buf[vstack.len - 1] += top;
      break;
    case TT_SUB:
      if (vstack.len < 2) {
        vstack.buf[vstack.len - 1] *= -1.0;
        break;
      }
      const double top_2 = vector_pop_double(&vstack);
      vstack.buf[vstack.len - 1] -= top_2;
      break;
    case TT_MULTIPLY:
      if (vstack.len < 2) {
        *res = ER_MISSING_OPERAND;
        return NAN;
      }
      const double top_3 = vector_pop_double(&vstack);
      vstack.buf[vstack.len - 1] *= top_3;
      break;
    case TT_DIVIDE:
      if (vstack.len < 2) {
        *res = ER_MISSING_OPERAND;
        return NAN;
      }
      const double top_4 = vector_pop_double(&vstack);
      vstack.buf[vstack.len - 1] /= top_4;
      break;
    case TT_EOF:
    case TT_EMPTY:
    case TT_ERROR:
    case TT_OPENPAR:
    case TT_CLOSEPAR:
    default:
      vector_free_token(tokens);
      vector_free_double(&vstack);
      *res = ER_INVALID_OPERATOR;
      return NAN;
    }
  }

  if (vstack.len != 1) {
    *res = ER_MULTIPLE_RESULTS;
    return NAN;
  }

  double evaluated_res = vstack.buf[0];

  vector_free_double(&vstack);

  *res = ER_OK;
  return evaluated_res;
}
