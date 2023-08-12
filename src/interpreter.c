/**
 * @file interpreter.c
 * @author Derek Tan
 * @brief Implements main interpreter functions, but configuation is a todo.
 * @date 2023-08-05
 */

#include "backend/runner/interpreter.h"

int interpreter_init(Interpreter *runner, Script *program)
{
    if (!runner || !program) return 0;

    int ctx_ok = ctx_init(&runner->context, program);
    
    runner->script_ref = program;

    return ctx_ok;
}

void interpreter_dispose(Interpreter *runner)
{
    if (!runner) return;

    dispose_script(runner->script_ref);
    ctx_destroy(&runner->context);
}

int interpreter_load_natives(Interpreter *runner, FuncGroup *native_module)
{
    if (!native_module) return 0;

    return ctx_load_funcgroup(&runner->context, native_module);
}

void interpreter_log_err(Interpreter *runner, unsigned int top_stmt_num, RunStatus status)
{
    switch (status)
    {
    case ERR_TYPE:
        printf("TypeErr at stmt %u: %s\n", top_stmt_num, "Invalid types for operator.");
        break;
    case ERR_NULL_VAL:
        printf("NullErr at stmt %u: %s\n", top_stmt_num, "Yielded undefined value in operation.");
        break;
    case ERR_MEMORY:
        printf("MemoryErr at stmt %u: %s\n", top_stmt_num, "Allocation failure or invalid reference passed.");
        break;
    case ERR_NO_IMPL:
        printf("NoImplErr at stmt %u: %s\n", top_stmt_num, "Item not found in scope.");
        break;
    case ERR_GENERAL:
        printf("BaseRunErr at stmt %u: %s\n", top_stmt_num, "Unknown runtime error.");
        break;
    default:
        break;
    }
}

void interpreter_run(Interpreter *runner)
{
    RunnerContext *ctx_ref = &(runner->context); // interpreter context
    unsigned int prgm_len = runner->script_ref->count; // top-level statement count
    Statement **prgm_stmts = runner->script_ref->stmts; // statement vector
    Statement *stmt_ref = NULL; // current top-level statement to do
    RunStatus status = OK_IDLE;

    for (unsigned int i = 0; (i < prgm_len) && (status <= OK_ENDED); i++)
    {
        stmt_ref = *prgm_stmts;

        status = exec_stmt(ctx_ref, stmt_ref);
        interpreter_log_err(runner, i, status);

        prgm_stmts++;
    }
}
