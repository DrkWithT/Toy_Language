/**
 * @file functions.c
 * @author Derek Tan
 * @brief Implements function binding API for Rubel.
 * @date 2023-07-31
 */

#include "backend/api/functions.h"

/// SECTION: Args Impl.

FuncArgs *funcargs_create(unsigned short argc)
{
    unsigned short checked_argc = argc;

    if (checked_argc > FUNC_ARGV_MAX_SZ) checked_argc = FUNC_ARGV_MAX_SZ;

    FuncArgs *argv = malloc(sizeof(FuncArgs));

    if (argv != NULL)
    {
        VarValue **temp_args = malloc(sizeof(VarValue *) * checked_argc);

        for (unsigned short i = 0; i < checked_argc; i++)
        {
            temp_args[i] = NULL;
        }

        argv->args = temp_args;
        argv->argc = checked_argc;
    }

    return argv;
}

void funcargs_dispose(FuncArgs *argv)
{
    if (argv->args != NULL)
    {
        free(argv->args);
        argv->args = NULL;
    }

    argv->argc = 0;
}

void funcargs_destroy(FuncArgs *argv)
{
    unsigned short count = argv->argc;
    VarValue **target_cursor = argv->args;
    VarValue *target = NULL;

    for (unsigned short i = 0; i < count; i++)
    {
        target = *target_cursor;

        switch (target->type)
        {
        case STR_TYPE:
            
            break;
        case LIST_TYPE:
            
            break;
        default:
            break;
        }

        target_cursor++;
    }
    
}

int funcargs_set_at(FuncArgs *argv, unsigned short index, VarValue *arg)
{
    if (index >= argv->argc) return 0;

    argv->args[index] = arg;

    return 1;
}

VarValue *funcargs_get_at(const FuncArgs *argv, unsigned short index)
{
    if (index >= argv->argc) return NULL;

    return argv->args[index];
}


/// SECTION: Params Impl.

FuncParams *funcparams_create(unsigned short capacity)
{
    unsigned short checked_capacity = capacity;

    if (checked_capacity < FUNC_ARGV_MIN_SZ) checked_capacity = FUNC_ARGV_MIN_SZ;

    FuncParams *fn_params = malloc(sizeof(FuncParams));

    if (!fn_params) return NULL;
    
    Variable **temp_cells = malloc(sizeof(Variable *) * checked_capacity);

    if (!temp_cells)
    {
        fn_params->param_refs = NULL;
        fn_params->capacity = 0;
    }
    else
    {
        for (size_t i = 0; i < checked_capacity; i++)
        {    
            temp_cells[i] = NULL;
        }

        fn_params->param_refs = temp_cells;
        fn_params->capacity = checked_capacity;
    }

    fn_params->count = 0;

    return fn_params;
}

void funcparams_dispose(FuncParams *args)
{
    if (args->capacity == 0) return;

    size_t argc = args->count;

    for (size_t i = 0; i < argc; i++)
    {
        args->param_refs[i] = NULL; // unbind arg value object managed by AST
    }

    free(args->param_refs);
    args->param_refs = NULL;
}

void funcparams_destroy(FuncParams *args)
{
    if (args->capacity == 0) return;

    size_t argc = args->count;

    for (size_t i = 0; i < argc; i++)
    {
        variable_destroy(args->param_refs[i]);
        free(args->param_refs[i]);
    }

    free(args->param_refs);
    args->param_refs = NULL;
}

int funcparams_put(FuncParams *args, Variable *var_arg_obj)
{
    if (!var_arg_obj || args->capacity == 0) return 0;

    size_t next_spot = args->count;
    size_t curr_capacity = args->capacity;
    size_t new_capacity = curr_capacity << 1;

    if (next_spot <= curr_capacity - 1)
    {
        args->param_refs[next_spot] = var_arg_obj;
        args->count++;
        return 1;
    }

    Variable **temp_argv = realloc(args->param_refs, sizeof(Variable *) * new_capacity);

    if (!temp_argv) return 0;

    for (size_t i = next_spot; i < new_capacity; i++)
    {
        temp_argv[i] = NULL;
    }

    temp_argv[next_spot] = var_arg_obj;
    args->param_refs = temp_argv;
    args->capacity = new_capacity;
    args->count++;

    return 1;
}

Variable *funcparams_get(FuncParams *args, unsigned short index)
{
    if (index >= args->count) return NULL;

    return args->param_refs[index];
}

/// SECTION: Function Impl.

FuncObj *func_native_create(char *name, int arity, NativeFunc *fn_ptr)
{
    FuncObj *fn_obj = malloc(sizeof(FuncObj));

    if (fn_obj != NULL)
    {
        fn_obj->type = FUNC_NATIVE;
        fn_obj->arity = arity;
        fn_obj->name = name;
        fn_obj->content.fn_ptr = fn_ptr;
    }

    return fn_obj;
}

FuncObj *func_ast_create(char *name, int arity, Statement *fn_ast)
{
    FuncObj *fn_obj = malloc(sizeof(FuncObj));

    if (fn_obj != NULL)
    {
        fn_obj->type = FUNC_NORMAL;
        fn_obj->arity = arity;
        fn_obj->name = name;
        fn_obj->content.fn_ast = fn_ast;
    }

    return fn_obj;
}

void func_dispose(FuncObj *fn_obj)
{
    if (fn_obj->name != NULL && fn_obj->type != FUNC_NATIVE)
    {
        free(fn_obj->name);
        fn_obj->name = NULL;
    }

    switch (fn_obj->type)
    {
    case FUNC_NORMAL:
        fn_obj->content.fn_ast = NULL;
        break;
    case FUNC_NATIVE:
        fn_obj->content.fn_ptr = NULL;
        break;
    default:
        break;
    }
}

/// SECTION: Function Storage Impl. TODO!!

FuncGroup *funcgroup_create(char *name, unsigned int buckets)
{
    FuncObj **temp_buckets = NULL;
    FuncGroup *fn_group = malloc(sizeof(FuncGroup));

    if (fn_group != NULL)
    {
        temp_buckets = malloc(sizeof(FuncObj *) * buckets);
    }

    if (!temp_buckets)
    {
        fn_group->fn_buckets = NULL;
        fn_group->count = 0;
        fn_group->name = name;
        fn_group->used = 0;
        return fn_group;
    }

    for (size_t i = 0; i < buckets; i++) temp_buckets[i] = NULL;

    fn_group->fn_buckets = temp_buckets;
    fn_group->count = buckets;
    fn_group->name = name;

    return fn_group;
}

void funcgroup_dispose(FuncGroup *fn_group)
{
    fn_group->name = NULL; // TODO: add check for module's nativeness before unbind OR freeing the name string.

    size_t fn_count = fn_group->count;

    for (size_t i = 0; i < fn_count; i++)
    {
        FuncObj *fn_ref = fn_group->fn_buckets[i];
        func_dispose(fn_ref);
        free(fn_ref);
        fn_group->fn_buckets[i] = NULL;
    }

    if (fn_group->count >= 1)
    {
        free(fn_group->fn_buckets);
        fn_group->fn_buckets = NULL;
    }
}

void funcgroup_mark_used(FuncGroup *fn_group, int flag)
{
    fn_group->used = (flag == 1);
}

int funcgroup_is_used(const FuncGroup *fn_group)
{
    return fn_group->used;
}

int funcgroup_put(FuncGroup *fn_group, FuncObj *fn_obj)
{
    if (!fn_obj || fn_group->count == 0) return 0;

    size_t bucket_index = hash_key(fn_obj->name) % fn_group->count;

    if (!fn_group->fn_buckets[bucket_index])
    {
        fn_group->fn_buckets[bucket_index] = fn_obj;
        return 1;
    }

    return 0;
}

const FuncObj *funcgroup_get(const FuncGroup *fn_group, const char *fn_name)
{
    if (!fn_name || fn_group->count == 0 || !fn_group->used) return NULL;

    size_t bucket_index = hash_key(fn_name) % fn_group->count;

    return fn_group->fn_buckets[bucket_index];
}

FuncEnv *funcenv_create(unsigned int capacity)
{
    FuncGroup **temp_fn_groups = NULL;
    FuncEnv *fenv = malloc(sizeof(FuncEnv));

    if (fenv != NULL)
    {
        temp_fn_groups = malloc(sizeof(FuncGroup *) * capacity);
    }

    if (!temp_fn_groups)
    {
        fenv->func_groups = NULL;
        fenv->count = 0;
        fenv->capacity = 0;
        return fenv;
    }

    for (size_t i = 0; i < capacity; i++)
    {
        temp_fn_groups[i] = NULL;
    }

    fenv->count = 0;
    fenv->capacity = capacity;
    fenv->func_groups = temp_fn_groups;

    return fenv;
} 

void funcenv_dispose(FuncEnv *fenv)
{
    if (fenv->count == 0) return;

    size_t fn_group_count = fenv->count;

    for (size_t i = 0; i < fn_group_count; i++)
    {
        funcgroup_dispose(fenv->func_groups[i]);
    }

    free(fenv->func_groups);
    fenv->func_groups = NULL;
    fenv->count = 0;
    fenv->capacity = 0;
}

int funcenv_append(FuncEnv *fenv, FuncGroup *fn_group_obj)
{
    unsigned int next_spot = fenv->count;
    unsigned int curr_capacity = fenv->capacity;
    unsigned int new_capacity = curr_capacity << 1;

    if (!fn_group_obj) return 0;

    if (next_spot <= curr_capacity - 1)
    {
        fenv->func_groups[next_spot] = fn_group_obj;
        fenv->count++;
        return 1;
    }

    FuncGroup **temp_group_array = realloc(fenv->func_groups, sizeof(FuncGroup *) * new_capacity);

    if (!temp_group_array) return 0;

    for (size_t i = next_spot; i < new_capacity; i++)
    {
        temp_group_array[i] = NULL;
    }

    temp_group_array[next_spot] = fn_group_obj;
    fenv->func_groups = temp_group_array;
    fenv->count++;
    fenv->capacity = new_capacity;

    return 1;
}

const FuncGroup *funcenv_fetch(const FuncEnv *fenv, const char *group_name)
{
    unsigned int countdown = fenv->count;
    FuncGroup **search_ptr = fenv->func_groups;

    while (countdown > 0)
    {
        if (strcmp((*search_ptr)->name, group_name) == 0)
        {
            return search_ptr;
        }

        countdown--;
        search_ptr++;
    }

    return NULL;
}
