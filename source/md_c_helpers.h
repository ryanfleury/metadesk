/* date = April 16th 2021 6:05 pm */

#ifndef MD_C_HELPERS_H
#define MD_C_HELPERS_H

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
MD_FUNCTION void MD_OutputTree_C_String(FILE *file, MD_Node *node);
MD_FUNCTION void MD_OutputTree_C_Struct(FILE *file, MD_Node *node);
MD_FUNCTION void MD_OutputTree_C_Decl(FILE *file, MD_Node *node);
MD_FUNCTION void MD_Output_C_DeclByNameAndType(FILE *file, MD_String8 name, MD_Expr *type);
MD_FUNCTION void MD_OutputExpr(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputExpr_C(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputType(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputType_C_LHS(FILE *file, MD_Expr *type);
MD_FUNCTION void MD_OutputType_C_RHS(FILE *file, MD_Expr *type);

#endif //MD_C_HELPERS_H
