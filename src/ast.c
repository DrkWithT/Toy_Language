/**
 * @file ast.c
 * @author Derek Tan
 * @brief Implements AST structures and functions.
 * @date 2023-07-23
 * @todo Modify AST node destroy_xx functions when interpreter is done: the str_obj and list_obj properties must be unbound as they are managed by Scope objects.
 */

#include "frontend/ast.h"

/// SECTION: Expression funcs

Expression *create_bool(int flag)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = BOOL_LITERAL;
        expr->syntax.bool_literal.flag = flag;
    }

    return expr;
}

Expression *create_int(int val)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = INT_LITERAL;
        expr->syntax.int_literal.value = val;
    }

    return expr;
}

Expression *create_real(float val)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = REAL_LITERAL;
        expr->syntax.real_literal.value = val;
    }

    return expr;
}

Expression *create_str(StringObj *str_obj)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = STR_LITERAL;
        expr->syntax.str_literal.str_obj = str_obj;
    }

    return expr;
}

Expression *create_list(ListObj *list_val)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = LIST_LITERAL;
        expr->syntax.list_literal.list_obj = list_val;
    }

    return expr;
}

Expression *create_var(int is_lvalue, char *name)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = VAR_USAGE;
        expr->syntax.variable.is_lvalue = is_lvalue;
        expr->syntax.variable.var_name = name;
    }

    return expr;
}

Expression *create_call(char *fn_name)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = FUNC_CALL;
        expr->syntax.fn_call.func_name = fn_name;
        expr->syntax.fn_call.argc = 0;
        expr->syntax.fn_call.cap = 4;
        expr->syntax.fn_call.args = malloc(sizeof(Expression *) * 4);

        if (!expr->syntax.fn_call.args)
            expr->syntax.fn_call.cap = 0;
    }

    return expr;
}

int add_arg_call(Expression *call_expr, Expression *arg_expr)
{
    size_t old_capacity = call_expr->syntax.fn_call.cap;
    size_t new_capacity = old_capacity << 1;

    Expression **raw_block = realloc(call_expr->syntax.fn_call.args, sizeof(Expression *) * new_capacity);

    if (raw_block != NULL)
    {
        for (size_t i = old_capacity; i < new_capacity; i++)
            raw_block[i] = NULL;
        
        call_expr->syntax.fn_call.args = raw_block;
        return 1;
    }

    return 0;
}

int pack_mem_call(Expression *call_expr)
{
    size_t curr_capacity = call_expr->syntax.fn_call.cap;
    size_t curr_count = call_expr->syntax.fn_call.argc;

    if (curr_capacity <= curr_count) return 1;

    Expression **raw_args = realloc(call_expr->syntax.fn_call.args, sizeof(Expression *) * curr_count);

    if (raw_args != NULL)
    {
        call_expr->syntax.fn_call.args = raw_args;
        return 1;
    }

    return 0;
}

int clear_mem_call(Expression *call_expr)
{
    size_t target_count = call_expr->syntax.fn_call.argc;
    Expression **args_cursor = call_expr->syntax.fn_call.args;
    Expression *target = NULL;

    if (!args_cursor)
        return 0;

    for (size_t i = 0; i < target_count; i++)
    {
        target = *args_cursor;

        if (target != NULL)
        {
            destroy_expr(target);
            free(target);
            target = NULL;
        }

        args_cursor++;
    }

    free(call_expr->syntax.fn_call.args);
    call_expr->syntax.fn_call.func_name = NULL;

    return 1;
}

Expression *create_unary(OpType op, Expression *expr)
{
    Expression *unary_expr = malloc(sizeof(Expression));

    if (unary_expr != NULL)
    {
        unary_expr->type = UNARY_OP;
        unary_expr->syntax.unary_op.op = op;
        unary_expr->syntax.unary_op.expr = expr;
    }

    return unary_expr;
}

Expression *create_binary(OpType op, Expression *left, Expression *right)
{
    Expression *expr = malloc(sizeof(Expression));

    if (expr != NULL)
    {
        expr->type = BINARY_OP;
        expr->syntax.binary_op.op = op;
        expr->syntax.binary_op.left = left;
        expr->syntax.binary_op.right = right;
    }

    return expr;
}

void destroy_expr(Expression *expr)
{
    if (expr->type == STR_LITERAL)
    {
        // TODO: remove destroy-free when interpreter is done.
        destroy_str_obj(expr->syntax.str_literal.str_obj);
        free(expr->syntax.str_literal.str_obj);
        expr->syntax.str_literal.str_obj = NULL;
    }
    else if (expr->type == LIST_LITERAL)
    {
        // TODO: remove this destroy-free too on interpreter finish.
        destroy_list_obj(expr->syntax.list_literal.list_obj);
        free(expr->syntax.list_literal.list_obj);
        expr->syntax.list_literal.list_obj = NULL;
    }
    else if (expr->type == VAR_USAGE)
    {
        // NOTE: free var name since only its hash is needed for running.
        free(expr->syntax.variable.var_name);
        expr->syntax.variable.var_name = NULL;
    }
    else if (expr->type == FUNC_CALL)
    {
        Expression **cursor = expr->syntax.fn_call.args;
        Expression *temp = NULL;
        size_t target_count = expr->syntax.fn_call.argc;

        for (size_t i = 0; i < target_count; i++)
        {
            temp = *cursor;

            if (temp != NULL)
            {
                destroy_expr(temp);
                temp = NULL;
            }

            cursor++;
        }

        free(cursor);
        expr->syntax.fn_call.args = NULL;

        // NOTE: free func name since only its hash is needed for running.
        free(expr->syntax.fn_call.func_name);
        expr->syntax.fn_call.func_name = NULL;
    }
    else if (expr->type == UNARY_OP)
    {
        destroy_expr(expr->syntax.unary_op.expr);
    }
    else if (expr->type == BINARY_OP)
    {
        destroy_expr(expr->syntax.binary_op.left);
        destroy_expr(expr->syntax.binary_op.right);
    }
}

/// SECTION: Statement funcs

Statement *create_block_stmt()
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = BLOCK_STMT;
        stmt->syntax.block.capacity = 4;
        stmt->syntax.block.count = 0;
        stmt->syntax.block.stmts = malloc(sizeof(Statement *) * 4);

        if (!stmt->syntax.block.stmts) stmt->syntax.block.capacity = 0; // NOTE: do not resize invalid vector memory.
    }

    return stmt;
}

int grow_block_stmt(Statement *block_stmt, Statement *new_stmt)
{
    if (!new_stmt)
        return 0;

    size_t curr_count = block_stmt->syntax.block.count;
    size_t old_capacity = block_stmt->syntax.block.capacity;
    size_t new_capacity = old_capacity << 1;

    if (curr_count < old_capacity)
    {
        block_stmt->syntax.block.stmts[curr_count - 1] = new_stmt;
        block_stmt->syntax.block.count++;
        return 1;
    }

    Statement **raw_block = realloc(block_stmt->syntax.block.stmts, sizeof(Statement *) * new_capacity);

    if (raw_block != NULL)
    {
        for (size_t i = old_capacity; i < new_capacity; i++)
            raw_block[i] = NULL;
        
        block_stmt->syntax.block.stmts = raw_block;
        block_stmt->syntax.block.stmts[old_capacity] = new_stmt;
        
        block_stmt->syntax.block.count++;
        return 1;
    }

    return 0;
}

int pack_block_stmt(Statement *block_stmt)
{
    size_t curr_capacity = block_stmt->syntax.block.capacity;
    size_t curr_count = block_stmt->syntax.block.count;

    if (curr_capacity <= curr_count) return 1;

    Statement **raw_block = realloc(block_stmt->syntax.block.stmts, sizeof(Statement *) * curr_count);

    if (raw_block != NULL)
    {
        block_stmt->syntax.block.stmts = raw_block;
        return 1;
    }

    return 0;
}

int clear_block_stmt(Statement *block_stmt)
{
    size_t target_count = block_stmt->syntax.block.count;
    Statement **raw_block = block_stmt->syntax.block.stmts;  // cursor for array of Statment ptrs.
    Statement *target = NULL;

    if (!raw_block)
        return 0;

    for (size_t i = 0; i < target_count; i++)
    {
        target = *raw_block;

        if (target != NULL)
        {
            destroy_stmt(target);
            free(target);
            target = NULL;
        }

        raw_block++;
    }

    return 1;
}

Statement *create_module_def(char *name)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = MODULE_DEF;
        stmt->syntax.module_def.module_name = name;
    }

    return stmt;
}

Statement *create_module_usage(char *name)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = MODULE_USE;
        stmt->syntax.module_usage.module_name = name;
    }

    return stmt;
}

Statement *create_var_decl(int is_const, char *var_name, Expression *rvalue)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = VAR_DECL;
        stmt->syntax.var_decl.is_const = is_const;
        stmt->syntax.var_decl.var_name = var_name;
        stmt->syntax.var_decl.rvalue = rvalue;
    }

    return stmt;
}

Statement *create_func_stmt(char *fn_name, Statement *block)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = FUNC_DECL;
        stmt->syntax.func_decl.argc = 0;
        stmt->syntax.func_decl.cap = 4;
        stmt->syntax.func_decl.func_args = malloc(sizeof(Expression *) * 4);
        stmt->syntax.func_decl.stmts = block;

        if (!stmt->syntax.func_decl.func_args)
            stmt->syntax.func_decl.cap = 0;  // NOTE: mark bad memory as empty!
    }

    return stmt;
}

int put_arg_func_stmt(Statement *fn_decl, Expression *arg_expr)
{
    size_t old_capacity = fn_decl->syntax.func_decl.cap;
    size_t new_capacity = old_capacity << 1;

    Expression **raw_args = realloc(fn_decl->syntax.func_decl.func_args, sizeof(Expression *) * new_capacity);

    if (raw_args != NULL)
    {
        for (size_t i = old_capacity; i < new_capacity; i++)
            raw_args[i] = NULL;

        fn_decl->syntax.func_decl.func_args = raw_args;

        return 1;
    }

    return 0;
}

int pack_args_func_stmt(Statement *fn_decl)
{
    size_t curr_capacity = fn_decl->syntax.func_decl.cap;
    size_t curr_count = fn_decl->syntax.func_decl.argc;

    if (curr_capacity <= curr_count) return 1;

    Expression **raw_args = realloc(fn_decl->syntax.func_decl.func_args, sizeof(Expression *) * curr_count);

    if (raw_args != NULL)
    {
        fn_decl->syntax.func_decl.func_args = raw_args;
        return 1;
    }

    return 0;
}

void clear_func_stmt(Statement *fn_decl)
{
    // 1. Free memory for argument objects.
    Expression **args_cursor = fn_decl->syntax.func_decl.func_args;
    Expression *target = NULL;
    size_t arg_count = fn_decl->syntax.func_decl.argc;

    for (size_t i = 0; i < arg_count; i++)
    {
        target = *args_cursor;

        if (target != NULL)
        {
            destroy_expr(target);
            free(target);
            target = NULL;
        }

        args_cursor++;
    }

    // 2. Free memory for statement objects.
    destroy_stmt(fn_decl->syntax.func_decl.stmts);
}

Statement *create_while_stmt(Expression *conditional, Statement *block)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = WHILE_STMT;
        stmt->syntax.while_stmt.condition = conditional;
        stmt->syntax.while_stmt.stmts = block;
    }

    return stmt;
}

Statement *create_if_stmt(Expression *conditional, Statement *first, Statement *other)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = IF_STMT;
        stmt->syntax.if_stmt.condition = conditional;
        stmt->syntax.if_stmt.first = first;
        stmt->syntax.if_stmt.other = other;
    }

    return stmt;
}

Statement *create_otherwise_stmt(Statement *block)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = OTHERWISE_STMT;
        stmt->syntax.otherwise_stmt.stmts = block;
    }

    return stmt;
}

Statement *create_break_stmt(int depth)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = BREAK_STMT;
        stmt->syntax.break_stmt.depth = depth;
    }
    
    return stmt;
}

Statement *create_return_stmt(Expression *result)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = RETURN_STMT;
        stmt->syntax.return_stmt.result = result;
    }

    return stmt;
}

Statement *create_expr_stmt(Expression *expr)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (stmt != NULL)
    {
        stmt->type = EXPR_STMT;
        stmt->syntax.expr_stmt.expr = expr;
    }

    return stmt;
}

/// TODO: implement destroy_stmt!
void destroy_stmt(Statement *stmt)
{
    if (stmt->type == BLOCK_STMT)
    {
        clear_block_stmt(stmt);
    }
    else if (stmt->type == FUNC_DECL)
    {
        clear_block_stmt(stmt->syntax.func_decl.stmts);
        stmt->syntax.func_decl.func_name = NULL;
    }
    else if (stmt->type == WHILE_STMT)
    {
        destroy_expr(stmt->syntax.while_stmt.condition);
        clear_block_stmt(stmt->syntax.while_stmt.stmts);
    }
    else if (stmt->type == IF_STMT)
    {
        destroy_expr(stmt->syntax.if_stmt.condition);
        clear_block_stmt(stmt->syntax.if_stmt.first);
        clear_block_stmt(stmt->syntax.if_stmt.other);
    }
    else if (stmt->type == OTHERWISE_STMT)
    {
        clear_block_stmt(stmt->syntax.otherwise_stmt.stmts);
    }
    else if (stmt->type == RETURN_STMT)
    {
        destroy_expr(stmt->syntax.return_stmt.result);
    }
}

/// SECTION: Script

void init_script(Script *script, const char *name, unsigned int old_count)
{
    script->name = name;
    script->count = old_count;
    script->stmts = malloc(sizeof(Statement*) * old_count);

    if (!script->stmts) script->capacity = 0; // NOTE: do not resize invalid vector memory.
}

void grow_script(Script *script, Statement *stmt_obj)
{
    size_t curr_count = script->count;
    size_t old_capacity = script->capacity;
    size_t new_capacity = old_capacity << 1;

    if (curr_count < old_capacity)
    {
        script->stmts[curr_count - 1] = stmt_obj;
        script->count++;
        return;
    }

    Statement **raw_block = realloc(script->stmts, sizeof(Statement *) * new_capacity);

    if (raw_block != NULL)
    {
        for (size_t i = old_capacity; i < new_capacity; i++)
            raw_block[i] = NULL;
        
        script->stmts = raw_block;
        script->stmts[old_capacity] = stmt_obj;
        
        script->capacity++;
    }
}

void dispose_script(Script *script)
{
    if (!script)
        return;
    
    size_t stmt_count = script->count;
    Statement *target = NULL;
    Statement **cursor = script->stmts;

    for (size_t i = 0; i < stmt_count; i++)
    {
        target = *cursor;

        if (target != NULL)
        {
            destroy_stmt(target);
            free(target);
            target = NULL;
        }

        cursor++;
    }

    // NOTE: unbind script file name since it's a static c-string passed by argv!
    script->name = NULL;
}
