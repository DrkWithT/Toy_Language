#ifndef VARENV_H
#define VARENV_H

#include "backend/values/vartypes.h"
#include "utils/hashing.h"

/// SECTION: Macros

#define VAR_ENV_SIZE 8

/// SECTION: Variable

typedef struct st_variable
{
    char *name;
    VarValue *value;
} Variable;

Variable *variable_create(char *var_name, VarValue *var_value);

void variable_destroy(Variable *var_obj);

int variable_is_const(const Variable *var_obj);

DataType variable_get_type(const Variable *var_obj);

/// SECTION: Variable Table Bucket List

typedef struct st_bucklist_node
{
    Variable *var;
    struct st_bucklist_node *next;
} EnvBuckListNode;

EnvBuckListNode *bucklistnode_create(Variable *var_obj, EnvBuckListNode *next_ptr);

void bucklistnode_destroy(EnvBuckListNode *node);

typedef struct st_env_bucklist
{
    size_t count;
    EnvBuckListNode *head;
    EnvBuckListNode *last;
} EnvBuckList;

EnvBuckList *envbucklist_create();

void envbucklist_destroy(EnvBuckList *bucklist);

const EnvBuckListNode *envbucklist_fetch(const EnvBuckList *bucklist, const char *var_name);

int envbucklist_append(EnvBuckList *bucklist, EnvBuckListNode *node);

/// SECTION: Variable Environment

typedef struct st_varenv
{
    size_t count;
    EnvBuckList **entries;
} VarEnv;

int varenv_init(VarEnv *venv, size_t buckets);

void varenv_destroy(VarEnv *venv);

Variable *varenv_get_var_ref(const VarEnv *venv, const char *var_name);

int varenv_set_var_ref(VarEnv *venv, Variable *var_obj);

#endif