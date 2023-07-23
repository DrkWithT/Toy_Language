#ifndef AST_H
#define AST_H

typedef enum
{
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
    BINARY_OP
} ExpressionType;

typedef struct st_statement
{
    StatementType type;
    union
    {
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
        } var_decl;

        struct
        {
            char *func_name;
            struct st_expression *func_args;
            struct st_statement **func_body;
            unsigned int count;
        } func_decl;

        struct
        {
            struct st_expression *condition;
            struct st_statement **stmt_block;
            unsigned int count;
        } while_stmt;

        struct
        {
            struct st_expression *condition;
            struct st_statement **stmt_block;
            unsigned int count;
        } if_stmt;
        
        struct
        {
            struct st_statement **stmt_block;
            unsigned int count;
        } otherwise_stmt;

        struct
        {
            char *keyword;
        } break_stmt;

        struct
        {
            char *keyword;
        } ret_stmt;

        struct
        {
            struct st_expression *expr;
        } expr_stmt;
    } syntax;
} Statement;

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
            struct st_expression *left;
            struct st_expression *right;
        } binary_op;
    } syntax;
} Expression;

typedef struct st_script
{
    char *name;
    unsigned int count;
    Statement **stmts;
} Script;

#endif