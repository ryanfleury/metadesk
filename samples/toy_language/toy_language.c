#include "md.h"
#include "md_c_helpers.h"
#include "md.c"
#include "md_c_helpers.c"

typedef struct NodeSubList NodeSubList;
struct NodeSubList
{
    MD_Node *first;
    MD_Node *last;
};

typedef struct NamespaceNode NamespaceNode;
struct NamespaceNode
{
    NamespaceNode *parent;
    MD_Map symbol_map;
};

static NodeSubList
MakeNodeSubList(MD_Node *first, MD_Node *last)
{
    NodeSubList sublist = { first, last };
    return sublist;
}

static void
InsertNodeSubListToNamespace(NamespaceNode *ns, MD_String8 string, NodeSubList sublist)
{
    NodeSubList *sublist_store = malloc(sizeof(*sublist_store));
    *sublist_store = sublist;
    MD_StringMap_Insert(&ns->symbol_map, MD_MapCollisionRule_Chain, string, sublist_store);
}

static NodeSubList
NodeSubListFromNamespaceAndString(NamespaceNode *ns, MD_String8 string)
{
    NodeSubList sublist = {MD_NilNode(), MD_NilNode()};
    for(NamespaceNode *n = ns; n; n = n->parent)
    {
        MD_MapSlot *slot = MD_StringMap_Lookup(&n->symbol_map, string);
        if(slot && slot->value)
        {
            sublist = *(NodeSubList *)slot->value;
            break;
        }
    }
    return sublist;
}

static MD_f64 EvaluateScope(NamespaceNode *ns, MD_Node *code);
static MD_f64 EvaluateExpr(NamespaceNode *ns, MD_C_Expr *expr);

static MD_f64
EvaluateExpr(NamespaceNode *ns, MD_C_Expr *expr)
{
    MD_f64 result = 0;
    switch(expr->kind)
    {
#define BinaryOp(name, op) case MD_C_ExprKind_##name: { result = EvaluateExpr(ns, expr->sub[0]) op EvaluateExpr(ns, expr->sub[1]); }break
        BinaryOp(Add,      +);
        BinaryOp(Subtract, -);
        BinaryOp(Multiply, *);
        BinaryOp(Divide,   /);
#undef BinaryOp
        
        case MD_C_ExprKind_Call:
        {
            MD_Node *call = expr->node;
            NodeSubList callee = NodeSubListFromNamespaceAndString(ns, expr->sub[0]->node->string);
            if(!MD_NodeIsNil(callee.first) && !MD_NodeIsNil(callee.last))
            {
                //- rjf: find top-level namespace
                NamespaceNode *top_level_ns = ns;
                for(NamespaceNode *n = ns; n; n = n->parent)
                {
                    top_level_ns = n;
                }
                
                //- rjf: build namespace for function
                NamespaceNode args_ns = {0};
                MD_Node *param = callee.first->first_child;
                for(MD_Node *arg_first = call->first_child; !MD_NodeIsNil(arg_first); param = param->next)
                {
                    MD_Node *arg_last = MD_SeekNodeWithFlags(arg_first, MD_NodeFlag_AfterComma|MD_NodeFlag_AfterSemicolon);
                    InsertNodeSubListToNamespace(&args_ns, param->string, MakeNodeSubList(arg_first, arg_last));
                    arg_first = arg_last->next;
                }
                
                args_ns.parent = ns;
                result = EvaluateScope(&args_ns, callee.first->next);
            }
        }break;
        
        case MD_C_ExprKind_Atom:
        {
            if(expr->node->flags & MD_NodeFlag_Identifier)
            {
                NodeSubList decl_value = NodeSubListFromNamespaceAndString(ns, expr->node->string);
                MD_C_Expr *decl_expr = MD_C_ParseAsExpr(decl_value.first, decl_value.last);
                result = EvaluateExpr(ns, decl_expr);
            }
            else if(expr->node->flags & MD_NodeFlag_Numeric)
            {
                result = MD_F64FromString(expr->node->string);
            }
        }break;
        
        default: break;
    }
    return result;
}

static MD_f64
EvaluateScope(NamespaceNode *ns, MD_Node *code)
{
    MD_f64 result = 0;
    
    NamespaceNode local_namespace = {0};
    local_namespace.parent = ns;
    
    for(MD_Node *first = code->first_child; !MD_NodeIsNil(first);)
    {
        MD_Node *last = MD_SeekNodeWithFlags(first, MD_NodeFlag_AfterSemicolon|MD_NodeFlag_AfterComma);
        
        //- rjf: declaration
        if(first == last && first->string.size != 0 && !MD_NodeIsNil(first->first_child))
        {
            InsertNodeSubListToNamespace(&local_namespace, first->string,
                                         MakeNodeSubList(first->first_child, first->last_child));
        }
        //- rjf: expr
        else
        {
            MD_C_Expr *expr = MD_C_ParseAsExpr(first, last);
            if(!MD_C_ExprIsNil(expr))
            {
                result = EvaluateExpr(&local_namespace, expr);
            }
        }
        
        //- rjf: bump
        first = last->next;
    }
    
    return result;
}

int main(int argument_count, char **arguments)
{
    //- rjf: parse command line
    MD_CommandLine cmdln = MD_CommandLineFromOptions(MD_StringListFromArgCV(argument_count, arguments));
    
    //- rjf: parse all input files
    MD_Node *first_file = MD_NilNode();
    MD_Node *last_file = MD_NilNode();
    for(MD_String8Node *n = cmdln.inputs.first; n; n = n->next)
    {
        MD_ParseResult parse = MD_ParseWholeFile(n->string);
        MD_PushSibling(&first_file, &last_file, parse.node);
    }
    
    //- rjf: gather top-level symbol map
    NamespaceNode global_ns_node = {0};
    for(MD_EachNode(file, first_file))
    {
        for(MD_EachNode(top_level, file->first_child))
        {
            if(top_level->string.size > 0)
            {
                InsertNodeSubListToNamespace(&global_ns_node, top_level->string,
                                             MakeNodeSubList(top_level, top_level));
            }
        }
    }
    
    //- rjf: find `main` procedure
    NodeSubList main_proc = NodeSubListFromNamespaceAndString(&global_ns_node, MD_S8Lit("main"));
    MD_Node *main_code = main_proc.first->next;
    if(MD_NodeIsNil(main_proc.first))
    {
        fprintf(stderr, "no `main` procedure found");
        goto end;
    }
    
    //- rjf: validate that we got `main` code
    if(MD_NodeIsNil(main_code))
    {
        fprintf(stderr, "`main` requires an implementation");
        goto end;
    }
    
    //- rjf: start interpreting at `main`
    MD_f64 result = EvaluateScope(&global_ns_node, main_code);
    printf("result: %f\n", result);
    
    end:;
    return 0;
}
