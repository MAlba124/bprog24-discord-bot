#ifndef __H_PARSER
#define __H_PARSER 1

typedef enum {
  TT_EOF,
  TT_EMPTY,
  TT_ERROR,
  TT_NUM,
  TT_OPENPAR,
  TT_CLOSEPAR,
  TT_ADD,
  TT_SUB,
  TT_MULTIPLY,
  TT_DIVIDE
} TokenType;

typedef enum {
  PE_OK,
  PE_INVALID_LEXEME,
  PE_MULTIPLE_DECIMAL_SPEARATORS,
  PE_STR_TO_DOUBLE_CONVERSION,
  PE_NOT_AN_OPERATOR,
  PE_MISSING_OPEN_PAR
} ParseError;

typedef struct {
  TokenType type;
  double num;
} Token;

const char *parse_error_to_str(ParseError pe);

struct vector_token parse_math(char *expr, ParseError *error,
                               size_t *error_index);

#endif /* __H_PARSER */
