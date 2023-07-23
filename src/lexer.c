#include "frontend/lexer.h"

void lexer_init(Lexer *lexer, char *source)
{
    lexer->src = source;
    lexer->pos = 0;
    lexer->limit = strlen(source);
    lexer->line = 1;
}

Token lexer_lex_wspace(Lexer *lexer)
{
    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (MATCH_CHAR(c, '\n')) lexer->line++;
        
        if (!(IS_WSP(c)) || MATCH_CHAR(c, '\0')) break;

        src_cursor++;
        span++;
    }

    lexer->pos += span;

    return (Token){.type = WSPACE, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_comment(Lexer *lexer)
{
    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (MATCH_CHAR(c, '\n') || MATCH_CHAR(c, '\0')) break;
        
        src_cursor++;
        span++;
    }

    lexer->pos += span;

    return (Token){.type = COMMENT, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_single(Lexer *lexer, TokenType type)
{
    size_t begin = lexer->pos;
    size_t span = 1;

    lexer->pos++;

    return (Token){.type = type, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_keyword(Lexer *lexer, const char *keyword)
{
    char c;
    char keyword_c;
    size_t begin = lexer->pos;
    size_t span = 0;
    size_t kw_span = strlen(keyword);
    char *src_cursor = lexer->src + begin;
    const char *kw_cursor = keyword;
    int mismatch = 0;

    while (span <= lexer->limit)
    {
        c = *src_cursor;
        keyword_c = *kw_cursor;

        if (MATCH_CHAR(keyword_c, '\0')) break;

        if (c != keyword_c) mismatch = 1;

        src_cursor++;
        kw_cursor++;
        span++;
    }

    lexer->pos += span; // TODO: fix above and below checks for keyword or identifier.

    // Case 1: keyword matches token's spanned lexeme.
    if (span == kw_span && !mismatch)
        return (Token){.type = KEYWORD, .begin = begin, .span = span, .line = lexer->line};

    // Case 2: a lexical or length mismatch was found... backtrack to token start and invoke the identifier helper function!
    lexer->pos = begin;
    
    return lexer_lex_identifier(lexer);
}

Token lexer_lex_identifier(Lexer *lexer)
{
    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (!(IS_ALPHA(c)) || MATCH_CHAR(c, '\0')) break;

        src_cursor++;
        span++;
    }

    lexer->pos += span;

    return (Token){.type = IDENTIFIER, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_boolean(Lexer *lexer)
{
    lexer->pos++;  // Skip beginning mark of boolean: a '$'.

    size_t begin = lexer->pos;
    char c = lexer->src[begin];
    size_t span = 0;

    // Handle case of cut-off boolean at EOF.
    if (MATCH_CHAR(c, '\0'))
    {
        lexer->pos++;
        return (Token){.type = UNKNOWN, .begin = begin - 1, .span = 1, .line = lexer->line};
    }

    // Handle valid cases of $T or $F but minus 1 for counting the '$'.
    if (MATCH_CHAR(c, 'T'))
    {
        lexer->pos++;
        return (Token){.type = BOOLEAN, .begin = begin - 1, .span = 2, .line = lexer->line};
    }

    if (MATCH_CHAR(c, 'F'))
    {
        lexer->pos++;
        return (Token){.type = BOOLEAN, .begin = begin - 1, .span = 2, .line = lexer->line};
    }

    lexer->pos++;

    return (Token){.type = UNKNOWN, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_number(Lexer *lexer)
{
    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;
    int dot_count = 0;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (MATCH_CHAR(c, '.')) dot_count++;

        if (!(IS_NUMERIC(c)) || MATCH_CHAR(c, '\0')) break;

        src_cursor++;
        span++;
    }

    lexer->pos += span;

    // Case 1: 0 dots means an integer.
    if (dot_count == 0)
        return (Token){.type = INTEGER, .begin = begin, .span = span, .line = lexer->line};

    // Case 2: 1 dot means a decimal.
    if (dot_count == 1)
        return (Token){.type = REAL, .begin = begin, .span = span, .line = lexer->line};

    // Case 3: 2+ dots means garbage.
    return (Token){.type = UNKNOWN, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_string(Lexer *lexer)
{
    lexer->pos++; // Skip 1st double quote to avoid infinite loop!

    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;
    int invalid_closing = 0;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (MATCH_CHAR(c, '\"')) break;

        if (MATCH_CHAR(c, '\0'))
        {
            // If no ending double quote is found, the string is invalid. Mark this error.
            invalid_closing = 1;
            break;
        }

        src_cursor++;
        span++;
    }

    lexer->pos += span + 1; // Skip past last quote symbol to avoid a bad lexing.

    if (invalid_closing)
        return (Token){.type = UNKNOWN, .begin = begin, .span = span, .line = lexer->line};
    
    return (Token){.type = STRBODY, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_lex_operator(Lexer *lexer)
{
    char c;
    size_t begin = lexer->pos;
    size_t span = 0;
    char *src_cursor = lexer->src + begin;

    while (span <= lexer->limit)
    {
        c = *src_cursor;

        if (!(IS_OP_CHAR(c))) break;

        src_cursor++;
        span++;
    }

    lexer->pos += span;

    return (Token){.type = OPERATOR, .begin = begin, .span = span, .line = lexer->line};
}

Token lexer_next_token(Lexer *lexer)
{
    /// At end of source code, terminate the tokens with EOS (end of source)!
    if (lexer->pos >= lexer->limit)
        return (Token){.type = EOS, .begin = lexer->pos, 1, .line = lexer->line};

    char c = lexer->src[lexer->pos];

    // Handle usual tokens.
    if (IS_WSP(c)) return lexer_lex_wspace(lexer);
    else if (MATCH_CHAR(c, '#')) return lexer_lex_comment(lexer);
    else if (MATCH_CHAR(c, 'u')) return lexer_lex_keyword(lexer, "use");
    else if (MATCH_CHAR(c, 'l')) return lexer_lex_keyword(lexer, "let");
    else if (MATCH_CHAR(c, 'c')) return lexer_lex_keyword(lexer, "const");
    else if (MATCH_CHAR(c, 'p')) return lexer_lex_keyword(lexer, "proc");
    else if (MATCH_CHAR(c, 'i')) return lexer_lex_keyword(lexer, "if");
    else if (MATCH_CHAR(c, 'o')) return lexer_lex_keyword(lexer, "otherwise");
    else if (MATCH_CHAR(c, 'w')) return lexer_lex_keyword(lexer, "while");
    else if (MATCH_CHAR(c, 'e')) return lexer_lex_keyword(lexer, "end");
    else if (MATCH_CHAR(c, 'r')) return lexer_lex_keyword(lexer, "return");
    else if (IS_OP_CHAR(c)) return lexer_lex_single(lexer, OPERATOR);
    else if (IS_ALPHA(c)) return lexer_lex_identifier(lexer);
    else if (MATCH_CHAR(c, '$')) return lexer_lex_boolean(lexer);
    else if (IS_NUMERIC(c)) return lexer_lex_number(lexer);
    else if (MATCH_CHAR(c, '\"')) return lexer_lex_string(lexer);
    else if (MATCH_CHAR(c, '[')) return lexer_lex_single(lexer, LBRACK);
    else if (MATCH_CHAR(c, ']')) return lexer_lex_single(lexer, RBRACK);
    else if (MATCH_CHAR(c, '(')) return lexer_lex_single(lexer, LPAREN);
    else if (MATCH_CHAR(c, ')')) return lexer_lex_single(lexer, RPAREN);
    else if (MATCH_CHAR(c, ',')) return lexer_lex_single(lexer, COMMA);
    return lexer_lex_single(lexer, UNKNOWN);
}
