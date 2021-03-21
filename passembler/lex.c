/*

    Lexer for p-code assembler
    N.A. Moseley (c) 2021

*/

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "lex.h"

void lex_init(lex_context_t *context, const char *sourcePtr)
{
    context->src       = sourcePtr;
    context->srcidx    = 0;
    context->toklen    = 0;
    context->tokstr    = sourcePtr;
    context->lstate    = LS_IDLE;
    context->linenum   = 1;
    context->curtok    = TOK_UNDEFINED;
}

void lex_free(lex_context_t *context)
{

}

static bool isNumeric(char s)
{
    if ((s>='0') && (s<='9'))
        return true;  
    return false;
}

static bool isAlpha(char s)
{
    if ((s>='A') && (s<='Z'))
        return true;
    if ((s>='a') && (s<='z'))
        return true;
    return false;        
}

static uint8_t hex2dec(const char s)
{
    if ((s>='0') && (s<='9'))
        return s-'0';
    if ((s >= 'A') && (s <='F'))
        return s - 'A' + 10;
    if ((s >= 'a') && (s <='f'))
        return s - 'a' + 10;
    return 255;
}

void lex_acceptchar(lex_context_t *context)
{    
    context->toklen++;
    context->srcidx++;
}

void lex_ignorechar(lex_context_t *context)
{    
    context->srcidx++;
}

char lex_toupper(const char c)
{
    if ((c >= 'a') && (c <='z'))
    {
        return c - 'a' + 'A';
    }
}

// check if the current token string is a keyword
// and set the token accordingly
void lex_checkKeyword(lex_context_t *context)
{
    for(int kwindex=0; kwindex<NKEYWORDS; kwindex++)
    {
        const char *kw = keywords[kwindex];
        
        if (kw[context->toklen] != 0)
        {
            // keyword and token are not the same length
            // so skip this keyword
            continue;
        }

        bool fail = false;        
        for(int cindex=0; cindex < context->toklen; cindex++)
        {
            if (kw[cindex] != lex_toupper(context->tokstr[cindex]))
            {
                fail = true;
                break;
            }
        }
        
        if (fail)
            continue;

        context->curtok = 100 + kwindex;
        return;
    }
}

void lex_emit(lex_context_t *context, token_t tok)
{   
    context->curtok = tok;
    if (tok == TOK_IDENT)
    {
        lex_checkKeyword(context);       
    }
}

void lex_next(lex_context_t *context)
{   
    uint8_t tmp;
    while(1)
    {
        char c = context->src[context->srcidx];
        if (c == 0)
        {
            lex_emit(context, TOK_EOF);
            return;
        }

        switch(context->lstate)
        {
        case LS_IDLE:
            context->toklen = 0;
            context->tokstr = context->src + context->srcidx;
            if (isAlpha(c))
            {
                lex_acceptchar(context);
                context->lstate = LS_IDENT;
            }
            else if (isNumeric(c))
            {                
                lex_acceptchar(context);
                context->lit = c - '0';
                context->lstate = LS_INTEGER;
            }
            else if (c == 10)
            {
                lex_acceptchar(context);
                lex_emit(context, TOK_EOL);
                context->linenum++;
                return;
            }
            else if (c == '@')
            {
                context->lit = 0;
                context->lstate = LS_LABEL;
                context->tokstr++;
                lex_ignorechar(context);
            }
            else if (c == '$')
            {
                context->lit = 0;
                context->lstate = LS_HEX;
                lex_ignorechar(context);
            }
            else if (c == ';')
            {
                // line comment
                context->lstate = LS_COMMENT;
                lex_ignorechar(context);
            }
            else
            {
                // whitespace or something..
                // skip.
                lex_ignorechar(context);
            }
            break;
        case LS_HEX:
            tmp = hex2dec(c);
            if (tmp == 255) // not hex
            {
                lex_emit(context, TOK_INTEGER);
                context->lstate = LS_IDLE;
                return;
            }
            else
            {
                context->lit <<= 4;
                context->lit |= tmp;
                lex_acceptchar(context);
            }
            break;
        case LS_INTEGER:
            if (!isNumeric(c))
            {
                lex_emit(context, TOK_INTEGER);
                context->lstate = LS_IDLE;
                return;
            }
            else
            {
                uint16_t orig = context->lit;
                context->lit <<= 2;     // times 4
                context->lit += orig;   // times 5
                context->lit <<= 1;     // times 10
                context->lit += c - '0';                
                lex_acceptchar(context);
            }
            break;            
        case LS_IDENT:
            if (isNumeric(c) || isAlpha(c))
            {
                lex_acceptchar(context);
            }
            else
            {
                lex_emit(context, TOK_IDENT);
                context->lstate = LS_IDLE;
                return;
            }
            break;
        case LS_LABEL:
            if (isNumeric(c) || isAlpha(c))
            {
                lex_acceptchar(context);
            }
            else
            {
                lex_emit(context, TOK_LABEL);
                context->lstate = LS_IDLE;
                return;
            }
            break;
        case LS_COMMENT:
            if (c != 10)
            {
                lex_ignorechar(context);                
            }
            else
            {
                context->lstate = LS_IDLE;
            }
            break;        
        default:
            context->lstate = LS_IDLE;
        }
    }
}
