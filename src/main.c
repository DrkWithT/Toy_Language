#include <stdio.h>
#include "frontend/lexer.h"

char *read_file(const char *file_path)
{
    // Check if file can be read.
    if (file_path == NULL) return NULL;

    FILE *freader = fopen(file_path, "r");

    if (!freader) return NULL;

    size_t file_len = 0;
    char *result = NULL;

    // Get file length to know how much to read as a c-string.
    fseek(freader, 0, SEEK_END);
    file_len = ftell(freader);
    fseek(freader, 0, SEEK_SET);

    result = malloc(file_len + 1);

    // Read N bytes or characters...
    size_t copy_count = fread(result, 1, file_len, freader);
    result[copy_count] = '\0';

    fclose(freader);
    freader = NULL;

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("argc = %i, usage: rubel --[version | run] ?<file name>", argc);
        return 1;
    }

    if (strcmp(argv[1], "--version") == 0)
    {
        puts("Rubel 0.0.1 by DerkWithT");
        return 0;
    }

    if (strcmp(argv[1], "--run") != 0)
    {
        puts("Invalid argument passed to Rubel.");
        return 1;
    }

    char *source = read_file(argv[2]);

    if (!source)
    {
        printf("Failed to read source file %s\n", argv[2]);
        return 1;
    }

    Token temp;
    Lexer lexer;
    lexer_init(&lexer, source);

    do
    {
        temp = lexer_next_token(&lexer);

        if (temp.type == EOS) break;
        
        char *lexeme = token_as_txt(&temp, source);
        printf("Token {type: %i, txt: \"%s\"}\n", temp.type, lexeme);

        free(lexeme);
    } while (1);

    free(source);

    return 0;
}