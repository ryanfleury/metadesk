//~ Expression and Type-Expression Helper

MD_GLOBAL MD_C_Expr _md_nil_expr =
{
    &_md_nil_node,
    MD_C_ExprKind_Nil,
    &_md_nil_expr,
    {&_md_nil_expr, &_md_nil_expr },
};

MD_FUNCTION_IMPL MD_C_Expr *
MD_C_NilExpr(void)
{
    return &_md_nil_expr;
}

MD_FUNCTION_IMPL MD_b32
MD_C_ExprIsNil(MD_C_Expr *expr)
{
    return expr == 0 || expr == &_md_nil_expr || expr->kind == MD_C_ExprKind_Nil;
}

typedef struct _MD_C_ExprKindMetadata _MD_C_ExprKindMetadata;
struct _MD_C_ExprKindMetadata
{
    MD_C_ExprKindGroup group;
    MD_C_ExprPrec prec;
    char *symbol;
    char *pre_symbol;
    char *post_symbol;
};

MD_FUNCTION_IMPL _MD_C_ExprKindMetadata *
_MD_MetadataFromExprKind(MD_C_ExprKind kind)
{
    // 0:  Invalid
    // 12: (unary) - ~ !
    // 11: . -> () []
    // 10: * / %
    // 9:  + -
    // 8:  << >>
    // 7:  < <= > >=
    // 6:  == !=
    // 5:  (bitwise) &
    // 4:  ^
    // 3:  |
    // 2:  &&
    // 1:  ||
    static _MD_C_ExprKindMetadata metadata[] =
    {
        {MD_C_ExprKindGroup_Nil,       +0, "NIL",   "",  "" },  // MD_C_ExprKind_Nil
        {MD_C_ExprKindGroup_Atom,      +0, "NIL",   "",  "" },  // MD_C_ExprKind_Atom
        {MD_C_ExprKindGroup_Binary,    +11, ".",     "",  "" },  // MD_C_ExprKind_Dot
        {MD_C_ExprKindGroup_Binary,    +11, "->",    "",  "" },  // MD_C_ExprKind_Arrow
        {MD_C_ExprKindGroup_PostUnary, +11, "",      "(", ")"},  // MD_C_ExprKind_Call
        {MD_C_ExprKindGroup_PostUnary, +11, "",      "[", "]"},  // MD_C_ExprKind_Subscript
        {MD_C_ExprKindGroup_PreUnary,  +12, "",      "*", "" },  // MD_C_ExprKind_Dereference
        {MD_C_ExprKindGroup_PreUnary,  +12, "",      "&", "" },  // MD_C_ExprKind_Reference
        {MD_C_ExprKindGroup_Binary,    +9,  "+",     "",  "" },  // MD_C_ExprKind_Add
        {MD_C_ExprKindGroup_Binary,    +9,  "-",     "",  "" },  // MD_C_ExprKind_Subtract
        {MD_C_ExprKindGroup_Binary,    +10, "*",     "",  "" },  // MD_C_ExprKind_Multiply
        {MD_C_ExprKindGroup_Binary,    +10, "/",     "",  "" },  // MD_C_ExprKind_Divide
        {MD_C_ExprKindGroup_Binary,    +10, "%",     "",  "" },  // MD_C_ExprKind_Mod
        {MD_C_ExprKindGroup_Binary,    +6,  "==",    "",  "" },  // MD_C_ExprKind_IsEqual
        {MD_C_ExprKindGroup_Binary,    +6,  "!=",    "",  "" },  // MD_C_ExprKind_IsNotEqual
        {MD_C_ExprKindGroup_Binary,    +7,  "<",     "",  "" },  // MD_C_ExprKind_LessThan
        {MD_C_ExprKindGroup_Binary,    +7,  ">",     "",  "" },  // MD_C_ExprKind_GreaterThan
        {MD_C_ExprKindGroup_Binary,    +7,  "<=",    "",  "" },  // MD_C_ExprKind_LessThanEqualTo
        {MD_C_ExprKindGroup_Binary,    +7,  ">=",    "",  "" },  // MD_C_ExprKind_GreaterThanEqualTo
        {MD_C_ExprKindGroup_Binary,    +2,  "&&",    "",  "" },  // MD_C_ExprKind_BoolAnd
        {MD_C_ExprKindGroup_Binary,    +1,  "||",    "",  "" },  // MD_C_ExprKind_BoolOr
        {MD_C_ExprKindGroup_PreUnary,  +12, "",      "!", "" },  // MD_C_ExprKind_BoolNot
        {MD_C_ExprKindGroup_Binary,    +5,  "&",     "",  "" },  // MD_C_ExprKind_BitAnd
        {MD_C_ExprKindGroup_Binary,    +3,  "|",     "",  "" },  // MD_C_ExprKind_BitOr
        {MD_C_ExprKindGroup_PreUnary,  +12, "",      "~", "" },  // MD_C_ExprKind_BitNot
        {MD_C_ExprKindGroup_Binary,    +4,  "^",     "",  "" },  // MD_C_ExprKind_BitXor
        {MD_C_ExprKindGroup_Binary,    +8,  "<<",    "",  "" },  // MD_C_ExprKind_LeftShift
        {MD_C_ExprKindGroup_Binary,    +8,  ">>",    "",  "" },  // MD_C_ExprKind_RightShift
        {MD_C_ExprKindGroup_PreUnary,  +12, "",      "-", "" },  // MD_C_ExprKind_Negative
        {MD_C_ExprKindGroup_Type,      +0,  "*",     "",  "" },  // MD_C_ExprKind_Pointer
        {MD_C_ExprKindGroup_Type,      +0,  "",      "[", "]"},  // MD_C_ExprKind_Array
    };
    return &metadata[kind];
}

MD_FUNCTION_IMPL MD_C_ExprKind
MD_C_PostUnaryExprKindFromNode(MD_Node *node)
{
    MD_C_ExprKind kind = MD_C_ExprKind_Nil;
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_C_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_C_ExprKind_Subscript;
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_C_ExprKind
MD_C_PreUnaryExprKindFromNode(MD_Node *node)
{
    MD_C_ExprKind kind = MD_C_ExprKind_Nil;
    // NOTE(rjf): Special-cases for calls/subscripts.
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_C_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_C_ExprKind_Subscript;
        }
    }
    else
    {
        for(MD_C_ExprKind kind_it = (MD_C_ExprKind)0; kind_it < MD_C_ExprKind_MAX;
            kind_it = (MD_C_ExprKind)((int)kind_it + 1))
        {
            _MD_C_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == MD_C_ExprKindGroup_PreUnary)
            {
                if(MD_StringMatch(node->string, MD_S8CString(metadata->pre_symbol), 0))
                {
                    kind = kind_it;
                    break;
                }
            }
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_C_ExprKind
MD_C_BinaryExprKindFromNode(MD_Node *node)
{
    MD_C_ExprKind kind = MD_C_ExprKind_Nil;
    if(node->kind == MD_NodeKind_Label && MD_NodeIsNil(node->first_child))
    {
        for(MD_C_ExprKind kind_it = (MD_C_ExprKind)0; kind_it < MD_C_ExprKind_MAX;
            kind_it = (MD_C_ExprKind)((int)kind_it + 1))
        {
            _MD_C_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == MD_C_ExprKindGroup_Binary)
            {
                if(MD_StringMatch(node->string, MD_S8CString(metadata->symbol), 0))
                {
                    kind = kind_it;
                    break;
                }
            }
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_C_ExprPrec
MD_C_ExprPrecFromExprKind(MD_C_ExprKind kind)
{
    _MD_C_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind);
    return metadata->prec;
}

MD_FUNCTION_IMPL MD_C_Expr *
MD_C_MakeExpr(MD_Node *node, MD_C_ExprKind kind, MD_C_Expr *left, MD_C_Expr *right)
{
    MD_C_Expr *expr = MD_PushArray(MD_C_Expr, 1);
    if(left == 0)  left  = MD_C_NilExpr();
    if(right == 0) right = MD_C_NilExpr();
    expr->node = node;
    expr->kind = kind;
    expr->sub[0] = left;
    expr->sub[1] = right;
    return expr;
}

typedef struct _MD_NodeParseCtx _MD_NodeParseCtx;
struct _MD_NodeParseCtx
{
    MD_Node *at;
    MD_Node *last;
    MD_Node *one_past_last;
};

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeAtom(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(ctx->at->kind == MD_NodeKind_Label &&
       MD_NodeIsNil(ctx->at->first_child))
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeSet(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(!MD_NodeIsNil(ctx->at->first_child) ||
       ctx->at->flags & MD_NodeFlag_ParenLeft ||
       ctx->at->flags & MD_NodeFlag_ParenRight ||
       ctx->at->flags & MD_NodeFlag_BracketLeft ||
       ctx->at->flags & MD_NodeFlag_BracketRight ||
       ctx->at->flags & MD_NodeFlag_BraceLeft ||
       ctx->at->flags & MD_NodeFlag_BraceRight)
    {
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
        result = 1;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_ConsumeLiteral(_MD_NodeParseCtx *ctx, MD_Node **out)
{
    MD_b32 result = 0;
    if(ctx->at->flags & MD_NodeFlag_StringLiteral ||
       ctx->at->flags & MD_NodeFlag_Numeric)
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_NodeParse_Consume(_MD_NodeParseCtx *ctx, MD_String8 string, MD_Node **out)
{
    MD_b32 result = 0;
    if(MD_StringMatch(ctx->at->string, string, 0))
    {
        result = 1;
        if(out)
        {
            *out = ctx->at;
        }
        ctx->at = ctx->at->next;
    }
    return result;
}

MD_PRIVATE_FUNCTION_IMPL void
_MD_NodeParse_Next(_MD_NodeParseCtx *ctx)
{
    ctx->at = ctx->at->next;
}

MD_PRIVATE_FUNCTION_IMPL MD_C_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in);

MD_PRIVATE_FUNCTION_IMPL MD_C_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx);

MD_PRIVATE_FUNCTION_IMPL MD_C_Expr *
_MD_ParseUnaryExpr(_MD_NodeParseCtx *ctx)
{
    MD_C_Expr *result = MD_C_NilExpr();
    MD_Node *set = 0;
    MD_Node *node = 0;
    
    // NOTE(rjf): Sub-expression
    if(_MD_NodeParse_ConsumeSet(ctx, &set))
    {
        result = MD_C_ParseAsExpr(set->first_child, set->last_child);
    }
    
    // NOTE(rjf): Literal
    else if(_MD_NodeParse_ConsumeLiteral(ctx, &node))
    {
        result = MD_C_MakeExpr(node, MD_C_ExprKind_Atom, 0, 0);
    }
    
    // NOTE(rjf): Literal
    else if(_MD_NodeParse_ConsumeAtom(ctx, &node))
    {
        result = MD_C_MakeExpr(node, MD_C_ExprKind_Atom, 0, 0);
    }
    
    // NOTE(rjf): Negative
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("-"), &node))
    {
        result = MD_C_MakeExpr(node, MD_C_ExprKind_Negative, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Bitwise Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("~"), &node))
    {
        result = MD_C_MakeExpr(node, MD_C_ExprKind_BitNot, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Boolean Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("!"), &node))
    {
        result = MD_C_MakeExpr(node, MD_C_ExprKind_BoolNot, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Post-Unary Sets (calls and subscripts)
    if(_MD_NodeParse_ConsumeSet(ctx, &set))
    {
        if(set->flags & MD_NodeFlag_ParenLeft && set->flags & MD_NodeFlag_ParenRight)
        {
            result = MD_C_MakeExpr(set, MD_C_ExprKind_Call, result, 0);
        }
        else if(set->flags & MD_NodeFlag_BracketLeft && set->flags & MD_NodeFlag_BracketRight)
        {
            result = MD_C_MakeExpr(set, MD_C_ExprKind_Subscript, result, MD_C_ParseAsExpr(set->first_child, set->last_child));
        }
    }
    
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_C_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in)
{
    MD_C_Expr *expr = _MD_ParseUnaryExpr(ctx);
    MD_C_ExprKind expr_kind;
    if(MD_C_ExprIsNil(expr))
    {
        goto end_parse;
    }
    
    expr_kind = MD_C_BinaryExprKindFromNode(ctx->at);
    if(expr_kind != MD_C_ExprKind_Nil)
    {
        for(int precedence = MD_C_ExprPrecFromExprKind(expr_kind);
            precedence >= precedence_in;
            precedence -= 1)
        {
            for(;;)
            {
                MD_Node *op_node = ctx->at;
                expr_kind = MD_C_BinaryExprKindFromNode(ctx->at);
                int operator_precedence = MD_C_ExprPrecFromExprKind(expr_kind);
                if(operator_precedence != precedence)
                {
                    break;
                }
                
                if(expr_kind == MD_C_ExprKind_Nil)
                {
                    break;
                }
                
                _MD_NodeParse_Next(ctx);
                
                MD_C_Expr *right = _MD_ParseExpr_(ctx, precedence+1);
                if(MD_C_ExprIsNil(right))
                {
                    // TODO(rjf): Error: "Expected right-hand-side of binary expression."
                    goto end_parse;
                }
                
                MD_C_Expr *left = expr;
                expr = MD_C_MakeExpr(op_node, expr_kind, left, right);
                expr->sub[0] = left;
                expr->sub[1] = right;
            }
        }
    }
    
    end_parse:;
    return expr;
}

MD_PRIVATE_FUNCTION_IMPL MD_C_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx)
{
    return _MD_ParseExpr_(ctx, 1);
}

MD_FUNCTION_IMPL MD_C_Expr *
MD_C_ParseAsExpr(MD_Node *first, MD_Node *last)
{
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
    return _MD_ParseExpr(ctx);
}

MD_FUNCTION_IMPL MD_C_Expr *
MD_C_ParseAsType(MD_Node *first, MD_Node *last)
{
    MD_C_Expr *expr = MD_C_NilExpr();
    MD_C_Expr *last_expr = expr;
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
#define _MD_PushType(x) if(MD_C_ExprIsNil(last_expr)) { expr = last_expr = x; } else { last_expr = last_expr->sub[0] = x; }
    MD_Node *set = 0;
    MD_Node *ptr = 0;
    MD_Node *base_type = 0;
    MD_Node *node = 0;
    for(;;)
    {
        if(_MD_NodeParse_Consume(ctx, MD_S8Lit("*"), &ptr))
        {
            MD_C_Expr *t = MD_C_MakeExpr(ptr, MD_C_ExprKind_Pointer, MD_C_NilExpr(), MD_C_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("volatile"), &node))
        {
            MD_C_Expr *t = MD_C_MakeExpr(node, MD_C_ExprKind_Volatile, MD_C_NilExpr(), MD_C_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("const"), &node))
        {
            MD_C_Expr *t = MD_C_MakeExpr(node, MD_C_ExprKind_Const, MD_C_NilExpr(), MD_C_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeSet(ctx, &set))
        {
            MD_C_Expr *t = MD_C_MakeExpr(set, MD_C_ExprKind_Array, MD_C_NilExpr(), MD_C_NilExpr());
            t->sub[1] = MD_C_ParseAsExpr(set->first_child, set->last_child);
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeAtom(ctx, &base_type))
        {
            MD_C_Expr *t = MD_C_MakeExpr(base_type, MD_C_ExprKind_Atom, MD_C_NilExpr(), MD_C_NilExpr());
            _MD_PushType(t);
        }
        else
        {
            break;
        }
    }
#undef _MD_PushType
    return expr;
}

MD_FUNCTION_IMPL MD_i64
MD_C_EvaluateExpr_I64(MD_C_Expr *expr)
{
    MD_i64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_C_ExprKind_##name: { result = MD_C_EvaluateExpr_I64(expr->sub[0]) op MD_C_EvaluateExpr_I64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_C_ExprKind_Atom: { result = MD_I64FromString(expr->node->string, 10); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_f64
MD_C_EvaluateExpr_F64(MD_C_Expr *expr)
{
    MD_f64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_C_ExprKind_##name: { result = MD_C_EvaluateExpr_F64(expr->sub[0]) op MD_C_EvaluateExpr_F64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_C_ExprKind_Atom: { result = MD_F64FromString(expr->node->string); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_C_ExprMatch(MD_C_Expr *a, MD_C_Expr *b, MD_MatchFlags flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind)
    {
        result = 1;
        if(a->kind == MD_C_ExprKind_Atom)
        {
            result = MD_StringMatch(a->node->string, b->node->string, flags);
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_C_ExprDeepMatch(MD_C_Expr *a, MD_C_Expr *b, MD_MatchFlags flags)
{
    MD_b32 result = MD_C_ExprMatch(a, b, flags);
    if(result && !MD_C_ExprIsNil(a) && !MD_C_ExprIsNil(b))
    {
        result = (MD_C_ExprDeepMatch(a->sub[0], b->sub[0], flags) &&
                  MD_C_ExprDeepMatch(a->sub[1], b->sub[1], flags));
    }
    return result;
}

//~ C Language Generation

MD_FUNCTION_IMPL void
MD_C_Generate_String(FILE *file, MD_Node *node)
{
    fprintf(file, "\"");
    for(MD_u64 i = 0; i < node->string.size; i += 1)
    {
        if(node->string.str[i] == '\n')
        {
            fprintf(file, "\\n\"\n\"");
        }
        else if(node->string.str[i] == '\r' &&
                i+1 < node->string.size && node->string.str[i+1] == '\n')
        {
            // NOTE(mal): Step over CR when quoting CRLF newlines
        }
        else
        {
            fprintf(file, "%c", node->string.str[i]);
        }
    }
    fprintf(file, "\"");
}

MD_FUNCTION_IMPL void
MD_C_Generate_Struct(FILE *file, MD_Node *node)
{
    if(node)
    {
        fprintf(file, "typedef struct %.*s %.*s;\n",
                MD_StringExpand(node->string),
                MD_StringExpand(node->string));
        fprintf(file, "struct %.*s\n{\n", MD_StringExpand(node->string));
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_C_Generate_Decl(file, child);
            fprintf(file, ";\n");
        }
        fprintf(file, "};\n\n");
    }
}

MD_FUNCTION_IMPL void
MD_C_Generate_Expr(FILE *file, MD_C_Expr *expr)
{
    if(!MD_NodeIsNil(expr->node))
    {
        _MD_C_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(expr->kind);
        switch(metadata->group)
        {
            case MD_C_ExprKindGroup_Atom:
            {
                fprintf(file, "%.*s", MD_StringExpand(expr->node->string));
            }break;
            
            case MD_C_ExprKindGroup_Binary:
            {
                fprintf(file, "(");
                MD_C_Generate_Expr(file, expr->sub[0]);
                fprintf(file, " %s ", metadata->symbol);
                MD_C_Generate_Expr(file, expr->sub[1]);
                fprintf(file, ")");
            }break;
            
            case MD_C_ExprKindGroup_PreUnary:
            {
                fprintf(file, "%s", metadata->pre_symbol);
                fprintf(file, "(");
                MD_C_Generate_Expr(file, expr->sub[0]);
                fprintf(file, ")");
            }break;
            
            case MD_C_ExprKindGroup_PostUnary:
            {
                fprintf(file, "(");
                MD_C_Generate_Expr(file, expr->sub[0]);
                fprintf(file, ")");
                fprintf(file, "%s", metadata->post_symbol);
            }break;
            
            default: break;
        }
    }
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_OutputType_C_NeedsParens(MD_C_Expr *type)
{
    MD_b32 result = 0;
    if (type->kind == MD_C_ExprKind_Pointer &&
        type->sub[0]->kind == MD_C_ExprKind_Array)
    {
        result = 1;
    }
    return(result);
}

MD_FUNCTION_IMPL void
MD_C_Generate_TypeLHS(FILE *file, MD_C_Expr *type)
{
    switch (type->kind)
    {
        case MD_C_ExprKind_Atom:
        {
            MD_Node *node = type->node;
            fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
        }break;
        
        case MD_C_ExprKind_Pointer:
        {
            MD_C_Generate_TypeLHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
            fprintf(file, "*");
        }break;
        
        case MD_C_ExprKind_Array:
        {
            MD_C_Generate_TypeLHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
        }break;
        
        case MD_C_ExprKind_Volatile: { fprintf(file, "volatile "); }break;
        case MD_C_ExprKind_Const:    { fprintf(file, "const "); }break;
        
        default:
        {
            fprintf(file, "{ unexpected MD_C_ExprKind (%i) in type info for node \"%.*s\" }",
                    type->kind,
                    MD_StringExpand(type->node->whole_string));
        }break;
    }
}

MD_FUNCTION_IMPL void
MD_C_Generate_TypeRHS(FILE *file, MD_C_Expr *type)
{
    switch (type->kind)
    {
        case MD_C_ExprKind_Atom:
        {}break;
        
        case MD_C_ExprKind_Pointer:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            MD_C_Generate_TypeRHS(file, type->sub[0]);
        }break;
        
        case MD_C_ExprKind_Array:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            fprintf(file, "[");
            MD_C_Generate_Expr(file, type->sub[1]);
            fprintf(file, "]");
            MD_C_Generate_TypeRHS(file, type->sub[0]);
        }break;
        
        case MD_C_ExprKind_Volatile: { fprintf(file, "volatile "); }break;
        case MD_C_ExprKind_Const:    { fprintf(file, "const "); }break;
        
        default:
        {}break;
    }
}

MD_FUNCTION_IMPL void
MD_C_Generate_DeclByNameAndType(FILE *file, MD_String8 name, MD_C_Expr *type)
{
    MD_C_Generate_TypeLHS(file, type);
    fprintf(file, " %.*s", MD_StringExpand(name));
    MD_C_Generate_TypeRHS(file, type);
}

MD_FUNCTION_IMPL void
MD_C_Generate_Decl(FILE *file, MD_Node *node)
{
    if(node)
    {
        MD_C_Expr *type = MD_C_ParseAsType(node->first_child, node->last_child);
        MD_C_Generate_DeclByNameAndType(file, node->string, type);
    }
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
