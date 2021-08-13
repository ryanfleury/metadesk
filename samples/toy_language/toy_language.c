#include "md.h"
#include "md_c_helpers.h"
#include "md.c"
#include "md_c_helpers.c"

typedef struct NamespaceNode NamespaceNode;
struct NamespaceNode
{
    NamespaceNode *parent;
    MD_Map symbol_map;
};

typedef enum ValueKind
{
    ValueKind_Null,
    ValueKind_Number,
    ValueKind_Procedure,
}
ValueKind;

typedef struct Value Value;
struct Value
{
    ValueKind kind;
    MD_f64 number;
    MD_Node *node;
};

static MD_Arena *arena = 0;

static Value
MakeValue_Number(MD_f64 v)
{
    Value value = {0};
    value.kind = ValueKind_Number;
    value.number = v;
    return value;
}

static Value
MakeValue_Procedure(MD_Node *node)
{
    Value value = {0};
    value.kind = ValueKind_Procedure;
    value.node = node;
    return value;
}

static void
InsertValueToNamespace(NamespaceNode *ns, MD_String8 string, Value v)
{
    Value *v_store = malloc(sizeof(*v_store));
    *v_store = v;
    MD_MapInsert(arena, &ns->symbol_map, MD_MapKeyStr(string), v_store);
}

static Value
ValueFromString(NamespaceNode *ns, MD_String8 string)
{
    Value v = {0};
    for(NamespaceNode *n = ns; n; n = n->parent)
    {
        MD_MapSlot *slot = MD_MapLookup(&n->symbol_map, MD_MapKeyStr(string));
        if(slot && slot->val)
        {
            v = *(Value *)slot->val;
            break;
        }
    }
    return v;
}

static Value EvaluateScope(NamespaceNode *ns, MD_Node *code);
static Value EvaluateExpr(NamespaceNode *ns, MD_C_Expr *expr);

static Value
EvaluateExpr(NamespaceNode *ns, MD_C_Expr *expr)
{
    Value result = {0};
    switch(expr->kind)
    {
        
#define BinaryOp(name, op) \
case MD_C_ExprKind_##name:\
{\
Value left = EvaluateExpr(ns, expr->sub[0]);\
Value right = EvaluateExpr(ns, expr->sub[1]);\
result = MakeValue_Number(left.number op right.number);\
}break
        
        BinaryOp(Add,      +);
        BinaryOp(Subtract, -);
        BinaryOp(Multiply, *);
        BinaryOp(Divide,   /);
        
#undef BinaryOp
        
        case MD_C_ExprKind_Call:
        {
            MD_Node *call = expr->node;
            Value callee = ValueFromString(ns, expr->sub[0]->node->string);
            if(!MD_NodeIsNil(callee.node))
            {
                //- rjf: find top-level namespace
                NamespaceNode *top_level_ns = ns;
                for(NamespaceNode *n = ns; n; n = n->parent)
                {
                    top_level_ns = n;
                }
                
                //- rjf: build namespace for function
                NamespaceNode args_ns = {0};
                MD_Node *param = callee.node->first_child;
                for(MD_Node *arg_first = call->first_child; !MD_NodeIsNil(arg_first); param = param->next)
                {
#if 0
                    MD_Node *arg_last = MD_SeekNodeWithFlags(arg_first, MD_NodeFlag_IsAfterComma|MD_NodeFlag_IsAfterSemicolon);
                    MD_C_Expr *expr = MD_C_ParseAsExpr(arg_first, arg_last);
                    InsertValueToNamespace(&args_ns, param->string, EvaluateExpr(ns, expr));
                    arg_first = arg_last->next;
#endif
                }
                
                args_ns.parent = top_level_ns;
                result = EvaluateScope(&args_ns, callee.node->next);
            }
        }break;
        
        case MD_C_ExprKind_Atom:
        {
            if(expr->node->flags & MD_NodeFlag_Identifier)
            {
                result = ValueFromString(ns, expr->node->string);
            }
            else if(expr->node->flags & MD_NodeFlag_Numeric)
            {
                result = MakeValue_Number(MD_F64FromString(expr->node->string));
            }
        }break;
        
        default: break;
    }
    return result;
}

static Value
EvaluateScope(NamespaceNode *ns, MD_Node *code)
{
    Value result = {0};
    
    NamespaceNode local_namespace = {0};
    local_namespace.parent = ns;
    
    // TODO(rjf): fix this, using last instead of opl
    
    for(MD_Node *first = code->first_child; !MD_NodeIsNil(first);)
    {
        MD_Node *opl = MD_NodeFromFlags(first->next, MD_NilNode(), MD_NodeFlag_IsAfterSemicolon|MD_NodeFlag_IsAfterComma);
        
        //- rjf: declaration
        if(first->next == opl && first->string.size != 0 && !MD_NodeIsNil(first->first_child))
        {
            MD_C_Expr *expr = MD_C_ParseAsExpr(arena, first->first_child, first->last_child);
            InsertValueToNamespace(&local_namespace, first->string, EvaluateExpr(&local_namespace, expr));
        }
        //- rjf: expr
        else
        {
            MD_C_Expr *expr = MD_C_ParseAsExpr(arena, first, opl);
            if(!MD_C_ExprIsNil(expr))
            {
                result = EvaluateExpr(&local_namespace, expr);
            }
        }
        
        //- rjf: bump
        first = opl;
    }
    
    return result;
}

int main(int argument_count, char **arguments)
{
    arena = MD_ArenaAlloc(1ull << 40);
    
    //- rjf: parse command line
    MD_String8List arg_list = MD_StringListFromArgCV(arena, argument_count, arguments);
    MD_CmdLine cmdln = MD_MakeCmdLineFromOptions(arena, arg_list);
    
    //- rjf: parse all input files
    MD_Node *file_list = MD_MakeList(arena);
    for(MD_String8Node *n = cmdln.inputs.first; n; n = n->next)
    {
        MD_ParseResult parse = MD_ParseWholeFile(arena, n->string);
        MD_PushNewReference(arena, file_list, parse.node);
    }
    
    //- rjf: gather top-level symbol map
    NamespaceNode global_ns_node = {0};
    for(MD_EachNode(file_ref, file_list->first_child))
    {
        MD_Node *file = MD_NodeFromReference(file_ref);
        for(MD_EachNode(top_level, file->first_child))
        {
            if(MD_NodeHasTag(top_level, MD_S8Lit("proc"), 0))
            {
                InsertValueToNamespace(&global_ns_node, top_level->string, MakeValue_Procedure(top_level));
            }
        }
    }
    
    //- rjf: find `main` procedure
    Value main_proc = ValueFromString(&global_ns_node, MD_S8Lit("main"));
    MD_Node *main_code = main_proc.node->next;
    if(MD_NodeIsNil(main_proc.node))
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
    Value result = EvaluateScope(&global_ns_node, main_code);
    switch(result.kind)
    {
        default:
        case ValueKind_Null:
        {
            printf("[null]\n");
        }break;
        
        case ValueKind_Number:
        {
            printf("[number] %f\n", result.number);
        }break;
        
        case ValueKind_Procedure:
        {
            printf("[proc] %.*s\n", MD_S8VArg(result.node->string));
        }break;
    }
    
    end:;
    return 0;
}
