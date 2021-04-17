/* date = April 16th 2021 6:05 pm */

#ifndef MD_C_HELPERS_H
#define MD_C_HELPERS_H

//~ Expression and Type-Expression parser helper types.

typedef enum MD_ExprKind
{
    // VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
    // it is kept in-sync with the _MD_MetadataFromExprKind function.
    
    MD_ExprKind_Nil,
    
    // NOTE(rjf): Atom
    MD_ExprKind_Atom,
    
    // NOTE(rjf): Access
    MD_ExprKind_Dot,
    MD_ExprKind_Arrow,
    MD_ExprKind_Call,
    MD_ExprKind_Subscript,
    MD_ExprKind_Dereference,
    MD_ExprKind_Reference,
    
    // NOTE(rjf): Arithmetic
    MD_ExprKind_Add,
    MD_ExprKind_Subtract,
    MD_ExprKind_Multiply,
    MD_ExprKind_Divide,
    MD_ExprKind_Mod,
    
    // NOTE(rjf): Comparison
    MD_ExprKind_IsEqual,
    MD_ExprKind_IsNotEqual,
    MD_ExprKind_LessThan,
    MD_ExprKind_GreaterThan,
    MD_ExprKind_LessThanEqualTo,
    MD_ExprKind_GreaterThanEqualTo,
    
    // NOTE(rjf): Bools
    MD_ExprKind_BoolAnd,
    MD_ExprKind_BoolOr,
    MD_ExprKind_BoolNot,
    
    // NOTE(rjf): Bitwise
    MD_ExprKind_BitAnd,
    MD_ExprKind_BitOr,
    MD_ExprKind_BitNot,
    MD_ExprKind_BitXor,
    MD_ExprKind_LeftShift,
    MD_ExprKind_RightShift,
    
    // NOTE(rjf): Unary numeric
    MD_ExprKind_Negative,
    
    // NOTE(rjf): Type
    MD_ExprKind_Pointer,
    MD_ExprKind_Array,
    MD_ExprKind_Volatile,
    MD_ExprKind_Const,
    
    MD_ExprKind_MAX,
}
MD_ExprKind;

typedef enum MD_ExprKindGroup
{
    MD_ExprKindGroup_Nil,
    MD_ExprKindGroup_Atom,
    MD_ExprKindGroup_Binary,
    MD_ExprKindGroup_PreUnary,
    MD_ExprKindGroup_PostUnary,
    MD_ExprKindGroup_Type,
}
MD_ExprKindGroup;

typedef MD_i32 MD_ExprPrec;

typedef struct MD_Expr MD_Expr;
struct MD_Expr
{
    MD_Node *node;
    MD_ExprKind kind;
    MD_Expr *parent;
    MD_Expr *sub[2];
};

//~ Expression and Type-Expression Helper
MD_FUNCTION MD_Expr *     MD_NilExpr(void);
MD_FUNCTION MD_b32        MD_ExprIsNil(MD_Expr *expr);
MD_FUNCTION MD_ExprKind   MD_PreUnaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_ExprKind   MD_BinaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_ExprPrec   MD_ExprPrecFromExprKind(MD_ExprKind kind);
MD_FUNCTION MD_Expr *     MD_MakeExpr(MD_Node *node, MD_ExprKind kind, MD_Expr *left, MD_Expr *right);
MD_FUNCTION MD_Expr *     MD_ParseAsExpr(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_Expr *     MD_ParseAsType(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_i64        MD_EvaluateExpr_I64(MD_Expr *expr);
MD_FUNCTION MD_f64        MD_EvaluateExpr_F64(MD_Expr *expr);
MD_FUNCTION MD_b32        MD_ExprMatch(MD_Expr *a, MD_Expr *b, MD_MatchFlags flags);
MD_FUNCTION MD_b32        MD_ExprDeepMatch(MD_Expr *a, MD_Expr *b, MD_MatchFlags flags);

//~ C Language Generation
MD_FUNCTION void MD_C_Generate_String(FILE *file, MD_Node *node);
MD_FUNCTION void MD_C_Generate_Struct(FILE *file, MD_Node *node);
MD_FUNCTION void MD_C_Generate_Decl(FILE *file, MD_Node *node);
MD_FUNCTION void MD_C_Generate_DeclByNameAndType(FILE *file, MD_String8 name, MD_Expr *type);
MD_FUNCTION void MD_C_Generate_Expr(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_C_Generate_LHS(FILE *file, MD_Expr *type);
MD_FUNCTION void MD_C_Generate_RHS(FILE *file, MD_Expr *type);

#endif //MD_C_HELPERS_H

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
