#include "frontend/token.h"

/**
 * @file token.c
 * @author Derek Tan
 * @brief Implements token functions.
 * @date 2023-07-22 
 */

void token_init(Token *token, TokenType type, size_t begin, size_t span, size_t line)
{
    token->type = type;
    token->begin = begin;
    token->span = span;
    token->line = line;
}

char *token_as_txt(Token *token, const char *source)
{
    const char *src_cursor = source + token->begin;
    size_t pending_chars = token->span;
    size_t copied_chars = 0;

    char *result = malloc(pending_chars + 1); // add 1 for null terminator

    if (!result)
        return result;

    while (pending_chars > 0)
    {
        *result = *src_cursor;

        result++;
        src_cursor++;
        pending_chars--;
        copied_chars++;
    }

    result -= copied_chars;
    
    return result;
}
