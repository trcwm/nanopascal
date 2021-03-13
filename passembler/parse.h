/*

    Parser for p-code assembler
    N.A. Moseley (c) 2021

*/

#pragma once
#include <stdbool.h>
#include "lex.h"

bool parse(const char *src);
