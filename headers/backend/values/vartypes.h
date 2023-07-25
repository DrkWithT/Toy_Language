#ifndef VARTYPES_H
#define VARTYPES_H

#include <stdlib.h>

typedef enum
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
    char *name;
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

VarValue *create_str_varval(int is_const, char *content);

VarValue *create_list_varval(int is_const, void *value);

typedef struct st_str_obj
{
    size_t capacity;
    size_t length;
    char *source;
} StringObj;

StringObj *create_str_obj(char *source);

StringObj *index_str_obj(StringObj *str, size_t index);

void destroy_str_obj(StringObj *str);

int concat_str_obj(StringObj *str, StringObj *other);

typedef struct st_list_obj
{
    size_t count;
    ListObj *head;
    ListObj *last;
} ListObj;

ListObj *create_list_obj();

void destroy_list_obj(ListObj *list);

size_t lengthof_str_obj(const ListObj *list);

int append_list_obj(ListObj *list, VarValue *data);

typedef struct st_list_node_obj
{
    VarValue *value;
    ListNodeObj *next;
} ListNodeObj;

void list_node_obj_init(ListNodeObj *list_node, VarValue *data, ListNodeObj *next);

#endif