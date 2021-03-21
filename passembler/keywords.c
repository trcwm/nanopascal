#include "keywords.h"

// define PL/0 keywords
// the order must be the same
// as the TOK_ definitions starting at value 100

const char* keywords[NKEYWORDS] =
{
    "LIT",  // 0
    "OPR",
    "LOD",
    "STO",
    "CAL",
    "INT",
    "JMP",
    "JPC",
    "HALT",
    "RET",
    "NEG",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "ODD",
    "LES",
    "LEQ",
    "GRE",
    "GEQ",
    "NEQ",
    "EQU",
    "SHL",
    "SHR",
    "SAR",
    "OUTCHAR",
    "OUTINT",
    "INCHAR",
    "ININT"
};

