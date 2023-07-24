#ifndef AST_H
#define AST_H

/**
 * @file ast.h
 * @author Derek Tan
 * @brief Parts are enums, Expression, Statement, and Script. 1 is the true success return value.
 */

#include <stdlib.h>

typedef enum
{
    MODULE_DEF,
    MODULE_USE,
    EXPR_STMT,
    VAR_DECL,
    BLOCK_STMT,
    FUNC_DECL,
    WHILE_STMT,
    IF_STMT,
    OTHERWISE_STMT,
    BREAK_STMT,
    RETURN_STMT
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
    FUNC_CALL,
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
            void *str_obj;
        } str_literal;

        struct
        {
            void *list_obj;
        } list_literal;

        struct
        {
            int is_lvalue;
            char *var_name;
        } variable;

        struct
        {
            char *func_name;
            unsigned int cap;
            unsigned int argc;
            struct st_expression **args;
        } fn_call;

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

Expression *create_int(int val);

Expression *create_real(float val);

Expression *create_str(void *str_obj);

Expression *create_list(void *list_val);

Expression *create_var(int is_lvalue, char *name);

Expression *create_call(char *fn_name);

void add_arg_call(Expression *call_expr, Expression *arg_expr);

/**
 * @brief Downsizes the memory taken by this Expression. Arg vector capacity is shrunk to its count.
 * @param call_expr
 */
void pack_mem_call(Expression *call_expr);

/**
 * @brief Clears the internal Expression vector. Names are unbound instead, as they are managed by the interpreter context.
 * @param call_expr 
 */
int clear_mem_call(Expression *call_expr);

Expression *create_unary(OpType op, Expression *expr);

Expression *create_binary(OpType op, Expression *left, Expression *right);

/**
 * @brief Deallocates wrapper Expressions, but names are unbound instead since they are managed by the interpreter context.
 * @param expr 
 */
void destroy_expr(Expression *expr);

typedef struct st_statement
{
    StatementType type;
    union
    {
        struct
        {
            struct st_statement **stmts;
            unsigned int capacity;
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
            int is_const;
            char *var_name;
            struct st_expression *rvalue;
        } var_decl;

        struct
        {
            char *func_name;
            unsigned int cap;
            unsigned int argc;
            struct st_expression **func_args;
            struct st_statement *stmts;
        } func_decl;

        struct
        {
            struct st_expression *condition;
            struct st_statement *stmts;
        } while_stmt;

        struct
        {
            struct st_expression *condition;
            struct st_statement *stmts;
        } if_stmt;
        
        struct
        {
            struct st_statement *stmts;
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

/**
 * @brief Resizes the internal statement ptr array like a vector when count reaches capacity. Capacity is then doubled.
 * @param stmt
 */
int grow_block_stmt(Statement *block_stmt);

int pack_block_stmt(Statement *block_stmt);

int clear_block_stmt(Statement *block_stmt);

Statement *create_module_def(char *name);

Statement *create_module_usage(char *name);

Statement *create_var_decl(int is_const, char *var_name, Expression *rvalue);

Statement *create_func_stmt(char *fn_name, Statement *block);

int put_arg_func_stmt(Statement *fn_decl, Expression *arg_expr);

int pack_args_func_stmt(Statement *fn_decl);

int clear_func_stmt(Statement *fn_decl);

Statement *create_while_stmt(Expression *conditional, Statement *block);

Statement *create_if_stmt(Expression *conditional, Statement *block);

Statement *create_otherwise_stmt(Statement *block);

Statement *create_break_stmt(int depth);

Statement *create_return_stmt(Expression *result);

Statement *create_expr_stmt(Expression *expr);

/**
 * @brief Unbinds any dynamic data pointer within a Statement structure since the interpreter context stores name-data mappings.
 * 
 * @param stmt 
 */
void destroy_stmt(Statement *stmt);

typedef struct st_script
{
    char *name;
    unsigned int capacity;
    unsigned int count;
    Statement **stmts;
} Script;

void init_script(Script *script, unsigned int old_count);

void grow_script(Script *script, Statement *stmt_obj);

/**
 * @brief Unbinds any dynamic memory ptrs since names to entity mappings are managed by the interpreter context. Wrappers get deallocated on the outer level.
 * @note block statements recursively have their entity ptrs unbound before deallocation.
 * @param script
 */
void dispose_script(Script *script);

#endif