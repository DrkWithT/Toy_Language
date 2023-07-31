/**
 * @file scope.c
 * @author Derek Tan
 * @brief Implements Scope for Rubel interpreter.
 * @date 2023-07-30
 */

#include "backend/values/scope.h"

/// SECTION: Scope Impl.

RubelScope *scope_create(RubelScope *parent)
{
    RubelScope *scope = malloc(sizeof(RubelScope));

    if (!scope) return NULL;

    scope->parent = parent;

    if (!varenv_init(&scope->venv, VAR_ENV_SIZE))
    {
        free(scope);
        scope = NULL;
    }

    return scope;
}

void scope_destroy(RubelScope *scope)
{
    varenv_destroy(&scope->venv);
}

Variable *scope_get_var_ref(const RubelScope *scope, const char *var_name)
{
    return varenv_get_var_ref(&scope->venv, var_name);
}

int scope_put_var(RubelScope *scope, Variable *var_obj)
{
    return varenv_set_var_ref(&scope->venv, var_obj);
}

/// SECTION: Scope Stack Impl.

int scopestack_init(ScopeStack *stack, int capacity)
{
    int checked_capacity = capacity;

    if (checked_capacity < SCOPE_STACK_SIZE) checked_capacity = SCOPE_STACK_SIZE;

    RubelScope **temp_scopes = malloc(sizeof(RubelScope *) * checked_capacity);

    if (!temp_scopes)
    {
        stack->stack_ptr = -1;
        stack->scopes = NULL;
        stack->capacity = 0;
        return 0;
    }
    
    for (size_t i = 0; i < checked_capacity; i++) temp_scopes[i] = NULL;

    stack->stack_ptr = 0;
    stack->scopes = temp_scopes;
    stack->capacity = checked_capacity;

    return 1;
}

void scopestack_destroy(ScopeStack *stack)
{
    if (stack->capacity == 0) return;

    RubelScope *target = NULL;

    do
    {
        target = scopestack_pop_scope(stack);

        scope_destroy(target);
        free(target);
    } while (!scopestack_is_empty(stack));
}

int scopestack_is_full(const ScopeStack *stack)
{
    return stack->stack_ptr == stack->capacity - 1;
}

int scopestack_is_empty(const ScopeStack *stack)
{
    return stack->stack_ptr == -1;
}

int scopestack_push_scope(ScopeStack *stack, RubelScope *scope)
{
    if (!scope || scopestack_is_full(stack)) return 0;

    stack->stack_ptr++;
    stack->scopes[stack->stack_ptr] = scope;

    return 1;
}

RubelScope *scopestack_pop_scope(ScopeStack *stack)
{
    if (scopestack_is_empty(stack)) return NULL;
 
    RubelScope *scope_ref = stack->scopes[stack->stack_ptr];
    stack->stack_ptr--;

    return scope_ref;
}
