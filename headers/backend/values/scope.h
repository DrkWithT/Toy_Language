#ifndef SCOPE_H
#define SCOPE_H

#include "backend/values/varenv.h"

#define SCOPE_IS_GLOBAL(scope_ptr) scope_ptr->parent == NULL

typedef struct st_scope
{
    struct st_scope *parent;
    VarEnv venv;
} RubelScope;

int scope_init(RubelScope *scope, RubelScope *parent);

void scope_destroy(RubelScope *scope);

Variable *scope_ref_var(const RubelScope *scope, const char *var_name);

int scope_put_var(RubelScope *scope, Variable *var_obj);

#endif