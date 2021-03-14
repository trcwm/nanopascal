
#include <stdio.h>
#include "fixuptbl.h"

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

bool fix_init(fixtbl_t *tbl)
{
    tbl->Nentries = 0;
}

bool fix_add(fixtbl_t *tbl, const char *name, uint16_t namelen, uint16_t address)
{    
    tbl->fixups[tbl->Nentries].namelen = namelen;
    tbl->fixups[tbl->Nentries].name    = malloc(namelen);
    tbl->fixups[tbl->Nentries].address = address;
    cpy(tbl->fixups[tbl->Nentries].name, name, namelen);
    tbl->Nentries++;
    return true;
}

fixup_t* fix_lookup(fixtbl_t *tbl, const char *name, uint16_t namelen)
{
    for(uint16_t i=0; i<tbl->Nentries; i++)
    {
        if (namelen == tbl->fixups[i].namelen)
        {
            if (cmp(name, tbl->fixups[i].name, namelen))
            {
                return &tbl->fixups[i];
            }
        }        
    }
    return NULL;
}

void fix_dump(fixtbl_t *tbl)
{
    for(uint16_t i=0; i<tbl->Nentries; i++)
    {
        printf(";  ");
        for(uint16_t j=0; j<tbl->fixups[i].namelen; j++)
        {
            putchar(tbl->fixups[i].name[j]);
        }
        printf(" 0x%04X\n", tbl->fixups[i].address);
    }
}
