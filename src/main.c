#include "frontend/parser.h"

/**
 * @file main.c 
 * @author Derek Tan
 * @brief Driver code for the Rubel command line utility.
 * @date 2023-07-22
 * @todo See README todos for tasks.
 */

/// SECTION: Debug code

static const char *stmt_names[] = {
    "MODULE_DEF",
    "MODULE_USE",
    "EXPR_STMT",
    "VAR_DECL",
    "VAR_ASSIGN",
    "BLOCK_STMT",
    "FUNC_DECL",
    "WHILE_STMT",
    "IF_STMT",
    "OTHERWISE_STMT",
    "BREAK_STMT",
    "RETURN_STMT"
};

void print_stmt(const Statement *stmt)
{
    if (!stmt)
    {
        puts("(NULL)");
        return;
    }

    printf("type \"%s\" stmt\n", stmt_names[stmt->type]);
}

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

    if (!program)
    {
        printf("Failed to parse program. :(\n");
        free(source);
        return 1;
    }

    for (size_t i = 0; i < program->count; i++)
    {
        print_stmt(program->stmts[i]);
    }

    dispose_script(program);
    free(program);
    free(source);

    return 0;
}