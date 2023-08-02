#ifndef NATIVEFUNCS_H
#define NATIVEFUNCS_H

#include <stdio.h>
#include "backend/api/functions.h"

/// SECTION: macros

#define RUBEL_INPUT_READ_MAX 32

/// SECTION: module "io" natives

VarValue *rubel_print(FuncArgs *args);

VarValue *rubel_input(FuncArgs *args);

/// SECTION: module "lists" natives

VarValue *rubel_list_len(FuncArgs *args);

VarValue *rubel_list_at(FuncArgs *args);

// VarValue *rubel_list_set(FuncArgs *args); // TODO!

#endif