#include <stdbool.h>
#include "lexer.h"

void lexer_init(lexer_context_t *context, char *source)
{
    context->src      = source;
    context->tokstart = source;
    context->toklen   = 0;
    context->token    = TOK_NONE;
    context->state    = LS_IDLE;
}

bool isWhitespace(const char c)
{
    return (c==' ') || (c=='\t');
}

bool isAlpha(const char c)
{
    return ((c>='A') && (c<='Z')) || ((c>='a') && (c<='z'));
}

bool isNumeric(const char c)
{
    return (c>='0') && (c<='9');
}

bool isAlphaNum(const char c)
{
    return isAlpha(c) || isNumeric(c);
}

void lexer_accept(lexer_context_t *context)
{
    context->toklen++;
}

void lexer_skip(lexer_context_t *context)
{
    context->tokstart++;
    context->toklen = 0;
}

void lexer_emit(lexer_context_t *context, token_t tok)
{
    context->token = tok;
    context->state = LS_IDLE;
}

bool lexer_next(lexer_context_t *context)
{    
    // advance past the previously emitted token
    context->tokstart += context->toklen;
    context->toklen    = 0;

    while(1)
    {
        char c = context->tokstart[context->toklen];

        if (c == 0)
        {
            lexer_emit(context, TOK_EOF);
            return true;
        }

        switch(context->state)
        {
        case LS_IDLE:
            context->toklen = 0;

            // skip whitespace
            if (isWhitespace(c))
            {
                lexer_skip(context);
            }
            else if (isAlpha(c))
            {
                context->state = LS_IDENT;
                lexer_accept(context);
            }
            else if (isNumeric(c))
            {
                context->state = LS_INTEGER;
                lexer_accept(context);
            }
            else if (c == 10)
            {
                lexer_accept(context);
                lexer_emit(context, TOK_EOL);
                return true;
            }
            else
            {
                // some kind of operator?
                switch(c)
                {
                default:
                    // skip everything else
                    lexer_skip(context);
                }
            }
            break;
        case LS_IDENT:
            if (isAlphaNum(c))
            {
                lexer_accept(context);
            }
            else
            {
                lexer_emit(context, TOK_IDENT);
                context->state = LS_IDLE;
                return true;
            }
            break;
        case LS_INTEGER:
            if (isNumeric(c))
            {
                lexer_accept(context);
            }
            else
            {
                lexer_emit(context, TOK_INTEGER);
                context->state = LS_IDLE;
                return true;
            }
            break;
        default:
            return false; // incorrect lexer state       
        }
    }
}
