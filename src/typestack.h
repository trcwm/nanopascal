/*

    Type stack - for expression type checking

*/

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "ptypes.h"

#define STACKMAXDEPTH 32

typedef struct 
{
    uint8_t   stackptr;
    vartype_t stack[STACKMAXDEPTH];
} typestack_t;

void ts_init(typestack_t *stk);
void ts_error(const char *txt);

bool ts_push(typestack_t *stk, vartype_t t);
bool ts_pop(typestack_t *stk);

vartype_t ts_item(const typestack_t *stk, uint8_t offset);

/** dump typestack */
void ts_dump(const typestack_t *stk);