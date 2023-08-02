#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "frontend/ast.h"
#include "backend/values/scope.h"

/// SECTION: Macros

#define FUNC_ARGV_MIN_SZ 4

/// SECTION: Type Decls.

typedef VarValue *(*NativeFunc)(const struct st_func_argv *args);

typedef enum en_function_type
{
    FUNC_NATIVE,  // wrapper for C function
    FUNC_NORMAL,  // wrapper for AST nodes to traverse
    FUNC_UNKNOWN  // reserved for bytecode usage??
} FuncType;

/// SECTION: Args Impl.

typedef struct st_func_argv
{
    unsigned short count;
    unsigned short capacity; // length of fixed arg ptr array
    Variable **argv_dupe_refs; // array of argument ptrs (as variable or literal) 
} FuncArgs;

void funcargs_init(FuncArgs *args, unsigned short capacity);

/**
 * @brief Unbinds referencing ptrs to arg values before freeing the ptr array.
 * @param args 
 */
void funcargs_destroy(FuncArgs *args);

int funcargs_put(FuncArgs *args, VarValue *arg_obj);

VarValue *funcargs_get(FuncArgs *args, unsigned short index);

/// SECTION: Function Decl.

/**
 * @brief Function object managed by interpreter. Memory in ptrs is usually unbound before freeing of the structure.
 */
typedef struct st_function
{
    FuncType type; // function content type
    int arity; // accepted arg count
    char *name; // function name
    union
    {
        NativeFunc fn_ptr; // native C function reference ptr
        Statement *fn_ast; // AST stmts reference ptr
    } content;
} FuncObj;

FuncObj *func_native_create(char *name, int arity, NativeFunc *fn_ptr);

FuncObj *func_ast_create(char *name, int arity, Statement *fn_ast);

// VarValue *func_call(const FuncObj *func, FuncArgs *argv, RubelScope *var_scope, struct st_func_env *fn_scope); // TODO: move to interpreter.h!

/// SECTION: Function Storage

/**
 * @brief Crude named dictionary for functions of an imported module.
 */
typedef struct st_func_group
{
    char *name;
    unsigned int count;
    FuncObj **fn_buckets;
} FuncGroup;

FuncGroup *funcgroup_create(char *name, unsigned int buckets);

/**
 * @brief Frees internal memory within this function dictionary. FuncObj has its names and contents unbound before freeing. Finally the outer bucket array is freed.
 * @param fn_group 
 */
void funcgroup_dispose(FuncGroup *fn_group);

int funcgroup_put(FuncGroup *fn_group, FuncObj *fn_obj);

const FuncObj *funcgroup_get(const FuncGroup *fn_group, const char *fn_name);

/**
 * @brief A crude vector of FuncGroup objects. Used as the function specific environment during execution of other functions.
 */
typedef struct st_func_env
{
    unsigned int count;
    unsigned int capacity;
    FuncGroup **func_groups;
} FuncEnv;

FuncEnv *funcenv_create(unsigned int capacity); 

void funcenv_dispose(FuncEnv *fenv);

int funcenv_append(FuncEnv *fenv, FuncGroup *fn_group_obj);

const FuncGroup *funcenv_fetch(const FuncEnv *fenv, const char *group_name);

#endif