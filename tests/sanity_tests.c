#include "md.h"
#include "md.c"

static struct
{
    int number_of_tests;
    int number_passed;
}
test_ctx;

static void
BeginTest(char *name)
{
    int length = MD_CalculateCStringLength(name);
    int spaces = 25 - length;
    if(spaces < 0)
    {
        spaces = 0;
    }
    printf("\"%s\" %.*s [", name, spaces, "------------------------------");
    test_ctx.number_of_tests = 0;
    test_ctx.number_passed = 0;
}

static void
TestResult(MD_b32 result)
{
    test_ctx.number_of_tests += 1;
    test_ctx.number_passed += !!result;
    printf(result ? "." : "X");
}

static void
EndTest(void)
{
    int spaces = 10 - test_ctx.number_of_tests;
    if(spaces < 0) { spaces = 0; }
    printf("]%.*s ", spaces, "                                      ");
    printf("[%i/%i] %i passed, %i tests, ",
           test_ctx.number_passed, test_ctx.number_of_tests,
           test_ctx.number_passed, test_ctx.number_of_tests);
    if(test_ctx.number_of_tests == test_ctx.number_passed)
    {
        printf("SUCCESS ( )");
    }
    else
    {
        printf("FAILED  (X)");
    }
    printf("\n");
}

#define Test(name) for(int _i_ = (BeginTest(name), 0); !_i_; _i_ += 1, EndTest())

static MD_Node *
MakeTestNode(MD_NodeKind kind, MD_String8 string)
{
    return MD_MakeNodeFromString(kind, MD_S8Lit("`TEST_NODE`"), 0, 0, string);
}

static MD_Expr *
AtomExpr(char *str)
{
    return MD_MakeExpr(MakeTestNode(MD_NodeKind_Label, MD_S8CString(str)), MD_ExprKind_Atom, MD_NilExpr(), MD_NilExpr());
}

static MD_Expr *
BinOpExpr(MD_ExprKind kind, MD_Expr *left, MD_Expr *right)
{
    return MD_MakeExpr(MD_NilNode(), kind, left, right);
}

static MD_Expr *
TypeExpr(MD_ExprKind kind, MD_Expr *sub)
{
    return MD_MakeExpr(MD_NilNode(), kind, sub, MD_NilExpr());
}

static MD_b32
MatchParsedWithNode(MD_String8 string, MD_Node *tree)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    return MD_NodeDeepMatch(tree, parse.node, 0, MD_NodeMatchFlag_Tags | MD_NodeMatchFlag_TagArguments);
}

static MD_b32
MatchParsedWithExpr(MD_String8 string, MD_Expr *expr)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    MD_Expr *parse_expr = MD_ParseAsExpr(parse.node->first_child, parse.node->last_child);
    return MD_ExprDeepMatch(expr, parse_expr, 0);
}

static MD_b32
MatchParsedWithType(MD_String8 string, MD_Expr *expr)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    MD_Expr *parse_expr = MD_ParseAsType(parse.node->first_child, parse.node->last_child);
    return MD_ExprDeepMatch(expr, parse_expr, 0);
}

static MD_b32
TokenMatch(MD_Token token, MD_String8 string, MD_TokenKind kind)
{
    return MD_StringMatch(string, token.string, 0) && token.kind == kind;
}

int main(void)
{
    
    Test("Lexer")
    {
        MD_String8 string = MD_S8Lit("abc def 123 456 123_456 abc123 123abc");
        MD_ParseCtx ctx = MD_Parse_InitializeCtx(MD_S8Lit(""), string);
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit("abc"), MD_TokenKind_Identifier));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit("def"), MD_TokenKind_Identifier));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit("123"), MD_TokenKind_NumericLiteral));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit("456"), MD_TokenKind_NumericLiteral));
        TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit(" "), MD_TokenKind_Whitespace));
        // TODO(rjf): Enable once numeric literal lexing is fixed
        //TestResult(TokenMatch(MD_Parse_LexNext(&ctx), MD_S8Lit("123_456"), MD_TokenKind_NumericLiteral));
    }
    
    Test("Empty Sets")
    {
        TestResult(MatchParsedWithNode(MD_S8Lit("{}"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("()"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("[]"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("[)"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("(]"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""))));
    }
    
    Test("Simple Unnamed Sets")
    {
        {
            MD_String8 string = MD_S8Lit("{a, b, c}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("a")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("b")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("c")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        {
            MD_String8 string = MD_S8Lit("(1 2 3 4 5)");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("1")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("2")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("3")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("4")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("5")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        {
            MD_String8 string = MD_S8Lit("{a}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("a")));
            TestResult(MatchParsedWithNode(string, tree));
        }
    }
    
    Test("Simple Named Sets")
    {
        MD_String8 string = MD_S8Lit("simple_set: {a, b, c}");
        MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit("simple_set"));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("a")));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("b")));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("c")));
        TestResult(MatchParsedWithNode(string, tree));
    }
    
    Test("Nested Sets")
    {
        {
            MD_String8 string = MD_S8Lit("{a b:{1 2 3} c}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("a")));
            {
                MD_Node *sub = MakeTestNode(MD_NodeKind_Label, MD_S8Lit("b"));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("1")));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("2")));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("3")));
                MD_PushChild(tree, sub);
            }
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("c")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        
        {
            MD_String8 string = MD_S8Lit("foo: { (size: u64) -> *void }");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Label, MD_S8Lit("foo"));
            MD_Node *params = MakeTestNode(MD_NodeKind_Label, MD_S8Lit(""));
            MD_Node *size = MakeTestNode(MD_NodeKind_Label, MD_S8Lit("size"));
            MD_PushChild(size, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("u64")));
            MD_PushChild(params, size);
            MD_PushChild(tree, params);
            // TODO(rjf): This test will fail once we have digraphs implemented. Adjust the separate
            // "-" and ">" set members, and combine them to form a single "->" set member.
            // {
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("-")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit(">")));
            // }
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("*")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Label, MD_S8Lit("void")));
            TestResult(MatchParsedWithNode(string, tree));
        }
    }
    
    Test("Non-Sets")
    {
        TestResult(MatchParsedWithNode(MD_S8Lit("foo"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit("foo"))));
        TestResult(MatchParsedWithNode(MD_S8Lit("123"), MakeTestNode(MD_NodeKind_Label, MD_S8Lit("123"))));
        TestResult(MatchParsedWithNode(MD_S8Lit("+"),   MakeTestNode(MD_NodeKind_Label, MD_S8Lit("+"))));
    }
    
    Test("Set Border Flags")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(0, 100)"));
            TestResult(parse.node->flags & MD_NodeFlag_ParenLeft &&
                       parse.node->flags & MD_NodeFlag_ParenRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(0, 100]"));
            TestResult(parse.node->flags & MD_NodeFlag_ParenLeft &&
                       parse.node->flags & MD_NodeFlag_BracketRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("[0, 100)"));
            TestResult(parse.node->flags & MD_NodeFlag_BracketLeft &&
                       parse.node->flags & MD_NodeFlag_ParenRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("[0, 100]"));
            TestResult(parse.node->flags & MD_NodeFlag_BracketLeft &&
                       parse.node->flags & MD_NodeFlag_BracketRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("{0, 100}"));
            TestResult(parse.node->flags & MD_NodeFlag_BraceLeft &&
                       parse.node->flags & MD_NodeFlag_BraceRight);
        }
    }
    
    Test("Node Separator Flags")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a, b)"));
            TestResult(parse.node->first_child->flags & MD_NodeFlag_BeforeComma);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a; b)"));
            TestResult(parse.node->first_child->flags & MD_NodeFlag_BeforeSemicolon);
        }
        {
            // TODO(rjf): Enable this once we have digraphs.
            // MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a -> b)"));
            // TestResult(parse.node->first_child->flags & MD_NodeFlag_BeforeArrow);
        }
    }
    
    Test("Node Text Flags")
    {
        TestResult(MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("123")).node->flags &
                   MD_NodeFlag_Numeric);
        TestResult(MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("123_456_789")).node->flags &
                   MD_NodeFlag_Numeric);
        TestResult(MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("abc")).node->flags &
                   MD_NodeFlag_Identifier);
        TestResult(MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("\"foo\"")).node->flags &
                   MD_NodeFlag_StringLiteral);
        TestResult(MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("'foo'")).node->flags &
                   MD_NodeFlag_CharLiteral);
    }
    
    Test("Expression Evaluation")
    {
        // NOTE(rjf): 5 + 3
        {
            MD_Expr *expr = BinOpExpr(MD_ExprKind_Add, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_EvaluateExpr_I64(expr) == 8);
        }
        
        // NOTE(rjf): 5 - 3
        {
            MD_Expr *expr = BinOpExpr(MD_ExprKind_Subtract, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_EvaluateExpr_I64(expr) == 2);
        }
        
        // NOTE(rjf): 5 * 3
        {
            MD_Expr *expr = BinOpExpr(MD_ExprKind_Multiply, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_EvaluateExpr_I64(expr) == 15);
        }
        
        // NOTE(rjf): 10 / 2
        {
            MD_Expr *expr = BinOpExpr(MD_ExprKind_Divide, AtomExpr("10"), AtomExpr("2"));
            TestResult(MD_EvaluateExpr_I64(expr) == 5);
        }
        
        // NOTE(rjf): (3 + 4) * (2 + 6)
        {
            MD_Expr *left  = BinOpExpr(MD_ExprKind_Add, AtomExpr("3"), AtomExpr("4"));
            MD_Expr *right = BinOpExpr(MD_ExprKind_Add, AtomExpr("2"), AtomExpr("6"));
            MD_Expr *expr  = BinOpExpr(MD_ExprKind_Multiply, left, right);
            TestResult(MD_EvaluateExpr_I64(expr) == 56);
        }
    }
    
    Test("Expression Parsing")
    {
        {
            MD_String8 string = MD_S8Lit("(1 + 2)");
            MD_Expr *expr  = BinOpExpr(MD_ExprKind_Add, AtomExpr("1"), AtomExpr("2"));
            TestResult(MatchParsedWithExpr(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("((3 + 4) * (2 + 6))");
            MD_Expr *left  = BinOpExpr(MD_ExprKind_Add, AtomExpr("3"), AtomExpr("4"));
            MD_Expr *right = BinOpExpr(MD_ExprKind_Add, AtomExpr("2"), AtomExpr("6"));
            MD_Expr *expr  = BinOpExpr(MD_ExprKind_Multiply, left, right);
            TestResult(MatchParsedWithExpr(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(1*2+3)");
            MD_Expr *left  = BinOpExpr(MD_ExprKind_Multiply, AtomExpr("1"), AtomExpr("2"));
            MD_Expr *expr  = BinOpExpr(MD_ExprKind_Add, left, AtomExpr("3"));
            TestResult(MatchParsedWithExpr(string, expr));
        }
    }
    
    Test("Type Parsing")
    {
        {
            MD_String8 string = MD_S8Lit("(i32)");
            MD_Expr *expr = AtomExpr("i32");
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(*i32)");
            MD_Expr *expr = TypeExpr(MD_ExprKind_Pointer, AtomExpr("i32"));
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(**i32)");
            MD_Expr *expr = TypeExpr(MD_ExprKind_Pointer, TypeExpr(MD_ExprKind_Pointer, AtomExpr("i32")));
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(*void)");
            MD_Expr *expr = TypeExpr(MD_ExprKind_Pointer, AtomExpr("void"));
            TestResult(MatchParsedWithType(string, expr));
        }
    }
    
    Test("Style Strings")
    {
        {
            MD_String8 str = MD_StyledStringFromString(MD_S8Lit("THIS_IS_A_TEST"), MD_WordStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_StringMatch(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_StyledStringFromString(MD_S8Lit("this_is_a_test"), MD_WordStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_StringMatch(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_StyledStringFromString(MD_S8Lit("ThisIsATest"), MD_WordStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_StringMatch(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_StyledStringFromString(MD_S8Lit("Here is another test."), MD_WordStyle_UpperCamelCase, MD_S8Lit(""));
            TestResult(MD_StringMatch(str, MD_S8Lit("HereIsAnotherTest."), 0));
        }
    }
    
    Test("Enum Strings")
    {
        TestResult(MD_StringMatch(MD_StringFromNodeKind(MD_NodeKind_Label), MD_S8Lit("Label"), 0));
        TestResult(MD_StringMatch(MD_StringFromNodeKind(MD_NodeKind_Label), MD_S8Lit("Label"), 0));
        MD_String8List list = MD_StringListFromNodeFlags(MD_NodeFlag_CharLiteral | MD_NodeFlag_ParenLeft | MD_NodeFlag_BeforeSemicolon);
        MD_b32 match = 1;
        for(MD_String8Node *node = list.first; node; node = node->next)
        {
            if(!MD_StringMatch(node->string, MD_S8Lit("CharLiteral"), 0)     &&
               !MD_StringMatch(node->string, MD_S8Lit("ParenLeft"), 0)       &&
               !MD_StringMatch(node->string, MD_S8Lit("BeforeSemicolon"), 0))
            {
                match = 0;
                break;
            }
        }
        TestResult(match);
    }
    
    return 0;
}