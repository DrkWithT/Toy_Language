#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "backend/runner/runctx.h"

/**
 * @brief Interpreter object. Tracks scopes and other execution state while walking the AST.
 * @note Warning to users: the runtime errors will be vague. I'll fix that when I have time.
 * @author Derek Tan
 * @todo Move configuration logic to component within the Interpreter structure.
 */

typedef struct st_interpreter
{
    RunnerContext context;
    Script *script_ref;
} Interpreter;

int interpreter_init(Interpreter *runner, Script *program);

/**
 * @brief Invokes clean up functions to destroy the program AST, clean up interpreter state, and free most other dynamic memory except the source code C-String. The source code should be freed just after the Script object is made. 
 * @param runner The interpeter ref ptr.
 */
void interpreter_dispose(Interpreter *runner);

/**
 * @brief Binds a collection of native C functions to the RunnerContext of this interpreter. 
 * @param runner The interpreter ref ptr.
 * @param native_module A prefilled FuncGroup object.
 * @return int 1 on success.
 */
int interpreter_load_natives(Interpreter *runner, FuncGroup *native_module);

void interpreter_log_err(Interpreter *runner, unsigned int top_stmt_num, RunStatus status);

void interpreter_run(Interpreter *runner);

#endif