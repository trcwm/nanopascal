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
    //case NODE_EXPR:
    //    printf("EXPR\n");
    //    break;
    case NODE_IDENT:
        printf("IDENT %s\n", node->m_txt.c_str());
        break;
    case NODE_DECLCONSTINTEGER:
        printf("%d\n", node->m_integer);
        break;
    case NODE_CONSTSTRING:
        printf("%s\n", node->m_txt.c_str());
        break;
    case NODE_DECLVARINTEGER:
        printf("DECLARE VAR %s\n", node->m_txt.c_str());
        break;
    case NODE_VARINTEGER:
        printf("LOAD VAR %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_PROCDECL:
        printf("PROCDECL %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_ARGDECL:
        printf("ARG %s\n", node->m_txt.c_str());
        break;
    case AST::NODE_FUNCCALL:
        printf("CALL %s\n", node->m_txt.c_str());
        break;
    case NODE_FORSTATEMENT:
        printf("FOR %s (update=%d)\n", node->m_txt.c_str(), node->m_integer);
        break;
    case NODE_IFSTATEMENT:
        if (node->m_children.size() == 3)
        {
            printf("IF THEN ELSE\n");
        }
        else
        {
            printf("IF THEN\n");
        }
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
    case NODE_WRITE:
        printf("WRITE\n");
        break;
    }

    // iterate child nodes
    for(uint32_t i=0; i<node->m_children.size(); i++)
    {
        dumpASTree(node->m_children[i], level+1);
    }
}



// write a node to the DOT file
uint32_t writeNode(FILE *fout, const AST::ASTNode *node, uint32_t nodeID)
{
    std::vector<uint32_t> m_childID;

    uint32_t thisNodeID = nodeID;
    for(uint32_t i=0; i<node->m_children.size(); i++)
    {
        m_childID.push_back(nodeID+1);
        uint32_t cID = writeNode(fout, node->m_children[i], nodeID+1);
        nodeID = cID;
    }

    fprintf(fout, " %d [label=\"", thisNodeID);
    switch(node->m_type)
    {
    case AST::NODE_PROGRAM:
        fprintf(fout, "PROGRAM");
        break;
    case AST::NODE_BLOCK:
        fprintf(fout, "BLOCK");
        break;
    case AST::NODE_PROGBLOCK:
        fprintf(fout, "PROGBLOCK");
        break;
    case AST::NODE_CONSTDECL:
        fprintf(fout, "CONSTDECL");
        break;
    case AST::NODE_VARDECL:
        fprintf(fout, "VARDECL");
        break;
    case AST::NODE_STATEMENT:
        fprintf(fout, "STATEMENT");
        break;
    //case AST::NODE_EXPR:
    //    fprintf(fout, "EXPR");
    //    break;
    case AST::NODE_IDENT:
        fprintf(fout, "IDENT %s", node->m_txt.c_str());
        break;
    case AST::NODE_DECLCONSTINTEGER:
        fprintf(fout, "%d", node->m_integer);
        break;
    case AST::NODE_PROCDECL:
        fprintf(fout, "PROC %s", node->m_txt.c_str());
        break;
    case AST::NODE_ARGDECL:
        fprintf(fout, "ARG %s", node->m_txt.c_str());
        break;
    case AST::NODE_FUNCCALL:
        fprintf(fout, "CALL %s", node->m_txt.c_str());
        break;
    case AST::NODE_CONSTSTRING:
        fprintf(fout, "%s", node->m_txt.c_str());
        break;
    case AST::NODE_DECLVARINTEGER:
        fprintf(fout, "INT %s", node->m_txt.c_str());
        break;
    case AST::NODE_VARINTEGER:
        fprintf(fout, "VAR %s", node->m_txt.c_str());
        break;
    case AST::NODE_FORSTATEMENT:
        if (node->m_integer > 0)
            fprintf(fout, "FOR %s ++", node->m_txt.c_str());
        else
            fprintf(fout, "FOR %s --", node->m_txt.c_str());
        break;
    case AST::NODE_IFSTATEMENT:
        if (node->m_children.size() == 3)
        {
            fprintf(fout, "IF THEN ELSE");
        }
        else
        {
            fprintf(fout, "IF THEN");
        }
        break;
    case AST::NODE_LOGIC:
    case AST::NODE_ARITH:
        switch(node->m_optype)
        {
        case AST::LNODE_EQUAL:
            fprintf(fout, "=");
            break;
        case AST::LNODE_GREATER:
            fprintf(fout, ">");
            break;
        case AST::LNODE_GREATEROREQUAL:
            fprintf(fout, ">=");
            break;
        case AST::LNODE_LESS:
            fprintf(fout, "<");
            break;
        case AST::LNODE_LESSOREQUAL:
            fprintf(fout, "<=");
            break;
        case AST::LNODE_NOTEQUAL:
            fprintf(fout, "<>");
            break;
        case AST::LNODE_NOT:
            fprintf(fout, "NOT");
            break;
        case AST::ANODE_MINUS:
            fprintf(fout, "-");
            break;
        case AST::ANODE_PLUS:
            fprintf(fout, "+");
            break;
        case AST::ANODE_MUL:
            fprintf(fout, "*");
            break;
        case AST::ANODE_DIV:
            fprintf(fout, "/ ");
            break;
        case AST::ANODE_UMINUS:
            fprintf(fout, "U-");
            break;
        case AST::ANODE_COPY:
            fprintf(fout, "COPY");
            break;
        default:
            fprintf(fout, "???");
            break;
        }
        break;
    case AST::NODE_LITERALINTEGER:
        fprintf(fout, "%d", node->m_integer);
        break;
    case AST::NODE_ASSIGN:
        fprintf(fout, "%s :=", node->m_txt.c_str());
        break;
    case AST::NODE_WRITE:
        fprintf(fout, "WRITE");
        break;
    }
    fprintf(fout, "\"];\n");

    for(uint32_t i=0; i<m_childID.size(); i++)
    {
        fprintf(fout, "%d -> %d;\n", thisNodeID, m_childID[i]);
    }

    return nodeID;
}



void AST::dumpASTreeToDot(const AST::ASTNode *head, const char *filename)
{
    FILE *fout = fopen(filename, "wt");
    fprintf(fout, "digraph BST {\n");
    fprintf(fout, "  node [fontname=\"Arial\"];\n");

    writeNode(fout, head, 0);

    fprintf(fout, "}\n");
    fclose(fout);
}
