/*

  Parser for the Micro Pascal
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#ifndef parser_h
#define parser_h

#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include "ast.h"
#include "tokenizer.h"

/** object that keeps track of the state */
class ParseContext
{
public:
    size_t                tokIdx;
    Reader::position_info tokPos;

#if 0
    /** get variable by name, or -1 if not found */
    int32_t getVariableByName(const std::string &name);

    /** create a variable and return its index */
    int32_t createVariable(const std::string &name, varInfo::type_t varType = varInfo::TYPE_VAR);

    /** add a statement to the list of statement */
    void addStatement(ASTNode* statement);

    /** get (const) access to the statements */
    const statements_t& getStatements() const
    {
        return m_statements;
    }
#endif

    std::vector<AST::IdentInfo> m_variables;
    AST::ASTNode*               m_astHead;
};



/** Parser to translate token stream from tokenizer/lexer to operation stack. */
class Parser
{
public:
    Parser();

    /** Process a list of tokens and list of statements.
        false is returned when a parse error occurs.
        When an error occurs, call getLastError() to get
        a human-readable string of the error.
    */
    bool process(const std::vector<token_t> &tokens, ParseContext &context);

    /** Return a description of the last parse error that occurred. */
    std::string getLastError() const
    {
        return m_lastError;
    }

    /** Get the position in the source code where the last error occurred. */
    Reader::position_info getLastErrorPos() const
    {
        return m_lastErrorPos;
    }

protected:
    /* The following methods return true if the tokens starting from
       index 'tokIdx' are consistent with the production from the
       BasicDSP grammar.
    */

    /** accept program.
        production: block EOF
    */
    bool acceptProgram(ParseContext &context);

    /** production: block -> ((constdecl)* | (vardecl)*) prog_block */
    AST::ASTNode* acceptBlock(ParseContext &s);

    /** production: constdecl -> CONST IDENT = CONSTANT ; (const_opt)* */
    AST::ASTNode* acceptConstDecl(ParseContext &s);

    /** production: const_opt -> IDENT = CONSTANT ; */
    AST::ASTNode* acceptConstOpt(ParseContext &s);

    /** production: vardecl -> VAR IDENT (, IDENT)* : INTEGERKW ; */
    AST::ASTNode* acceptVarDecl(ParseContext &s);

    /** production: prog_block -> BEGIN (statement)* END */
    AST::ASTNode* acceptProgBlock(ParseContext &s);

    /** production: statement -> everything ; */
    AST::ASTNode* acceptStatement(ParseContext &s);

    /** production: constant -> INTEGER | STRING */
    AST::ASTNode* acceptConstant(ParseContext &s);

    /** production: forstatement ->
          FOR IDENT := EXPRESSION (TO|DOWNTO) EXPRESSION DO statement
    */
    AST::ASTNode* acceptForStatement(ParseContext &s);

    /** production: simple_expr | (simple_expr logic simple_expr) */
    AST::ASTNode* acceptExpression(ParseContext &s);

    /** production: (+|-|epsilon) term ((+|-) term)* */
    AST::ASTNode* acceptSimpleExpr(ParseContext &s);

    /** production: = | > | < | <> | >= | <= */
    AST::ASTNode* Parser::acceptLogic(ParseContext &s);

    /** production: factor | factor ((MUL|DIV) factor)* */
    AST::ASTNode* Parser::acceptTerm(ParseContext &s);

    /** production: INTEGER | IDENTIFIER | ( expression ) */
    AST::ASTNode* Parser::acceptFactor(ParseContext &s);

    /** production: assignment -> IDENT = expr ; */
    AST::ASTNode* acceptAssignment(ParseContext &s);

#if 0
    /** production: assignment -> IDENT = expr */
    ASTNode* acceptAssignment(ParseContext &s);

    /** production: expr -> term expr' */
    ASTNode* acceptExpr(ParseContext &s);

    /** production: DELAY IDENTIFIER '[' INTEGER ']' */
    ASTNode* acceptDelayDefinition(ParseContext &s);

    /** production: expr' -> - term expr' | + term expr' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptExprAccent(ParseContext &s, ASTNode *leftNode);

    /** production: expr' -> - term expr' */
    ASTNode* acceptExprAccent1(ParseContext &s, ASTNode *leftNode);

    /** production: expr' -> + term expr' */
    ASTNode* acceptExprAccent2(ParseContext &s, ASTNode *leftNode);

    /** production: term -> factor term' */
    ASTNode* acceptTerm(ParseContext &s);

    /** production: term' -> * factor term' | / factor term' | e

        This function will return leftNode when an
        epsilon production is invoked. Therfore,
        it will never return NULL.
    */
    ASTNode* acceptTermAccent(ParseContext &s, ASTNode *leftNode);

    /** production: term' -> * factor term' */
    ASTNode* acceptTermAccent1(ParseContext &s, ASTNode *leftNode);

    /** production: term' -> / factor term' */
    ASTNode* acceptTermAccent2(ParseContext &s, ASTNode *leftNode);

    /** production:
       acceptFactor1 | acceptFactor2 |
       acceptFactor3 | acceptFactor4 |
       INTEGER | FLOAT | IDENT */
    ASTNode* acceptFactor(ParseContext &s);

    /** production: DELAYIDENT [ expr ] */
    ASTNode* acceptFactor1(ParseContext &s);

    /** production: FUNCTION ( expr ) */
    ASTNode* acceptFactor2(ParseContext &s);

    /** production: ( expr ) */
    ASTNode* acceptFactor3(ParseContext &s);

    /** production: - factor */
    ASTNode* acceptFactor4(ParseContext &s);

    /** match a token, return true if matched and advance the token index. */
    bool match(ParseContext &s, uint32_t tokenID);

    /** match a NULL-terminated list of tokens. */
    bool matchList(ParseContext &s, const uint32_t *tokenIDlist);

    /** Advance the token index and get the next token */
    token_t next(ParseContext &s)
    {
        s.tokIdx++;
        token_t tok = getToken(s);
        s.tokPos = tok.pos;
        return getToken(s);
    }

    /** Get the current token, which or without an offset w.r.t.*/
    token_t getToken(const ParseContext &s, int32_t offset = 0)
    {
        token_t dummy_token;

        if (m_tokens == 0)
            return dummy_token;

        if ((s.tokIdx+offset) < m_tokens->size())
        {
            return m_tokens->at(s.tokIdx+offset);
        }
        else
        {
            return dummy_token;
        }
    }

    /** Report an error */
    void error(const ParseContext &s, const std::string &txt);
    void error(const std::string &txt);

    std::string   m_lastError;
    Reader::position_info m_lastErrorPos;

#endif

    /** match a token, return true if matched and advance the token index. */
    bool match(ParseContext &s, uint32_t tokenID);

    /** Advance the token index and get the next token */
    token_t next(ParseContext &s)
    {
        s.tokIdx++;
        token_t tok = getToken(s);
        s.tokPos = tok.pos;
        return getToken(s);
    }

    /** Get the current token, which or without an offset w.r.t.*/
    token_t getToken(const ParseContext &s, int32_t offset = 0)
    {
        token_t dummy_token;

        if (m_tokens == 0)
            return dummy_token;

        if ((s.tokIdx+offset) < m_tokens->size())
        {
            return m_tokens->at(s.tokIdx+offset);
        }
        else
        {
            return dummy_token;
        }
    }

    /** Report an error */
    void error(const ParseContext &s, const std::string &txt);
    void error(const std::string &txt);

    const std::vector<token_t>  *m_tokens;
    Reader::position_info       m_lastErrorPos;
    std::string                 m_lastError;
};

#endif
