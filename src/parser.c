#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  case PE_MISSING_OPEN_PARENTHESES:
    return "MISSING_OPEN_PARENTHESES:";
  case PE_UNCLOSED_PARENTHESES:
    return "UNCLOSED_PARENTHESES";
  case PE_INVALID_FUNCTION:
    return "INVALID_FUNCTION";
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
  case TT_POW:
    return 4;
  case TT_SQRT:
  case TT_SIN:
  case TT_COS:
  case TT_TAN:
    return 5;
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

struct FunctionTableEntry {
  char *label;
  TokenType tt;
};

const struct FunctionTableEntry functions_table[] = {
    {.label = "sqrt", .tt = TT_SQRT},
    {.label = "sin", .tt = TT_SIN},
    {.label = "cos", .tt = TT_COS},
    {.label = "tan", .tt = TT_TAN}};

#define RET_TOKEN(__tt)                                                        \
  (*str)++;                                                                    \
  (*error_index)++;                                                            \
  return (Token) { .type = __tt }

static Token next_token(char **str, ParseError *error, size_t *error_index) {
  if (isspace(**str)) {
    RET_TOKEN(TT_EMPTY);
  }

  switch (**str) {
  case '\0':
    RET_TOKEN(TT_EOF);
  case '(':
    RET_TOKEN(TT_OPENPAR);
  case ')':
    RET_TOKEN(TT_CLOSEPAR);
  case '+':
    RET_TOKEN(TT_ADD);
  case '-':
    RET_TOKEN(TT_SUB);
  case '*':
    RET_TOKEN(TT_MULTIPLY);
  case '/':
    RET_TOKEN(TT_DIVIDE);
  case '^':
    RET_TOKEN(TT_POW);
  case '0' ... '9':
    struct vector_char numbuf;
    vector_init_char(&numbuf);
    bool has_dot = false;
    while ((**str >= '0' && **str <= '9') || **str == '.') {
      (*error_index)++;
      if (**str == '.' && has_dot) {
        *error = PE_MULTIPLE_DECIMAL_SPEARATORS;
        return (Token){.type = TT_ERROR};
      } else if (**str == '.') {
        has_dot = true;
      }
      vector_push_char(&numbuf, **str);
      (*str)++;
    }
    vector_push_char(&numbuf, '\0');
    char *endptr;
    double n = strtod(numbuf.buf, &endptr);
    if (numbuf.buf == endptr) {
      vector_free_char(&numbuf);
      *error = PE_STR_TO_DOUBLE_CONVERSION;
      return (Token){.type = TT_ERROR};
    }
    vector_free_char(&numbuf);
    return (Token){.type = TT_NUM, .num = n};
  case 'a' ... 'z':
    size_t function_start = *error_index;
    struct vector_char funcbuf;
    vector_init_char(&funcbuf);
    while (**str >= 'a' && **str <= 'z') {
      vector_push_char(&funcbuf, **str);
      (*error_index)++;
      (*str)++;
    }

    if (**str != '(') {
      *error_index = function_start;
      *error = PE_INVALID_FUNCTION;
      return (Token){.type = TT_ERROR};
    }

    for (size_t ft_i = 0;
         ft_i < sizeof(functions_table) / sizeof(functions_table[0]); ft_i++) {
      if (strlen(functions_table[ft_i].label) != funcbuf.len) {
        continue;
      }
      if (strncmp(funcbuf.buf, functions_table[ft_i].label, funcbuf.len) == 0) {
        vector_free_char(&funcbuf);
        return (Token){.type = functions_table[ft_i].tt};
      }
    }

    vector_free_char(&funcbuf);

    *error_index = function_start;
    *error = PE_INVALID_FUNCTION;
    return (Token){.type = TT_ERROR};
  default:
    (*error_index)++;
    *error = PE_INVALID_LEXEME;
    return (Token){.type = TT_ERROR};
  }
}

struct vector_token parse_math(char *expr, ParseError *error,
                               size_t *error_index) {
  struct vector_token out;
  vector_init_token(&out);
  struct vector_token ops;
  vector_init_token(&ops);

  int open_pars = 0;
  size_t last_open_par = 0;

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
    case TT_SQRT:
    case TT_SIN:
    case TT_COS:
    case TT_TAN:
      vector_push_token(&ops, tok);
      break;
    case TT_OPENPAR:
      if (open_pars == 0) {
        last_open_par = *error_index;
      }
      open_pars++;
      vector_push_token(&ops, tok);
      break;
    case TT_CLOSEPAR:
      while (ops.len > 0 && ops.buf[ops.len - 1].type != TT_OPENPAR) {
        vector_push_token(&out, vector_pop_token(&ops));
      }
      if (ops.len == 0 || vector_pop_token(&ops).type != TT_OPENPAR) {
        vector_free_token(&ops);
        *error = PE_MISSING_OPEN_PARENTHESES;
        return out;
      }
      open_pars--;
      break;
    case TT_ADD:
    case TT_SUB:
    case TT_MULTIPLY:
    case TT_DIVIDE:
    case TT_POW:
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

  if (open_pars > 0) {
    *error_index = last_open_par;
    vector_free_token(&ops);
    *error = PE_UNCLOSED_PARENTHESES;
    return out;
  }

  while (ops.len > 0) {
    vector_push_token(&out, vector_pop_token(&ops));
  }

  vector_free_token(&ops);

  return out;
}
