#include <stdbool.h>
#include <string.h>
#include "lex.h"

void lex_init(lex_context_t *context, const char *sourcePtr)
{
    context->src       = sourcePtr;
    context->srcidx    = 0;
    context->tokidx    = 0;
    context->tokstr[0] = 0;
    context->lstate    = LS_IDLE;
}

void lex_free(lex_context_t *context)
{

}

static bool isAlpha(char s)
{
    if ((s>='A') && (s<='Z'))
        return true;
    if ((s>='a') && (s<='z'))
        return true;
    return false;        
}

static bool isNumeric(char s)
{
    if ((s>='0') && (s<='9'))
        return true;  
    return false;
}

void lex_acceptchar(lex_context_t *context, char c)
{    
    if (context->tokidx < sizeof(context->tokstr))
    {
        context->tokstr[context->tokidx] = c;
        context->tokidx++;
    }
    context->srcidx++;
}

void lex_emit(lex_context_t *context, token_t tok)
{    
    context->curtok = tok;
    context->tokstr[context->tokidx] = 0;
    context->tokidx = 0;

    if (tok == TOK_IDENT)
    {
        if (strnlen(context->tokstr, sizeof(context->tokstr)) == 3)
        {
            if (strncmp(context->tokstr, "INT", 3)==0)
                context->curtok = TOK_INT;            
            else if (strncmp(context->tokstr, "LIT", 3)==0)
                context->curtok = TOK_LIT;
            else if (strncmp(context->tokstr, "OPR", 3)==0)
                context->curtok = TOK_OPR;
            else if (strncmp(context->tokstr, "LOD", 3)==0)
                context->curtok = TOK_LOD;
            else if (strncmp(context->tokstr, "STO", 3)==0)
                context->curtok = TOK_STO;
            else if (strncmp(context->tokstr, "CAL", 3)==0)
                context->curtok = TOK_CAL;                
            else if (strncmp(context->tokstr, "JMP", 3)==0)
                context->curtok = TOK_JMP;
            else if (strncmp(context->tokstr, "JPC", 3)==0)
                context->curtok = TOK_JPC;
        }
    }
}

void lex_next(lex_context_t *context)
{
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
            context->tokidx = 0;
            if (isAlpha(c))
            {
                lex_acceptchar(context, c);
                context->lstate = LS_IDENT;
            }
            else if (isNumeric(c))
            {
                lex_acceptchar(context, c);
                context->lstate = LS_INTEGER;
            }
            else if (c == 10)
            {
                lex_acceptchar(context, c);
                lex_emit(context, TOK_EOL);
                return;
            }
            else
            {
                // whitespace or something..
                // skip.
                context->srcidx++;
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
                lex_acceptchar(context, c);
            }
            break;
        case LS_IDENT:
            if (isNumeric(c) || isAlpha(c))
            {
                lex_acceptchar(context, c);                
            }
            else
            {
                lex_emit(context, TOK_IDENT);
                context->lstate = LS_IDLE;
                return;
            }
            break;
        default:
            context->lstate = LS_IDLE;
        }
    }
}