#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <string.h>

typedef enum en_token_type
{
    WSPACE,
    COMMENT,
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    BOOLEAN,
    INTEGER,
    REAL,
    STRBODY,
    LBRACK,
    RBRACK,
    LPAREN,
    RPAREN,
    COMMA,
    EOS,
    UNKNOWN
} TokenType;

typedef struct st_token
{
    TokenType type;
    size_t begin;
    size_t span;
    size_t line;
} Token;

void token_init(Token *token, TokenType type, size_t begin, size_t span, size_t line);

char *token_as_txt(Token *token, const char *source);

#endif