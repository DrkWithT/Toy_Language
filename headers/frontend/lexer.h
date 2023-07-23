#ifndef LEXER_H
#define LEXER_H

#include "frontend/token.h"

#define IS_WSP(c) (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n')
#define MATCH_CHAR(c, t) c == t
#define IS_ALPHA(c) (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'
#define IS_NUMERIC(c) (c >= '0' && c <= '9')

typedef struct st_lexer
{
    char *src;
    size_t pos;
    size_t limit;
    size_t line;
} Lexer;

void lexer_init(Lexer *lexer, char *source);

Token lexer_lex_wspace(Lexer *lexer);

Token lexer_lex_comment(Lexer *lexer);

Token lexer_lex_single(Lexer *lexer, TokenType type);

Token lexer_lex_keyword(Lexer *lexer, const char *keyword);

Token lexer_lex_identifier(Lexer *lexer);

Token lexer_lex_boolean(Lexer *lexer);

Token lexer_lex_number(Lexer *lexer);

Token lexer_lex_string(Lexer *lexer);

Token lexer_next_token(Lexer *lexer);

#endif