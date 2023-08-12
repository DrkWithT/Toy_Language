#ifndef SCOPE_H
#define SCOPE_H

#include "backend/values/varenv.h"

/// SECTION: Macros

#define SCOPE_IS_GLOBAL(scope_ptr) scope_ptr->parent == NULL
#define SCOPE_STACK_SIZE 24

/// SECTION: Scopes

typedef struct st_scope
{
    struct st_scope *parent;
    VarEnv venv;
} RubelScope;

RubelScope *scope_create(RubelScope *parent);

void scope_destroy(RubelScope *scope);

Variable *scope_get_var_ref(const RubelScope *scope, const char *var_name);

int scope_put_var(RubelScope *scope, Variable *var_obj);

/// SECTION: Static Scope Stack (For nested blocks or maybe recursion.)

typedef struct st_scope_stack
{
    int stack_ptr;
    int capacity;
    RubelScope **scopes;
} ScopeStack;

int scopestack_init(ScopeStack *stack, int capacity);

void scopestack_destroy(ScopeStack *stack);

int scopestack_is_full(const ScopeStack *stack);

int scopestack_is_empty(const ScopeStack *stack);

int scopestack_push_scope(ScopeStack *stack, RubelScope *scope);

RubelScope *scopestack_pop_scope(ScopeStack *stack);

#endif