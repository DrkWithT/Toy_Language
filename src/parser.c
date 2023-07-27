/**
 * @file parser.c
 * @author Derek Tan
 * @brief Implements Parser using recursive descent.
 * @date 2023-07-23
 * @todo Add a parse_assign_expr() function later. See https://craftinginterpreters.com/statements-and-state.html#assignment-syntax
 * @todo Use parse_logical() after checking its grammar precedence.
 */

#include "frontend/parser.h"

void parser_init(Parser *parser, char *src)
{
    parser->src_copy_ptr = src;
    parser->ready_flag = src != NULL;
    lexer_init(&parser->lexer, src);
    token_init(&parser->previous, UNKNOWN, 0, 0, 0);
    token_init(&parser->current, UNKNOWN, 0, 0, 0);
    parser_advance(parser);
}

int parser_at_end(Parser *parser)
{
    return parser->current.type == EOS;
}

Token parser_peek_back(Parser *parser)
{
    return parser->previous;
}

Token parser_peek_curr(Parser *parser)
{
    return parser->current;
}

void parser_advance(Parser *parser)
{
    if (parser_at_end(parser)) return;

    // advance previous token hold...
    parser->previous = parser->current;

    // advance current token until next valid one... report bad tokens!
    while (1)
    {
        parser->current = lexer_next_token(&parser->lexer);
        
        if (parser->current.type == WSPACE || parser->current.type == COMMENT)
            continue;

        if (parser->current.type != UNKNOWN)
            break;

        parser_log_err(parser, parser->current.line, "Unknown or unexpected token.");
    }
}

void parser_consume(Parser *parser, TokenType type, const char *err_msg)
{
    if (parser->current.type == type)
    {
        parser_advance(parser);
        return;
    }

    parser_log_err(parser, parser->current.line, "Unexpected token.");
}

void parser_log_err(Parser *parser, size_t line, const char *msg)
{
    fprintf(stderr, "ParseError at line %zu: %s\n", line, msg);
}

char *parser_stringify_token(Parser *parser, Token *token_ptr)
{
    if (!token_ptr)
        return NULL;

    return token_as_txt(token_ptr, parser->src_copy_ptr);
}

/// SECTION: Expressions

Expression *parse_primitive(Parser *parser)
{
    puts("parse_primitive");
    Token token = parser_peek_curr(parser);
    char *lexeme = parser_stringify_token(parser, &token);
    Expression *expr = NULL;

    if (!lexeme)
        return expr;

    switch (token.type)
    {
    case BOOLEAN:
        expr = create_bool(strncmp(lexeme, "$T", 2) == 0);
        free(lexeme);
        break;
    case INTEGER:
        expr = create_int(atoi(lexeme));
        free(lexeme);
        break;
    case REAL:
        expr = create_real(strtof(lexeme, NULL));
        free(lexeme);
        break;
    case STRBODY:
        expr = create_str(create_str_obj(lexeme));
        printf("parse_primitive: STRBODY \"%s\"\n", lexeme);
        // NOTE: StringObj now owns the lexeme content. No free needed yet.
        break;
    default:
        parser_log_err(parser, token.line, "Expected primitive value.");
        free(lexeme);
        break;
    }

    parser_advance(parser);

    return expr;
}

Expression *parse_list(Parser *parser)
{
    puts("parse_list");
    int bad_comma = 0;
    int comma_expected = 0;
    Token tok = parser_peek_curr(parser);
    Expression *expr = NULL;
    Expression *literal = NULL;

    // validate list syntax start!
    if (tok.type != LBRACK)
    {
        parser_log_err(parser, tok.line, "Unexpected token.");
        return expr;
    }

    // enter list body!
    parser_advance(parser);

    ListObj *list_val = create_list_obj();

    if (!list_val)
        return expr;

    // consume 1st literal and so on...
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type == RBRACK)
            break;

        if ((tok.type == COMMA && !comma_expected) || bad_comma)
        {
            bad_comma = 1;
            break;
        }

        switch (tok.type)
        {
        case BOOLEAN:
            literal = parse_primitive(parser);
            append_list_obj(list_val, create_bool_varval(0, literal->syntax.bool_literal.flag));

            destroy_expr(literal);
            free(literal);

            break;
        case INTEGER:
            literal = parse_primitive(parser);
            append_list_obj(list_val, create_int_varval(0, literal->syntax.int_literal.value));

            destroy_expr(literal);
            free(literal);

            comma_expected = 1;
            break;
        case REAL:
            literal = parse_primitive(parser);
            append_list_obj(list_val, create_real_varval(0, literal->syntax.real_literal.value));

            destroy_expr(literal);
            free(literal);

            comma_expected = 1;
            break;
        case STRBODY:
            literal = parse_primitive(parser);
            append_list_obj(list_val, create_str_varval(0, (StringObj *)(literal->syntax.str_literal.str_obj)));

            free(literal);

            comma_expected = 1;
            break;
        case LBRACK:
            literal = parse_list(parser);
            append_list_obj(list_val, create_list_varval(0, literal->syntax.list_literal.list_obj));
            free(literal);

            comma_expected = 1;
            break;
        case COMMA:
            comma_expected = 0;
            parser_advance(parser);
            break;
        default:
            comma_expected = 0;
            bad_comma = 1;
            break;
        }
    }

    parser_advance(parser);  // NOTE: pass list's closing bracket

    if (bad_comma)
    {
        parser_log_err(parser, tok.line, "Unexpected comma nearby.");
        destroy_expr(expr);
        free(expr);
        expr = NULL;

        destroy_list_obj(list_val);
        free(list_val);
        return expr;
    }

    expr = create_list(list_val);

    return expr;
}

Expression *parse_literal(Parser *parser)
{
    puts("parse_literal");
    Token token = parser_peek_curr(parser);
    Expression *expr = NULL;

    switch (token.type)
    {
    case LPAREN:
        puts("nested expr");
        parser_advance(parser);
        expr = parse_expr(parser);

        token = parser_peek_curr(parser);

        if (token.type != RPAREN)
        {
            destroy_expr(expr);
            free(expr);
            expr = NULL;
        }
        else
        {
            puts("pass RPAREN");
            parser_advance(parser);
        }

        break;
    case BOOLEAN:
    case INTEGER:
    case REAL:
    case STRBODY:
        expr = parse_primitive(parser);
        break;
    case LBRACK:
        expr = parse_list(parser);
        break;
    case IDENTIFIER:
        expr = create_var(0, parser_stringify_token(parser, &token));
        parser_advance(parser);
        break;
    default:
        break;
    }

    return expr;
}

Expression *parse_call(Parser *parser)
{
    puts("parse_call");
    // NOTE: parse_call technically should be named parse_usage for handling identifier expressions.
    Token prev;
    Token tok = parser_peek_curr(parser);
    Expression *expr = NULL;
    char *lexeme = NULL;
    int expect_comma = 0;
    int bad_comma = 0;

    if (tok.type != IDENTIFIER)
    {
        parser_log_err(parser, tok.line, "Expected identifier.");
        return expr;
    }

    parser_advance(parser);
    tok = parser_peek_curr(parser);
    prev = parser_peek_back(parser);

    // handle case of lone identifier usage (a variable!)
    if (tok.type != LPAREN)
    {
        lexeme = parser_stringify_token(parser, &prev);
        expr = create_var(0, lexeme);

        return expr;
    }

    // prepare call expression node with identifier string
    lexeme = parser_stringify_token(parser, &prev);
    printf("making call expr for \"%s\"\n", lexeme);
    expr = create_call(lexeme);

    // process argument listing until ')'
    parser_advance(parser);

    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type == RPAREN)
            break;

        if (tok.type == COMMA && !expect_comma)
        {
            bad_comma = 1;
            break;
        }

        if (tok.type != COMMA)
            add_arg_call(expr, parse_expr(parser));
        else
            parser_advance(parser); // NOTE: don't advance twice on non-comma tokens!

        expect_comma ^= 1;
    }

    parser_advance(parser);

    if (bad_comma)
    {
        destroy_expr(expr);
        free(expr);
        expr = NULL;
    }

    return expr;
}

Expression *parse_unary(Parser *parser)
{
    puts("parse_unary");
    Token tok = parser_peek_curr(parser);
    char operator_symbol = parser->src_copy_ptr[tok.begin];
    Expression *expr = NULL;

    if (tok.type == OPERATOR && tok.span == 1 && operator_symbol == '-')
    {
        parser_advance(parser);
        expr = create_unary(OP_NEG, parse_literal(parser));
    }
    else if (tok.type == IDENTIFIER)
    {
        // NOTE: parse a function call expression here... var uses also handled here!
        expr = parse_call(parser);
    }
    else
    {
        // NOTE: an unary can also be another positive value (no minus!)
        expr = parse_literal(parser);
    }

    return expr;
}

Expression *parse_factor(Parser *parser)
{
    puts("parse_factor");
    // parse left side
    OpType operation;
    Token tok;
    Expression *left = parse_unary(parser);
    Expression *right = NULL;
    Expression *temp;  // next expr node on right side
    Expression *expr = left;  // result expr node
    int valid_oper = 0;

    // parse and validate operator-literal pairs after left side (parser auto advances!)
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type != OPERATOR && !valid_oper)
            return expr;

        char operator_symbol = parser->src_copy_ptr[tok.begin];

        printf("parse_factor: symbol1 = '%c'\n", operator_symbol);

        if (tok.span != 1 || (tok.type == OPERATOR && operator_symbol != '*' && operator_symbol != '/'))
        {
            puts("pre-leave parse_factor");
            return expr;
        }
        else if (tok.type == OPERATOR)
        {
            operation = (operator_symbol == '*') ? OP_MUL : OP_DIV;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (tok.type == RPAREN)
        {
            // NOTE: stop parsing at ')' which marks expr end!
            break;
        }
        else
        {
            puts("make factor expr");
            right = parse_unary(parser);
            temp = create_binary(operation, expr, right);
            expr = temp;
        }
    }

    return expr;
}

Expression *parse_term(Parser *parser)
{
    puts("parse_term");
    OpType operation;
    Token tok;
    Expression *left = parse_factor(parser);
    Expression *right = NULL;
    Expression *temp;  // next expr node on right side
    Expression *expr = left;  // result expr node
    int valid_oper = 0;

    // parse and validate operator-literal pairs after left side (parser auto advances!)
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type != OPERATOR && !valid_oper)
            return expr;

        char operator_symbol = parser->src_copy_ptr[tok.begin];

        printf("parse_term: symbol1 = '%c'\n", operator_symbol);

        if (tok.span != 1 || (tok.type == OPERATOR && operator_symbol != '+' && operator_symbol != '-'))
        {
            puts("pre-leave parse_term");
            return expr;
        }
        else if (tok.type == OPERATOR)
        {
            operation = (operator_symbol == '+') ? OP_ADD : OP_SUB;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (tok.type == RPAREN)
        {
            break;
        }
        else
        {
            puts("make term expr");
            right = parse_factor(parser);
            temp = create_binary(operation, expr, right);
            expr = temp;
        }
    }

    return expr;
}

Expression *parse_comparison(Parser *parser)
{
    puts("parse_comparison");
    OpType operation;
    Token tok;
    Expression *left = parse_term(parser);
    Expression *right = NULL;
    Expression *temp;  // next expr node on right side
    Expression *expr = left;  // result expr node
    int valid_oper = 0;

    // parse and validate operator-literal pairs after left side (parser auto advances!)
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type != OPERATOR && !valid_oper)
            return expr;

        char *lexeme = parser_stringify_token(parser, &tok);

        if (strncmp(lexeme, ">=", 2) == 0)
        {
            operation = OP_GTE;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (strncmp(lexeme, "<=", 2) == 0)
        {
            operation = OP_LTE;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (lexeme[0] == '>' && lexeme[1] == '\0')
        {
            operation = OP_GT;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (lexeme[0] == '<' && lexeme[1] == '\0')
        {
            operation = OP_LT;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (valid_oper)
        {
            puts("make comparison expr");
            right = parse_term(parser);
            temp = create_binary(operation, expr, right);
            expr = temp;
        }
        else if (tok.type == OPERATOR)
        {
            puts("pre-leave parse_comparison");
            return expr;
        }
        else if (tok.type == RPAREN)
        {
            break;
        }
        else
        {
            // NOTE: a malformed expr should be discarded... not runnable anyways
            parser_log_err(parser, tok.line, "Invalid operator or token in expr.");

            free(lexeme);
            destroy_expr(expr);
            free(expr);

            expr = NULL;
            return expr;
        }

        free(lexeme);
    }

    return expr;
}

Expression *parse_equality(Parser *parser)
{
    puts("parse_equality");
    OpType operation;
    Token tok;
    Expression *left = parse_comparison(parser);
    Expression *right = NULL;
    Expression *temp;  // next expr node on right side
    Expression *expr = left;  // result expr node
    int valid_oper = 0;

    // parse and validate operator-literal pairs after left side (parser auto advances!)
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type != OPERATOR && !valid_oper)
            return expr;

        char *lexeme = parser_stringify_token(parser, &tok);

        if (strncmp(lexeme, "!=", 2) == 0)
        {
            operation = OP_NEQ;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (strncmp(lexeme, "==", 2) == 0)
        {
            operation = OP_EQ;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (valid_oper)
        {
            right = parse_comparison(parser);
            temp = create_binary(operation, expr, right);
            expr = temp;
        }
        else if (tok.type == OPERATOR)
        {
            puts("pre-leave parse_equality");
            return expr;
        }
        else if (tok.type == RPAREN)
        {
            break;
        }
        else
        {
            parser_log_err(parser, tok.line, "Invalid operator or token in expr.");

            free(lexeme);
            destroy_expr(expr);
            free(expr);
            expr = NULL;

            return expr;
        }

        free(lexeme);
    }

    return expr;
}

/*Expression *parse_logical(Parser *parser)
{
    puts("parse_logical");
    OpType operation;
    Token tok;
    Expression *left = parse_equality(parser);
    Expression *right = NULL;
    Expression *temp;  // next expr node on right side
    Expression *expr = left;  // result expr node
    int valid_oper = 0;

    // parse and validate operator-literal pairs after left side (parser auto advances!)
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type != OPERATOR && !valid_oper)
            return expr;

        char *lexeme = parser_stringify_token(parser, &tok);

        if (strncmp(lexeme, "||", 2) == 0)
        {
            operation = OP_OR;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (strncmp(lexeme, "&&", 2) == 0)
        {
            operation = OP_AND;
            valid_oper = 1;
            parser_advance(parser);
        }
        else if (valid_oper)
        {
            right = parse_equality(parser);
            temp = create_binary(operation, expr, right);
            expr = temp;
        }
        else if (tok.type == RPAREN)
        {
            break;
        }
        else
        {
            parser_log_err(parser, tok.line, "Invalid operator or token in expr.");

            free(lexeme);
            destroy_expr(expr);
            free(expr);
            expr = NULL;

            return expr;
        }

        free(lexeme);
    }

    return expr;
}*/

Expression *parse_expr(Parser *parser)
{
    if (!parser_at_end(parser))
        return parse_equality(parser);

    return NULL;
}

/// SECTION: Statements

Statement *parse_var_decl(Parser *parser)
{
    puts("parse_var_decl");
    Token tok = parser_peek_curr(parser);
    char *lexeme = NULL;
    Statement *var_decl = NULL;
    int is_const = 0;

    if (tok.type != KEYWORD)
    {
        parser_log_err(parser, tok.line, "Expected 'let' or 'const'.");
        return var_decl;
    }

    lexeme = parser_stringify_token(parser, &tok);

    // check whether to parse as const variable or not!
    if (strcmp(lexeme, "let") == 0)
    {
        is_const = 0;
        free(lexeme);
    }
    else if (strcmp(lexeme, "const") == 0)
    {
        is_const = 1;
        free(lexeme);  // dispose temp keyword string
    }
    else
    {
        // reject invalid var declarations as NULL AST nodes
        parser_log_err(parser, tok.line, "Expected 'let' or 'const'.");
        is_const = -1;
        free(lexeme);
        return var_decl;
    }

    // check possible identifier and process it when valid
    parser_advance(parser);
    tok = parser_peek_curr(parser);

    if (tok.type != IDENTIFIER)
        return var_decl;

    lexeme = parser_stringify_token(parser, &tok);
    var_decl = create_var_decl(is_const, lexeme, NULL);
    lexeme = NULL;

    // check possible assignment token
    parser_advance(parser);
    tok = parser_peek_curr(parser);

    if (tok.type != OPERATOR || parser->src_copy_ptr[tok.begin] != '=')
    {
        parser_log_err(parser, tok.line, "Expected '='.");
        destroy_stmt(var_decl);
        free(var_decl);
        var_decl = NULL;

        return var_decl; // reject uninitialized vars by my varDecl rule!
    }

    // handle rvalue expression
    parser_advance(parser);
    tok = parser_peek_curr(parser);
    var_decl->syntax.var_decl.rvalue = parse_expr(parser);

    return var_decl;
}

Statement *parse_otherwise_stmt(Parser *parser)
{
    puts("parse_otherwise_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = NULL;
    Statement *block_stmt = NULL;
    Statement *otherwise_stmt = NULL;

    // check for "otherwise" keyword to validate stmt
    if (tok.type != KEYWORD)
        return otherwise_stmt;

    lexeme = parser_stringify_token(parser, &tok);

    if (strncmp(lexeme, "otherwise", 9) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'otherwise'.");
        free(lexeme);
        return otherwise_stmt;
    }

    free(lexeme);

    // continue parsing otherwise block stmt
    block_stmt = parse_block_stmt(parser);

    // reject failed parses of otherwise blocks!
    if (!block_stmt)
        return otherwise_stmt;

    otherwise_stmt = create_otherwise_stmt(block_stmt);

    return otherwise_stmt;
}

Statement *parse_if_stmt(Parser *parser)
{
    puts("parse_if_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = NULL;
    Statement *if_stmt = NULL;
    Statement *first_block = NULL;

    // check for starting keyword "if" to validate stmt
    if (tok.type != KEYWORD)
        return if_stmt;

    lexeme = parser_stringify_token(parser, &tok);
    
    if (strncmp(lexeme, "if", 2) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'if'.");
        free(lexeme);
        return if_stmt;
    }

    free(lexeme);

    // parse conditional expr and then if block!
    parser_advance(parser);

    Expression *cond = parse_expr(parser);

    if (!cond)
    {
        // reject malformed if stmts since condition is needed!
        parser_log_err(parser, tok.line, "Expected conditions.");
        destroy_stmt(if_stmt);
        free(if_stmt);
        if_stmt = NULL;

        return if_stmt;
    }

    if_stmt = create_if_stmt(cond, NULL, NULL);

    first_block = parse_block_stmt(parser);

    if (!first_block)
    {
        // reject malformed if stmts since if block is needed!
        parser_log_err(parser, tok.line, "Could not find stmt block.");
        destroy_stmt(if_stmt);
        free(if_stmt);
        if_stmt = NULL;

        return if_stmt;
    }

    if_stmt->syntax.if_stmt.first = first_block;
    if_stmt->syntax.if_stmt.other = parse_otherwise_stmt(parser);;

    return if_stmt;
}

Statement *parse_while_stmt(Parser *parser)
{
    puts("parse_while_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = NULL;
    Statement *while_stmt = NULL;
    Statement *loop_block = NULL;

    // validate starting keyword token
    if (tok.type != KEYWORD)
        return while_stmt;
    
    lexeme = parser_stringify_token(parser, &tok);
    
    if (strncmp(lexeme, "while", 5) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'while'.");
        free(lexeme);
        return while_stmt;
    }

    free(lexeme);

    // parse loop conditional
    parser_advance(parser);
    while_stmt = create_while_stmt(NULL, NULL);

    while_stmt->syntax.while_stmt.condition = parse_expr(parser);
    loop_block = parse_block_stmt(parser);

    if (!loop_block)
    {
        tok = parser_peek_curr(parser);
        parser_log_err(parser, tok.line, "Could not parse loop block.");
        destroy_stmt(while_stmt);
        free(while_stmt);
        while_stmt = NULL;

        return while_stmt;
    }

    while_stmt->syntax.while_stmt.stmts = loop_block;

    return while_stmt;
}

Statement *parse_return_stmt(Parser *parser)
{
    puts("parse_return_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = parser_stringify_token(parser, &tok);
    Expression *result_expr = NULL;
    Statement *ret_stmt = NULL;

    // validate starting "return" token
    if (tok.type != KEYWORD && strncmp(lexeme, "return", 6) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'return'.");
        free(lexeme);
        return ret_stmt;
    }

    free(lexeme);

    // parse result expr
    parser_advance(parser);

    result_expr = parse_expr(parser);

    if (!result_expr)
    {
        parser_log_err(parser, tok.line, "Could not find expression.");
        return ret_stmt;
    }

    ret_stmt = create_return_stmt(result_expr);

    return ret_stmt;
}

Statement *parse_block_stmt(Parser *parser)
{
    puts("parse_block_stmt");
    Token checked_tok;
    char *lexeme = NULL;
    Statement *temp_stmt = NULL;
    Statement *block_stmt = create_block_stmt();

    while (!parser_at_end(parser))
    {
        checked_tok = parser_peek_curr(parser);
        lexeme = parser_stringify_token(parser, &checked_tok);

        printf("parsing stmt with begin: \"%s\"\n", lexeme);

        if (strncmp(lexeme, "while", 5) == 0)
            temp_stmt = parse_while_stmt(parser);
        else if (strncmp(lexeme, "if", 2) == 0)
            temp_stmt = parse_if_stmt(parser);
        else if (strncmp(lexeme, "end", 3) == 0)
        {
            free(lexeme); // discard "end" token as it's just a block marker
            parser_advance(parser);
            break;
        }
        else if (strncmp(lexeme, "return", 6) == 0)
            temp_stmt = parse_return_stmt(parser);
        else if (strncmp(lexeme, "let", 3) == 0 || strncmp(lexeme, "const", 5) == 0)
            temp_stmt = parse_var_decl(parser);
        else
            temp_stmt = parse_expr_stmt(parser);

        free(lexeme);

        if (!grow_block_stmt(block_stmt, temp_stmt))
        {
            parser_log_err(parser, checked_tok.line, "Could not construct stmt block by bad alloc.");
            destroy_stmt(block_stmt);
            free(block_stmt);
            block_stmt = NULL;

            break;
        }
    }

    return block_stmt;
}

Statement *parse_func_stmt(Parser *parser)
{
    puts("parse_func_stmt");
    int bad_syntax = 0;
    int comma_expected = 0;
    Token tok = parser_peek_curr(parser);
    char *lexeme = NULL;
    Expression *param_expr = NULL;
    Statement *fn_stmt = NULL;

    // check starting keyword "proc"
    if (tok.type != KEYWORD)
        return fn_stmt;

    puts("parse func begin");
    lexeme = parser_stringify_token(parser, &tok);

    if (strncmp(lexeme, "proc", 4) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'proc'.");
        free(lexeme);
        return fn_stmt;
    }

    // parse identifier: check parse order of ident, lparen, etc!
    parser_advance(parser);
    tok = parser_peek_curr(parser);
    lexeme = parser_stringify_token(parser, &tok);
    fn_stmt = create_func_stmt(lexeme, NULL);
    lexeme = NULL;

    // parse args
    parser_advance(parser);
    tok = parser_peek_curr(parser);

    if (tok.span != 1 && parser->src_copy_ptr[tok.begin] != '(')
        return fn_stmt;

    parser_advance(parser);

    puts("parsing func args");
    while (!parser_at_end(parser))
    {
        tok = parser_peek_curr(parser);

        if (tok.type == RPAREN)
        {
            puts("end args");
            parser_advance(parser);
            break;
        }

        if (tok.type == COMMA && !comma_expected)
        {
            bad_syntax = 1;
            break;
        }
        else if (tok.type == COMMA)
        {
            comma_expected = 0;
            parser_advance(parser);
        }
        else
        {
            param_expr = parse_literal(parser);
            comma_expected = 1;
        }

        if (param_expr->type != VAR_USAGE)
        {
            bad_syntax = 1;
            break;
        }

        put_arg_func_stmt(fn_stmt, param_expr);
    }

    if (bad_syntax)
    {
        parser_log_err(parser, tok.line, "Unexpected comma.");
        destroy_stmt(fn_stmt);
        free(fn_stmt);
        fn_stmt = NULL;

        return fn_stmt;
    }

    // parse function body
    puts("parsing func body");
    Statement *fn_body = parse_block_stmt(parser);

    if (!fn_body)
    {
        parser_log_err(parser, tok.line, "Failed to find function body.");
        destroy_stmt(fn_stmt);
        free(fn_stmt);
        fn_stmt = NULL;

        return fn_stmt;
    }

    fn_stmt->syntax.func_decl.stmts = fn_body;

    return fn_stmt;
}

Statement *parse_module_stmt(Parser *parser)
{
    puts("parse_module_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = parser_stringify_token(parser, &tok);
    Statement *use_stmt = NULL;

    if (tok.type != KEYWORD || strncmp(lexeme, "module", 6) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'module'.");
        free(lexeme);
        return use_stmt;
    }

    free(lexeme);

    // check identifier
    parser_advance(parser);
    tok = parser_peek_curr(parser);
    lexeme = parser_stringify_token(parser, &tok);
    use_stmt = create_module_def(lexeme);
    lexeme = NULL;

    parser_advance(parser);

    return use_stmt;
}

Statement *parse_use_stmt(Parser *parser)
{
    puts("parse_use_stmt");
    Token tok = parser_peek_curr(parser);
    char *lexeme = parser_stringify_token(parser, &tok);
    Statement *use_stmt = NULL;

    if (tok.type != KEYWORD || strncmp(lexeme, "use", 3) != 0)
    {
        parser_log_err(parser, tok.line, "Expected 'use'.");
        free(lexeme);
        return use_stmt;
    }

    free(lexeme);

    // check identifier
    parser_advance(parser);
    tok = parser_peek_curr(parser);
    lexeme = parser_stringify_token(parser, &tok);
    use_stmt = create_module_usage(lexeme);
    lexeme = NULL;

    parser_advance(parser);

    return use_stmt;
}

Statement *parse_expr_stmt(Parser *parser)
{
    puts("parse_expr_stmt");
    Expression *inner_expr = parse_expr(parser);

    if (!inner_expr)
        return NULL;

    puts("finished expr stmt");
    return create_expr_stmt(inner_expr);
}

Statement *parse_stmt(Parser *parser)
{
    Token tok = parser_peek_curr(parser);
    char *lexeme = parser_stringify_token(parser, &tok);
    Statement *stmt = NULL;

    if (tok.type == IDENTIFIER)
        stmt = parse_expr_stmt(parser);
    else if (strncmp(lexeme, "use", 3) == 0)
        stmt = parse_use_stmt(parser);
    else if (strncmp(lexeme, "module", 6) == 0)
        stmt = parse_module_stmt(parser);
    else if (strncmp(lexeme, "proc", 4) == 0)
        stmt = parse_func_stmt(parser);
    else if (strncmp(lexeme, "let", 3) == 0 || strncmp(lexeme, "const", 5) == 0)
        stmt = parse_var_decl(parser);
    else;

    free(lexeme);

    return stmt;
}

Script *parser_parse_all(Parser *parser, const char *script_name)
{
    if (!parser->ready_flag)
        return NULL;

    int done_flag = 0;
    Script *program = malloc(sizeof(Script));
    init_script(program, script_name, 4);
    Statement *temp = NULL;

    while (!done_flag)
    {
        temp = parse_stmt(parser);
        done_flag = parser_at_end(parser) == 1;

        if (!temp && !done_flag)
        {
            dispose_script(program);
            free(program);
            program = NULL;
            break;
        }

        grow_script(program, temp);
    }

    parser->src_copy_ptr = NULL; // NOTE: unbind source string when done!

    return program;
}
