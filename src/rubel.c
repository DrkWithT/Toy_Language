#include "frontend/parser.h"
#include "backend/runner/interpreter.h"

/**
 * @file main.c 
 * @author Derek Tan
 * @brief Driver code for the Rubel command line utility.
 * @date 2023-07-22
 * @todo See README todos for tasks.
 */

/// SECTION: Debug code

// static const char *stmt_names[] = {
//     "MODULE_DEF",
//     "MODULE_USE",
//     "EXPR_STMT",
//     "VAR_DECL",
//     "VAR_ASSIGN",
//     "BLOCK_STMT",
//     "FUNC_DECL",
//     "WHILE_STMT",
//     "IF_STMT",
//     "OTHERWISE_STMT",
//     "BREAK_STMT",
//     "RETURN_STMT"
// };

// void print_stmt(const Statement *stmt)
// {
//     if (!stmt)
//     {
//         puts("(NULL)");
//         return;
//     }

//     printf("type \"%s\" stmt\n", stmt_names[stmt->type]);
// }

/// SECTION: Driver code

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("argc = %i, usage: rubel --[version | run] ?<file name>", argc);
        return 1;
    }

    if (strcmp(argv[1], "--version") == 0)
    {
        puts("Rubel v0.0.1\nBy DrkWithT");
        return 0;
    }

    if (strcmp(argv[1], "--run") != 0)
    {
        puts("Invalid argument passed to Rubel.");
        return 1;
    }

    // Load script file.
    char *source = load_file(argv[2]);

    if (!source)
    {
        printf("Failed to read source file %s\n", argv[2]);
        return 1;
    }

    // Use Parser.
    Parser parser;
    parser_init(&parser, source);

    Script *program = parser_parse_all(&parser, argv[2]);

    // Discard old copied source code string.
    free(source);
    source = NULL;

    if (!program)
    {
        printf("Failed to parse program. :(\n");
        free(source);
        return 1;
    }

    // for (size_t i = 0; i < program->count; i++)
    // {
    //     print_stmt(program->stmts[i]);
    // }

    /// Make and bind native modules.
    FuncGroup *io_module = funcgroup_create("io", 4);
    funcgroup_put(io_module, func_native_create("print", 1, rubel_print));
    funcgroup_put(io_module, func_native_create("input", 0, rubel_input));

    FuncGroup *lists_module = funcgroup_create("lists", 4);
    funcgroup_put(lists_module, func_native_create("at", 2, rubel_list_at));
    funcgroup_put(lists_module, func_native_create("length", 1, rubel_list_len));

    Interpreter prgm_runner;
    if (!interpreter_init(&prgm_runner, program))
    {
        puts("Failed to init interpreter.");
        return 1;
    }

    int loaded_io = interpreter_load_natives(&prgm_runner, io_module);
    int loaded_lists = interpreter_load_natives(&prgm_runner, lists_module);

    // Check if needed modules were loaded so execution is a bit safer.
    if (!loaded_io || !loaded_lists)
    {
        interpreter_dispose(&prgm_runner);
        return 1;
    }

    /// Test run interpreter.
    interpreter_run(&prgm_runner);

    interpreter_dispose(&prgm_runner);

    return 0;
}