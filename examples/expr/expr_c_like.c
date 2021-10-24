/* 
** Example: c like expressions
**
** This example has the setup for parsing C expressions with the Metadesk
** expression parser.
**
** In the accompanying expr_c_like.mdesk there are more comments about using
** the expression system from the side of the Metadesk files.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

//~ expression setup and helpers //////////////////////////////////////////////

#define C_LIKE_OPS_NO_SIDE_EFFECTS(X) \
X(ArraySubscript,      "[]",        Postfix,                18) \
X(Member,              ".",         Binary,                 18) \
X(PointerMember,       "->",        Binary,                 18) \
X(UnaryPlus,           "+",         Prefix,                 17) \
X(UnaryMinus,          "-",         Prefix,                 17) \
X(LogicalNot,          "!",         Prefix,                 17) \
X(BitwiseNot,          "~",         Prefix,                 17) \
X(Dereference,         "*",         Prefix,                 17) \
X(AddressOf,           "&",         Prefix,                 17) \
X(SizeOf,              "sizeof",    Prefix,                 17) \
X(Multiplication,      "*",         Binary,                 15) \
X(Division,            "/",         Binary,                 15) \
X(Modulo,              "%",         Binary,                 15) \
X(Addition,            "+",         Binary,                 14) \
X(Subtraction,         "-",         Binary,                 14) \
X(LeftShift,           "<<",        Binary,                 13) \
X(RightShift,          ">>",        Binary,                 13) \
X(LessThan,            "<",         Binary,                 11) \
X(LessThanOrEqual,     "<=",        Binary,                 11) \
X(GreaterThan,         ">",         Binary,                 11) \
X(GreaterThanOrEqual,  ">=",        Binary,                 11) \
X(Equal,               "==",        Binary,                 10) \
X(NotEqual,            "!=",        Binary,                 10) \
X(BitwiseAnd,          "&",         Binary,                  9) \
X(BitwiseXor,          "^",         Binary,                  8) \
X(BitwiseOr,           "|",         Binary,                  7) \
X(LogicalAnd,          "&&",        Binary,                  6) \
X(LogicalOr,           "||",        Binary,                  5)

#define C_LIKE_OPS_CALLS(X) \
X(Call,                "()",        Postfix,                18)

#define C_LIKE_OPS_WITH_SIDE_EFFECTS(X) \
X(PostFixIncrement,    "++",        Postfix,                18) \
X(PostFixDecrement,    "--",        Postfix,                18) \
X(PreFixIncrement,     "++",        Prefix,                 17) \
X(PreFixDecrement,     "--",        Prefix,                 17) \
X(Assign,              "=",         BinaryRightAssociative,  3) \
X(AssignAddition,      "+=",        BinaryRightAssociative,  3) \
X(AssignSubtraction,   "-=",        BinaryRightAssociative,  3) \
X(AssignMultiplication,"*=",        BinaryRightAssociative,  3) \
X(AssignDivision,      "/=",        BinaryRightAssociative,  3) \
X(AssignModulo,        "%=",        BinaryRightAssociative,  3) \
X(AssignLeftShift,     "<<=",       BinaryRightAssociative,  3) \
X(AssignRightShift,    ">>=",       BinaryRightAssociative,  3) \
X(AssignBitwiseAnd,    "&=",        BinaryRightAssociative,  3) \
X(AssignBitwiseXor,    "^=",        BinaryRightAssociative,  3) \
X(AssignBitwiseOr,     "|=",        BinaryRightAssociative,  3)

enum Op
{
#define DEF_ENUM(e,t,k,p) Op##e,
    C_LIKE_OPS_NO_SIDE_EFFECTS(DEF_ENUM)
        C_LIKE_OPS_CALLS(DEF_ENUM)
        C_LIKE_OPS_WITH_SIDE_EFFECTS(DEF_ENUM)
#undef DEF_ENUM
};

void
print_expression(FILE *out, MD_Expr *expr)
{
    MD_ExprOpr *op = expr->op;
    if (op == 0)
    {
        MD_Node *node = expr->md_node;
        if (node->raw_string.size != 0)
        {
            fprintf(out, "%.*s", MD_S8VArg(node->raw_string));
        }
        else if (!MD_NodeIsNil(node->first_child))
        {
            char c1 = 0;
            char c2 = 0;
            if (node->flags & MD_NodeFlag_HasParenLeft)
            {
                c1 = '(';
            }
            if (node->flags & MD_NodeFlag_HasBraceLeft)
            {
                c1 = '{';
            }
            if (node->flags & MD_NodeFlag_HasBracketLeft)
            {
                c1 = '[';
            }
            if (node->flags & MD_NodeFlag_HasParenRight)
            {
                c2 = ')';
            }
            if (node->flags & MD_NodeFlag_HasBraceRight)
            {
                c2 = '}';
            }
            if (node->flags & MD_NodeFlag_HasBracketRight)
            {
                c2 = ']';
            }
            fprintf(out, "%c...%c", c1, c2);
        }
        else
        {
            MD_CodeLoc loc = MD_CodeLocFromNode(node);
            MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                            MD_S8Lit("the expression system does not expect this kind of node"));
        }
    }
    else
    {
        switch (op->kind)
        {
            default:
            {
                MD_Node *node = expr->md_node;
                MD_CodeLoc loc = MD_CodeLocFromNode(node);
                MD_PrintMessage(stderr, loc, MD_MessageKind_FatalError,
                                MD_S8Lit("this is an unknown kind of operator"));
            }break;
            
            case MD_ExprOprKind_Prefix:
            {
                MD_Node *node = expr->md_node;
                fprintf(out, "%.*s(", MD_S8VArg(op->string));
                print_expression(out, expr->unary_operand);
                fprintf(out, ")");
            }break;
            
            case MD_ExprOprKind_Postfix:
            {
                fprintf(out, "(");
                print_expression(out, expr->unary_operand);
                MD_String8 op_string = op->string;
                if ((expr->md_node->flags & MD_NodeFlag_MaskSetDelimiters) != 0)
                {
                    fprintf(out, ")%c...%c", op_string.str[0], op_string.str[1]);
                }
                else
                {
                    fprintf(out, ")%.*s", MD_S8VArg(op_string));
                }
            }break;
            
            case MD_ExprOprKind_Binary:
            case MD_ExprOprKind_BinaryRightAssociative:
            {
                fprintf(out, "(");
                print_expression(out, expr->left);
                fprintf(out, " %.*s ", MD_S8VArg(op->string));
                print_expression(out, expr->right);
                fprintf(out, ")");
            }break;
        }
    }
}


//~ main //////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
#if 1
    char *argv_dummy[2] = {
        0,
        "W:\\metadesk\\examples\\expr\\expr_c_like.mdesk",
    };
    argc = 2;
    argv = argv_dummy;
#endif
    
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for (int i = 1; i < argc; i += 1)
    {
        
        // parse the file
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        // print metadesk errors
        for (MD_Message *message = parse_result.errors.first;
             message != 0;
             message = message->next)
        {
            MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(stdout, code_loc, message->kind, message->string);
        }
        
        // save to parse results list
        if (parse_result.errors.max_message_kind < MD_MessageKind_Error)
        {
            MD_PushNewReference(arena, list, parse_result.node);
        }
    }
    
    // setup the expression system
    MD_ExprOprTable table = {0};
    {
        MD_ExprOprList list = {0};
        
#define PUSH_OP(e,t,k,p) \
MD_ExprOprPush(arena, &list, MD_ExprOprKind_##k, p, MD_S8Lit(t), Op##e, 0);
        C_LIKE_OPS_NO_SIDE_EFFECTS(PUSH_OP);
        C_LIKE_OPS_CALLS(PUSH_OP);
        C_LIKE_OPS_WITH_SIDE_EFFECTS(PUSH_OP);
#undef PUSH_OP
        
        table = MD_ExprBakeOprTableFromList(arena, &list);
    }
    
    // print the verbose parse results
    for (MD_EachNode(root_it, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(root_it);
        for (MD_EachNode(node, root->first_child))
        {
            MD_ExprParseResult parse = MD_ExprParse(arena, &table, node->first_child, MD_NilNode());
            
            // print errors
            for (MD_Message *message = parse.errors.first;
                 message != 0;
                 message = message->next)
            {
                MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
                MD_PrintMessage(stdout, code_loc, message->kind, message->string);
            }
            
            // print the expression
            if (parse.expr != 0)
            {
                if (node->string.size != 0)
                {
                    fprintf(stdout, "%.*s = ", MD_S8VArg(node->string));
                }
                print_expression(stdout, parse.expr);
                fprintf(stdout, ";\n");
            }
        }
    }
    
    return 0;
}
