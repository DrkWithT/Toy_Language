/**
 * @file runctx.c
 * @author Derek Tan
 * @brief Implements RunnerContext for interpreter state.
 * @date 2023-08-02
 * @todo Add module resolution like io.print(...) to avoid func redefinitions?
 */

#include "backend/runner/runctx.h"

/// SECTION: Context utils

int ctx_init(RunnerContext *ctx, Script *program)
{
    ctx_set_status(ctx, OK_IDLE);
    FuncEnv *script_fenv = funcenv_create(4);
    int flag_success = 0;
    int fenv_ok, global_scope_ok;
    
    if (!program || !script_fenv) return 0;

    if (!scopestack_init(&ctx->scopes, SCOPE_STACK_SIZE))
    {
        funcenv_dispose(script_fenv);
        free(script_fenv);
        ctx_set_status(ctx, ERR_MEMORY);
        return flag_success;
    }

    RubelScope *script_scope = scope_create(NULL);
    
    if(!script_scope)
    {
        funcenv_dispose(script_fenv);
        free(script_fenv);
        ctx_set_status(ctx, ERR_MEMORY);
        return flag_success;
    }

    FuncGroup *script_funcs = funcgroup_create(NULL, program->count);

    if (!script_funcs)
    {
        funcenv_dispose(script_fenv);
        free(script_fenv);
        scopestack_destroy(&ctx->scopes);
        ctx_set_status(ctx, ERR_MEMORY);
        return flag_success;
    }

    // NOTE: global (no-name) module is always "used"!
    funcgroup_mark_used(script_funcs, 1);

    fenv_ok = funcenv_append(script_fenv, script_funcs);
    ctx->function_env = script_fenv;
    global_scope_ok = scopestack_push_scope(&ctx->scopes, script_scope);
    flag_success = fenv_ok && global_scope_ok;

    return flag_success;
}

void ctx_destroy(RunnerContext *ctx)
{
    funcenv_dispose(ctx->function_env);
    free(ctx->function_env);
    ctx->function_env = NULL;

    scopestack_destroy(&ctx->scopes);
    ctx_set_status(ctx, OK_ENDED);
}

void ctx_set_status(RunnerContext *ctx, RunStatus status)
{
    ctx->status = status;
}

int ctx_load_funcgroup(RunnerContext *ctx, FuncGroup *module)
{
    if (!module) return 0;

    funcgroup_mark_used(module, 1);

    return funcenv_append(ctx->function_env, module);
}

/// SECTION: function helpers

const FuncObj *ctx_get_func(const RunnerContext *ctx, const char *fn_name)
{
    if (!fn_name) return NULL;

    FuncGroup **fenv_cursor = ctx->function_env->func_groups;
    FuncGroup *module_ref = NULL;
    const FuncObj *result_fn = NULL;
    size_t fenv_len = ctx->function_env->count;

    // NOTE: brute force search for function decl... poor Big-O (n^2?) but will work for uniquely named ones for now.
    for (size_t i = 0; i < fenv_len; i++)
    {
        module_ref = *fenv_cursor;

        if (!funcgroup_is_used(module_ref)) break; // NOTE: this should ignore unused function modules.

        result_fn = funcgroup_get(module_ref, fn_name);

        if (result_fn != NULL)
        {
            if (strcmp(result_fn->name, fn_name) == 0) return result_fn;
        }

        fenv_cursor++;
    }

    return NULL;
}

VarValue *ctx_call_func(RunnerContext *ctx, unsigned short argc, const char *fn_name, FuncArgs *args)
{
    if (!fn_name)
    {
        ctx_set_status(ctx, ERR_NULL_VAL);
        return NULL;
    }

    // prepare and check callee first!
    const FuncObj *callee_ref = ctx_get_func(ctx, fn_name);
    VarValue *result = NULL;

    // reject unknown callees in the context!
    if (!callee_ref)
    {
        ctx_set_status(ctx, ERR_NULL_VAL);
        return result;
    }

    FuncType callee_type = callee_ref->type;
    unsigned short callee_argc = callee_ref->arity;

    // reject wrong argument array length since the function decl cannot match it!
    if (callee_argc != argc)
    {
        ctx_set_status(ctx, ERR_NO_IMPL);
        return result;
    }

    // NOTE: check for native function to avoid making an interpreter scope for it since ISA/machine-code handles native vars scope!
    if (callee_type == FUNC_NATIVE)
    {
        result = callee_ref->content.fn_ptr(args);

        funcargs_destroy(args); // NOTE: still, free up args!
        free(args);

        return result;
    }

    // push current call scope to the scope stack for tracking!
    RubelScope *parent_scope = ctx->scopes.scopes[ctx->scopes.stack_ptr];
    RubelScope *call_scope = scope_create(parent_scope);

    if (!call_scope)
    {
        ctx_set_status(ctx, ERR_MEMORY);
        return result;
    }

    // NOTE: populate scope with parameters before pushing it to stack for tracking!
    for (unsigned short i = 0; i < argc; i++)
    {
        Expression *fn_decl_param = callee_ref->param_exprs[i];
        scope_put_var(call_scope, variable_create(fn_decl_param->syntax.variable.var_name, args->args[i]));
    }

    if (!scopestack_push_scope(&ctx->scopes, call_scope))
    {
        // NOTE: check scope stack "fullness" to prevent excessive recursion?
        scope_destroy(call_scope);
        free(call_scope);

        ctx_set_status(ctx, ERR_GENERAL);
        return result;
    }

    // NOTE: here, the function will be non-native, so we can run it with interpreter scope!
    result = exec_block(ctx, callee_ref->content.fn_ast);

    // NOTE: dispose & free args first...
    funcargs_dispose(args);
    free(args);

    // NOTE: destroy call entry in scope stack for cleanup!
    call_scope = scopestack_pop_scope(&ctx->scopes);
    scope_destroy(call_scope);
    free(call_scope);
    ctx_set_status(ctx, OK_RAN_CMD);

    return result;
}

/// SECTION: Variable helpers

Variable *ctx_get_var(const RunnerContext *ctx, const char *var_name)
{
    RubelScope *curr_scope = ctx->scopes.scopes[ctx->scopes.stack_ptr];
    Variable *temp_var_ref = NULL;

    while (curr_scope->parent != NULL)
    {
        temp_var_ref = scope_get_var_ref(curr_scope, var_name);

        if (temp_var_ref != NULL) return temp_var_ref;

        curr_scope = curr_scope->parent;
    }

    return NULL;
}

int ctx_create_var(RunnerContext *ctx, char *var_name, VarValue *var_val)
{
    if (!var_name || !var_val) return 0;

    RubelScope *curr_scope = ctx->scopes.scopes[ctx->scopes.stack_ptr];
    Variable *new_var = variable_create(var_name, var_val);

    if (!new_var) return 0;

    return scope_put_var(curr_scope, new_var);
}

int ctx_update_var(RunnerContext *ctx, char *var_name, VarValue *var_val)
{
    if (!var_name || !var_val) return 0;

    Variable *used_var = ctx_get_var(ctx, var_name);
    DataType lval_type = variable_get_type(used_var);
    DataType rval_type = var_val->type;
    StringObj *optional_str = NULL;
    ListObj *optional_list = NULL;

    if (variable_is_const(used_var)) return 0;

    if (lval_type != rval_type) return 0;

    switch (lval_type)
    {
    case INT_TYPE:
        used_var->value->data.int_val.value = var_val->data.int_val.value;
        break;
    case REAL_TYPE:
        used_var->value->data.real_val.value = var_val->data.real_val.value;
        break;
    case BOOL_TYPE:
        used_var->value->data.bool_val.flag = var_val->data.bool_val.flag;
        break;
    case STR_TYPE:
        // clear old string
        optional_str = used_var->value->data.str_type.value;
        destroy_str_obj(optional_str);
        free(optional_str);

        // set new one
        used_var->value->data.str_type.value = var_val->data.str_type.value;
        break;
    case LIST_TYPE:
        // clear old list
        optional_list = used_var->value->data.list_type.value;
        destroy_list_obj(optional_list);
        free(optional_list);

        used_var->value->data.list_type.value = var_val->data.list_type.value;
        break;
    default:
        return 0;
    }

    return 1;
}

/// SECTION: Expr. helpers

VarValue *eval_literal(RunnerContext *ctx, Expression *expr)
{
    VarValue *result = NULL;
    ExpressionType expr_type = expr->type;

    switch (expr_type)
    {
    case BOOL_LITERAL:
        result = create_bool_varval(1, expr->syntax.bool_literal.flag);
        break;
    case INT_LITERAL:
        result = create_int_varval(1, expr->syntax.int_literal.value);
        break;
    case REAL_LITERAL:
        result = create_real_varval(1, expr->syntax.real_literal.value);
        break;
    case STR_LITERAL:
        result = create_str_varval(1, expr->syntax.str_literal.str_obj);
        break;
    case LIST_LITERAL:
        result = create_list_varval(1, expr->syntax.list_literal.list_obj);
        break;
    case VAR_USAGE:
        result = eval_var_usage(ctx, expr);
    default:
        break;
    }

    return result;
}

VarValue *eval_var_usage(RunnerContext *ctx, Expression *expr)
{
    RubelScope *curr_scope = ctx->scopes.scopes[ctx->scopes.stack_ptr];
    Variable *var_ref = NULL;
    VarValue *result = NULL;
    const char *var_name = expr->syntax.variable.var_name; 

    var_ref = scope_get_var_ref(curr_scope, var_name);

    if (!var_ref)
    {
        ctx->status = ERR_MEMORY;
        return result;
    }

    switch (var_ref->value->type)
    {
    case BOOL_LITERAL:
        result = create_bool_varval(1, var_ref->value->data.bool_val.flag);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case INT_LITERAL:
        result = create_int_varval(1, var_ref->value->data.int_val.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case REAL_LITERAL:
        result = create_real_varval(1, var_ref->value->data.real_val.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case STR_LITERAL:
        result = create_str_varval(1, var_ref->value->data.str_type.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case LIST_LITERAL:
        result = create_list_varval(1, var_ref->value->data.list_type.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    default:
        ctx_set_status(ctx, ERR_TYPE);
        break;
    }

    return result;
}

VarValue *eval_call(RunnerContext *ctx, Expression *expr)
{
    const char *fn_name = expr->syntax.fn_call.func_name;
    unsigned short argc = (unsigned short)(expr->syntax.fn_call.argc);
    FuncArgs *call_args = funcargs_create(argc);
    VarValue *fn_result = NULL; // function return value!
    
    if (!call_args)
    {
        ctx_set_status(ctx, ERR_MEMORY);
        return fn_result;
    }

    // populate params to later bind to callee scope
    for(unsigned short arg_index = 0; arg_index < argc; arg_index++)
    {
        Expression *arg_expr = expr->syntax.fn_call.args[arg_index];
        VarValue *temp_arg = eval_expr(ctx, arg_expr);

        if (!funcargs_set_at(call_args, arg_index, temp_arg)) break;
    }

    fn_result = ctx_call_func(ctx, argc, fn_name, call_args);

    ctx->status = (ctx->status <= OK_ENDED) ? ctx->status : ERR_GENERAL;

    return fn_result;
}

VarValue *eval_unary(RunnerContext *ctx, Expression *expr)
{
    VarValue *result = NULL;
    ExpressionType expr_type = expr->type;
    OpType operation = expr->syntax.unary_op.op;

    if (operation != OP_NEG)
    {
        ctx->status = ERR_GENERAL;
        return result;
    }

    switch (expr_type)
    {
    case INT_LITERAL:
        result = create_int_varval(1, 0 - expr->syntax.int_literal.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case REAL_LITERAL:
        result = create_real_varval(1, 0 - expr->syntax.real_literal.value);
        ctx_set_status(ctx, OK_RAN_CMD);
        break;
    case BOOL_LITERAL:
    case STR_LITERAL:
    case LIST_LITERAL:
    default:
        ctx_set_status(ctx, ERR_TYPE);
        break;
    }

    return result;
}

int compare_primitives(OpType op, VarValue *left_val, VarValue *right_val)
{
    DataType left_type = left_val->type; // NOTE: assumed to be same type as right value since this is called after a type check! 
    int left_n, right_n;
    float left_f, right_f;
    int pre_eq, pre_gt, pre_lt; // NOTE: precomputed compare flags (nots are inverted from these!)

    // Get values by tagged union type...
    switch (left_type)
    {
    case BOOL_TYPE:
        left_n = left_val->data.bool_val.flag;
        right_n = right_val->data.bool_val.flag;
        pre_eq = left_n == right_n;
        pre_gt = left_n > right_n;
        pre_lt = left_n < right_n;
        break;
    case INT_TYPE:
        left_n = left_val->data.int_val.value;
        right_n = right_val->data.int_val.value;
        pre_eq = left_n == right_n;
        pre_gt = left_n > right_n;
        pre_lt = left_n < right_n;
        break;
    case REAL_TYPE:
        left_f = left_val->data.real_val.value;
        right_f = right_val->data.real_val.value;
        pre_eq = left_f == right_f;
        pre_gt = left_f > right_f;
        pre_lt = left_f < right_f;
        break;
    default:
        return -1; // NOTE: mark invalid comparison types!
    }

    // compare by operation value
    if (op == OP_EQ) return pre_eq;
    if (op == OP_NEQ) return !pre_eq;
    if (op == OP_GT) return pre_gt;
    if (op == OP_GTE) return pre_gt || pre_eq;
    if (op == OP_LT) return pre_lt;
    if (op == OP_LTE) return pre_lt || pre_eq;

    return -2; // NOTE: mark invalid operation type!
}

VarValue *math_primitives(OpType op, VarValue *left_val, VarValue *right_val)
{
    DataType left_type = left_val->type;
    VarValue *result = NULL;
    int left_int, right_int;
    float left_flt, right_flt;

    switch (left_type)
    {
    case INT_TYPE:
        left_int = left_val->data.int_val.value;
        right_int = right_val->data.int_val.value;
        switch (op)
        {
        case OP_ADD:
            result = create_int_varval(1, left_int + right_int);
            break;
        case OP_SUB:
            result = create_int_varval(1, left_int - right_int);
            break;
        case OP_MUL:
            result = create_int_varval(1, left_int * right_int);
            break;
        case OP_DIV:
            if (right_int != 0) result = create_int_varval(1, left_int / right_int);
            break;
        default:
            break;
        }
        break;
    case REAL_TYPE:
        left_flt = left_val->data.real_val.value;
        right_flt = right_val->data.real_val.value;
        switch (op)
        {
        case OP_ADD:
            result = create_real_varval(1, left_flt + right_flt);
            break;
        case OP_SUB:
            result = create_real_varval(1, left_flt - right_flt);
            break;
        case OP_MUL:
            result = create_real_varval(1, left_flt * right_flt);
            break;
        case OP_DIV:
            if (right_flt != 0) result = create_real_varval(1, left_flt / right_flt);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return result;
}

VarValue *eval_comparison(RunnerContext *ctx, OpType op, VarValue *left_val, VarValue *right_val)
{
    int flag = 0;
    VarValue *result = NULL;

    if (left_val->type != right_val->type)
    {
        ctx_set_status(ctx, ERR_TYPE);
        return result;
    }

    flag = compare_primitives(op, left_val, right_val);

    // reject invalid flags from bad type mismatches, etc.
    if (flag < 0)
    {
        ctx_set_status(ctx, ERR_GENERAL);
        return result;
    }

    return create_bool_varval(1, flag);
}

VarValue *eval_binary(RunnerContext *ctx, Expression *expr)
{
    Expression *left = expr->syntax.binary_op.left;
    Expression *right = expr->syntax.binary_op.right;
    OpType operation = expr->syntax.binary_op.op;
    VarValue *result = NULL;

    if (!left || !right)
    {
        ctx_set_status(ctx, ERR_MEMORY);
        return result;
    }

    VarValue *left_val = eval_expr(ctx, left);
    VarValue *right_val = eval_expr(ctx, right);

    if (operation == OP_EQ || operation == OP_NEQ || operation == OP_GT || operation == OP_GTE || operation == OP_LT || operation == OP_LTE)
    {
        result = eval_comparison(ctx, operation, left_val, right_val);
    }
    else if (operation == OP_ADD || operation == OP_SUB || operation == OP_MUL || operation == OP_DIV)
    {
        result = math_primitives(operation, left_val, right_val);
    }

    ctx_set_status(ctx, OK_RAN_CMD);

    return result;
}

VarValue *eval_expr(RunnerContext *ctx, Expression *expr)
{
    VarValue *expr_result = NULL;

    switch (expr->type)
    {
    case BOOL_LITERAL:
    case INT_LITERAL:
    case REAL_LITERAL:
    case STR_LITERAL:
    case LIST_LITERAL:
        expr_result = eval_literal(ctx, expr);
        break;
    case VAR_USAGE:
        expr_result = eval_var_usage(ctx, expr);
        break;
    case FUNC_CALL:
        expr_result = eval_call(ctx, expr);
        break;
    case UNARY_OP:
        expr_result = eval_unary(ctx, expr);
        break;
    case BINARY_OP:
        expr_result = eval_binary(ctx, expr);
        break;
    default:
        ctx_set_status(ctx, ERR_NO_IMPL);
        break;
    }

    return expr_result;
}

/// SECTION: Statement helpers

RunStatus exec_module_decl(RunnerContext *ctx, Statement *stmt)
{
    return ERR_NO_IMPL; // TODO: implement this to prepare a module's FuncGroup to later put in RunnerContext.
}

RunStatus exec_module_usage(RunnerContext *ctx, Statement *stmt)
{
    const FuncEnv *ctx_modules = ctx->function_env;
    const char *module_name = stmt->syntax.module_usage.module_name;

    FuncGroup *module_ref = funcenv_fetch(ctx_modules, module_name);

    if (!module_ref) return ERR_NO_IMPL;

    funcgroup_mark_used(module_ref, 1);

    return OK_RAN_CMD;
}

RunStatus exec_var_decl(RunnerContext *ctx, Statement *stmt)
{
    RubelScope *curr_scope = ctx->scopes.scopes[ctx->scopes.stack_ptr];
    char *var_name = stmt->syntax.var_decl.var_name;
    Expression *rvalue_expr = stmt->syntax.var_decl.rvalue;
    VarValue *var_decl_val = NULL;
    Variable *var_decl_result = NULL;

    // NOTE: reject re-declarations as they're bad practice!
    if (scope_get_var_ref(curr_scope, var_name) != NULL) return ERR_GENERAL;

    var_decl_val = eval_expr(ctx, rvalue_expr);

    if (!var_decl_val) return ERR_MEMORY;

    var_decl_result = variable_create(var_name, var_decl_val);

    if (!scope_put_var(curr_scope, var_decl_result)) return ERR_MEMORY;

    return OK_RAN_CMD;
}

RunStatus exec_var_assign(RunnerContext *ctx, Statement *stmt)
{
    char *lvalue_name = stmt->syntax.var_assign.var_name;
    Expression *rvalue_expr = stmt->syntax.var_assign.rvalue;
    Variable *lvalue_ref = ctx_get_var(ctx, lvalue_name);
    VarValue *new_value = NULL;

    // NOTE: fail execution on: undefined vars, const rewrites...
    if (!lvalue_ref) return ERR_NULL_VAL;

    if (lvalue_ref->value->is_const) return ERR_GENERAL;

    new_value = eval_expr(ctx, rvalue_expr);

    // NOTE: also fail execution on value allocation failure...
    if (!new_value) return ERR_MEMORY;

    if (new_value->type != lvalue_ref->value->type) return ERR_TYPE; // NOTE: this will leave a memory leak of a few bytes, but type mismatches are fatal... Exit!

    ctx_update_var(ctx, lvalue_name, new_value);

    return OK_RAN_CMD;
}

RunStatus exec_func_decl(RunnerContext *ctx, Statement *stmt)
{
    FuncGroup *script_module = ctx->function_env->func_groups[0];
    unsigned short fn_arity = stmt->syntax.func_decl.argc;
    char *fn_name = stmt->syntax.func_decl.func_name;
    Expression **fn_params = stmt->syntax.func_decl.func_params;
    Statement *fn_block = stmt->syntax.func_decl.stmts;

    FuncObj *fn_obj = func_ast_create(fn_name, fn_arity, fn_params, fn_block);

    if (!fn_obj) return ERR_MEMORY;
    
    if (!funcgroup_put(script_module, fn_obj)) return ERR_MEMORY;

    return OK_RAN_CMD;
}

VarValue *exec_while(RunnerContext *ctx, Statement *stmt)
{
    Expression *while_condition = stmt->syntax.while_stmt.condition;
    Statement *while_block = stmt->syntax.while_stmt.stmts;
    VarValue *expr_value = eval_expr(ctx, while_condition);
    VarValue *optional_result = NULL;
    RunStatus status = OK_RAN_CMD;
    int flag = 1;

    while(flag && ctx->status <= OK_ENDED)
    {
        // NOTE: these guard returns will also leave a tiny memory leak, but this error is fatal anyways... Exit!
        if (!expr_value)
        {
            status = ERR_MEMORY;
            break;
        }

        if (expr_value->type != BOOL_TYPE)
        {
            status = ERR_TYPE;
            break;
        }

        // run block of loop for true checks...
        optional_result = exec_block(ctx, while_block);

        if (optional_result != NULL)
        {
            // NOTE: since loops can ONLY be in functions, treat the presence of result as a function return!
            status = OK_CTRL_RETURN;
            break;
        }

        // clean up temp condition value
        varval_destroy(expr_value);
        free(expr_value);

        expr_value = eval_expr(ctx, while_condition);
        flag = expr_value->data.bool_val.flag;
    }

    varval_destroy(expr_value);
    free(expr_value);

    ctx_set_status(ctx, status);

    return optional_result;
}

VarValue *exec_block(RunnerContext *ctx, Statement *stmt)
{
    unsigned int block_len = stmt->syntax.block.count;
    Statement **stmt_cursor = stmt->syntax.block.stmts;
    Statement *curr_stmt = NULL;
    VarValue *optional_value = NULL; // possible return value if in a function!
    RunStatus exec_status = OK_RAN_CMD;

    for (unsigned int i = 0; i < block_len; i++)
    {
        curr_stmt = *stmt_cursor;

        // NOTE: return statements are ONLY parsed within function blocks, so I can assume that the return value of a block must be exiting a function.
        if (curr_stmt->type == RETURN_STMT)
        {
            optional_value = exec_return(ctx, curr_stmt);
            break;
        }
        else if (curr_stmt->type == IF_STMT)
        {
            optional_value = exec_ifotherwise(ctx, curr_stmt);
        }
        else if (curr_stmt->type == WHILE_STMT)
        {
            optional_value = exec_while(ctx, curr_stmt);
        }

        // NOTE: a present optional_value from a function's composite stmt return should be bubbled out!
        if (optional_value != NULL)
        {
            ctx_set_status(ctx, OK_CTRL_RETURN);
            break;
        }

        exec_status = exec_stmt(ctx, curr_stmt);

        if (exec_status == OK_CTRL_BREAK)
        {
            ctx_set_status(ctx, exec_status);
            break;
        }

        // NOTE: bail out of a block on errors!
        if (exec_status > OK_ENDED)
        {
            ctx_set_status(ctx, exec_status);
            return NULL;
        }

        stmt_cursor++;
    }

    return optional_value;
}

VarValue *exec_ifotherwise(RunnerContext *ctx, Statement *stmt)
{
    Expression *condition_expr = stmt->syntax.if_stmt.condition;
    VarValue *check_result = NULL;
    VarValue *optional_result = NULL;
    Statement *if_stmt = stmt->syntax.if_stmt.first;
    Statement *other_stmt = stmt->syntax.if_stmt.other;

    check_result = eval_expr(ctx, condition_expr);

    if (!check_result)
    {
        ctx_set_status(ctx, ERR_MEMORY);
        return optional_result;
    }

    if (check_result->type != BOOL_TYPE)
    {
        ctx_set_status(ctx, ERR_TYPE);
        return optional_result;
    }

    if (check_result->data.bool_val.flag) optional_result = exec_block(ctx, if_stmt);
    else optional_result = exec_block(ctx, other_stmt->syntax.otherwise_stmt.stmts);

    ctx->status = (ctx->status <= OK_ENDED) ? ctx->status : ERR_GENERAL;

    return optional_result;
}

RunStatus exec_break(RunnerContext *ctx, Statement *stmt)
{
    return OK_CTRL_BREAK;
}

VarValue *exec_return(RunnerContext *ctx, Statement *stmt)
{
    // todo: don't call in top level exec_stmt!
    Expression *expr_ref = stmt->syntax.return_stmt.result;
    VarValue *expr_val = eval_expr(ctx, expr_ref);

    if (!expr_val)
    {
        ctx_set_status(ctx, ERR_MEMORY);
        return NULL;
    }

    ctx_set_status(ctx, OK_RAN_CMD);
    return expr_val;
}

RunStatus exec_expr_stmt(RunnerContext *ctx, Statement *stmt)
{
    // NOTE: results may be destroyed in exec_stmt, as expr_stmts usually should be lone function calls like print("hi").
    Expression *expr = stmt->syntax.expr_stmt.expr;
    ExpressionType expr_type = expr->type;
    VarValue *discarded_val = NULL;

    // NOTE: only discard values from alone function calls since unused expression values in expr. statements will be useless.
    if (expr_type != FUNC_CALL) return OK_UNUSED_VAL;

    discarded_val = eval_expr(ctx, expr);

    if (discarded_val != NULL)
    {
        varval_destroy(discarded_val);
        free(discarded_val);
    }

    return OK_RAN_CMD;
}

RunStatus exec_stmt(RunnerContext *ctx, Statement *stmt)
{
    RunStatus exec_status = ERR_GENERAL;
    StatementType stmt_type = stmt->type;

    switch (stmt_type)
    {
    case EXPR_STMT:
        exec_status = exec_expr_stmt(ctx, stmt);
        break;
    case MODULE_DEF:
        exec_status = exec_module_decl(ctx, stmt);
        break;
    case MODULE_USE:
        exec_status = exec_module_usage(ctx, stmt);
        break;
    case VAR_DECL:
        exec_status = exec_var_decl(ctx, stmt);
        break;
    case VAR_ASSIGN:
        exec_status = exec_var_assign(ctx, stmt);
        break;
    case FUNC_DECL:
        exec_status = exec_func_decl(ctx, stmt);
        break;
    default:
        break;
    }

    return exec_status;
}
