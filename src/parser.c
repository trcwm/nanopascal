/*

    PL/0 parser
    N.A. Moseley 2021

*/

#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "symtbl.h"
#include "typestack.h"
#include "opcodes.h"

typedef struct
{
    lexer_context_t lex;
    symtbl_t        symtbl;         ///< the symbol table
    char            *matchstart;    ///< pointer to string of last matched token
    int16_t         matchlen;       ///< string length of last matched token
    token_t         matchtok;       ///< matched token

    typestack_t     typestack;      ///< hold type information during expression parsing.

    uint16_t        labelid;        ///< id of next label to be emitted.
    uint16_t        number;         ///< last number emitted from the lexer
    uint8_t         proclevel;      ///< nesting level of procedure

} parse_context_t;

void emit_txt(const char *comment)
{
    printf("%s", comment);
}

void emit_tokstr(const char *tokstr, uint16_t len)
{
    for(uint16_t i=0; i<len; i++)
    {
        putchar(tokstr[i]);
    }
}

void emit_label(uint16_t id)
{
    printf("@L%d:\n", id);
}

bool emit_with_label(opcode_t op, uint16_t labelid)
{
    uint8_t level = (op >> 4) & 0xF;
    op &= 0xF;
    switch(op)
    {
    case VM_JMP:
        printf("JMP @L%d\n",labelid);
        return true;
    case VM_JPC:
        printf("JPC @L%d\n",labelid);
        return true;
    case VM_CAL:
        printf("CAL %d @L%d\n",level,labelid);
        return true;
    default:
        printf("Cannot emit instruction type with label\n");
        break;
    }
    return false;
}

void emit(parse_context_t *context, opcode_t op, opr_t aluop, uint8_t level, uint16_t imm16)
{    
    switch(op)
    {
    case VM_LIT:
        printf("LIT %d\n", imm16);
        break;
    case VM_OPR:    // alu op
        switch(aluop)
        {
        case OPR_RET:
            printf("RET\n");
            break;
        case OPR_ADD:
            printf("ADD\n");
            break;
        case OPR_SUB:
            printf("SUB\n");
            break;
        case OPR_MUL:
            printf("MUL\n");
            break;
        case OPR_DIV:
            printf("DIV\n");
            break;            
        case OPR_NEG:
            printf("NEG\n");
            break;
        case OPR_ODD:
            printf("ODD\n");
            break;
        case OPR_LEQ:
            printf("LEQ\n");
            break;
        case OPR_GEQ:
            printf("GEQ\n");
            break;    
        case OPR_LESS:
            printf("LES\n");
            break;
        case OPR_GREATER:
            printf("GRE\n");
            break;
        case OPR_NEQ:
            printf("NEQ\n");
            break;
        case OPR_EQ:
            printf("EQU\n");
            break;  
        case OPR_ININT:
            printf("ININT\n");
            break;
        case OPR_OUTINT:
            printf("OUTINT\n");
            break;        
        case OPR_INCHAR:
            printf("INCHAR\n");
            break;                            
        case OPR_OUTCHAR:
            printf("OUTCHAR\n");
            break;    
        case OPR_SAR:
            printf("SAR\n");
            break;        
        case OPR_SHL:
            printf("SHL\n");
            break;                            
        case OPR_SHR:
            printf("SHR\n");
            break;                          
        default:
            printf("?? code:%d\n", aluop);
            break;
        }
        break;        
    case VM_LOD:
        printf("LOD %d %d\n", level, imm16);
        break;
    case VM_STO:
        printf("STO %d %d\n", level, imm16);
        break;
    case VM_INT:
        printf("INT %d\n", imm16);
        break;
    case VM_JMP:
        printf("JMP $%04X\n", imm16);
        break;
    case VM_CAL:
        printf("CAL %d $%04X\n", level, imm16);
        break;
    case VM_JPC:
        printf("JPC $%04X\n", imm16);
        break;
    case VM_HALT:
        printf("HALT\n");
        break;
    case VM_LODX:
        printf("LODX %d %d\n", level, imm16);
        break;
    case VM_STOX:
        printf("STOX %d %d\n", level, imm16);
        break;        
    default:
        printf("??? code:%d\n", op);
        break;
    }
}

void check_tstack(const parse_context_t *context)
{
    // check that the operation stack is empty
    if (context->typestack.stackptr != 0)
    {
        emit_txt("Expected typestack to be emtpy\n");
        ts_dump(&context->typestack);
    }
}

// --======== LOCAL PARSER FUNCTIONS ========--

static void parse_error(const char *errstr, int16_t lineNum)
{
    fprintf(stderr, "Line %d: %s", lineNum, errstr);
}

// Get the next token from the lexer
static bool nextToken(parse_context_t *context)
{
    return lexer_next(&(context->lex));
}

// See if the specified token matches the one from the lexer
// if it matched, save the token string in the parse context
// and go to the next token.
static bool match(parse_context_t *context, const token_t tok)
{
    if (context->lex.token == tok)
    {
        context->matchstart = context->lex.tokstart;
        context->matchlen   = context->lex.toklen;
        context->matchtok   = context->lex.token;
        context->number     = context->lex.number;
        nextToken(context);
        return true;
    }
    return false;
}

// --======== GRAMMAR/PRODUCTIONS ========--

// predeclarations
bool parse_block(parse_context_t *context, uint16_t labelid);
bool parse_expression(parse_context_t *context);

bool parse_const_id(parse_context_t *context)
{
    const sym_t* s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
    if (s == NULL)
    {
        return false;
    }

    if (s->type == TYPE_CONST)
    {
        ts_push(&context->typestack, TYPE_CONST);
        emit(context, VM_LIT, 0, 0, s->offset);
        return true;
    }
    return false;
}

/** load the stack with the variable requested */
bool parse_variable_id(parse_context_t *context)
{
    const sym_t* s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
    if (s == NULL)
    {
        return false;
    }

    if ((s->type == TYPE_INT) || (s->type == TYPE_CHAR))
    {
        // the offset is w.r.t. the base pointer
        // which holds T,B and the return address
        // so local variables are offset by an additional 3.
        ts_push(&context->typestack, s->type);
        emit(context, VM_LOD, 0, context->proclevel - s->level, s->offset + 3);
        return true;
    }
    return false;
}

/** load the stack with the array element requested */
bool parse_array_id(parse_context_t *context)
{
    const sym_t* s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
    if (s == NULL)
    {
        return false;
    }

    if (s->type == TYPE_ARRAY)
    {
        // expect [ number ]
        if (!match(context, TOK_LBRACKET))
        {
            parse_error("Expected '['\n", context->lex.linenum);
            return false;
        }

        if (!parse_expression(context))
        {
            parse_error("Expected an expression between '[ ]'\n", context->lex.linenum);
        }

        // check type is integer
        vartype_t op1 = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        if (!((op1 == TYPE_INT) || (op1 == TYPE_CONST)))
        {
            parse_error("Expected an INTEGER or CONST type between '[ ]'\n", context->lex.linenum);
            return false;
        }

        if (!match(context, TOK_RBRACKET))
        {
            parse_error("Expected ']'\n", context->lex.linenum);
            return false;
        }

        ts_push(&context->typestack, s->subtype);

        // the offset is w.r.t. the base pointer
        // which holds T,B and the return address
        // so local variables are offset by an additional 3.
        emit(context, VM_LODX, 0, context->proclevel - s->level, s->offset + 3);
        return true;
    }
    return false;
}

bool parse_array_type(parse_context_t *context, uint16_t startSymbolId)
{
    if (!match(context, TOK_LBRACKET))
    {
        parse_error("Expected [ after array\n", context->lex.linenum);
        return false;
    }

    if (!match(context, TOK_NUMBER))
    {
        parse_error("Expected NUMBER after [\n", context->lex.linenum);
        return false;
    }

    uint16_t arraylen = context->number;

    if (!match(context, TOK_RBRACKET))
    {
        parse_error("Expected ] in array\n", context->lex.linenum);
        return false;        
    }

    if (!match(context, TOK_OF))
    {
        parse_error("Expected OF in array\n", context->lex.linenum);
        return false;        
    }

    if (match(context, TOK_INTEGER) || match(context, TOK_CHAR))
    {
        //FIXME: add array to sym table
        switch(context->matchtok)
        {
        case TOK_INTEGER:
            for(uint16_t id = startSymbolId; id < context->symtbl.Nsymbols; id++)
                sym_update(&context->symtbl, id, TYPE_ARRAY, TYPE_INT, arraylen);
            break;
        case TOK_CHAR:
            for(uint16_t id = startSymbolId; id < context->symtbl.Nsymbols; id++)
                sym_update(&context->symtbl, id, TYPE_ARRAY, TYPE_CHAR, arraylen);        
            break;
        default:
            break;
        }
        
        return true;
    }
    else
    {
        parse_error("Expected INTEGER or CHAR in array\n", context->lex.linenum);
        return false;        
    }
}

bool parse_factor(parse_context_t *context)
{
    if (match(context, TOK_IDENT))
    {
        // accept constants, variables or arrays

        if (parse_const_id(context))
        {
            return true;
        }

        if (parse_variable_id(context))
        {
            return true;
        }

        if (parse_array_id(context))
        {
            return true;
        }

        parse_error("Incompatible type\n", context->lex.linenum);
        return false;
    }
    else if (match(context, TOK_NUMBER))
    {
        // literal!
        ts_push(&context->typestack, TYPE_CONST);
        emit(context, VM_LIT, 0, 0, context->number);
        return true;
    }
    else if (match(context, TOK_LPAREN))
    {
        if (!parse_expression(context))
        {
            return false;
        }
        
        if (!match(context, TOK_RPAREN))
        {
            parse_error("Expected )\n", context->lex.linenum);
            return false;
        }
        return true;
    }

    return false;
}

bool parse_term(parse_context_t *context)
{
    if (!parse_factor(context))
    {
        return false;
    }

    // optional * or / followed by another factor
    while (match(context, TOK_STAR) || match(context, TOK_SLASH))
    {
        token_t optok = context->matchtok;

        if (!parse_factor(context))
        {
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        vartype_t op2 = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        if (op1 == TYPE_CONST) op1 = TYPE_INT;
        if (op2 == TYPE_CONST) op2 = TYPE_INT;

        if ((op1 != TYPE_INT) || (op2 != TYPE_INT))
        {
            parse_error("MUL/DIV expect integers as operands\n", context->lex.linenum);
            return false;            
        }

        if (optok == TOK_STAR)
        {
            emit(context, VM_OPR, OPR_MUL,0,0);
        }
        else
        {
            emit(context, VM_OPR, OPR_DIV,0,0);
        }

        ts_push(&context->typestack, TYPE_INT);
    }

    return true;
}

bool parse_call(parse_context_t *context)
{
    if (!match(context, TOK_IDENT))
    {
        parse_error("Expected a procedure identifier\n", context->lex.linenum);
        return false;
    }

    sym_t *s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
    if ((s==NULL) || (s->type != TYPE_PROCEDURE))
    {
        parse_error("Cannot find procedure\n", context->lex.linenum);
        return false;
    }

    uint8_t opcode = VM_CAL | ((context->proclevel - s->level) << 4);

    emit_with_label(opcode, s->offset /* used as label id */);
    return true;
}

bool parse_assignment(parse_context_t *context, const char *identname, uint16_t identlen)
{
    // check if the identifier is an array
    const sym_t* s = sym_lookup(&context->symtbl, identname, identlen);
    if (s == NULL)
    {
        parse_error("Cannot find symbol\n", context->lex.linenum);
        return false;
    }
    
    if (s->type == TYPE_ARRAY)
    {
        if (!match(context,TOK_LBRACKET))
        {
            return false;
        }

        // parse the expression to get the index
        if (!parse_expression(context))
        {
            return false;
        }

        if (!match(context,TOK_RBRACKET))
        {
            return false;
        }

        if (!match(context, TOK_ASSIGN))
        {
            parse_error("Expected :=\n", context->lex.linenum);
            return false;
        }

        // parse the expression to get the data to be stored
        if (!parse_expression(context))
        {
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 1);
        ts_pop(&context->typestack);
        ts_pop(&context->typestack);

        if ((op1 != TYPE_CONST) && (op1 != TYPE_INT))
        {
            parse_error("Array index type must be INT or CONST\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_STOX, 0, context->proclevel - s->level, s->offset+3);
    }
    else if (s->type == TYPE_INT)
    {
        if (!match(context, TOK_ASSIGN))
        {
            parse_error("Expected :=\n", context->lex.linenum);
            return false;
        }

        if (!parse_expression(context))
        {
            return false;
        }

        ts_pop(&context->typestack);
        emit(context, VM_STO, 0, context->proclevel - s->level, s->offset+3);
    }
    else
    {
        parse_error("Wrong type\n", context->lex.linenum);
        return false;
    }

    return true;
}

bool parse_expression(parse_context_t *context)
{
    // SHR expression ?
    if (match(context, TOK_SHR))
    {
        if (!parse_expression(context))
        {
            parse_error("Expect an expression after SHR\n", context->lex.linenum);
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
        {
            parse_error("argument of SHR must be INTEGER or CONSTANT\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_SHR, 0, 0);
        return true;
    }

    // SHL expression ?
    if (match(context, TOK_SHL))
    {
        if (!parse_expression(context))
        {
            parse_error("Expect an expression after SHL\n", context->lex.linenum);
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 0);
        
        if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
        {
            parse_error("argument of SHL must be INTEGER or CONSTANT\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_SHL,0,0);
        return true;
    }

    // SAR expression ?
    if (match(context, TOK_SAR))
    {
        if (!parse_expression(context))
        {
            parse_error("Expect an expression after SAR\n", context->lex.linenum);
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 0);
        
        if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
        {
            parse_error("argument of SHL must be INTEGER or CONSTANT\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_SAR,0,0);
        return true;
    }

    // check for unary + or -
    if (match(context, TOK_PLUS))
    {
        // nothing.
    }
    else if (match(context, TOK_MINUS))
    {
        vartype_t op1 = ts_item(&context->typestack, 0);
        
        if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
        {
            parse_error("argument of unary minus must be INTEGER or CONSTANT\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_NEG, 0, 0);
    }

    if (!parse_term(context))
    {
        return false;
    }

    // more terms may follow
    while(match(context, TOK_PLUS) || match(context, TOK_MINUS))
    {
        token_t optok = context->matchtok;

        if (!parse_term(context))
        {
            return false;
        }

        if (optok == TOK_PLUS)
        {
            vartype_t op1 = ts_item(&context->typestack, 0);
            
            if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
            {
                parse_error("argument of + be INTEGER or CONSTANT\n", context->lex.linenum);
                return false;
            }

            vartype_t op2 = ts_item(&context->typestack, 1);
            
            if ((op2 != TYPE_INT) && (op2 != TYPE_CONST))
            {
                parse_error("argument of + be INTEGER or CONSTANT\n", context->lex.linenum);
                return false;
            }

            ts_pop(&context->typestack);

            emit(context, VM_OPR, OPR_ADD, 0,0);
        }
        else
        {
           vartype_t op1 = ts_item(&context->typestack, 0);
            
            if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
            {
                parse_error("argument of - must be INTEGER or CONSTANT\n", context->lex.linenum);
                return false;
            }

            vartype_t op2 = ts_item(&context->typestack, 1);
            
            if ((op2 != TYPE_INT) && (op2 != TYPE_CONST))
            {
                parse_error("argument of - must be INTEGER or CONSTANT\n", context->lex.linenum);
                return false;
            }

            ts_pop(&context->typestack);

            emit(context, VM_OPR, OPR_SUB, 0,0);
        }
    }

    return true;
}

bool parse_condition(parse_context_t *context)
{
    if (match(context, TOK_ODD))
    {
        if (!parse_expression(context))
        {
            parse_error("Expected a statement\n", context->lex.linenum);
            return false;
        }

        vartype_t op1 = ts_item(&context->typestack, 0);
            
        if ((op1 != TYPE_INT) && (op1 != TYPE_CONST))
        {
            parse_error("argument of ODD must be INTEGER or CONSTANT\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_ODD, 0, 0);

        return true;    // accept condition.
    }

    // try EXPRESSION op EXPRESSION
    if (!parse_expression(context))
    {
        parse_error("Expected an expression in condition\n", context->lex.linenum);
        return false;
    }

    token_t condition;
    if (match(context, TOK_EQUAL))
    {
        condition = context->matchtok;
    }
    else if (match(context, TOK_HASH))
    {
        condition = context->matchtok;
    }
    else if (match(context, TOK_LESS))
    {
        condition = context->matchtok;
    }
    else if (match(context, TOK_LEQ))
    {
        condition = context->matchtok;
    }
    else if (match(context, TOK_GREATER))
    {
        condition = context->matchtok;
    }
    else if (match(context, TOK_GEQ))
    {
        condition = context->matchtok;
    }
    else 
    {
        parse_error("Expected condition operator\n", context->lex.linenum);
        return false;
    }

    if (!parse_expression(context))
    {
        parse_error("Expected an expression in condition\n", context->lex.linenum);
        return false;
    }

    vartype_t op1 = ts_item(&context->typestack, 0);
    vartype_t op2 = ts_item(&context->typestack, 1);

    ts_pop(&context->typestack);
    ts_pop(&context->typestack);

    if (op1 == TYPE_CONST) op1 = TYPE_INT;
    if (op2 == TYPE_CONST) op2 = TYPE_INT;

    if (op1 != op2)
    {
        parse_error("Types must be identical\n", context->lex.linenum);
        return false;        
    }

    if (condition == TOK_EQUAL)
    {
        emit(context, VM_OPR, OPR_EQ,0,0);
    }
    else if (condition == TOK_HASH)
    {
        emit(context, VM_OPR, OPR_NEQ,0,0);
    }
    else if (condition == TOK_GEQ)
    {
        emit(context, VM_OPR, OPR_GEQ,0,0);
    }
    else if (condition == TOK_LEQ)
    {
        emit(context, VM_OPR, OPR_LEQ,0,0);
    }
    else if (condition == TOK_LESS)
    {
        emit(context, VM_OPR, OPR_LESS,0,0);
    }    
    else if (condition == TOK_GREATER)
    {
        emit(context, VM_OPR, OPR_GREATER,0,0);
    }
    else
    {
        parse_error("Condition code not matched\n", context->lex.linenum);
        return false;
    }

    ts_push(&context->typestack, TYPE_INT); // condition result
    return true;
}

bool parse_statement(parse_context_t *context)
{
    // IDENT := ..
    if (match(context, TOK_IDENT))
    {
        if (!parse_assignment(context, context->matchstart, context->matchlen))
            return false;        
    }
    // CALL IDENT
    else if (match(context, TOK_CALL))
    {
        if (!parse_call(context))
            return false;
    }
    // ? IDENT
    else if (match(context, TOK_QUESTION))
    {
        if (!match(context, TOK_IDENT))
        {
            parse_error("Expected IDENT\n", context->lex.linenum);
            return false;
        }        

        sym_t *s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
        if (s == NULL)
        {
            parse_error("Cannot find variable\n", context->lex.linenum);
            return false;
        }

        switch(s->type)
        {
        case TYPE_INT:
            emit(context, VM_OPR, OPR_ININT,0,0);  // read value onto stack
            break;
        case TYPE_CHAR:
            emit(context, VM_OPR, OPR_INCHAR,0,0);  // read value onto stack
            break;
        default:
            parse_error("Expected INT or CHAR type\n", context->lex.linenum);
            return false;
        }
        emit(context, VM_STO,0, context->proclevel - s->level, s->offset+3);
    }
    // ! expression
    else if (match(context, TOK_EXCLAMATION))
    {
        if (!parse_expression(context))
        {
            parse_error("! expression invalid\n", context->lex.linenum);
        }

        vartype_t op = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        switch(op)
        {
        case TYPE_INT:
        case TYPE_CONST:
            emit(context, VM_OPR, OPR_OUTINT,0,0);  // read value onto stack
            break;
        case TYPE_CHAR:
            emit(context, VM_OPR, OPR_OUTCHAR,0,0);  // read value onto stack
            break;
        default:
            parse_error("Expected INT, CHAR or CONST type\n", context->lex.linenum);
            return false;
        }

        while(match(context, TOK_COMMA))
        {
            if (!parse_expression(context))
            {
                parse_error("! expression invalid\n", context->lex.linenum);
            }            

            emit(context, VM_LIT, 0,0,32);
            emit(context, VM_OPR, OPR_OUTCHAR,0,0);
            
            vartype_t op = ts_item(&context->typestack, 0);
            ts_pop(&context->typestack);

            switch(op)
            {
            case TYPE_INT:
            case TYPE_CONST:
                emit(context, VM_OPR, OPR_OUTINT,0,0);  // read value onto stack
                break;
            case TYPE_CHAR:
                emit(context, VM_OPR, OPR_OUTCHAR,0,0);  // read value onto stack
                break;
            default:
                parse_error("Expected INT, CHAR or CONST type\n", context->lex.linenum);
                return false;
            }
        }

        // line feed
        emit(context, VM_LIT, 0,0,10);
        emit(context, VM_OPR, OPR_OUTCHAR,0,0);
        emit(context, VM_LIT, 0,0,13);
        emit(context, VM_OPR, OPR_OUTCHAR,0,0);
    }
    // BEGIN .. END
    else if (match(context, TOK_BEGIN))
    {
        if (!parse_statement(context))
            return false;

        // optional statements
        while(match(context, TOK_SEMICOL))
        {
            if (!parse_statement(context))
                return false;
        }

        if (!match(context, TOK_END))
        {
            parse_error("Expected END\n", context->lex.linenum);
            return false;
        }
    }
    // IF .. THEN .. ELSE
    else if (match(context, TOK_IF))
    {
        emit_txt("; IF\n");
        if (!parse_condition(context))
        {
            parse_error("Expected a condition in IF statement\n", context->lex.linenum);
            return false;            
        }

        ts_pop(&context->typestack);    

        // jump over the THEN code if condition is false
        // the jump address needs a fixup
        uint16_t jpc_label = context->labelid++;
        if (!emit_with_label(VM_JPC, jpc_label))
            return false;

        emit_txt("; THEN\n");
        if (!match(context, TOK_THEN))
        {
            parse_error("Expected THEN in IF statement\n", context->lex.linenum);
            return false;                        
        }
        if (!parse_statement(context))
        {
            parse_error("Expected statement after THEN\n", context->lex.linenum);
            return false;                                    
        }
        
        // jump over the ELSE statements.
        uint16_t jmp_label = context->labelid++;
        if (!emit_with_label(VM_JMP, jmp_label))
            return false;

        emit_label(jpc_label);

        // optional else statement
        if (match(context, TOK_ELSE))
        {
            emit_txt("; ELSE\n");
            if (!parse_statement(context))
            {
                parse_error("Expected statement after ELSE\n", context->lex.linenum);
                return false;                                    
            }            
        }
        emit_label(jmp_label);
        emit_txt("; END IF\n");
    }
    // WHILE .. DO
    else if (match(context, TOK_WHILE))
    {
        // save the address at this point
        // so we can jump back to it at the end of 
        // the while block

        uint16_t jmp_label = context->labelid++;
        emit_txt("; WHILE\n");
        emit_label(jmp_label);
        
        if (!parse_condition(context))
        {
            parse_error("Expected a condition in IF statement\n", context->lex.linenum);
            return false;            
        }

        ts_pop(&context->typestack);

        // forward jump
        uint16_t jpc_label = context->labelid++;
        emit_with_label(VM_JPC, jpc_label);
        
        if (!match(context, TOK_DO))
        {
            parse_error("Expected DO in WHILE statement\n", context->lex.linenum);
            return false;                        
        }
        if (!parse_statement(context))
        {
            parse_error("Expected statement after DO\n", context->lex.linenum);
            return false;                                    
        }
        emit_with_label(VM_JMP, jmp_label);
        emit_label(jpc_label);
        emit_txt("; END WHILE\n");
    }
    // FOR .. TO/DOWNTO .. DO
    else if (match(context, TOK_FOR))
    {
        emit_txt("; FOR\n");
        if (!match(context, TOK_IDENT))
        {
            parse_error("Expected an identifier after FOR\n", context->lex.linenum);
            return false;
        }

        sym_t *ident = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);

        if (ident == NULL)
        {
            parse_error("Cannot find symbol\n", context->lex.linenum);
            return false;
        }

        if (!match(context, TOK_ASSIGN))
        {
            parse_error("Expected := after FOR <variable>\n", context->lex.linenum);
            return false;
        }        

        // init expression
        if (!parse_expression(context))
        {
            parse_error("Expected an identifier after FOR <variable>:=\n", context->lex.linenum);
            return false;            
        }

        vartype_t expr = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        if (expr == TYPE_CONST)
        {
            expr = TYPE_INT;
        }

        if (ident->type == expr)
        {
            emit(context, VM_STO, 0, context->proclevel - ident->level, ident->offset+3);
        }
        else
        {
            parse_error("Type mismatch after FOR\n", context->lex.linenum);
            return false;
        }

        if (!match(context, TOK_TO))
        {
            parse_error("Expected TO in FOR\n", context->lex.linenum);
            return false;
        }

        uint16_t jmp_label = context->labelid++;
        emit_label(jmp_label);
        emit_txt("; FOR check expression\n");

        // check expression
        uint16_t exit_label = context->labelid++;
        emit(context, VM_LOD, 0, context->proclevel - ident->level, ident->offset+3);

        if (!parse_expression(context))
        {
            parse_error("Expected an identifier after TO\n", context->lex.linenum);
            return false;            
        }

        expr = ts_item(&context->typestack, 0);
        ts_pop(&context->typestack);

        if (expr == TYPE_CONST)
            expr = TYPE_INT;

        if (ident->type != expr)
        {
            parse_error("Type mismatch after TO\n", context->lex.linenum);
            return false;            
        }

        emit(context, VM_OPR, OPR_LEQ, 0,0);
        emit_with_label(VM_JPC, exit_label);

        if (!match(context, TOK_DO))
        {
            parse_error("Expected DO in FOR\n", context->lex.linenum);
            return false;
        }            

        emit_txt("; FOR DO expression\n");

        if (!parse_statement(context))
        {
            parse_error("Expected an statement in FOR loop\n", context->lex.linenum);
            return false;            
        }

        // increment the loop counter
        emit(context, VM_LOD, 0, context->proclevel - ident->level, ident->offset+3);
        emit(context, VM_LIT, 0,0,1);
        emit(context, VM_OPR, OPR_ADD,0,0);
        emit(context, VM_STO, 0, context->proclevel - ident->level, ident->offset+3);
        emit_with_label(VM_JMP, jmp_label);

        emit_label(exit_label);
        emit_txt("; end FOR\n");
    }

    // check that the operation stack is empty
    check_tstack(context);

    // everything is optional, so we always return true.
    return true;
}

bool parse_const(parse_context_t *context)
{
    if (!match(context, TOK_IDENT))
    {
        parse_error("Expected IDENT\n", context->lex.linenum);
        return false;
    }

    const char *ident    = context->matchstart;
    uint16_t    identlen = context->matchlen;

    if (!match(context, TOK_EQUAL))
    {
        parse_error("Expected =\n", context->lex.linenum);
        return false;
    }

    if (!match(context, TOK_NUMBER))
    {
        parse_error("Expected NUMBER\n", context->lex.linenum);
        return false;
    }    

    sym_add(&context->symtbl, ident, identlen);
    sym_set_const(&context->symtbl, context->symtbl.Nsymbols-1, context->number);

    while(match(context, TOK_COMMA))
    {
        if (!match(context, TOK_IDENT))
        {
            parse_error("Expected IDENT\n", context->lex.linenum);
            return false;
        }

        ident    = context->matchstart;
        identlen = context->matchlen;

        if (!match(context, TOK_EQUAL))
        {
            parse_error("Expected =\n", context->lex.linenum);
            return false;
        }
        if (!match(context, TOK_NUMBER))
        {
            parse_error("Exptected NUMBER\n", context->lex.linenum);
            return false;
        }

        sym_add(&context->symtbl, ident, identlen);
        sym_set_const(&context->symtbl, context->symtbl.Nsymbols-1, context->number);
    }

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;\n", context->lex.linenum);
        return false;
    }
    return true;
}

bool parse_var(parse_context_t *context)
{
    if (!match(context, TOK_IDENT))
    {
        parse_error("Expected IDENT\n", context->lex.linenum);
        return false;
    }

    const char *ident    = context->matchstart;
    uint16_t    identlen = context->matchlen;

    // Add the symbol to the symbol table
    // at this point, we don't know the type,
    // so we specify TYPE_NONE. It will be
    // changed/updated at the end of this function.

    uint16_t startSymbolId = context->symtbl.Nsymbols;
    sym_add(&context->symtbl, ident, identlen);

    while(match(context, TOK_COMMA))
    {
        if (!match(context, TOK_IDENT))
        {
            parse_error("Expected IDENT\n", context->lex.linenum);
            return false;
        }

        ident    = context->matchstart;
        identlen = context->matchlen;
        sym_add(&context->symtbl, ident, identlen);
    }

    if (!match(context, TOK_COLON))
    {
        parse_error("Expected :\n", context->lex.linenum);
        return false;        
    }

    // parse the type information

    if (match(context, TOK_ARRAY))
    {
        if (!parse_array_type(context, startSymbolId))
        {
            return false;
        }
    }
    else if (match(context, TOK_CHAR) || match(context, TOK_INTEGER))
    {
        switch(context->matchtok)
        {
        case TOK_CHAR:
            for(uint16_t id = startSymbolId; id < context->symtbl.Nsymbols; id++)
                sym_update(&context->symtbl, id, TYPE_CHAR, TYPE_NONE, 1);
            break;
        case TOK_INTEGER:
            for(uint16_t id = startSymbolId; id < context->symtbl.Nsymbols; id++)
                sym_update(&context->symtbl, id, TYPE_INT, TYPE_NONE, 1);
            break;
        default:
            // we should never end up here.
            parse_error("parse_var: internal error\n", context->lex.linenum);
            return false;
        }
    }
    else
    {
        parse_error("Expected type name CHAR, INTEGER or ARRAY\n", context->lex.linenum);
        return false;
    }

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;\n", context->lex.linenum);
        return false;
    }

    return true;
}

bool parse_procedure(parse_context_t *context)
{   
    if (!match(context, TOK_IDENT))
    {
        parse_error("Expected IDENT\n", context->lex.linenum);
        return false;
    }

    const char *procname    = context->matchstart;
    uint16_t    procnamelen = context->matchlen;

    emit_txt("; PROCEDURE ");
    emit_tokstr(procname, procnamelen);
    emit_txt("\n");

    if (!sym_add(&context->symtbl, procname, procnamelen))
        return false;
    
    uint16_t proc_label = context->labelid++;
    
    // add label id to the procedure symbol
    //context->symtbl.syms[context->symtbl.Nsymbols-1].offset = proc_label;
    sym_set_procedure(&context->symtbl, context->symtbl.Nsymbols-1, proc_label);

    sym_enter(&context->symtbl);

    if (context->proclevel == 15)
    {
        parse_error("Too many nested procedures\n", context->lex.linenum);
        return false;
    }

    context->proclevel++;

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;\n", context->lex.linenum);
        return false;
    }

    if (!parse_block(context, proc_label))
    {
        return false;
    }

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;", context->lex.linenum);
        return false;
    }

    emit(context, VM_OPR, OPR_RET,0,0);

    sym_dump(&context->symtbl);

    emit_txt("; ENDPROC\n\n");

    sym_leave(&context->symtbl);
    context->proclevel--;
    return true;
}

bool parse_block(parse_context_t *context, uint16_t labelid)
{
    // zero or more const
    while (match(context, TOK_CONST))
    {
        if (!parse_const(context))
            return false;
    }

    // zero or more var
    while (match(context, TOK_VAR))
    {
        if (!parse_var(context))
            return false;
    }

    // zero or more procedures
    while(match(context, TOK_PROCEDURE))
    {
        if (!parse_procedure(context))
            return false;
    }

    emit_label(labelid);

    // create space for local variables    
    uint16_t space_required = sym_get_local_space(&context->symtbl);
    emit_txt("INT ");
    printf("%d\n", space_required+3);   // 3 for local call pointers?    

    // one statement
    if (!parse_statement(context))
        return false;

    // check that the operation stack is empty
    check_tstack(context);

    return true;
}

bool parse(char *src)
{   
    parse_context_t context;
    context.matchlen    = 0;
    context.matchstart  = src;
    context.proclevel   = 0;
    context.labelid     = 0;

    lexer_init(&context.lex, src);
    sym_init(&context.symtbl);
    ts_init(&context.typestack);

    // get first token
    if (!nextToken(&context))
    {
        return false;
    }

    
    uint16_t entry_label = context.labelid++;
    emit_with_label(VM_JMP, entry_label);

    // parse program
    if (!parse_block(&context, entry_label))
    {
        parse_error("Parse error\n", context.lex.linenum);
        return false;
    }

    // expect '.'
    if (!match(&context, TOK_PERIOD))
    {
        parse_error("Expected .\n", context.lex.linenum);
        return false;
    }

    emit(&context,VM_HALT,0,0,0);

    sym_dump(&context.symtbl);

    return true;
}
