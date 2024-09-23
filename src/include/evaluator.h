#ifndef __H_EVALUATOR
#define __H_EVALUATOR 1

#include "vector.h"

typedef enum {
  ER_OK,
  ER_MULTIPLE_RESULTS,
  ER_MISSING_OPERAND,
  ER_INVALID_OPERATOR
} EvaluatorResult;

const char *evaluator_result_to_str(EvaluatorResult er);

double evaluate(struct vector_token *tokens, EvaluatorResult *res);

#endif /* __H_EVALUATOR */
