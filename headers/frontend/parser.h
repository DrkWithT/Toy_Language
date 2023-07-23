#ifndef PARSER_H
#define PARSER_H

#include "frontend/lexer.h"
#include "frontend/ast.h"

typedef struct
{
    char *src_copy_ptr;
    Lexer lexer;
    Token previous;
    Token current;
} Parser;

void parser_init(Parser *parser, char *src);

int parser_at_end(Parser *parser);

Token parser_peek_back(Parser *parser);

Token parser_peek_curr(Parser *parser);

void parser_advance(Parser *parser);

void parser_consume(Parser *parser, TokenType type, const char *err_msg);

Expression parse_list(Parser *parser);

Expression parse_literal(Parser *parser);

Expression parse_unary(Parser *parser);

Expression parse_term(Parser *parser);

Expression parse_factor(Parser *parser);

Expression parse_comparison(Parser *parser);

Expression parse_equality(Parser *parser);

Expression parse_expr(Parser *parser);

Statement parse_expr_stmt(Parser *parser);

Statement parse_use_stmt(Parser *parser);

Statement parse_module_stmt(Parser *parser);

Statement parse_block_stmt(Parser *parser);

Statement parse_if_stmt(Parser *parser);

Statement parse_otherwise_stmt(Parser *parser);

Statement parse_while_stmt(Parser *parser);

Statement parse_func_stmt(Parser *parser);

Statement parse_var_decl(Parser *parser);

Statement parse_stmt(Parser *parser);

Statement parser_parse_all(Parser *parser);

#endif