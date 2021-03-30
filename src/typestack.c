#include "typestack.h"

void ts_error(const char *txt)
{
    fprintf(stderr,"typestack: %s\n",txt);
}

void ts_init(typestack_t *stk)
{
    stk->stackptr = 0;
}

bool ts_push(typestack_t *stk, vartype_t t)
{
    if (stk == NULL)
        return false;

    if (stk->stackptr >= STACKMAXDEPTH)
    {
        ts_error("stack overflow!");
        return false;
    }
    else
    {
        stk->stack[stk->stackptr] = t;
        stk->stackptr++;
        return true;
    }
}

bool ts_pop(typestack_t *stk)
{
    if (stk == NULL)
        return false;

    if (stk->stackptr == 0)
    {
        ts_error("stack underflow!");
        return false;
    }
    else
    {
        stk->stackptr--;
        return true;
    }
}

vartype_t ts_item(const typestack_t *stk, uint8_t offset)
{
    if (stk == NULL)
        return TYPE_ERROR;

    if ((stk->stackptr - offset - 1) < STACKMAXDEPTH)
    {
        return stk->stack[stk->stackptr - offset - 1];
    }
    else
    {
        ts_error("stack item under/overflow!");
        return TYPE_ERROR;
    }
}

void ts_dump(const typestack_t *stk)
{
    uint8_t idx = stk->stackptr;
    while(idx > 0)
    {
        idx--;
        switch(stk->stack[stk->stackptr - idx])
        {
        case TYPE_INT:
            printf("  INT\n");
            break;
        case TYPE_CONST:
            printf("  CONST\n");
            break;
        case TYPE_CHAR:
            printf("  CHAR\n");
            break;
        case TYPE_ARRAY:
            printf("  ARRAY\n");
            break;            
        default:
            printf("  ???\n");
            break;
        }
    }
}
