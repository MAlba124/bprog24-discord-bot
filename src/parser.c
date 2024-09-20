#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "vector.h"

static void unreachable() { assert("UNREACHABLE" == NULL); }

const char *parse_error_to_str(ParseError pe) {
  switch (pe) {
  case PE_OK:
    return "OK";
  case PE_INVALID_LEXEME:
    return "INVALID_LEXEME";
  case PE_MULTIPLE_DECIMAL_SPEARATORS:
    return "MULTIPLE_DECIMAL_SEPARATORS";
  case PE_STR_TO_DOUBLE_CONVERSION:
    return "STR_TO_DOUBLE_CONVERSION";
  case PE_NOT_AN_OPERATOR:
    return "NOT_AN_OPERATOR";
  case PE_MISSING_OPEN_PAR:
    return "MISSING_OPEN_PAR";
  }
  return "N/A";
}

static int tt_to_precedence(TokenType tt) {
  switch (tt) {
  case TT_ADD:
    return 2;
  case TT_SUB:
    return 2;
  case TT_MULTIPLY:
    return 3;
  case TT_DIVIDE:
    return 3;
  case TT_EOF:
  case TT_EMPTY:
  case TT_ERROR:
  case TT_NUM:
  case TT_OPENPAR:
  case TT_CLOSEPAR:
    return -1;
  }
  return 0;
}

#define RET_TOKEN(__token_name, __tt)                                          \
  (*str)++;                                                                    \
  (*error_index)++;                                                            \
  return (Token) { .type = __tt }

static Token next_token(char **str, ParseError *error, size_t *error_index) {
  if (isspace(**str)) {
    RET_TOKEN(t_empty, TT_EMPTY);
  }

  switch (**str) {
  case '\0':
    RET_TOKEN(t_eof, TT_EOF);
  case '(':
    RET_TOKEN(t_op, TT_OPENPAR);
  case ')':
    RET_TOKEN(t_cp, TT_CLOSEPAR);
  case '+':
    RET_TOKEN(t_add, TT_ADD);
  case '-':
    RET_TOKEN(t_sub, TT_SUB);
  case '*':
    RET_TOKEN(t_mult, TT_MULTIPLY);
  case '/':
    RET_TOKEN(t_div, TT_DIVIDE);
  case '0' ... '9':
    struct vector_char numbuf;
    vector_init_char(&numbuf);
    bool has_dot = false;
    while ((**str >= '0' && **str <= '9') || **str == '.') {
      if (**str == '.' && has_dot) {
        *error = PE_MULTIPLE_DECIMAL_SPEARATORS;
        return (Token){.type = TT_ERROR};
      } else if (**str == '.') {
        has_dot = true;
      }
      vector_push_char(&numbuf, **str);
      (*error_index)++;
      (*str)++;
    }
    vector_push_char(&numbuf, '\0');
    char *endptr;
    double n = strtod(numbuf.buf, &endptr);
    if (numbuf.buf != endptr) {
        vector_free_char(&numbuf);
        *error = PE_STR_TO_DOUBLE_CONVERSION;
        return (Token){.type = TT_ERROR};
    }
    vector_free_char(&numbuf);
    return (Token){.type = TT_NUM, .num = n};
  default:
    *error = PE_INVALID_LEXEME;
    return (Token){.type = TT_NUM, .num = TT_ERROR};
  }
}

struct vector_token parse_math(char *expr, ParseError *error,
                               size_t *error_index) {
  struct vector_token out;
  vector_init_token(&out);
  struct vector_token ops;
  vector_init_token(&ops);

  while (true) {
    Token tok = next_token(&expr, error, error_index);
    if (tok.type == TT_EOF) {
      break;
    } else if (tok.type == TT_ERROR) {
      vector_free_token(&ops);
      return out;
    }

    switch (tok.type) {
    case TT_EOF:
    case TT_ERROR:
      unreachable();
    case TT_EMPTY:
      break;
    case TT_NUM:
      vector_push_token(&out, tok);
      break;
    case TT_OPENPAR:
      vector_push_token(&ops, tok);
      break;
    case TT_CLOSEPAR:
      while (ops.len > 0 && ops.buf[ops.len - 1].type != TT_OPENPAR) {
        vector_push_token(&out, vector_pop_token(&ops));
      }
      Token ops_top = vector_pop_token(&ops);
      if (ops_top.type != TT_OPENPAR) {
        vector_free_token(&ops);
        *error = PE_MISSING_OPEN_PAR;
        return out;
      }
      break;
    case TT_ADD:
    case TT_SUB:
    case TT_MULTIPLY:
    case TT_DIVIDE:
      while (ops.len > 0 && ops.buf[ops.len - 1].type != TT_OPENPAR) {
        int op_precedence = tt_to_precedence(ops.buf[ops.len - 1].type);
        if (op_precedence == -1) {
          vector_free_token(&ops);
          *error = PE_NOT_AN_OPERATOR;
          return out;
        }
        int tok_precedence = tt_to_precedence(tok.type);
        if (tok_precedence == -1) {
          vector_free_token(&ops);
          *error = PE_NOT_AN_OPERATOR;
          return out;
        }
        if (op_precedence >= tok_precedence) {
          vector_push_token(&out, vector_pop_token(&ops));
        } else {
          break;
        }
      }
      vector_push_token(&ops, tok);
      break;
    }
  }

  while (ops.len > 0) {
    vector_push_token(&out, vector_pop_token(&ops));
  }

  vector_free_token(&ops);

  return out;
}
