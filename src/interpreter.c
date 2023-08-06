/**
 * @file interpreter.c
 * @author Derek Tan
 * @brief Implements main interpreter functions, but configuation is a todo.
 * @date 2023-08-05
 */

#include "backend/runner/interpreter.h"

int interpreter_init(Interpreter *runner, Script *program)
{
    if (!runner) return 0;

    return ctx_init(&runner->context, program);
}

void interpreter_dispose(Interpreter *runner)
{
    if (!runner) return;

    dispose_script(runner->script_ref);
    ctx_destroy(&runner->context);
}

int interpreter_load_natives(Interpreter *runner, FuncGroup *native_module);

void interpreter_run(Interpreter *runner);
