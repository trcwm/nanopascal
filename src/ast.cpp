/*

  Abstract syntax tree for the Micro Pascal
  Copyright Niels A. Moseley 2017
  Moseley Instruments
  http://www.moseleyinstruments.com

  Licence: GPL v2.0

*/

#include "ast.h"

void AST::dumpASTree(const ASTNode *node, uint32_t level)
{
    // print indentation depending on level
    for(uint32_t i=0; i<level; i++)
    {
        printf(" ");
    }

    switch(node->m_type)
    {
    case NODE_PROGRAM:
        printf("PROGRAM\n");
        break;
    case NODE_BLOCK:
        printf("BLOCK\n");
        break;
    case NODE_PROGBLOCK:
        printf("PROGBLOCK\n");
        break;
    case NODE_CONSTDECL:
        printf("CONSTDECL\n");
        break;
    case NODE_VARDECL:
        printf("VARDECL\n");
        break;
    case NODE_STATEMENT:
        printf("STATEMENT\n");
        break;
    case NODE_EXPR:
        printf("EXPR\n");
        break;
    case NODE_IDENT:
        printf("IDENT %s\n", node->m_txt.c_str());
        break;
    case NODE_CONSTINTEGER:
        printf("%d\n", node->m_integer);
        break;
    case NODE_CONSTSTRING:
        printf("%s\n", node->m_txt.c_str());
        break;
    case NODE_DECLVARINTEGER:
        printf("DECLARE VAR %s\n", node->m_txt.c_str());
        break;
    case NODE_USEVARINTEGER:
        printf("LOAD VAR %s\n", node->m_txt.c_str());
        break;
    case NODE_FORSTATEMENT:
        printf("FOR %s (update=%d)\n", node->m_txt.c_str(), node->m_integer);
        break;
    case NODE_LOGIC:
    case NODE_ARITH:
        switch(node->m_optype)
        {
        case AST::LNODE_EQUAL:
            printf("=\n");
            break;
        case AST::LNODE_GREATER:
            printf(">\n");
            break;
        case AST::LNODE_GREATEROREQUAL:
            printf(">=\n");
            break;
        case AST::LNODE_LESS:
            printf("<\n");
            break;
        case AST::LNODE_LESSOREQUAL:
            printf("<=\n");
            break;
        case AST::LNODE_NOTEQUAL:
            printf("<>\n");
            break;
        case AST::LNODE_NOT:
            printf("NOT\n");
            break;
        case AST::ANODE_MINUS:
            printf("-\n");
            break;
        case AST::ANODE_PLUS:
            printf("+\n");
            break;
        case AST::ANODE_MUL:
            printf("*\n");
            break;
        case AST::ANODE_DIV:
            printf("/ \n");
            break;
        case AST::ANODE_UMINUS:
            printf("U-\n");
            break;
        case AST::ANODE_COPY:
            printf("COPY\n");
            break;
        default:
            printf("UNDEFINED ARITH NODE!\n");
            break;
        }
        break;
    case NODE_LITERALINTEGER:
        printf("%d\n", node->m_integer);
        break;
    case NODE_ASSIGN:
        printf("%s :=\n", node->m_txt.c_str());
        break;
    }

    // iterate child nodes
    for(uint32_t i=0; i<node->m_children.size(); i++)
    {
        dumpASTree(node->m_children[i], level+1);
    }
}
