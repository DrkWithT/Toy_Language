/**
 * @file nativefuncs.c
 * @author Derek Tan
 * @brief Implements Rubel language's native functions within modules "io", "lists", etc. These are bound on startup to interpreter function "groups" which represent native or user-defined modules.
 * @date 2023-08-01
 */

#include "backend/api/natives/nativefuncs.h"

/// SECTION: module io

VarValue *rubel_print(FuncArgs *args)
{
    Variable *arg1 = funcargs_get(args, 0);
    DataType arg1_type = arg1->value->type;
    VarValue *arg1_value = arg1->value;

    if (!arg1_value) return NULL;

    switch (arg1_type)
    {
    case INT_TYPE:
        printf("%i", arg1_value->data.int_val.value);
        break;
    case REAL_TYPE:
        printf("%f", arg1_value->data.real_val.value);
        break;
    case STR_TYPE:
        printf("%s", arg1_value->data.str_type.value->source);
        break;
    case BOOL_TYPE:
        if (arg1_value->data.bool_val.flag) printf("boolean($T)");
        else printf("boolean($F)");
        break;
    case LIST_TYPE:
        printf("list[%i]", arg1_value->data.list_type.value->count);
        break;
    default:
        break;
    }

    return arg1_value; // return the argument if it existed anyways... tells interpreter that print was OK
}

VarValue *rubel_input(FuncArgs *args)
{
    char *input_buffer = malloc(sizeof(char) * (RUBEL_INPUT_READ_MAX + 1));

    if (!input_buffer) return NULL; // NULL to signal memory or execution error

    memset(input_buffer, '\0', RUBEL_INPUT_READ_MAX + 1);
    
    if (!fgets(input_buffer, RUBEL_INPUT_READ_MAX, stdin))
    {
        free(input_buffer);
        return NULL;
    }

    StringObj *input_str = create_str_obj(input_buffer);
    VarValue *varval_result = create_str_varval(0, input_str);

    if (!varval_result)
    {
        destroy_str_obj(input_str);
        free(input_str);
        return NULL;
    }

    return varval_result;
}

/// SECTION: module lists

VarValue *rubel_list_len(FuncArgs *args)
{
    Variable *arg1 = funcargs_get(args, 0);
    DataType arg1_type = arg1->value->type;

    if (arg1_type != LIST_TYPE) return NULL;

    return create_int_varval(0, arg1->value->data.list_type.value->count);
}

VarValue *rubel_list_at(FuncArgs *args)
{
    Variable *arg1 = funcargs_get(args, 0);
    Variable *arg2 = funcargs_get(args, 1);
    DataType arg1_type = arg1->value->type;
    DataType arg2_type = arg2->value->type;

    if (arg1_type != LIST_TYPE || arg2_type != INT_TYPE) return NULL;

    return get_at_list_obj(arg1->value->data.list_type.value, (size_t)arg2->value->data.int_val.value);
}