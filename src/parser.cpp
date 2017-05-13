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
//   parseContext
// ********************************************************************************

#if 0
void ParseContext::addStatement(ASTNode *statement)
{
    m_statements.push_back(statement);
}

int32_t ParseContext::createVariable(const std::string &name, varInfo::type_t varType)
{
    int32_t checkIdx = getVariableByName(name);
    if (checkIdx == -1)
    {
        varInfo info;
        info.m_name = name;
        info.m_type = varType;
        m_variables.push_back(info);
        return (int32_t)(m_variables.size()-1);
    }
    else
    {
        return checkIdx;
    }
}

int32_t ParseContext::getVariableByName(const std::string &name)
{
    const int32_t N=(int32_t)m_variables.size();

    for(int32_t i=0; i<N; i++)
    {
        if (m_variables[i].m_name == name)
            return i;
    }
    return -1;
}
#endif

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
    std::cout << txt.c_str() << std::endl;
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
    /** production: prog_block -> BEGIN (STATEMENT)* END */
    AST::ASTNode *prgblock = new AST::ASTNode(AST::NODE_PROGBLOCK);

    ParseContext savestate = s;
    if (!match(s,TOK_BEGIN))
    {
        s = savestate;
        return NULL;
    }

    // eat all the statements
    bool accepted = true;
    while(accepted)
    {
        accepted = false;
        AST::ASTNode *node = acceptStatement(s);
        if (node != 0)
        {
            // add statement to the program block
            prgblock->m_children.push_back(node);
            accepted = true;
        }
    }

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
    /** production: constdecl -> CONST IDENT = CONSTANT ; (const_opt)* */
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
    constdecl->m_children.push_back(constant);

    // we have found one constant,
    // now we check for more...

    do
    {
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
                return NULL;
            }
            constdecl->m_children.push_back(constant);
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
    AST::ASTNode *node = new AST::ASTNode(AST::NODE_VARINTEGER);

    // we've found an identifier: our first variable
    node->m_txt = getToken(s, -1).txt;  // store var name
    vardecl->m_children.push_back(node);

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
            node = new AST::ASTNode(AST::NODE_VARINTEGER);
            node->m_txt = getToken(s, -1).txt;  // store var name
            vardecl->m_children.push_back(node);
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



AST::ASTNode* Parser::acceptConstant(ParseContext &s)
{
    AST::ASTNode *node = 0;
    // constants can be:
    //   TOK_INTEGER
    //   TOK_STRING
    if (match(s, TOK_INTEGER))
    {
        node = new AST::ASTNode(AST::NODE_CONSTINTEGER);
        std::string number= getToken(s, -1).txt;
        node->m_integer = atoi(number.c_str());
        return node;
    }
    if (match(s, TOK_STRING))
    {
        node = new AST::ASTNode(AST::NODE_CONSTSTRING);
        node->m_txt = getToken(s, -1).txt;
        return node;
    }
    return NULL;
}

AST::ASTNode* Parser::acceptStatement(ParseContext &s)
{
    // for testing purposes,
    // statement -> FOR statement | progblock | assignment

    // is it a FOR statement?
    AST::ASTNode *statement = acceptForStatement(s);
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

AST::ASTNode* Parser::acceptAssignment(ParseContext &s)
{
    // assignment -> IDENT := expression ;
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

    if (!match(s, TOK_SEMICOL))
    {
        error(s,"Semicolon expected at end of assignment statement");
        delete assign;
        return NULL;
    }

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
    // constant | IDENTIFIER | ( expression )
    //
    // for now, we'll assume an identifier is
    // always a variable.
    //

    ParseContext savestate = s;
    AST::ASTNode *constNode = acceptConstant(s);
    if (constNode != NULL)
    {
        return constNode;
    }

    s = savestate;

    if (match(s, TOK_IDENT))
    {
        // assume it's a variable
        AST::ASTNode *variable = new AST::ASTNode(AST::NODE_VARINTEGER);
        variable->m_txt = getToken(s,-1).txt;
        return variable;
    }
    else if (match(s, TOK_LPAREN))
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

#if 0

AST::ASTNode* Parser::acceptAssignment(ParseContext &s)
{
    // production: IDENT EQUAL expr SEMICOL
    ParseContext savestate = s;

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_EQUAL))
    {
        error(s,"Expected '='");
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -2).txt;

    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == 0)
    {
        //error(s,"Expression expected");
        s = savestate;
        return NULL;
    }

    /* we've match an assignment node! */
    int32_t varIdx = s.createVariable(identifier);
    ASTNode *assignNode = 0;
    if (s.m_variables[varIdx].m_type == varInfo::TYPE_DELAY)
    {
        // this is a delay assignment!
        assignNode = new ASTNode(ASTNode::NodeDelayAssign);
        assignNode->m_varIdx = varIdx;
        assignNode->right = exprNode;
    }
    else
    {
        // this is regular assignment!
        assignNode = new ASTNode(ASTNode::NodeAssign);
        assignNode->m_varIdx = varIdx;
        assignNode->right = exprNode;
    }

    return assignNode;
}


ASTNode* Parser::acceptDelayDefinition(ParseContext &s)
{
    // production: DELAY IDENT '[' INTEGER ']'
    ParseContext savestate = s;

    if (!match(s,TOK_DELAY))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_LBRACKET))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_INTEGER))
    {
        error(s,"Integer expected");
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_RBRACKET))
    {
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -4).txt;
    std::string lenstr     = getToken(s,-2).txt;

    int32_t delayLen = atoi(lenstr.c_str());
    if (delayLen <= 0)
    {
        error(s, "Delay length cannot be zero or negative!");
        return NULL;
    }

    /* create delay variable.
       Note: do not allocate the delay memory here
             because the destructor will de-allocate it
             and varInfo's are copied before they
             reach the virtual machine.

             Allocation is done by the VM itself!
    */

    ASTNode *delayNode = new ASTNode(ASTNode::NodeDelayDefinition);
    delayNode->m_varIdx = s.createVariable(identifier, varInfo::TYPE_DELAY);
    s.m_variables[delayNode->m_varIdx].m_length = delayLen;
    return delayNode;
}


ASTNode* Parser::acceptExpr(ParseContext &s)
{
    // productions: term expr'
    //
    // the term is the first term
    // and must therefore be
    // added as the left leaf
    //

    ParseContext savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptTerm(s)) != NULL)
    {
        // the term is the left-hand size of the expr'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, exprAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *exprAccentNode = acceptExprAccent(s, leftNode);
        return exprAccentNode;
    }
    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptExprAccent(ParseContext &s, ASTNode *leftNode)
{
    // production: - term expr' | + term expr' | epsilon
    //
    // we already have the left-hand side of the
    // addition or subtraction.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    ParseContext savestate = s;

    ASTNode *topNode = 0;
    if ((topNode = acceptExprAccent1(s, leftNode)) != 0)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptExprAccent2(s, leftNode)) != 0)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptExprAccent1(ParseContext &s, ASTNode *leftNode)
{
    // production: - term expr'
    ParseContext savestate = s;

    if (!match(s, TOK_MINUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeSub);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptExprAccent2(ParseContext &s, ASTNode *leftNode)
{
    // production: + term expr'
    ParseContext savestate = s;

    if (!match(s, TOK_PLUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeAdd);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTerm(ParseContext &s)
{
    // production: factor term'
    ParseContext savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptFactor(s)) != NULL)
    {
        // the term is the left-hand size of the term'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, termAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *termAccentNode = acceptTermAccent(s, leftNode);
        return termAccentNode;
    }
    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptTermAccent(ParseContext &s, ASTNode *leftNode)
{
    // production: * factor term' | / factor term' | epsilon
    //
    // we already have the left-hand side of the
    // multiplication or division.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    ParseContext savestate = s;

    ASTNode *topNode = 0;
    if ((topNode = acceptTermAccent1(s, leftNode)) != 0)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptTermAccent2(s, leftNode)) != 0)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptTermAccent1(ParseContext &s, ASTNode* leftNode)
{
    // production: * factor term'
    ParseContext savestate = s;

    if (!match(s, TOK_STAR))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeMul);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTermAccent2(ParseContext &s, ASTNode* leftNode)
{
    // production: / factor term'
    ParseContext savestate = s;

    if (!match(s, TOK_SLASH))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeDiv);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptTermAccent will never return NULL
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}


ASTNode* Parser::acceptFactor(ParseContext &s)
{
    ParseContext savestate = s;

    // DELAYIDENT '[' expr ']'
    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor1(s)) != NULL)
    {
        return factorNode;
    }

    // FUNCTION ( expr )
    if ((factorNode=acceptFactor2(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // ( expr )
    if ((factorNode=acceptFactor3(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // - factor
    if ((factorNode=acceptFactor4(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    if (match(s, TOK_INTEGER))
    {
        factorNode = new ASTNode(ASTNode::NodeInteger);
        factorNode->m_literalInt = atoi(getToken(s, -1).txt.c_str());
        return factorNode;    // INTEGER
    }

    if (match(s, TOK_FLOAT))
    {
        factorNode = new ASTNode(ASTNode::NodeFloat);
        factorNode->m_literalFloat = atof(getToken(s, -1).txt.c_str());
        return factorNode;    // FLOAT
    }

    if (match(s, TOK_IDENT))
    {        
        // we have to check here if we have a delay
        // variable because these cannot be used as
        // regular variables
        uint32_t varIdx = s.getVariableByName(getToken(s, -1).txt);
        if (varIdx != -1)
        {
           if (s.m_variables[varIdx].m_type == varInfo::TYPE_DELAY)
           {
               error(s, "Cannot use a delay variable without an index");
               return NULL;
           }
        }

        // if needed, create the variable
        varIdx = s.createVariable(getToken(s,-1).txt);
        factorNode = new ASTNode(ASTNode::NodeIdent);
        factorNode->m_varIdx = varIdx;
        return factorNode;    // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return NULL;
}


ASTNode* Parser::acceptFactor1(ParseContext &s)
{
    // DELAYIDENT '[' expr ']'
    ParseContext savestate = s;
    if (!match(s, TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_LBRACKET))
    {
        s = savestate;
        return NULL;
    }

    std::string varname = getToken(s,-2).txt;

    // get the expression argument
    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // check if identifier is of a delay type!
    int32_t varIdx = s.getVariableByName(varname);
    if (varIdx < 0)
    {
        delete exprNode;
        s = savestate;
        error(s,"Undefined delay variable!");
        return NULL;
    }

    if (!match(s, TOK_RBRACKET))
    {
        delete exprNode;
        s = savestate;
        return NULL;
    }

    // create a delayline lookup node
    ASTNode *delayNode = new ASTNode(ASTNode::NodeDelayLookup);
    delayNode->m_varIdx = varIdx;
    delayNode->left = exprNode;
    return delayNode;
}


ASTNode* Parser::acceptFactor2(ParseContext &s)
{
    // production: FUNCTION ( expr )

    ParseContext savestate = s;
    token_t func = getToken(s);
    if (func.tokID < 100)
    {
        s = savestate;
        return NULL;
    }
    next(s);

    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }

    // check how many argument this function expects

    uint32_t nargs = functionDefs::getNumberOfArguments(func.tokID);
    if (nargs == -1)
    {
        // function not found!
        // this is a severe error because the tokenizer recognized
        // the function but for some reason we can't find it
        // here anymore.. !?!
        error(s, "Internal parser error: cannot find function!");
        return NULL;
    }

    ASTNode* factorNode = new ASTNode(ASTNode::NodeFunction);
    factorNode->m_functionID = func.tokID;
    factorNode->left = 0;
    factorNode->right = 0;

    uint32_t argcnt = 0;
    while(argcnt < nargs)
    {
        ASTNode *exprNode = 0;
        if ((exprNode=acceptExpr(s)) == NULL)
        {
            error(s,"Invalid argument or number of arguments");
            s = savestate;
            delete factorNode;
            return NULL;
        }
        factorNode->m_functionArgs.push_back(exprNode);
        argcnt++;

        // if there are arguments left, we need to see a comma
        if (argcnt != nargs)
        {
            if (!match(s, TOK_COMMA))
            {
                error(s,"Expected a comma in arguments list");
                s = savestate;
                delete factorNode;
                return NULL;
            }
        }
    }

    if (!match(s, TOK_RPAREN))
    {
        delete factorNode;
        s = savestate;
        return NULL;
    }

    return factorNode;
}


ASTNode* Parser::acceptFactor3(ParseContext &s)
{
    ParseContext savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }
    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }
    if (!match(s, TOK_RPAREN))
    {
        delete exprNode;
        s = savestate;
        return NULL;
    }
    return exprNode;
}


ASTNode* Parser::acceptFactor4(ParseContext &s)
{
    // production: - factor
    ParseContext savestate = s;
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return NULL;
    }
    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // unary minus node
    ASTNode *exprNode = new ASTNode(ASTNode::NodeUnaryMinus);
    exprNode->left = 0;
    exprNode->right = factorNode;

    return exprNode;
}

#endif
