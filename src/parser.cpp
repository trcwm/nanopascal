/*

  Parser for the Micro Pascal
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#include <iostream>
#include "functiondefs.h"
#include "parser.h"

// ********************************************************************************
//   Parser
// ********************************************************************************

Parser::Parser() : m_tokens(NULL)
{
    m_lastErrorPos.line=0;
    m_lastErrorPos.offset=0;
    m_lastErrorPos.pos=0;
    m_lastError = std::string("Unknown error");
}


void Parser::error(const ParseContext &s, const std::string &txt)
{
    m_lastError = txt;
    m_lastErrorPos = s.tokPos;
    std::cout << "Error on line " << s.tokPos.line+1 << " :" << txt.c_str() << std::endl;
}

void Parser::error(const std::string &txt)
{
    m_lastError = txt;
    std::cout << txt.c_str() << std::endl;
}

bool Parser::match(ParseContext &s, uint32_t tokenID)
{
    token_t tok = getToken(s);
    if (tok.tokID != tokenID)
    {
        return false;
    }
    next(s);
    return true;
}

bool Parser::process(const std::vector<token_t> &tokens, ParseContext &context)
{
    m_lastError.clear();

    if (tokens.size() == 0)
    {
        error("Internal error: token list is empty");
        return false;
    }

    // prepare for iteration
    m_tokens = &tokens;

    context.tokIdx = 0;
    return acceptProgram(context);
}

bool Parser::acceptProgram(ParseContext &context)
{
    // production: block EOF
    context.m_astHead = 0;
    context.m_symTable = new SymbolTable::ScopedTable(NULL);

    AST::ASTNode *node = acceptBlock(context);
    if (node == NULL)
        return false;

    context.m_astHead = node;
    return true;
}

AST::ASTNode* Parser::acceptBlock(ParseContext &context)
{
    /** production: block -> ((constdecl)* | (vardecl)*) prog_block */

    // first eat all the constdecl and vardecl blocks

    AST::ASTNode *block = new AST::ASTNode(AST::NODE_BLOCK);

    bool accepted = true;
    while(accepted)
    {
        AST::ASTNode *node = 0;
        accepted = false;
        if ((node=acceptConstDecl(context)) != 0)
        {
            // add node to block
            block->m_children.push_back(node);
            accepted = true;
        }
        else if ((node=acceptVarDecl(context)) !=0)
        {
            // add node to block
            block->m_children.push_back(node);
            accepted = true;
        }
        else if ((node=acceptProcDecl(context)) !=0)
        {
            // add node to block
            block->m_children.push_back(node);
            accepted = true;
        }
    }

    // all constdecl and vardecl blocks eaten
    // we must get a prog_block now!
    AST::ASTNode *node = acceptProgBlock(context);
    if (node == NULL)
    {
        error(context,"Expected a BEGIN .. END block");
        return NULL;
    }

    // add prog block to block node
    block->m_children.push_back(node);
    return block;
}


AST::ASTNode* Parser::acceptProgBlock(ParseContext &s)
{
    /** production: prog_block -> BEGIN (statement)* END */
    AST::ASTNode *prgblock = new AST::ASTNode(AST::NODE_PROGBLOCK);

    ParseContext savestate = s;
    if (!match(s,TOK_BEGIN))
    {
        s = savestate;
        return NULL;
    }

    // eat all the statements
    do
    {
        AST::ASTNode *node = acceptStatement(s);
        if (node != 0)
        {
            // add statement to the program block
            prgblock->m_children.push_back(node);
        }
        /*
        else if (match(s, TOK_END))
        {
            // allow the END to appear after a semicol
            // for convenience
            return prgblock;
        }
        */
    } while(match(s, TOK_SEMICOL));

    // we must have an END token here
    savestate = s;
    if (!match(s,TOK_END))
    {
        error(s,"END not found");
        s = savestate;
        delete prgblock;
        return NULL;
    }

    return prgblock;
}


AST::ASTNode* Parser::acceptConstDecl(ParseContext &s)
{
    /** production: constdecl -> CONST (IDENT = CONSTANT ;)+ */
    ParseContext savestate = s;
    if (!match(s,TOK_CONST))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_EQUAL))
    {
        s = savestate;
        return NULL;
    }

    // save identifier for later
    std::string ident = getToken(s,-2).txt;
    s.m_symTable->addIdentifier(ident, SymbolTable::SymbolInfo::TYPE_UINT16, true);

    AST::ASTNode *constant = acceptConstant(s);
    if (constant == 0)
    {
        error(s,"Expected a constant");
        return NULL;
    }

    if (!match(s,TOK_SEMICOL))
    {
        error(s,"Expected a semicolon");
        delete constant;
        return NULL;
    }

    AST::ASTNode *constdecl = new AST::ASTNode(AST::NODE_CONSTDECL);
    constant->m_txt = ident;
    constdecl->m_children.push_back(constant);

    // we have found one constant,
    // now we check for more...

    do
    {
        constant = NULL;
        if (!match(s,TOK_IDENT))
        {
            break;
        }

        if (!match(s,TOK_EQUAL))
        {
            break;
        }

        ident = getToken(s,-2).txt;

        constant = acceptConstant(s);
        if (constant != NULL)
        {
            // found another constant!
            // need matching ;
            if (!match(s,TOK_SEMICOL))
            {
                error(s,"Expected a semicolon");
                delete constant;
                delete constdecl;
                s = savestate;
                return NULL;
            }
            constant->m_txt = ident;
            constdecl->m_children.push_back(constant);

            //FIXME: for now we set the type to uint, but it
            // can be something else.. need to figure out later!
            s.m_symTable->addIdentifier(ident, SymbolTable::SymbolInfo::TYPE_UINT16, true);
        }
        else
        {
            error(s, "Constant expected");
            s = savestate;
            return NULL;
        }
    }
    while(constant != NULL);

    return constdecl;
}


AST::ASTNode* Parser::acceptVarDecl(ParseContext &s)
{
    //production: vardecl -> VAR IDENT (, IDENT)* : INTEGERKW ; */

    //ParseContext savestate = s;
    if (!match(s,TOK_VAR))
    {
        return NULL;
    }

    if (!match(s,TOK_IDENT))
    {
        return NULL;
    }

    AST::ASTNode *vardecl = new AST::ASTNode(AST::NODE_VARDECL);
    AST::ASTNode *node = new AST::ASTNode(AST::NODE_DECLVARINTEGER);

    // we've found an identifier: our first variable
    node->m_txt = getToken(s, -1).txt;  // store var name
    vardecl->m_children.push_back(node);

    // add variable to the symbol table!
    s.m_symTable->addIdentifier(node->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16);

    // match other variabels
    while(!match(s,TOK_COLON))
    {
        // should have a comma here
        if (!match(s,TOK_COMMA))
        {
            error(s,"Expected a comma or colon in variable declaration");
            delete vardecl;
            return NULL;
        }
        if (match(s,TOK_IDENT))
        {
            // found another var name
            // FIXME: we don't know what the type is
            // until we have found the type identifier
            // .. we will need to fix this later
            // for now, we assume INT
            node = new AST::ASTNode(AST::NODE_DECLVARINTEGER);
            node->m_txt = getToken(s, -1).txt;  // store var name
            vardecl->m_children.push_back(node);

            s.m_symTable->addIdentifier(node->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16);
        }
        else
        {
            error(s,"Identifier expected in variable declaration");
            delete vardecl;
            return NULL;
        }
    }

    // matched the colon, now we expect the type
    // which can only be an INTEGER keyword in our case
    if (!match(s,TOK_INTEGERKW))
    {
        error(s,"Type specified was not INTEGER");
        delete vardecl;
        return NULL;
    }

    // match a semicol
    if (!match(s,TOK_SEMICOL))
    {
        error(s,"Expected a semicolon");
        delete vardecl;
        return NULL;
    }

    return vardecl;
}

AST::ASTNode* Parser::acceptProcDecl(ParseContext &s)
{
    /** production: procdecl -> PROCEDURE IDENT ( '(' IDENT ( , IDENT )* ')' | epsilon ) ; BLOCK */

    ParseContext savestate = s;
    if (!match(s,TOK_PROCEDURE))
    {
        return NULL;
    }

    if (!match(s,TOK_IDENT))
    {
        return NULL;
    }


    AST::ASTNode *procdecl = new AST::ASTNode(AST::NODE_PROCDECL);
    procdecl->m_txt = getToken(s, -1).txt;  // store procedure name

    // store the procedure information in the symbol table!
    SymbolTable::SymbolInfo *procinfo = s.m_symTable->addProcedure(procdecl->m_txt);

    // also create a new local symbol table
    SymbolTable::ScopedTable *localSym = new SymbolTable::ScopedTable(s.m_symTable);

    // check if we have at least one argument
    if (match(s,TOK_LPAREN))
    {
        // yes, we have arguments!
        // store them in an NODE_ARGDECL node
        do
        {
            AST::ASTNode *argdecl = new AST::ASTNode(AST::NODE_ARGDECL);
            if (!match(s, TOK_IDENT))
            {
                error(s, "Identifier expected");
                s = savestate;
                return NULL;
            }
            argdecl->m_txt = getToken(s, -1).txt;
            procdecl->m_children.push_back(argdecl);

            // add argument tot the procedure definition
            // in the symbol table
            SymbolTable::SymbolInfo arg;
            arg.m_name = argdecl->m_txt;
            arg.m_type = SymbolTable::SymbolInfo::TYPE_UINT16;  // only INTs for now!
            procinfo->m_args.push_back(arg);

            // add argument to the local procedure scope
            localSym->addIdentifier(argdecl->m_txt, SymbolTable::SymbolInfo::TYPE_UINT16);

        } while(match(s, TOK_COMMA));

        if (!match(s, TOK_RPAREN))
        {
            error(s, "Parentheses expected");
            s = savestate;
            return NULL;
        }
    }

    if (!match(s, TOK_SEMICOL))
    {
        error(s, "Procedure definition must end with a semicolon");
        s = savestate;
        delete procdecl;
        return NULL;
    }

    // change symbol table to internal procedure scope
    SymbolTable::ScopedTable *prevScope = s.m_symTable;
    s.m_symTable = localSym;
    AST::ASTNode *block = acceptBlock(s);
    if (block == NULL)
    {
        error(s,"Error in procedure body");
        s = savestate;
        delete procdecl;
        return NULL;
    }

    if (!match(s, TOK_SEMICOL))
    {
        error(s,"Procedure block must end with a semicolon");
        s = savestate;
        delete procdecl;
        return NULL;
    }

    procdecl->m_children.push_back(block);

    // restore scope
    s.m_symTable = prevScope;
    return procdecl;
}

AST::ASTNode* Parser::acceptConstant(ParseContext &s)
{
    AST::ASTNode *node = 0;
    // constants can be:
    //   TOK_INTEGER
    //   TOK_STRING
    if (match(s, TOK_INTEGER))
    {
        node = new AST::ASTNode(AST::NODE_DECLCONSTINTEGER);
        std::string number= getToken(s, -1).txt;
        node->m_integer = atoi(number.c_str());
        return node;
    }
    if (match(s, TOK_STRING))
    {
        node = new AST::ASTNode(AST::NODE_CONSTSTRING);
        node->m_string = getToken(s, -1).txt;
        return node;
    }
    return NULL;
}

AST::ASTNode* Parser::acceptStatement(ParseContext &s)
{
    // for testing purposes,
    // statement -> FOR statement | IF statement | progblock | assignment

    // is it a FOR statement?
    AST::ASTNode *statement = acceptForStatement(s);
    if (statement != NULL)
    {
        return statement;
    }

    // is it an IF statement?
    statement = acceptIfStatement(s);
    if (statement != NULL)
    {
        return statement;
    }

    // is it a WRITE function?
    statement = acceptWriteFunction(s);
    if (statement != NULL)
    {
        return statement;
    }

    // is it a procedure call
    statement = acceptProcedureCall(s);
    if (statement != NULL)
    {
        return statement;
    }

    // is this an assigment?
    statement = acceptAssignment(s);
    if (statement != NULL)
    {
        return statement;
    }

    // is this a BEGIN .. END block?
    statement = acceptProgBlock(s);
    if (statement != NULL)
    {
        return statement;
    }

#if 0
    while(!match(s, TOK_SEMICOL))
    {
        // check for EOF match
        if (match(s, TOK_EOF))
        {
            error(s,"Unexpected end of file");
            return NULL;
        }

        next(s); // skip token
    }

    AST::ASTNode *statement = new AST::ASTNode(AST::NODE_STATEMENT);
#endif
    return NULL;
}



AST::ASTNode* Parser::acceptForStatement(ParseContext &s)
{
    // production: forstatement ->
    //   FOR IDENT := EXPRESSION (TO|DOWNTO) EXPRESSION DO statement

    ParseContext savestate = s;
    if (!match(s,TOK_FOR))
    {
        return NULL;
    }
    if (!match(s,TOK_IDENT))
    {
        return NULL;
    }
    if (!match(s,TOK_ASSIGN))
    {
        return NULL;
    }
    AST::ASTNode *fornode = new AST::ASTNode(AST::NODE_FORSTATEMENT);
    fornode->m_txt = getToken(s,-2).txt; // save loop variable/identifier

    AST::ASTNode *expr1 = acceptExpression(s);
    if (expr1 == NULL)
    {
        error(s,"Expected an expression in FOR statement");
        s = savestate;
        return NULL;
    }
    fornode->m_children.push_back(expr1);

    if (match(s,TOK_TO))
    {
        fornode->m_integer = 1; //increment
    }
    else if (match(s,TOK_DOWNTO))
    {
        fornode->m_integer = -1; //decrement
    }
    else
    {
        // nothing matched!
        error(s, "TO or DOWNTO exepected in FOR statement");
        delete fornode;
        return NULL;
    }

    AST::ASTNode *expr2 = acceptExpression(s);
    if (expr2 == NULL)
    {
        error(s,"Expected an expression in FOR statement");
        s = savestate;
        delete fornode;
        return NULL;
    }
    fornode->m_children.push_back(expr2);

    if (!match(s, TOK_DO))
    {
        delete fornode;
        return NULL;
    }

    AST::ASTNode *statement = acceptStatement(s);
    if (statement == NULL)
    {
        error(s,"Statement expected");
        delete fornode;
        return NULL;
    }
    fornode->m_children.push_back(statement);

    return fornode;
}


AST::ASTNode* Parser::acceptIfStatement(ParseContext &s)
{
    // production: forstatement ->
    //   IF expression THEN statement (ELSE statement | epsilon)

    ParseContext savestate = s;
    if (!match(s,TOK_IF))
    {
        return NULL;
    }

    AST::ASTNode *expr = acceptExpression(s);
    if (expr == NULL)
    {
        error(s, "Expression expected after IF keyword");
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_THEN))
    {
        error(s, "THEN keyword expected");
        delete expr;
        s = savestate;
        return NULL;
    }

    AST::ASTNode *statement = acceptStatement(s);
    if (statement == NULL)
    {
        error(s, "Statement expected after THEN keyword");
        delete expr;
        s = savestate;
        return NULL;
    }

    // check for optional ELSE
    AST::ASTNode *elseStatement = NULL;
    if (match(s,TOK_ELSE))
    {
        elseStatement = acceptStatement(s);
        if (elseStatement == NULL)
        {
            error(s, "Statement expected after ELSE keyword");
            delete expr;
            delete statement;
            s = savestate;
            return NULL;
        }
    }

    // create IF node
    AST::ASTNode *ifnode = new AST::ASTNode(AST::NODE_IFSTATEMENT);
    ifnode->m_children.push_back(expr);
    ifnode->m_children.push_back(statement);
    if (elseStatement != NULL)
    {
        ifnode->m_children.push_back(elseStatement);
    }

    return ifnode;
}


AST::ASTNode* Parser::acceptWriteFunction(ParseContext &s)
{
    // WRITE '(' expression (, expression)* ')'

    ParseContext savestate = s;
    if (!match(s, TOK_WRITE))
    {
        return NULL;
    }

    if (!match(s, TOK_LPAREN))
    {
        error(s,"Expected left parentheses after WRITE function");
        return NULL;
    }

    AST::ASTNode *writeNode = new AST::ASTNode(AST::NODE_WRITE);
    do
    {
        AST::ASTNode *expr = acceptExpression(s);
        if (expr == NULL)
        {
            error(s, "Expected an expression in WRITE function argument");
            delete writeNode;
            s = savestate;
            return NULL;
        }
        writeNode->m_children.push_back(expr);
    } while(match(s, TOK_COMMA));

    if (!match(s, TOK_RPAREN))
    {
        error(s, "Expected closing parentheses in WRITE function");
        delete writeNode;
        s = savestate;
        return NULL;
    }

    return writeNode;
}


AST::ASTNode* Parser::acceptAssignment(ParseContext &s)
{
    // assignment -> IDENT := expression
    ParseContext savestate = s;

    if (!match(s, TOK_IDENT))
    {
        return NULL;
    }

    if (!match(s, TOK_ASSIGN))
    {
        return NULL;
    }

    std::string name = getToken(s, -2).txt;

    AST::ASTNode *expr = acceptExpression(s);
    if (expr == NULL)
    {
        s = savestate;
        error(s, "Expected an expression");
        return NULL;
    }

    AST::ASTNode *assign = new AST::ASTNode(AST::NODE_ASSIGN);
    assign->m_txt = name;
    assign->m_children.push_back(expr);

    return assign;
}

AST::ASTNode* Parser::acceptExpression(ParseContext &s)
{
    // expression -> simple_expr | (simple_expr LOGICOP simple_expr);

    AST::ASTNode *simple_expr = acceptSimpleExpr(s);
    if (simple_expr == NULL)
    {
        return NULL;
    }

    // check for logic op
    AST::ASTNode *logic = acceptLogic(s);
    if (logic == NULL)
    {
        // only a simple_expr
        return simple_expr;
    }

    // accepted a logic operation, so the top node
    // is a logic operation node
    logic->m_children.push_back(simple_expr);

    simple_expr = acceptSimpleExpr(s);
    if (simple_expr == NULL)
    {
        error(s,"Expected an expression after a logic operator");
        delete logic;
        return NULL;
    }

    logic->m_children.push_back(simple_expr);

    return logic;
}


AST::ASTNode* Parser::acceptSimpleExpr(ParseContext &s)
{
    AST::ASTNode *topnode = 0;

    // production: (+|-|epsilon) term ((+|-) term)*
    if (match(s,TOK_PLUS))
    {
        // we ignore the plus sign, as it does not
        // change the result of the
        // expression.
    }
    else if (match(s,TOK_MINUS))
    {
        // we need to add a top negation node!
        topnode = new AST::ASTNode(AST::NODE_ARITH);
        topnode->m_optype = AST::ANODE_UMINUS;
    }

    // This is where is gets slightly more difficult:
    // The first terms encountered must end up
    // on the left side of the tree, at the
    // very bottom, so they are evaluated last.
    //
    // so T1 + T2 - T3 becomes
    //
    //       (-)
    //      /   \
    //    (+)   T3
    //   /   \
    //  T1   T2
    //

    // we need at least one term
    // if there isn't already a top node
    // we create one using this term
    // otherwise, we add it to the
    // unary minus topnode.
    AST::ASTNode *term1 = acceptTerm(s);
    if (term1 == NULL)
    {
        if (topnode != 0)
        {
            delete topnode;
        }
        return NULL;
    }
    if (topnode == 0)
    {
        topnode = term1;
    }
    else
    {
        topnode->m_children.push_back(term1);
    }

    bool accepted = true;
    while(accepted)
    {
        if (match(s,TOK_PLUS))
        {
            AST::ASTNode *newNode = new AST::ASTNode(AST::NODE_ARITH);
            newNode->m_children.push_back(topnode);
            newNode->m_optype = AST::ANODE_PLUS;
            topnode = newNode;
        }
        else if (match(s, TOK_MINUS))
        {
            AST::ASTNode *newNode = new AST::ASTNode(AST::NODE_ARITH);
            newNode->m_children.push_back(topnode);
            newNode->m_optype = AST::ANODE_MINUS;
            topnode = newNode;
        }
        else
        {
            // no new term found:
            // epsilon production!
            return topnode;
        }

        // when we end up here,
        // we've just accepted an arithmetic operation
        // that needs another term
        AST::ASTNode *term2 = acceptTerm(s);
        if (term2 == NULL)
        {
            error(s,"Expected a term after plus or minus");
            delete topnode;
            return NULL;
        }
        topnode->m_children.push_back(term2);
        accepted = true;
    }

    error(s,"Unexpected parser state in acceptSimpleExpr.");

    if (topnode != 0)
    {
        delete topnode;
    }

    return NULL;
}


AST::ASTNode* Parser::acceptTerm(ParseContext &s)
{
    // production: factor | factor ((MUL|DIV) factor)*
    AST::ASTNode *topnode = 0;

    // This is where is gets slightly more difficult:
    // The first terms encountered must end up
    // on the left side of the tree, at the
    // very bottom, so they are evaluated last.
    //
    // so T1 * T2 / T3 becomes
    //
    //       (/)
    //      /   \
    //    (*)   T3
    //   /   \
    //  T1   T2
    //

    // we need at least one factor
    AST::ASTNode *factor1 = acceptFactor(s);
    if (factor1 == NULL)
    {
        return NULL;
    }

    topnode = factor1;

    bool accepted = true;
    while(accepted)
    {
        if (match(s,TOK_STAR))
        {
            AST::ASTNode *newNode = new AST::ASTNode(AST::NODE_ARITH);
            newNode->m_children.push_back(topnode);
            newNode->m_optype = AST::ANODE_MUL;
            topnode = newNode;
        }
        else if (match(s, TOK_SLASH))
        {
            AST::ASTNode *newNode = new AST::ASTNode(AST::NODE_ARITH);
            newNode->m_children.push_back(topnode);
            newNode->m_optype = AST::ANODE_DIV;
            topnode = newNode;
        }
        else
        {
            // no new factor found:
            // epsilon production!
            return topnode;
        }

        // when we end up here,
        // we've just accepted an arithmetic operation
        // that needs another term
        AST::ASTNode *factor2 = acceptFactor(s);
        if (factor2 == NULL)
        {
            error(s,"Expected a factor after multiply or division");
            delete topnode;
            return NULL;
        }
        topnode->m_children.push_back(factor2);
        accepted = true;
    }

    error(s,"Unexpected parser state in acceptTerm.");

    if (topnode != 0)
    {
        delete topnode;
    }

    return NULL;
}

AST::ASTNode* Parser::acceptFactor(ParseContext &s)
{
    // constant | variable | procedure call | ( expression )
    //
    // for now, we'll assume an identifier is
    // always a variable.
    //

    ParseContext savestate = s;
    AST::ASTNode *node = acceptConstant(s);
    if (node != NULL)
    {
        return node;
    }

    s = savestate;

    node = acceptProcedureCall(s);
    if (node != NULL)
    {
        return node;
    }

    s = savestate;

    node = acceptVariable(s);
    if (node != NULL)
    {
        return node;
    }

    s = savestate;

    if (match(s, TOK_LPAREN))
    {
        AST::ASTNode *expr = acceptExpression(s);
        if (expr == NULL)
        {
            s = savestate;
            error(s,"Expected an expression between parentheses");
            return NULL;
        }
        if (!match(s, TOK_RPAREN))
        {
            s = savestate;
            delete expr;
            error(s,"Expected closing parentheses after expression");
            return NULL;
        }
        return expr;
    }
    else
    {
        return NULL;
    }
}

AST::ASTNode* Parser::acceptVariable(ParseContext &s)
{
    ParseContext savestate = s;

    if (match(s, TOK_IDENT))
    {
        std::string ident = getToken(s,-1).txt;
        const SymbolTable::SymbolInfo *info = s.m_symTable->lookupSymbol(ident);
        if (info == NULL)
        {
            s = savestate;
            return NULL;
        }
        AST::ASTNode *node = 0;
        switch(info->m_type)
        {
        case SymbolTable::SymbolInfo::TYPE_UINT16:
            node = new AST::ASTNode(AST::NODE_USEVARINTEGER);
            node->m_txt = ident;
            return node;
        case SymbolTable::SymbolInfo::TYPE_STRING:
            error(s,"Strings not implemented!");
            break;
        default:
            break;
        }
    }

    s = savestate;
    return NULL;
}

#if 0
{
    std::string ident = getToken(s,-1).txt;
    const SymbolTable::SymbolInfo *info = s.m_symTable->lookupSymbol(ident);
    if (info == NULL)
    {
        error(s, "Identifier is not a variable, function or procedure!");
        s = savestate;
        return NULL;
    }
    AST::ASTNode *node = 0;
    switch(info->m_type)
    {

    case SymbolTable::SymbolInfo::TYPE_FUNCTION:
        node = acceptFunctionCall(s, info);
        break;
    case SymbolTable::SymbolInfo::TYPE_PROCEDURE:
        node = acceptProcedureCall(s, info);
        break;
    default:
        error(s,"Internal compiler error (acceptFactor)");
        break;
    }
    return node;
}

#endif

AST::ASTNode* Parser::acceptLogic(ParseContext &s)
{
    // logic -> = | < | > | <> | >= | <=

    AST::ASTNode *logic = new AST::ASTNode(AST::NODE_LOGIC);
    if (match(s,TOK_EQUAL))
    {
        logic->m_optype = AST::LNODE_EQUAL;
        return logic;
    }

    if (match(s,TOK_SMALLER))
    {
        // check for equal sign
        if (match(s,TOK_EQUAL))
        {
            logic->m_optype = AST::LNODE_LESSOREQUAL;
            return logic;
        }
        else if (match(s,TOK_LARGER))
        {
            logic->m_optype = AST::LNODE_NOTEQUAL;
            return logic;
        }
        else
        {
            logic->m_optype = AST::LNODE_LESS;
            return logic;
        }
    }

    if (match(s,TOK_LARGER))
    {
        // check for equal sign
        if (match(s,TOK_EQUAL))
        {
            logic->m_optype = AST::LNODE_GREATEROREQUAL;
            return logic;
        }
        else
        {
            logic->m_optype = AST::LNODE_GREATER;
            return logic;
        }
    }

    return NULL;
}

AST::ASTNode* Parser::acceptProcedureCall(ParseContext &s)
{
    ParseContext savestate = s;

    if (!match(s, TOK_IDENT))
    {
        return NULL;
    }

    std::string ident = getToken(s,-1).txt;
    const SymbolTable::SymbolInfo *info = s.m_symTable->lookupSymbol(ident);
    if (info == NULL)
    {
        s = savestate;
        return NULL;
    }

    // bail if it's not a procedure!
    if (info->m_type != SymbolTable::SymbolInfo::TYPE_PROCEDURE)
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_LPAREN))
    {
        error(s,"Parentheses expected for procedure call");
        return NULL;
    }

    AST::ASTNode *call = new AST::ASTNode(AST::NODE_CALL);
    call->m_txt = ident;

    size_t idx = 0;
    while(idx < info->m_args.size())
    {
        AST::ASTNode *expr = acceptExpression(s);
        if (expr == NULL)
        {
            error(s,"Not enough parameters in procedure call.");
            return NULL;
        }
        idx++;
        if (idx != info->m_args.size())
        {
            if (!match(s, TOK_COMMA))
            {
                error(s,"Not enough parameters in procedure call; comma expected.");
                return NULL;
            }
        }
        call->m_children.push_back(expr);
    }

    if (!match(s, TOK_RPAREN))
    {
        error(s,"Parentheses expected for procedure call");
        delete call;
        return NULL;
    }

    return call;
}

AST::ASTNode* Parser::acceptFunctionCall(ParseContext &s)
{
    ParseContext savestate = s;

    if (!match(s, TOK_IDENT))
    {
        return NULL;
    }

    std::string ident = getToken(s,-1).txt;
    const SymbolTable::SymbolInfo *info = s.m_symTable->lookupSymbol(ident);
    if (info == NULL)
    {
        s = savestate;
        return NULL;
    }

    // bail if it's not a procedure!
    if (info->m_type != SymbolTable::SymbolInfo::TYPE_FUNCTION)
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_LPAREN))
    {
        error(s,"Parentheses expected for function call");
        return NULL;
    }

    AST::ASTNode *call = new AST::ASTNode(AST::NODE_CALL);
    call->m_txt = ident;

    size_t idx = 0;
    while(idx < info->m_args.size())
    {
        AST::ASTNode *expr = acceptExpression(s);
        if (expr == NULL)
        {
            error(s,"Not enough parameters in procedure call.");
            return NULL;
        }
        idx++;
        if (idx != info->m_args.size())
        {
            if (!match(s, TOK_COMMA))
            {
                error(s,"Not enough parameters in procedure call; comma expected.");
                return NULL;
            }
        }
        call->m_children.push_back(expr);
    }

    if (!match(s, TOK_RPAREN))
    {
        error(s,"Parentheses expected for procedure call");
        delete call;
        return NULL;
    }

    return call;
}
