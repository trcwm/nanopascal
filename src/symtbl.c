#include <string.h>
#include <stdio.h>
#include "symtbl.h"

bool sym_init(symtbl_t *tbl)
{
    tbl->level    = 0;
    tbl->offset   = 0;
    tbl->Nsymbols = 0;
}

static void cpy(char *dst, const char *src, size_t len)
{
    for(uint8_t idx=0; idx<len; idx++)
    {
        dst[idx] = src[idx];
    }
}

static bool cmp(const char *s1, const char *s2, size_t len)
{
    for(uint8_t idx=0; idx<len; idx++)
    {
        if (s1[idx] != s2[idx])
            return false;
    }
    return true;
}

bool sym_update(symtbl_t *tbl, const uint16_t id, 
    const vartype_t tp, const vartype_t subtp,
    uint16_t size)
{
    if (id >= MAX_SYMS)
        return false;   // out of range

    sym_t *sym = &tbl->syms[id];

    sym->subtype = subtp;
    sym->type    = tp;
    sym->size    = size;
    
    // only variables are stored in the local stack context.
    // type NONE is a temporary type, which will be replaced
    // later in the parse cycle.
    if ((tp == TYPE_INT) || (tp == TYPE_CHAR) || (tp == TYPE_ARRAY) || (tp == TYPE_NONE))
    {
        sym->offset = tbl->offset;
        tbl->offset += size;
    }
    else
    {
        sym->offset = 0;
    }

    return true;
}

bool sym_set_const(symtbl_t *tbl, const uint16_t id, const uint16_t value)
{
    if (id >= MAX_SYMS)
        return false;   // out of range

    sym_t *sym = &tbl->syms[id];

    sym->subtype = TYPE_NONE;
    sym->type    = TYPE_CONST;
    sym->size    = 0;
    sym->offset  = value;

    return true;   
}

bool sym_set_procedure(symtbl_t *tbl, const uint16_t id, const uint16_t label)
{
    if (id >= MAX_SYMS)
        return false;   // out of range

    sym_t *sym = &tbl->syms[id];

    sym->subtype = TYPE_NONE;
    sym->type    = TYPE_PROCEDURE;
    sym->size    = 0;
    sym->offset  = label;

    return true;   
}

bool sym_add(symtbl_t *tbl, const char *name, uint16_t namelen)
{
    //FIXME: check if the symbol is already in the table?
    sym_t *newsym = &tbl->syms[tbl->Nsymbols];

    newsym->level   = tbl->level;
    newsym->subtype = TYPE_NONE;
    newsym->type    = TYPE_NONE;
    newsym->size    = 0;
    newsym->offset  = 0;
    newsym->name    = (char*)malloc(namelen);
    newsym->namelen= namelen;
    cpy(newsym->name, name, namelen);

    tbl->Nsymbols++;

    return true;
}

sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen)
{
    // backward search for the symbol
    uint16_t idx = tbl->Nsymbols;

    bool found = false;    
    while(idx != 0)
    {
        idx--;
        if (tbl->syms[idx].namelen == namelen)
        {
            if (cmp(tbl->syms[idx].name, name, namelen))
            {
                // match!
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        return &tbl->syms[idx];
    }
    else
    {
        return NULL;
    }
}

bool sym_enter(symtbl_t *tbl)
{
    tbl->offset=0;
    tbl->level++;
    if (tbl->level > 15)
    {
        // error! too many levels.
    }
    return true;
}

void free_sym(sym_t *s)
{
    free(s->name);
}

bool sym_leave(symtbl_t *tbl)
{
    // do backward search to find previous stack level
    uint16_t idx = tbl->Nsymbols;

    if (idx == 0)
    {
        // can't leave top-level context
        return false;
    }

    while(idx != 0)
    {
        idx--;
        if (tbl->syms[idx].level == tbl->level)
        {
            free_sym(&(tbl->syms[idx]));
            tbl->Nsymbols--;
        }
        else
        {
            // new context found, so exit
            break;
        }
    };

    if (tbl->level > 0)
    {
        tbl->level--;
    }
    else
    {
        // error!
        return false;
    }
    return true;
}

void sym_dump(symtbl_t *tbl)
{
    printf("; Dumping symbol table\n");
    for(uint16_t idx=0; idx<tbl->Nsymbols; idx++)
    {
        printf("; ");
        const sym_t *s = &(tbl->syms[idx]);
        for(uint8_t L=0; L<s->level; L++)
        {
            putchar(' ');
        }
        for(uint8_t i=0; i<s->namelen; i++)
        {
            putchar(s->name[i]);
        }
        
        const uint8_t remainder = 20-s->namelen;
        for(uint8_t i=0; i<remainder; i++)
        {
            putchar(' ');
        }

        switch(s->type)
        {
        case TYPE_CONST:
            printf("CONST %d\n", s->offset);
            break;
        case TYPE_INT:
            printf("INT   %d\n", s->offset);
            break;
        case TYPE_CHAR:
            printf("CHAR  %d (%c)\n", s->offset, (char)s->offset);
            break;
        case TYPE_ARRAY:
            printf("ARRAY %d [%d]\n", s->offset, s->size);
            break;                        
        case TYPE_PROCEDURE:
            printf("PROC  @L%d\n", s->offset);
            break;        
        default:
            printf("?????");
            break;                    
        }
    }
}

uint16_t sym_numvariables(symtbl_t *tbl)
{
    uint16_t idx   = tbl->Nsymbols;
    uint16_t count = 0;
    while(idx > 0)
    {
        idx--;
        if (tbl->syms[idx].level != tbl->level)
        {
            return count;
        }
        else
        {
            if (tbl->syms[idx].type == TYPE_INT)
                count++;
        }
    }
    return count;
}

uint16_t sym_get_local_space(symtbl_t *tbl)
{
    uint16_t idx   = tbl->Nsymbols;
    uint16_t space = 0;
    while(idx > 0)
    {
        idx--;
        if (tbl->syms[idx].level != tbl->level)
        {
            return space;
        }
        else
        {
            if ((tbl->syms[idx].type == TYPE_INT) || (tbl->syms[idx].type == TYPE_CHAR) 
                || (tbl->syms[idx].type == TYPE_ARRAY))
            {
                space += tbl->syms[idx].size;
            }
        }
    }
    return space;
}

void sym_settype(symtbl_t *tbl, const vartype_t tp, const vartype_t subtp, const uint16_t size)
{
    uint16_t idx = tbl->Nsymbols;
    while(idx > 0)
    {
        idx--;
        if (tbl->syms[idx].type == TYPE_NONE)
        {
            tbl->syms[idx].type    = tp;
            tbl->syms[idx].subtype = subtp;
            tbl->syms[idx].size    = size;
        }
        else
        {
            return;
        }
    }
}
