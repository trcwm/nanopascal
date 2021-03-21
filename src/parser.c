/*

    PL/0 parser
    N.A. Moseley 2021

*/

#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "symtbl.h"
#include "opcodes.h"

typedef struct
{
    lexer_context_t lex;
    symtbl_t        symtbl;         ///< the symbol table
    char            *matchstart;    ///< pointer to string of last matched token
    int16_t         matchlen;       ///< string length of last matched token
    token_t         matchtok;       ///< matched token

    uint16_t        labelid;
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
    default:
        printf("??? code:%d\n", op);
        break;
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
bool parse_block(parse_context_t *context);
bool parse_expression(parse_context_t *context);

bool parse_factor(parse_context_t *context)
{
    if (match(context, TOK_IDENT))
    {
        const sym_t* s = sym_lookup(&context->symtbl, context->matchstart, context->matchlen);
        if (s == NULL)
        {
            parse_error("Cannot find symbol", context->lex.linenum);
            return false;
        }
        if (s->type == TYPE_INT)
        {
            // the offset is w.r.t. the base pointer
            // which holds T,B and the return address
            // so local variables are offset by an additional 3.
            emit(context, VM_LOD, 0, context->proclevel - s->level, s->offset + 3);
        }
        else if (s->type == TYPE_CONST)
        {
            //emit(context, VM_LIT, 0, context->proclevel - s->level, s->offset);
            emit(context, VM_LIT, 0, 0, s->offset);
        }
        else
        {
            parse_error("Incompatible type", context->lex.linenum);
        }
        return true;
    }
    else if (match(context, TOK_INTEGER))
    {
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

        if (optok == TOK_STAR)
            emit(context, VM_OPR, OPR_MUL,0,0);
        else
            emit(context, VM_OPR, OPR_DIV,0,0);
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
    if (!match(context, TOK_ASSIGN))
    {
        parse_error("Expected :=\n", context->lex.linenum);
        return false;
    }

    if (!parse_expression(context))
    {
        return false;
    }

    const sym_t* s = sym_lookup(&context->symtbl, identname, identlen);
    if (s == NULL)
    {
        parse_error("Cannot find symbol", context->lex.linenum);
        return false;
    }
    if (s->type == TYPE_INT)
    {
        emit(context, VM_STO, 0, context->proclevel - s->level, s->offset+3);
    }
    else
    {
        parse_error("Wrong type", context->lex.linenum);
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
            parse_error("Expect an expression after SHR", context->lex.linenum);
            return false;
        }
        emit(context, VM_OPR, OPR_SHR,0,0);
        return true;
    }

    // SHL expression ?
    if (match(context, TOK_SHL))
    {
        if (!parse_expression(context))
        {
            parse_error("Expect an expression after SHL", context->lex.linenum);
            return false;
        }
        emit(context, VM_OPR, OPR_SHL,0,0);
        return true;
    }

    // SHL expression ?
    if (match(context, TOK_SAR))
    {
        if (!parse_expression(context))
        {
            parse_error("Expect an expression after SAR", context->lex.linenum);
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
        emit(context, VM_OPR, OPR_NEG,0,0);
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
            emit(context, VM_OPR, OPR_ADD, 0,0);
        }
        else
        {
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
        if ((s == NULL) || (s->type != TYPE_INT))
        {
            parse_error("Cannot find variable\n", context->lex.linenum);
            return false;
        }

        emit(context, VM_OPR, OPR_ININT,0,0);  // read value onto stack
        emit(context, VM_STO,0, context->proclevel - s->level, s->offset+3);        
    }
    // ! expression
    else if (match(context, TOK_EXCLAMATION))
    {
        if (!parse_expression(context))
        {
            parse_error("! expression invalid\n", context->lex.linenum);
        }

        emit(context, VM_OPR, OPR_OUTINT, 0,0);

        while(match(context, TOK_COMMA))
        {
            if (!parse_expression(context))
            {
                parse_error("! expression invalid\n", context->lex.linenum);
            }            

            emit(context, VM_LIT, 0,0,32);
            emit(context, VM_OPR, OPR_OUTCHAR,0,0);
            emit(context, VM_OPR, OPR_OUTINT,0,0);
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
    // IF .. THEN
    else if (match(context, TOK_IF))
    {
        emit_txt("; IF\n");
        if (!parse_condition(context))
        {
            parse_error("Expected a condition in IF statement\n", context->lex.linenum);
            return false;            
        }

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
        parse_error("Expected =", context->lex.linenum);
        return false;
    }

    if (!match(context, TOK_INTEGER))
    {
        parse_error("Expected INTEGER\n", context->lex.linenum);
        return false;
    }    

    sym_add(&context->symtbl, TYPE_CONST, ident, identlen);
    context->symtbl.syms[context->symtbl.Nsymbols-1].offset = context->number;

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
            parse_error("Expected =", context->lex.linenum);
            return false;
        }
        if (!match(context, TOK_INTEGER))
        {
            parse_error("Exptected INTEGER\n", context->lex.linenum);
            return false;
        }

        sym_add(&context->symtbl, TYPE_CONST, ident, identlen);
    }

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;", context->lex.linenum);
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
    sym_add(&context->symtbl, TYPE_INT, ident, identlen);

    while(match(context, TOK_COMMA))
    {
        if (!match(context, TOK_IDENT))
        {
            parse_error("Expected IDENT\n", context->lex.linenum);
            return false;
        }

        ident    = context->matchstart;
        identlen = context->matchlen;
        sym_add(&context->symtbl, TYPE_INT, ident, identlen);        
    }

    if (!match(context, TOK_SEMICOL))
    {
        parse_error("Expected ;", context->lex.linenum);
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

    if (!sym_add(&context->symtbl, TYPE_PROCEDURE, procname, procnamelen))
        return false;
    
    emit_label(context->labelid);

    // add label id to the procedure symbol
    context->symtbl.syms[context->symtbl.Nsymbols-1].offset = context->labelid++;

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

    if (!parse_block(context))
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

bool parse_block(parse_context_t *context)
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

    if (context->proclevel == 0)
    {
        // here we have the top-level entry point
        emit_txt("@ENTRY:\n");

        
    }

    // create space for local variables    
    uint16_t local_varcount = sym_numvariables(&context->symtbl);
    emit_txt("INT ");
    printf("%d\n", local_varcount+3);   // 3 for local call pointers?    

    // one statement
    if (!parse_statement(context))
        return false;

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

    // get first token
    if (!nextToken(&context))
    {
        return false;
    }

    emit_txt("JMP @ENTRY\n");

    // parse program
    if (!parse_block(&context))
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
