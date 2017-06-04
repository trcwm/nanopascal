/*

  Abstract syntax tree for the Micro Pascal
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#ifndef ast_h
#define ast_h

#include <string>
#include <vector>
#include <stdint.h>

// forward declaration
namespace SymbolTable {
    class ScopedTable;
}

namespace AST
{

enum ASTNodeType
{
    NODE_UNDEFINED = 0,
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_PROGBLOCK,
    /* DECLARATION STATEMENTS */
    NODE_CONSTDECL,
    NODE_VARDECL,
    NODE_PROCDECL,
    NODE_ARGDECL,
    NODE_DECLVARINTEGER,
    NODE_DECLCONSTINTEGER,
    NODE_CONSTSTRING,
    /* IDENTIFIER TYPES */
    NODE_STATEMENT,         // can probably be removed
    NODE_EXPR,              // can probably be removed
    NODE_IDENT,             // can probably be removed
    NODE_VARINTEGER,
    NODE_LITERALINTEGER,
    /* COMPOUND STATEMENTS */
    NODE_FORSTATEMENT,
    NODE_IFSTATEMENT,
    NODE_WRITE,
    /* ARITH AND LOGIC OPERATORS */
    NODE_LOGIC,
    NODE_ARITH,
    NODE_ASSIGN,
    /* OTHER */
    NODE_FUNCCALL
};

enum ASTNodeOperationType
{
    LNODE_EQUAL,
    LNODE_GREATER,
    LNODE_GREATEROREQUAL,
    LNODE_LESS,
    LNODE_LESSOREQUAL,
    LNODE_NOTEQUAL,
    LNODE_NOT,
    ANODE_MINUS,
    ANODE_PLUS,
    ANODE_UMINUS,
    ANODE_MUL,
    ANODE_DIV,
    ANODE_COPY      // no operation, just copy the value
};


/** Abstract Syntax Tree node with visitor support */
struct ASTNode
{
    ASTNode(ASTNodeType type)
    {
        m_type = type;
    }

    ~ASTNode()
    {
        for(uint32_t i=0; i<m_children.size(); i++)
        {
            delete m_children[i];
        }
    }

    ASTNodeType             m_type;         // node type
    std::vector<ASTNode*>   m_children;     // child nodes, from left to right

    std::string             m_txt;          // ident name
    std::string             m_string;       // string (const) value
    int32_t                 m_integer;      // integer, for inc/dec
    ASTNodeOperationType    m_optype;       // logic type

    std::vector<std::string > m_argNames;   // function argument names
};

/** print the abstract syntax tree to the console using an indented format */
void dumpASTree(const ASTNode *head, uint32_t level = 0);

/** dump the abstract syntax tree to a file in Graphviz DOT format */
void dumpASTreeToDot(const ASTNode *head, const char *filename);

} // end namespace

#endif
