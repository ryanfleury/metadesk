/* date = April 16th 2021 6:05 pm */

#ifndef MD_C_HELPERS_H
#define MD_C_HELPERS_H

//~ Expression and Type-Expression parser helper types.

typedef enum MD_C_ExprKind
{
    // VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
    // it is kept in-sync with the _MD_MetadataFromExprKind function.
    
    MD_C_ExprKind_Nil,
    
    // NOTE(rjf): Atom
    MD_C_ExprKind_Atom,
    
    // NOTE(rjf): Access
    MD_C_ExprKind_Dot,
    MD_C_ExprKind_Arrow,
    MD_C_ExprKind_Call,
    MD_C_ExprKind_Subscript,
    MD_C_ExprKind_Dereference,
    MD_C_ExprKind_Reference,
    
    // NOTE(rjf): Arithmetic
    MD_C_ExprKind_Add,
    MD_C_ExprKind_Subtract,
    MD_C_ExprKind_Multiply,
    MD_C_ExprKind_Divide,
    MD_C_ExprKind_Mod,
    
    // NOTE(rjf): Comparison
    MD_C_ExprKind_IsEqual,
    MD_C_ExprKind_IsNotEqual,
    MD_C_ExprKind_LessThan,
    MD_C_ExprKind_GreaterThan,
    MD_C_ExprKind_LessThanEqualTo,
    MD_C_ExprKind_GreaterThanEqualTo,
    
    // NOTE(rjf): Bools
    MD_C_ExprKind_BoolAnd,
    MD_C_ExprKind_BoolOr,
    MD_C_ExprKind_BoolNot,
    // NOTE(rjf): Bitwise
    MD_C_ExprKind_BitAnd,
    MD_C_ExprKind_BitOr,
    MD_C_ExprKind_BitNot,
    MD_C_ExprKind_BitXor,
    MD_C_ExprKind_LeftShift,
    MD_C_ExprKind_RightShift,
    
    // NOTE(rjf): Unary numeric
    MD_C_ExprKind_Negative,
    
    // NOTE(rjf): Type
    MD_C_ExprKind_Pointer,
    MD_C_ExprKind_Array,
    MD_C_ExprKind_Volatile,
    MD_C_ExprKind_Const,
    
    MD_C_ExprKind_MAX,
}
MD_C_ExprKind;

typedef enum MD_C_ExprKindGroup
{
    MD_C_ExprKindGroup_Nil,
    MD_C_ExprKindGroup_Atom,
    MD_C_ExprKindGroup_Binary,
    MD_C_ExprKindGroup_PreUnary,
    MD_C_ExprKindGroup_PostUnary,
    MD_C_ExprKindGroup_Type,
}
MD_C_ExprKindGroup;

typedef MD_i32 MD_C_ExprPrec;

typedef struct MD_C_Expr MD_C_Expr;
struct MD_C_Expr
{
    MD_Node *node;
    MD_C_ExprKind kind;
    MD_C_Expr *parent;
    MD_C_Expr *sub[2];
};

//~ C_Expression and Type-C_Expression Helper
MD_FUNCTION MD_C_Expr *   MD_C_NilExpr(void);
MD_FUNCTION MD_b32        MD_C_ExprIsNil(MD_C_Expr *expr);
MD_FUNCTION MD_C_ExprKind MD_C_PreUnaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_C_ExprKind MD_C_BinaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_C_ExprPrec MD_C_ExprPrecFromExprKind(MD_C_ExprKind kind);
MD_FUNCTION MD_C_Expr *   MD_C_MakeExpr(MD_Node *node, MD_C_ExprKind kind, MD_C_Expr *left, MD_C_Expr *right);
MD_FUNCTION MD_C_Expr *   MD_C_ParseAsExpr(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_C_Expr *   MD_C_ParseAsType(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_i64        MD_C_EvaluateExpr_I64(MD_C_Expr *expr);
MD_FUNCTION MD_f64        MD_C_EvaluateExpr_F64(MD_C_Expr *expr);
MD_FUNCTION MD_b32        MD_C_ExprMatch(MD_C_Expr *a, MD_C_Expr *b, MD_MatchFlags flags);
MD_FUNCTION MD_b32        MD_C_ExprDeepMatch(MD_C_Expr *a, MD_C_Expr *b, MD_MatchFlags flags);

//~ C Language Generation
MD_FUNCTION void MD_C_Generate_String(FILE *file, MD_Node *node);
MD_FUNCTION void MD_C_Generate_Struct(FILE *file, MD_Node *node);

MD_FUNCTION void MD_C_Generate_Expr(FILE *file, MD_C_Expr *expr);
MD_FUNCTION void MD_C_Generate_TypeLHS(FILE *file, MD_C_Expr *type);
MD_FUNCTION void MD_C_Generate_TypeRHS(FILE *file, MD_C_Expr *type);

MD_FUNCTION void MD_C_Generate_DeclByNameAndType(FILE *file, MD_String8 name, MD_C_Expr *type);
MD_FUNCTION void MD_C_Generate_Decl(FILE *file, MD_Node *node);

#endif //MD_C_HELPERS_H

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
