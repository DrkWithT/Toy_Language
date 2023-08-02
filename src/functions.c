/**
 * @file functions.c
 * @author Derek Tan
 * @brief Implements function binding API for Rubel.
 * @date 2023-07-31
 */

#include "backend/api/functions.h"

/// SECTION: Args Impl.

void funcargs_init(FuncArgs *args, unsigned short capacity)
{
    unsigned short checked_capacity = capacity;

    if (checked_capacity < FUNC_ARGV_MIN_SZ) checked_capacity = FUNC_ARGV_MIN_SZ;

    Variable **temp_cells = malloc(sizeof(Variable *) * checked_capacity);

    if (!temp_cells)
    {
        args->argv_dupe_refs = NULL;
        args->capacity = 0;
    }
    else
    {
        for (size_t i = 0; i < checked_capacity; i++)
        {    
            temp_cells[i] = NULL;
        }

        args->argv_dupe_refs = temp_cells;
        args->capacity = checked_capacity;
    }
    
    args->count = 0;
}

void funcargs_destroy(FuncArgs *args)
{
    if (args->capacity == 0) return;

    size_t argc = args->count;

    for (size_t i = 0; i < argc; i++)
    {
        args->argv_dupe_refs[i] = NULL; // unbind arg value object managed by AST
    }

    free(args->argv_dupe_refs);
    args->argv_dupe_refs = NULL;
}

int funcargs_put(FuncArgs *args, VarValue *arg_obj)
{
    if (!arg_obj || args->capacity == 0) return 0;

    size_t next_spot = args->count;
    size_t curr_capacity = args->capacity;
    size_t new_capacity = curr_capacity << 1;

    if (next_spot <= curr_capacity - 1)
    {
        args->argv_dupe_refs[next_spot] = arg_obj;
        args->count++;
        return 1;
    }

    Variable **temp_argv = realloc(args->argv_dupe_refs, sizeof(Variable *) * new_capacity);

    if (!temp_argv) return 0;

    for (size_t i = next_spot; i < new_capacity; i++)
    {
        temp_argv[i] = NULL;
    }

    temp_argv[next_spot] = arg_obj;
    args->argv_dupe_refs = temp_argv;
    args->capacity = new_capacity;
    args->count++;

    return 1;
}

VarValue *funcargs_get(FuncArgs *args, unsigned short index)
{
    if (index >= args->count) return NULL;

    return args->argv_dupe_refs[index];
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
    if (fn_group->name != NULL)
    {
        free(fn_group->name);
        fn_group->name = NULL;
    }

    size_t fn_count = fn_group->count;

    for (size_t i = 0; i < fn_count; i++)
    {
        free(fn_group->fn_buckets[i]);
        fn_group->fn_buckets[i] = NULL;
    }

    if (fn_group->count >= 1)
    {
        free(fn_group->fn_buckets);
        fn_group->fn_buckets = NULL;
    }
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
    if (!fn_name || fn_group->count == 0) return NULL;

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
