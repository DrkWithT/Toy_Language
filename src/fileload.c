/**
 * @file fileload.c
 * @author Derek Tan
 * @brief Implements file reader function.
 * @date 2023-07-23
 */

#include "frontend/fileload.h"

char *load_file(const char *file_path)
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
