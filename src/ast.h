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

namespace AST
{

enum IdentType
{
    VI_UNDEFINED = 0,
    VI_INTEGER   = 1,
    VI_STRING    = 2,
    VI_FUNCTION  = 3,
    VI_PROCEDURE = 4
};

#define VIFLAG_CONSTANT 0x1
#define VIFLAG_GLOBAL   0x2

/** information concerning a identifiers */
struct IdentInfo
{
    IdentType       m_type;
    std::string     m_name;     // variable name
    uint32_t        m_flags;    // VIFLAGs
};

enum ASTNodeType
{
    NODE_UNDEFINED = 0,
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_PROGBLOCK,
    NODE_CONSTDECL,
    NODE_VARDECL,
    NODE_STATEMENT,
    NODE_EXPR,
    NODE_IDENT,
    NODE_CONSTINTEGER,
    NODE_CONSTSTRING,
    NODE_VARINTEGER,
    NODE_FORSTATEMENT,
    NODE_LOGIC,
    NODE_ARITH,
    NODE_LITERALINTEGER,
    NODE_ASSIGN
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


struct ASTNode
{
    ASTNode(ASTNodeType type) {m_type = type;}

    ~ASTNode()
    {
        for(uint32_t i=0; i<m_children.size(); i++)
        {
            delete m_children[i];
        }
    }

    ASTNodeType             m_type;       // node type
    std::vector<ASTNode*>   m_children;   // child nodes, from left to right

    std::string             m_txt;        // ident name or const string.
    int32_t                 m_integer;    // integer, for inc/dec
    ASTNodeOperationType    m_optype;     // logic type
};

void dumpASTree(const ASTNode *head, uint32_t level = 0);

} // end namespace

#endif
