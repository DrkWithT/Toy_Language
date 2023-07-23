#ifndef PARSER_H
#define PARSER_H

#include "frontend/lexer.h"
#include "frontend/ast.h"

typedef struct
{
    char *src_copy_ptr;
    Lexer lexer;
    Token current;
    Token next;
} Parser;

void parser_init(Parser *parser, char *src);

Token parser_scan(Parser *parser);

void parser_advance(Parser *parser);

void parser_consume(Parser *parser, TokenType type, const char *err_msg);

#endif