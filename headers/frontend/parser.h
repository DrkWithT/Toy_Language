#ifndef PARSER_H
#define PARSER_H

#include "frontend/lexer.h"
#include "frontend/ast.h"
#include "frontend/fileload.h"
#include "backend/values/vartypes.h"

typedef struct
{
    char *src_copy_ptr;
    int ready_flag;
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

void parser_log_err(Parser *parser, size_t line, const char *msg);

char *parser_stringify_token(Parser *parser, Token *token_ptr);

/// SECTION: Expression Parsing

Expression *parse_primitive(Parser *parser);

Expression *parse_literal(Parser *parser);

Expression *parse_call(Parser *parser);

Expression *parse_unary(Parser *parser);

Expression *parse_factor(Parser *parser);

Expression *parse_term(Parser *parser);

Expression *parse_comparison(Parser *parser);

Expression *parse_equality(Parser *parser);

Expression *parse_conditions(Parser *parser);

Expression *parse_expr(Parser *parser);

/// SECTION: Statement Parsing

Statement *parse_var_decl(Parser *parser);

Statement *parse_if_stmt(Parser *parser);

Statement *parse_otherwise_stmt(Parser *parser);

Statement *parse_block_stmt(Parser *parser);

Statement *parse_return_stmt(Parser *parser);

Statement *parse_while_stmt(Parser *parser);

Statement *parse_func_stmt(Parser *parser);

Statement *parse_use_stmt(Parser *parser);

Statement *parse_module_stmt(Parser *parser);

Statement *parse_expr_stmt(Parser *parser);

Statement *parse_stmt(Parser *parser);

Script *parser_parse_all(Parser *parser, const char *script_name);

#endif