#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "frontend/ast.h"
#include "backend/values/varenv.h"

/// SECTION: Macros

#define FUNC_ARGV_MIN_SZ 4
#define FUNC_ARGV_MAX_SZ 32

/// SECTION: Type Decls.

typedef VarValue *(*NativeFunc)(const struct st_func_argv *args);

typedef enum en_function_type
{
    FUNC_NATIVE,  // wrapper for C function
    FUNC_NORMAL,  // wrapper for AST nodes to traverse
    FUNC_UNKNOWN  // reserved for bytecode usage??
} FuncType;

/// SECTION: Func Args Impl.

typedef struct st_func_args
{
    unsigned short argc;
    VarValue **args;
} FuncArgs;

FuncArgs *funcargs_create(unsigned short argc);

/**
 * @brief Unbinds referencing ptrs to arg values.
 * @param argv
 */
void funcargs_dispose(FuncArgs *argv);

/**
 * @brief Frees up memory of stored referencing ptrs to arg values.
 * @param argv 
 * @note Use only when there is no interpreter scope (native func calls!)
 */
void funcargs_destroy(FuncArgs *argv);

int funcargs_set_at(FuncArgs *argv, unsigned short index, VarValue *arg);

VarValue *funcargs_get_at(const FuncArgs *argv, unsigned short index);

/// SECTION: Func Params Impl.

typedef struct st_func_params
{
    unsigned short count;
    unsigned short capacity; // length of fixed ptr array
    Variable **param_refs; // array of parameter ptrs (as variable or literal) 
} FuncParams;

FuncParams *funcparams_create(unsigned short capacity);

/**
 * @brief Unbinds referencing ptrs to param vars before freeing the ptr array.
 * @param params 
 */
void funcparams_dispose(FuncParams *params);

/**
 * @brief Frees dynamic memory within the param ptr array. Use only for func calls with loaded params from a fn_call expression.
 * @param params 
 */
void funcparams_destroy(FuncParams *params);

int funcparams_put(FuncParams *params, Variable *var_arg_obj);

Variable *funcparams_get(FuncParams *params, unsigned short index);

/// SECTION: Function Decl.

/**
 * @brief Function object managed by interpreter. Memory in ptrs is usually unbound before freeing of the structure.
 * @note Don't free content ptrs as Script manages AST ptrs. and func ptrs. are static!
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

/// SECTION: Function Storage

/**
 * @brief Crude named dictionary for functions of an imported module.
 * @note The 1st FuncGroup is always the script's function module (grouping).
 */
typedef struct st_func_group
{
    int used;
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

void funcgroup_mark_used(FuncGroup *fn_group, int flag);

int funcgroup_is_used(const FuncGroup *fn_group);

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