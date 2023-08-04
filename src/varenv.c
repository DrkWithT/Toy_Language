/**
 * @file varenv.c
 * @author Derek Tan
 * @brief Implements variable environment for scopes.
 * @date 2023-07-30
 */

#include "backend/values/varenv.h"

/// SECTION: Variable impl.

Variable *variable_create(char *var_name, VarValue *var_value)
{
    Variable *var_obj = malloc(sizeof(Variable));

    if (var_obj != NULL)
    {
        var_obj->name = var_name;
        var_obj->value = var_value;
    }

    return var_obj;
}

void variable_destroy(Variable *var_obj)
{
    DataType val_type = var_obj->value->type;

    free(var_obj->name);
    var_obj->name = NULL;

    switch (val_type)
    {
    case STR_TYPE:
        destroy_str_obj(var_obj->value->data.str_type.value);
        var_obj->value->data.str_type.value = NULL;
        break;
    case LIST_TYPE:
        destroy_list_obj(var_obj->value->data.list_type.value);
        var_obj->value->data.list_type.value = NULL;
        break;
    default:
        break;
    }
}

int variable_is_const(const Variable *var_obj)
{
    return var_obj->value->is_const;
}

DataType variable_get_type(const Variable *var_obj)
{
    return var_obj->value->type;
}

/// SECTION: Variable Table Bucket List

EnvBuckListNode *bucklistnode_create(Variable *var_obj, EnvBuckListNode *next_ptr)
{
    EnvBuckListNode *node = malloc(sizeof(EnvBuckListNode));

    if (node != NULL)
    {
        node->var = var_obj;
        node->next = next_ptr;
    }

    return node;
}

void bucklistnode_destroy(EnvBuckListNode *node)
{
    variable_destroy(node->var);
}

EnvBuckList *envbucklist_create()
{
    EnvBuckList *bucklist = malloc(sizeof(EnvBuckList));

    if (bucklist != NULL)
    {
        bucklist->count = 0;
        bucklist->head = NULL;
        bucklist->last = NULL;
    }

    return bucklist;
}

void envbucklist_destroy(EnvBuckList *bucklist)
{
    if (bucklist->count == 0 || !bucklist->head) return;

    EnvBuckListNode *next_ptr = bucklist->head;

    while (bucklist->head != NULL)
    {
        next_ptr = next_ptr->next;
        bucklistnode_destroy(bucklist->head);
        free(bucklist->head);
        bucklist->head = next_ptr;
    }
}

const EnvBuckListNode *envbucklist_fetch(const EnvBuckList *bucklist, const char *var_name)
{
    EnvBuckListNode *node_ptr = bucklist->head;

    if (bucklist->count == 0 || !node_ptr) return NULL;

    do
    {
        if (strcmp(var_name, node_ptr->var->name) == 0) return node_ptr;

        node_ptr = node_ptr->next;
    } while (node_ptr != NULL);
    
    return NULL;
}

int envbucklist_append(EnvBuckList *bucklist, EnvBuckListNode *node)
{
    if (!node) return 0;

    if (bucklist->count == 0 || bucklist->head == NULL)
    {
        bucklist->head = node;
        bucklist->last = node;
        bucklist->count++;
        return 1;
    }

    bucklist->last->next = node;
    bucklist->last = bucklist->last->next;

    return 1;
}

/// SECTION: Variable Environment

int varenv_init(VarEnv *venv, size_t buckets)
{
    size_t checked_count = buckets;

    if (checked_count < VAR_ENV_SIZE) checked_count = VAR_ENV_SIZE;

    EnvBuckList **temp_entries = malloc(sizeof(EnvBuckList *) * checked_count);

    if (!temp_entries) return 0;

    for (size_t i = 0; i < checked_count; i++) temp_entries[i] = NULL;

    venv->entries = temp_entries;
    venv->count = checked_count;

    return 1;
}

void varenv_destroy(VarEnv *venv)
{
    if (venv->count == 0 || !venv->entries) return;

    EnvBuckList *target = NULL;
    size_t bucket_count = venv->count;

    for (size_t i = 0; i < bucket_count; i++)
    {
        target = venv->entries[i];
        envbucklist_destroy(target);
        free(target);
    }

    free(venv->entries);
    venv->entries = NULL;
}

Variable *varenv_get_var_ref(const VarEnv *venv, const char *var_name)
{
    size_t bucket_index = hash_key(var_name) % venv->count;
    EnvBuckList *bucket_chain = venv->entries[bucket_index];

    if (!bucket_chain) return NULL;

    if (bucket_chain->count == 1) return bucket_chain->head->var;

    const EnvBuckListNode *chain_node = envbucklist_fetch(bucket_chain, var_name);

    if (!chain_node) return NULL;

    return chain_node->var;
}

int varenv_set_var_ref(VarEnv *venv, Variable *var_obj)
{
    const char *var_obj_name = var_obj->name;
    size_t bucket_index = hash_key(var_obj_name) % venv->count;
    EnvBuckList *bucket_chain = venv->entries[bucket_index];
    EnvBuckListNode *node = bucklistnode_create(var_obj, NULL);

    if (!var_obj || !node) return 0; // NOTE: fail fast on NULL var or allocation failure for chained bucket node since NULL ptrs cannot be accessed anyway!

    if (!bucket_chain)
    {
        EnvBuckList *temp = envbucklist_create();

        if (!temp) return 0; // fail fast on allocation failure of bucket chain for same reason as node check 

        envbucklist_append(temp, node);
        venv->entries[bucket_index] = temp;

        return 1;
    }

    return envbucklist_append(bucket_chain, node);
}
