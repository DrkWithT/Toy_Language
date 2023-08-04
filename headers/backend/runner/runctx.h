#ifndef RUNCTX_H
#define RUNCTX_H

#include "backend/api/natives/nativefuncs.h"
#include "backend/values/scope.h"

/**
 * @brief Marks status of RunnerContext for specific error messages.
 */
typedef enum en_run_status
{
    OK_IDLE,
    OK_UNUSED_VAL,
    OK_RAN_CMD,
    OK_CTRL_BREAK,
    OK_CTRL_RETURN,
    OK_ENDED,
    ERR_TYPE,
    ERR_NULL_VAL,
    ERR_MEMORY,
    ERR_NO_IMPL,
    ERR_GENERAL
} RunStatus;

/**
 * @brief Stores important state for the interpreter run.
 */
typedef struct st_runner_ctx
{
    RunStatus status;  // error status
    FuncEnv *function_env; // actually the function "scope"
    ScopeStack scopes; // stack of scopes
} RunnerContext;

/// SECTION: Context utils

int ctx_init(RunnerContext *ctx, Script *program);

void ctx_destroy(RunnerContext *ctx);

inline void ctx_set_status(RunnerContext *ctx, RunStatus status);

int ctx_load_funcgroup(RunnerContext *ctx, FuncGroup *module);

/// SECTION: Function helpers

FuncObj *ctx_get_func(const RunnerContext *ctx, const char *fn_name);

VarValue *ctx_call_func(RunnerContext *ctx, unsigned short argc, const char *fn_name, FuncArgs *args);

/// SECTION: Variable helpers

Variable *ctx_get_var(const RunnerContext *ctx, const char *var_name);

int ctx_create_var(RunnerContext *ctx, const char *var_name, VarValue *var_val);

int ctx_update_var(RunnerContext *ctx, const char *var_name, VarValue *var_val);

/// SECTION: Expr helpers

VarValue *eval_literal(RunnerContext *ctx, Expression *expr);

VarValue *eval_var_usage(RunnerContext *ctx, Expression *expr);

VarValue *eval_call(RunnerContext *ctx, Expression *expr);

VarValue *eval_unary(RunnerContext *ctx, Expression *expr);

int compare_primitives(OpType op, VarValue *left_val, VarValue *right_val);

VarValue *math_primitives(OpType op, VarValue *left_val, VarValue *right_val);

VarValue *eval_comparison(RunnerContext *ctx, OpType op, VarValue *left_val, VarValue *right_val);

VarValue *eval_binary(RunnerContext *ctx, Expression *expr);

VarValue *eval_expr(RunnerContext *ctx, Expression *expr);

/// SECTION: Stmt helpers

/**
 * @brief Unimplemented for now. May later reserve a FuncGroup in the RunnerContext's FuncEnv for later use.
 * @param ctx Run context.
 * @param stmt Statement for a module declaration e.g. module "foo". 
 * @return RunStatus Returns ERR_NO_IMPL.
 */
RunStatus exec_module_decl(RunnerContext *ctx, Statement *stmt);

RunStatus exec_module_usage(RunnerContext *ctx, Statement *stmt);

RunStatus exec_var_decl(RunnerContext *ctx, Statement *stmt);

RunStatus exec_var_assign(RunnerContext *ctx, Statement *stmt);

RunStatus exec_func_decl(RunnerContext *ctx, Statement *stmt);

VarValue *exec_while(RunnerContext *ctx, Statement *stmt);

VarValue *exec_block(RunnerContext *ctx, Statement *stmt);

VarValue *exec_ifotherwise(RunnerContext *ctx, Statement *stmt);

RunStatus exec_break(RunnerContext *ctx, Statement *stmt);

VarValue *exec_return(RunnerContext *ctx, Statement *stmt);

RunStatus exec_expr_stmt(RunnerContext *ctx, Statement *stmt);

RunStatus exec_stmt(RunnerContext *ctx, Statement *stmt);

#endif