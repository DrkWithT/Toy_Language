#ifndef VARTYPES_H
#define VARTYPES_H

#include <stdlib.h>
#include <string.h>

typedef enum en_data_type
{
    BOOL_TYPE,
    INT_TYPE,
    REAL_TYPE,
    STR_TYPE,
    LIST_TYPE
} DataType;

/**
 * @brief Hybrid structure to represent literal or variable values.
 */
typedef struct var
{
    DataType type;
    int is_const;
    union
    {
        struct
        {
            int flag;
        } bool_val;

        struct
        {
            int value;
        } int_val;

        struct
        {
            /* data */
            float value;
        } real_val;

        struct
        {
            struct st_str_obj *value;
        } str_type;
        
        struct
        {
            struct st_list_obj *value;
        } list_type;
    } data;
} VarValue;

VarValue *create_bool_varval(int is_const, int flag);

VarValue *create_int_varval(int is_const, int value);

VarValue *create_real_varval(int is_const, float value);

VarValue *create_str_varval(int is_const, struct st_str_obj *value);

VarValue *create_list_varval(int is_const, struct st_list_obj *value);

void varval_destroy(VarValue *value);

DataType varval_get_type(const VarValue *variable);

int varval_is_const(const VarValue *variable);

typedef struct st_str_obj
{
    size_t length;
    char *source;
} StringObj;

StringObj *create_str_obj(char *source);

void destroy_str_obj(StringObj *str);

StringObj *copy_str_obj(const StringObj *str);

StringObj *index_str_obj(StringObj *str, size_t index);

StringObj *concat_str_obj(StringObj *str, StringObj *other);

typedef struct st_list_node_obj
{
    VarValue *value;
    struct st_list_node_obj *next;
} ListNodeObj;

ListNodeObj *create_listnode_obj(VarValue *data, ListNodeObj *next);

void destroy_listnode_obj(ListNodeObj *node);

typedef struct st_list_obj
{
    size_t count;
    ListNodeObj *head; // for fast prepend and destroying
    ListNodeObj *last; // for fast append
} ListObj;

ListObj *create_list_obj();

void destroy_list_obj(ListObj *list);

int append_list_obj(ListObj *list, VarValue *data);

// VarValue *pop_list_obj(ListObj *list);

VarValue *get_at_list_obj(const ListObj *list, size_t index);

#endif