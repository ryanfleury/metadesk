/* 
** Example: expressions intro
**
** This example shows how to use expression parsing in Metadesk. There is
** commentary about the setup as well as the features and limits of the
** expression parser.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

//~ expression setup and helpers //////////////////////////////////////////////

// @notes A common easy setup is to give each operator a statically known
//  integer code. We can switch on these integer codes later to interpret the
//  operator nodes in expressions.
enum
{
    OpAdd,
    OpMul,
    OpIllegal,
};

void
print_expression(FILE *out, MD_Expr *expr)
{
    MD_ExprOpr *op = expr->op;
    if (op == 0)
    {
        // @notes Any MD_Expr that doesn't have an operator attached must be a
        //  leaf of the expression. In this example we don't want to recognize
        //  any nodes with set delimiters as leaves.
        //
        //  Every expression node (MD_Expr) has an `md_node` regardless of
        //  whether it is a leaf or an operator. This node gives us a way to
        //  see information about this leaf, and also gives a way to create an
        //  MD_CodeLoc for error messages. The same works on operator nodes.
        MD_Node *node = expr->md_node;
        if ((node->flags & MD_NodeFlag_MaskSetDelimiters) == 0)
        {
            fprintf(out, "%.*s", MD_S8VArg(node->raw_string));
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
        // @notes Any MD_Expr that does have an operator attached is an
        //  internal node of the expression. In this example we only setup
        //  binary operators, so we don't bother looking at what kind of
        //  operator this is.
        fprintf(out, "(");
        print_expression(out, expr->left);
        fprintf(out, " %.*s ", MD_S8VArg(op->string));
        print_expression(out, expr->right);
        fprintf(out, ")");
        
        if (op->op_id == OpIllegal)
        {
            MD_CodeLoc loc = MD_CodeLocFromNode(expr->md_node);
            MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                            MD_S8Lit("this operator is not actually legal"));
        }
    }
}

// @notes Commonly a useful thing to do with an expression system is to
//  evaluate the expressions. Here's a quick sketch of what that might look
//  like.

MD_Map eval_map = {0};

int
eval_expression(MD_Expr *expr)
{
    int result = 0;
    MD_ExprOpr *op = expr->op;
    if (op == 0)
    {
        MD_Node *node = expr->md_node;
        if (node->flags & MD_NodeFlag_Numeric)
        {
            result = MD_CStyleIntFromString(node->string);
        }
        else if (node->flags & MD_NodeFlag_Identifier)
        {
            MD_MapSlot *slot = MD_MapLookup(&eval_map, MD_MapKeyStr(node->string));
            if (slot != 0)
            {
                result = (int)(MD_u64)slot->val;
            }
        }
    }
    else
    {
        int l = eval_expression(expr->left);
        int r = eval_expression(expr->right);
        
        // @notes The `op_id` on this op pointer is carried to use from the
        //  operator setup where we used the OpAdd and OpMul enum to assign
        //  assign static integers to each operator.
        switch (op->op_id)
        {
            case OpAdd:
            {
                result = l + r;
            }break;
            case OpMul:
            {
                result = l * r;
            }break;
        }
    }
    return(result);
}

//~ main //////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
#if 1
    char *argv_dummy[2] = {
        0,
        "W:\\metadesk\\examples\\expr\\expr_intro.mdesk",
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
        // @notes To start using the expression system we have to decide what
        //  the expression operator table is going to look like. To do this we
        //  build up a list of operators and then bake that list into an
        //  optimized operator table for the parser to use.
        //
        //  An operator string can be any string that parses as exactly one
        //  main node in metadesk. So it must count as a single token, and it
        //  cannot be a tag "@", set delimiter "()[]{}", or separator ",;".
        //
        //  The system does have one special case for operator strings. A
        //  postfix operator may be created with "()", "[]", "{}", "[)" or "(]"
        //  these get specially interpreted to mean that a set with those
        //  delimiters may be used as a postfix operator. This can be used to
        //  parse things like array indexers and function calls.
        //
        //  Here we just use symbol tokens as operators, but identifiers as
        //  operators are also allowed.
        //
        //  Here we can attach user data in two forms (the last two parameters)
        //  The first is intended for static integers like enum values here.
        //  The second is intended for non-static user data like a pointer to
        //  another data structure. Both are optional.
        //
        //  The bake function converts the list into a table optimized for
        //  parsing, but first it also checks the operator list. These checks
        //  include checking the names as described above, making sure
        //  there are no ambiguities from colliding operators, and making sure
        //  there are no mismatches of precedence levels and operator kinds
        //  that cannot be resolved.
        //
        //  The memory used in the list is also used by the table, so they
        //  should be on the same arena, or at the very least the list's arena
        //  should not be cleared while the table is still in use.
        
        MD_ExprOprList list = {0};
        MD_ExprOprPush(arena, &list, MD_ExprOprKind_Binary, 1, MD_S8Lit("+"), OpAdd, 0);
        MD_ExprOprPush(arena, &list, MD_ExprOprKind_Binary, 2, MD_S8Lit("*"), OpMul, 0);
        MD_ExprOprPush(arena, &list, MD_ExprOprKind_Binary, 3, MD_S8Lit("&"), OpIllegal, 0);
        
        table = MD_ExprBakeOprTableFromList(arena, &list);
    }
    
    // apply expression parsing to each top level node
    for (MD_EachNode(root_it, list->first_child))
    {
        // init eval map
        eval_map = MD_MapMake(arena);
        
        MD_Node *root = MD_ResolveNodeFromReference(root_it);
        for (MD_EachNode(node, root->first_child))
        {
            // @notes An expression parse is an extra stage of analysis on top
            //  of the initial Metadesk parse. It takes in a range of Metadesk
            //  nodes specified as (first, one-past-last). Here we want to
            //  parse all of the children of the top-level node as a single
            //  expression, so we use the node's `first_child` as the first and
            //  nil as the one-past-last.
            //
            //  The parser can return a list of new error messages, and an
            //  expression tree.
            // 
            //  The tree holds pointers back to the original operator data from
            //  the setup, so the tree should be on the same arena, or on at
            //  at least the operator memory should outlive the tree.
            
            // run the expression parse
            MD_ExprParseResult parse = MD_ExprParse(arena, &table, node->first_child, MD_NilNode());
            
            // print errors
            for (MD_Message *message = parse.errors.first;
                 message != 0;
                 message = message->next)
            {
                MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
                MD_PrintMessage(stdout, code_loc, message->kind, message->string);
            }
            
            if (parse.expr != 0)
            {
                // evaluate the expression
                int eval_result = eval_expression(parse.expr);
                MD_MapInsert(arena, &eval_map, MD_MapKeyStr(node->string), (void*)(MD_u64)eval_result);
                
                // print the expression
                if (node->string.size != 0)
                {
                    fprintf(stdout, "%.*s = ", MD_S8VArg(node->string));
                }
                print_expression(stdout, parse.expr);
                fprintf(stdout, "; (%d)\n", eval_result);
            }
        }
    }
    
    return 0;
}
