#ifndef AST_H
#define AST_H

/**
 * @file ast.h
 * @author Derek Tan
 * @brief Parts are enums, Expression, Statement, and Script.
 */

typedef enum
{
    MODULE_DEF,
    MODULE_USE,
    FUNC_CALL,
    VAR_DECL,
    FUNC_DECL,
    WHILE_STMT,
    IF_STMT,
    OTHERWISE_STMT,
    BREAK_STMT,
    RETURN_STMT,
    EXPR_STMT
} StatementType;

typedef enum
{
    OP_EQ,
    OP_NEQ,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_INDEX
} OpType;

typedef enum
{
    BOOL_LITERAL,
    INT_LITERAL,
    REAL_LITERAL,
    STR_LITERAL,
    LIST_LITERAL,
    VAR_USAGE,
    UNARY_OP,
    BINARY_OP
} ExpressionType;

typedef struct st_expression
{
    ExpressionType type;
    union
    {
        struct
        {
            int flag;
        } bool_literal;

        struct
        {
            int value;
        } int_literal;

        struct
        {
            float value;
        } real_literal;

        struct
        {
            char *content;
        } str_literal;

        struct
        {
            void *list_obj;
        } list_literal;

        struct
        {
            int is_lvalue;
            char *var_name;
        } var_name;

        struct
        {
            OpType op;
            struct st_expression *expr;
        } unary_op;

        struct
        {
            OpType op;
            struct st_expression *left;
            struct st_expression *right;
        } binary_op;
    } syntax;
} Expression;

Expression *create_bool(int flag);

Expression *create_int();

Expression *create_real();

Expression *create_str();

Expression *create_list(void *list_val);

Expression *create_var(int is_lvalue, char *name);

Expression *create_unary(OpType op, Expression *expr);

Expression *create_binary(OpType op, Expression *left, Expression *right);

typedef struct st_statement
{
    StatementType type;
    union
    {
        struct
        {
            struct st_statement **stmts;
            unsigned int count;
        } block;

        struct
        {
            char *module_name;
        } module_def;
        
        struct
        {
            char *module_name;
        } module_usage;

        struct
        {
            char *func_name;
            struct st_expression **args;
            unsigned int argc;
        } func_call;

        struct
        {
            int is_const;
            char *var_name;
            struct st_expression *rvalue;
        } var_decl;

        struct
        {
            char *func_name;
            struct st_expression *func_args;
            struct block *stmts;
        } func_decl;

        struct
        {
            struct st_expression *condition;
            struct block *stmts;
        } while_stmt;

        struct
        {
            struct st_expression *condition;
            struct block *stmts;
        } if_stmt;
        
        struct
        {
            struct block *stmts;
        } otherwise_stmt;

        struct
        {
            int depth;
        } break_stmt;

        struct
        {
            struct st_expression *result;
        } return_stmt;

        struct
        {
            struct st_expression *expr;
        } expr_stmt;
    } syntax;
} Statement;

Statement *create_block_stmt();

void grow_block_stmt(Statement *stmt);

Statement *create_module_def(char *name);

Statement *create_module_usage(char *name);

Statement *create_func_call(char *fn_name, Expression *args, unsigned int argc);

Statement *create_var_decl(char *var_name, Expression *rvalue);

Statement *create_func_stmt(char *fn_name, Statement *block);

Statement *create_while_stmt(Expression *conditional, Statement *block);

Statement *create_if_stmt(Expression *conditional, Statement *block);

Statement *create_otherwise_stmt(Statement *block);

Statement *create_break_stmt(int depth);

Statement *create_return_stmt(Expression *result);

Statement *create_expr_stmt(Expression *expr);

void stmt_destroy(Statement *stmt);

typedef struct st_script
{
    char *name;
    unsigned int count;
    Statement **stmts;
} Script;

void init_script(Script *script, size_t old_count);

void dispose_script(Script *script);

#endif