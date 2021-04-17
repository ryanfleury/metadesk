//~ Expression and Type-Expression Helper

MD_GLOBAL MD_Expr _md_nil_expr =
{
    &_md_nil_node,
    MD_ExprKind_Nil,
    &_md_nil_expr,
    {&_md_nil_expr, &_md_nil_expr },
};

MD_FUNCTION_IMPL MD_Expr *
MD_NilExpr(void)
{
    return &_md_nil_expr;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprIsNil(MD_Expr *expr)
{
    return expr == 0 || expr == &_md_nil_expr || expr->kind == MD_ExprKind_Nil;
}

typedef struct _MD_ExprKindMetadata _MD_ExprKindMetadata;
struct _MD_ExprKindMetadata
{
    MD_ExprKindGroup group;
    MD_ExprPrec prec;
    char *symbol;
    char *pre_symbol;
    char *post_symbol;
};

MD_FUNCTION_IMPL _MD_ExprKindMetadata *
_MD_MetadataFromExprKind(MD_ExprKind kind)
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
    static _MD_ExprKindMetadata metadata[] =
    {
        {MD_ExprKindGroup_Nil,       +0, "NIL",   "",  "" },  // MD_ExprKind_Nil
        {MD_ExprKindGroup_Atom,      +0, "NIL",   "",  "" },  // MD_ExprKind_Atom
        {MD_ExprKindGroup_Binary,    +11, ".",     "",  "" },  // MD_ExprKind_Dot
        {MD_ExprKindGroup_Binary,    +11, "->",    "",  "" },  // MD_ExprKind_Arrow
        {MD_ExprKindGroup_PostUnary, +11, "",      "(", ")"},  // MD_ExprKind_Call
        {MD_ExprKindGroup_PostUnary, +11, "",      "[", "]"},  // MD_ExprKind_Subscript
        {MD_ExprKindGroup_PreUnary,  +12, "",      "*", "" },  // MD_ExprKind_Dereference
        {MD_ExprKindGroup_PreUnary,  +12, "",      "&", "" },  // MD_ExprKind_Reference
        {MD_ExprKindGroup_Binary,    +9,  "+",     "",  "" },  // MD_ExprKind_Add
        {MD_ExprKindGroup_Binary,    +9,  "-",     "",  "" },  // MD_ExprKind_Subtract
        {MD_ExprKindGroup_Binary,    +10, "*",     "",  "" },  // MD_ExprKind_Multiply
        {MD_ExprKindGroup_Binary,    +10, "/",     "",  "" },  // MD_ExprKind_Divide
        {MD_ExprKindGroup_Binary,    +10, "%",     "",  "" },  // MD_ExprKind_Mod
        {MD_ExprKindGroup_Binary,    +6,  "==",    "",  "" },  // MD_ExprKind_IsEqual
        {MD_ExprKindGroup_Binary,    +6,  "!=",    "",  "" },  // MD_ExprKind_IsNotEqual
        {MD_ExprKindGroup_Binary,    +7,  "<",     "",  "" },  // MD_ExprKind_LessThan
        {MD_ExprKindGroup_Binary,    +7,  ">",     "",  "" },  // MD_ExprKind_GreaterThan
        {MD_ExprKindGroup_Binary,    +7,  "<=",    "",  "" },  // MD_ExprKind_LessThanEqualTo
        {MD_ExprKindGroup_Binary,    +7,  ">=",    "",  "" },  // MD_ExprKind_GreaterThanEqualTo
        {MD_ExprKindGroup_Binary,    +2,  "&&",    "",  "" },  // MD_ExprKind_BoolAnd
        {MD_ExprKindGroup_Binary,    +1,  "||",    "",  "" },  // MD_ExprKind_BoolOr
        {MD_ExprKindGroup_PreUnary,  +12, "",      "!", "" },  // MD_ExprKind_BoolNot
        {MD_ExprKindGroup_Binary,    +5,  "&",     "",  "" },  // MD_ExprKind_BitAnd
        {MD_ExprKindGroup_Binary,    +3,  "|",     "",  "" },  // MD_ExprKind_BitOr
        {MD_ExprKindGroup_PreUnary,  +12, "",      "~", "" },  // MD_ExprKind_BitNot
        {MD_ExprKindGroup_Binary,    +4,  "^",     "",  "" },  // MD_ExprKind_BitXor
        {MD_ExprKindGroup_Binary,    +8,  "<<",    "",  "" },  // MD_ExprKind_LeftShift
        {MD_ExprKindGroup_Binary,    +8,  ">>",    "",  "" },  // MD_ExprKind_RightShift
        {MD_ExprKindGroup_PreUnary,  +12, "",      "-", "" },  // MD_ExprKind_Negative
        {MD_ExprKindGroup_Type,      +0,  "*",     "",  "" },  // MD_ExprKind_Pointer
        {MD_ExprKindGroup_Type,      +0,  "",      "[", "]"},  // MD_ExprKind_Array
    };
    return &metadata[kind];
}

MD_FUNCTION_IMPL MD_ExprKind
MD_PostUnaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_ExprKind_Subscript;
        }
    }
    return kind;
}

MD_FUNCTION_IMPL MD_ExprKind
MD_PreUnaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    // NOTE(rjf): Special-cases for calls/subscripts.
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->flags & MD_NodeFlag_ParenLeft &&
           node->flags & MD_NodeFlag_ParenRight)
        {
            kind = MD_ExprKind_Call;
        }
        else if(node->flags & MD_NodeFlag_BracketLeft &&
                node->flags & MD_NodeFlag_BracketRight)
        {
            kind = MD_ExprKind_Subscript;
        }
    }
    else
    {
        for(MD_ExprKind kind_it = (MD_ExprKind)0; kind_it < MD_ExprKind_MAX;
            kind_it = (MD_ExprKind)((int)kind_it + 1))
        {
            _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == MD_ExprKindGroup_PreUnary)
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

MD_FUNCTION_IMPL MD_ExprKind
MD_BinaryExprKindFromNode(MD_Node *node)
{
    MD_ExprKind kind = MD_ExprKind_Nil;
    if(node->kind == MD_NodeKind_Label && MD_NodeIsNil(node->first_child))
    {
        for(MD_ExprKind kind_it = (MD_ExprKind)0; kind_it < MD_ExprKind_MAX;
            kind_it = (MD_ExprKind)((int)kind_it + 1))
        {
            _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind_it);
            if(metadata->group == MD_ExprKindGroup_Binary)
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

MD_FUNCTION_IMPL MD_ExprPrec
MD_ExprPrecFromExprKind(MD_ExprKind kind)
{
    _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(kind);
    return metadata->prec;
}

MD_FUNCTION_IMPL MD_Expr *
MD_MakeExpr(MD_Node *node, MD_ExprKind kind, MD_Expr *left, MD_Expr *right)
{
    MD_Expr *expr = _MD_PushArray(MD_Expr, 1);
    if(expr)
    {
        if(left == 0)  left  = MD_NilExpr();
        if(right == 0) right = MD_NilExpr();
        expr->node = node;
        expr->kind = kind;
        expr->sub[0] = left;
        expr->sub[1] = right;
    }
    else
    {
        expr = MD_NilExpr();
    }
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
    if(!MD_NodeIsNil(ctx->at->first_child))
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
       ctx->at->flags & MD_NodeFlag_CharLiteral   ||
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

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in);

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx);

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseUnaryExpr(_MD_NodeParseCtx *ctx)
{
    MD_Expr *result = MD_NilExpr();
    MD_Node *set = 0;
    MD_Node *node = 0;
    
    // NOTE(rjf): Sub-expression
    if(_MD_NodeParse_ConsumeSet(ctx, &set))
    {
        result = MD_ParseAsExpr(set->first_child, set->last_child);
    }
    
    // NOTE(rjf): Literal
    else if(_MD_NodeParse_ConsumeLiteral(ctx, &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_Atom, 0, 0);
    }
    
    // NOTE(rjf): Literal
    else if(_MD_NodeParse_ConsumeAtom(ctx, &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_Atom, 0, 0);
    }
    
    // NOTE(rjf): Negative
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("-"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_Negative, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Bitwise Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("~"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_BitNot, 0, _MD_ParseExpr(ctx));
    }
    
    // NOTE(rjf): Boolean Negate
    else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("!"), &node))
    {
        result = MD_MakeExpr(node, MD_ExprKind_BoolNot, 0, _MD_ParseExpr(ctx));
    }
    
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr_(_MD_NodeParseCtx *ctx, int precedence_in)
{
    MD_Expr *expr = _MD_ParseUnaryExpr(ctx);
    MD_ExprKind expr_kind;
    if(MD_ExprIsNil(expr))
    {
        goto end_parse;
    }
    
    expr_kind = MD_BinaryExprKindFromNode(ctx->at);
    if(expr_kind != MD_ExprKind_Nil)
    {
        for(int precedence = MD_ExprPrecFromExprKind(expr_kind);
            precedence >= precedence_in;
            precedence -= 1)
        {
            for(;;)
            {
                MD_Node *op_node = ctx->at;
                expr_kind = MD_BinaryExprKindFromNode(ctx->at);
                int operator_precedence = MD_ExprPrecFromExprKind(expr_kind);
                if(operator_precedence != precedence)
                {
                    break;
                }
                
                if(expr_kind == MD_ExprKind_Nil)
                {
                    break;
                }
                
                _MD_NodeParse_Next(ctx);
                
                MD_Expr *right = _MD_ParseExpr_(ctx, precedence+1);
                if(MD_ExprIsNil(right))
                {
                    // TODO(rjf): Error: "Expected right-hand-side of binary expression."
                    goto end_parse;
                }
                
                MD_Expr *left = expr;
                expr = MD_MakeExpr(op_node, expr_kind, left, right);
                expr->sub[0] = left;
                expr->sub[1] = right;
            }
        }
    }
    
    end_parse:;
    return expr;
}

MD_PRIVATE_FUNCTION_IMPL MD_Expr *
_MD_ParseExpr(_MD_NodeParseCtx *ctx)
{
    return _MD_ParseExpr_(ctx, 1);
}

MD_FUNCTION_IMPL MD_Expr *
MD_ParseAsExpr(MD_Node *first, MD_Node *last)
{
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
    return _MD_ParseExpr(ctx);
}

MD_FUNCTION_IMPL MD_Expr *
MD_ParseAsType(MD_Node *first, MD_Node *last)
{
    MD_Expr *expr = MD_NilExpr();
    MD_Expr *last_expr = expr;
    _MD_NodeParseCtx ctx_ = { first, last, last->next };
    _MD_NodeParseCtx *ctx = &ctx_;
#define _MD_PushType(x) if(MD_ExprIsNil(last_expr)) { expr = last_expr = x; } else { last_expr = last_expr->sub[0] = x; }
    MD_Node *set = 0;
    MD_Node *ptr = 0;
    MD_Node *base_type = 0;
    MD_Node *node = 0;
    for(;;)
    {
        if(_MD_NodeParse_Consume(ctx, MD_S8Lit("*"), &ptr))
        {
            MD_Expr *t = MD_MakeExpr(ptr, MD_ExprKind_Pointer, MD_NilExpr(), MD_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("volatile"), &node))
        {
            MD_Expr *t = MD_MakeExpr(node, MD_ExprKind_Volatile, MD_NilExpr(), MD_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_Consume(ctx, MD_S8Lit("const"), &node))
        {
            MD_Expr *t = MD_MakeExpr(node, MD_ExprKind_Const, MD_NilExpr(), MD_NilExpr());
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeSet(ctx, &set))
        {
            MD_Expr *t = MD_MakeExpr(set, MD_ExprKind_Array, MD_NilExpr(), MD_NilExpr());
            t->sub[1] = MD_ParseAsExpr(set->first_child, set->last_child);
            _MD_PushType(t);
        }
        else if(_MD_NodeParse_ConsumeAtom(ctx, &base_type))
        {
            MD_Expr *t = MD_MakeExpr(base_type, MD_ExprKind_Atom, MD_NilExpr(), MD_NilExpr());
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
MD_EvaluateExpr_I64(MD_Expr *expr)
{
    MD_i64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_ExprKind_##name: { result = MD_EvaluateExpr_I64(expr->sub[0]) op MD_EvaluateExpr_I64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_ExprKind_Atom: { result = MD_I64FromString(expr->node->string, 10); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_f64
MD_EvaluateExpr_F64(MD_Expr *expr)
{
    MD_f64 result = 0;
    switch(expr->kind)
    {
#define _MD_BinaryOp(name, op) case MD_ExprKind_##name: { result = MD_EvaluateExpr_I64(expr->sub[0]) op MD_EvaluateExpr_F64(expr->sub[1]); }break
        _MD_BinaryOp(Add,      +);
        _MD_BinaryOp(Subtract, -);
        _MD_BinaryOp(Multiply, *);
        _MD_BinaryOp(Divide,   /);
#undef _MD_BinaryOp
        case MD_ExprKind_Atom: { result = MD_F64FromString(expr->node->string); }break;
        default: break;
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprMatch(MD_Expr *a, MD_Expr *b, MD_MatchFlags flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind)
    {
        result = 1;
        if(a->kind == MD_ExprKind_Atom)
        {
            result = MD_StringMatch(a->node->string, b->node->string, flags);
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_ExprDeepMatch(MD_Expr *a, MD_Expr *b, MD_MatchFlags flags)
{
    MD_b32 result = MD_ExprMatch(a, b, flags);
    if(result && !MD_ExprIsNil(a) && !MD_ExprIsNil(b))
    {
        result = (MD_ExprDeepMatch(a->sub[0], b->sub[0], flags) &&
                  MD_ExprDeepMatch(a->sub[1], b->sub[1], flags));
    }
    return result;
}

//~ C Language Generation

MD_FUNCTION_IMPL void
MD_OutputTree_C_String(FILE *file, MD_Node *node)
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
MD_OutputTree_C_Struct(FILE *file, MD_Node *node)
{
    if(node)
    {
        fprintf(file, "typedef struct %.*s %.*s;\n",
                MD_StringExpand(node->string),
                MD_StringExpand(node->string));
        fprintf(file, "struct %.*s\n{\n", MD_StringExpand(node->string));
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_OutputTree_C_Decl(file, child);
            fprintf(file, ";\n");
        }
        fprintf(file, "};\n\n");
    }
}

MD_FUNCTION_IMPL void
MD_OutputTree_C_Decl(FILE *file, MD_Node *node)
{
    if(node)
    {
        MD_Expr *type = MD_ParseAsType(node->first_child, node->last_child);
        MD_Output_C_DeclByNameAndType(file, node->string, type);
    }
}

MD_FUNCTION_IMPL void
MD_Output_C_DeclByNameAndType(FILE *file, MD_String8 name, MD_Expr *type)
{
    MD_OutputType_C_LHS(file, type);
    fprintf(file, " %.*s", MD_StringExpand(name));
    MD_OutputType_C_RHS(file, type);
}

MD_FUNCTION_IMPL void
MD_OutputExpr(FILE *file, MD_Expr *expr)
{
    if(!MD_NodeIsNil(expr->node))
    {
        _MD_ExprKindMetadata *metadata = _MD_MetadataFromExprKind(expr->kind);
        switch(metadata->group)
        {
            case MD_ExprKindGroup_Atom:
            {
                fprintf(file, "%.*s", MD_StringExpand(expr->node->string));
            }break;
            
            case MD_ExprKindGroup_Binary:
            {
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, " %s ", metadata->symbol);
                MD_OutputExpr(file, expr->sub[1]);
                fprintf(file, ")");
            }break;
            
            case MD_ExprKindGroup_PreUnary:
            {
                fprintf(file, "%s", metadata->pre_symbol);
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, ")");
            }break;
            
            case MD_ExprKindGroup_PostUnary:
            {
                fprintf(file, "(");
                MD_OutputExpr(file, expr->sub[0]);
                fprintf(file, ")");
                fprintf(file, "%s", metadata->post_symbol);
            }break;
            
            default: break;
        }
    }
}

MD_FUNCTION_IMPL void
MD_OutputExpr_C(FILE *file, MD_Expr *expr)
{
    MD_OutputExpr(file, expr);
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_OutputType_C_NeedsParens(MD_Expr *type)
{
    MD_b32 result = 0;
    if (type->kind == MD_ExprKind_Pointer &&
        type->sub[0]->kind == MD_ExprKind_Array)
    {
        result = 1;
    }
    return(result);
}

MD_FUNCTION_IMPL void
MD_OutputType_C_LHS(FILE *file, MD_Expr *type)
{
    switch (type->kind)
    {
        case MD_ExprKind_Atom:
        {
            MD_Node *node = type->node;
            fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
        }break;
        
        case MD_ExprKind_Pointer:
        {
            MD_OutputType_C_LHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
            fprintf(file, "*");
        }break;
        
        case MD_ExprKind_Array:
        {
            MD_OutputType_C_LHS(file, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
        }break;
        
        case MD_ExprKind_Volatile: { fprintf(file, "volatile "); }break;
        case MD_ExprKind_Const:    { fprintf(file, "const "); }break;
        
        default:
        {
            fprintf(file, "{ unexpected MD_ExprKind (%i) in type info for node \"%.*s\" }",
                    type->kind,
                    MD_StringExpand(type->node->whole_string));
        }break;
    }
}

MD_FUNCTION_IMPL void
MD_OutputType_C_RHS(FILE *file, MD_Expr *type)
{
    switch (type->kind)
    {
        case MD_ExprKind_Atom:
        {}break;
        
        case MD_ExprKind_Pointer:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            MD_OutputType_C_RHS(file, type->sub[0]);
        }break;
        
        case MD_ExprKind_Array:
        {
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, ")");
            }
            fprintf(file, "[");
            MD_OutputExpr_C(file, type->sub[1]);
            fprintf(file, "]");
            MD_OutputType_C_RHS(file, type->sub[0]);
        }break;
        
        case MD_ExprKind_Volatile: { fprintf(file, "volatile "); }break;
        case MD_ExprKind_Const:    { fprintf(file, "const "); }break;
        
        default:
        {}break;
    }
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
