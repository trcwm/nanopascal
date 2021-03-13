
#include <stdio.h>
#include "symtbl.h"

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

bool sym_init(symtbl_t *tbl)
{
    tbl->Nsymbols = 0;
}

bool sym_add(symtbl_t *tbl, const char *name, uint16_t namelen, uint16_t address)
{    
    tbl->syms[tbl->Nsymbols].namelen = namelen;
    tbl->syms[tbl->Nsymbols].name    = malloc(namelen);
    tbl->syms[tbl->Nsymbols].address = address;
    cpy(tbl->syms[tbl->Nsymbols].name, name, namelen);
    tbl->Nsymbols++;
    return true;
}

sym_t* sym_lookup(symtbl_t *tbl, const char *name, uint16_t namelen)
{
    for(uint16_t i=0; i<tbl->Nsymbols; i++)
    {
        if (namelen == tbl->syms[i].namelen)
        {
            if (cmp(name, tbl->syms[i].name, namelen))
            {
                return &tbl->syms[i];
            }
        }        
    }
    return NULL;
}

void sym_dump(symtbl_t *tbl)
{
    for(uint16_t i=0; i<tbl->Nsymbols; i++)
    {
        printf(";  ");
        for(uint16_t j=0; j<tbl->syms[i].namelen; j++)
        {
            putchar(tbl->syms[i].name[j]);
        }
        printf(" 0x%04X\n", tbl->syms[i].address);
    }
}
