#include "frontend/parser.h"
// #include "frontend/parser.h"

/**
 * @file main.c 
 * @author Derek Tan
 * @brief Driver code for the parser with interpreter.
 * @date 2023-07-22
 */

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

    // char *source = read_file(argv[2]);

    // if (!source)
    // {
    //     printf("Failed to read source file %s\n", argv[2]);
    //     return 1;
    // }

    // free(source);

    return 0;
}