/**
 * @file vartypes.c
 * @author Derek Tan
 * @brief Implements interpreter value structures.
 * @date 2023-07-25
 */

#include "backend/values/vartypes.h"

/// SECTION: Variables

VarValue *create_bool_varval(int is_const, int flag)
{
    VarValue *boolvar = malloc(sizeof(VarValue));

    if (boolvar != NULL)
    {
        boolvar->type = BOOL_TYPE;
        boolvar->is_const = is_const;
        boolvar->data.bool_val.flag = flag;
    }

    return boolvar;
}

VarValue *create_int_varval(int is_const, int value)
{
    VarValue *intval = malloc(sizeof(VarValue));

    if (intval != NULL)
    {
        intval->type = INT_TYPE;
        intval->is_const = is_const;
        intval->data.int_val.value = value;
    }

    return intval;
}

VarValue *create_real_varval(int is_const, float value)
{
    VarValue *realval = malloc(sizeof(VarValue));

    if (realval != NULL)
    {
        realval->type = REAL_TYPE;
        realval->is_const = is_const;
        realval->data.real_val.value = value;
    }

    return realval;
}

VarValue *create_str_varval(int is_const, struct st_str_obj *value)
{
    VarValue *strval = malloc(sizeof(VarValue));

    if (strval != NULL)
    {
        strval->type = STR_TYPE;
        strval->is_const = is_const;
        strval->data.str_type.value = value;
    }

    return strval;
}

VarValue *create_list_varval(int is_const, struct st_list_obj *value)
{
    VarValue *strval = malloc(sizeof(VarValue));

    if (strval != NULL)
    {
        strval->type = LIST_TYPE;
        strval->is_const = is_const;
        strval->data.list_type.value = value;
    }

    return strval;
}

DataType varval_get_type(const VarValue *variable)
{
    return variable->type;
}

int varval_is_const(const VarValue *variable)
{
    return variable->is_const;
}

/// SECTION: StringObj

StringObj *create_str_obj(char *source)
{
    StringObj *str_obj = malloc(sizeof(StringObj));

    if (str_obj != NULL)
    {
        str_obj->source = source;
        str_obj->length = strlen(source);
    }

    return str_obj;
}

StringObj *index_str_obj(StringObj *str, size_t index)
{
    size_t parent_length = str->length;
    const char *str_view = str->source;
    char *buffer = NULL;
    StringObj *str_obj = NULL;

    if (index >= parent_length)
        return NULL;

    buffer = malloc(sizeof(char) * 2);

    if (!buffer)
        return NULL;

    str_obj = malloc(sizeof(StringObj));

    if (str_obj != NULL)
    {
        str_obj->length = 1;
        str_obj->source = buffer;
    }

    return str_obj;
}

void destroy_str_obj(StringObj *str)
{
    if (str->source != NULL)
    {
        free(str->source);
        str->source = NULL;
    }

    str->length = 0;
}

StringObj *concat_str_obj(StringObj *str, StringObj *other)
{
    size_t target_length = str->length;
    size_t total_length = target_length + other->length;
    char *new_buffer = NULL;

    // don't reallocate for a zero-length string append
    if (total_length == target_length)
        return str;

    new_buffer = realloc(str->source, total_length + 1);

    if (!new_buffer)
        return NULL; // NOTE: return null on any value errors for later null safety checks!

    strncpy(new_buffer + target_length, other->source, total_length);
    new_buffer[total_length] = '\0';

    return str;
}

/// SECTION: ListNodeObj

ListNodeObj *create_listnode_obj(VarValue *data, ListNodeObj *next)
{
    ListNodeObj *node = malloc(sizeof(ListNodeObj));

    if (node != NULL)
    {
        node->value = data;
        node->next = next;
    }

    return node;
}

void destroy_listnode_obj(ListNodeObj *node)
{
    VarValue *data = node->value;

    if (!data)
        return;

    switch (data->type)
    {
    case STR_TYPE:
        destroy_str_obj(data->data.str_type.value);
        free(data->data.str_type.value);
        data->data.str_type.value = NULL;
        break;
    case LIST_TYPE:
        destroy_list_obj(data->data.list_type.value);
        free(data->data.list_type.value);
        data->data.list_type.value = NULL;
        break;
    default:
        break;
    }
}

/// SECTION: ListObj

ListObj *create_list_obj()
{
    ListObj *list = malloc(sizeof(ListObj));

    if (list != NULL)
    {
        list->count = 0;
        list->head = NULL;
        list->last = NULL;
    }

    return list;
}

void destroy_list_obj(ListObj *list)
{
    if (list->count < 1)
        return;
    
    ListNodeObj *next_ptr = list->head;

    do
    {
        next_ptr = next_ptr->next;
        destroy_listnode_obj(list->head);
        free(list->head);
        list->head = next_ptr;
    } while (list->head != NULL);

    list->last = NULL;
    list->count = 0;
}

int append_list_obj(ListObj *list, VarValue *data)
{
    ListNodeObj *temp = create_listnode_obj(data, NULL);
    
    if (!temp)
        return 0;

    if (list->count == 0)
    {
        list->head = temp;
        list->last = temp;
        list->count++;

        return 1;
    } 

    list->last->next = temp;
    list->last = list->last->next;
    list->count++;

    return 1;
}
