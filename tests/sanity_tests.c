#include "md.h"
#include "md_c_helpers.h"

#include "md.c"
#include "md_c_helpers.c"

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
    
#if 0
    if(result == 0)
    {
        __debugbreak();
    }
#endif
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
    return MD_MakeNode(kind, string, string, 0);
}

static MD_C_Expr *
AtomExpr(char *str)
{
    return MD_C_MakeExpr(MakeTestNode(MD_NodeKind_Label, MD_S8CString(str)),
                         MD_C_ExprKind_Atom, MD_C_NilExpr(), MD_C_NilExpr());
}

static MD_C_Expr *
BinOpExpr(MD_C_ExprKind kind, MD_C_Expr *left, MD_C_Expr *right)
{
    return MD_C_MakeExpr(MD_NilNode(), kind, left, right);
}

static MD_C_Expr *
TypeExpr(MD_C_ExprKind kind, MD_C_Expr *sub)
{
    return MD_C_MakeExpr(MD_NilNode(), kind, sub, MD_C_NilExpr());
}

static MD_b32
MatchParsedWithNode(MD_String8 string, MD_Node *tree)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    return MD_NodeDeepMatch(tree, parse.node, MD_MatchFlag_Tags | MD_MatchFlag_TagArguments);
}

static MD_b32
MatchParsedWithExpr(MD_String8 string, MD_C_Expr *expr)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    MD_C_Expr *parse_expr = MD_C_ParseAsExpr(parse.node->first_child, parse.node->last_child);
    return MD_C_ExprDeepMatch(expr, parse_expr, 0);
}

static MD_b32
MatchParsedWithType(MD_String8 string, MD_C_Expr *expr)
{
    MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), string);
    MD_C_Expr *parse_expr = MD_C_ParseAsType(parse.node->first_child, parse.node->last_child);
    return MD_C_ExprDeepMatch(expr, parse_expr, 0);
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
                   MD_NodeFlag_StringLiteral);
    }
    
    Test("Expression Evaluation")
    {
        // NOTE(rjf): 5 + 3
        {
            MD_C_Expr *expr = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_C_EvaluateExpr_I64(expr) == 8);
        }
        
        // NOTE(rjf): 5 - 3
        {
            MD_C_Expr *expr = BinOpExpr(MD_C_ExprKind_Subtract, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_C_EvaluateExpr_I64(expr) == 2);
        }
        
        // NOTE(rjf): 5 * 3
        {
            MD_C_Expr *expr = BinOpExpr(MD_C_ExprKind_Multiply, AtomExpr("5"), AtomExpr("3"));
            TestResult(MD_C_EvaluateExpr_I64(expr) == 15);
        }
        
        // NOTE(rjf): 10 / 2
        {
            MD_C_Expr *expr = BinOpExpr(MD_C_ExprKind_Divide, AtomExpr("10"), AtomExpr("2"));
            TestResult(MD_C_EvaluateExpr_I64(expr) == 5);
        }
        
        // NOTE(rjf): (3 + 4) * (2 + 6)
        {
            MD_C_Expr *left  = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("3"), AtomExpr("4"));
            MD_C_Expr *right = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("2"), AtomExpr("6"));
            MD_C_Expr *expr  = BinOpExpr(MD_C_ExprKind_Multiply, left, right);
            TestResult(MD_C_EvaluateExpr_I64(expr) == 56);
        }
    }
    
    Test("Expression Parsing")
    {
        {
            MD_String8 string = MD_S8Lit("(1 + 2)");
            MD_C_Expr *expr  = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("1"), AtomExpr("2"));
            TestResult(MatchParsedWithExpr(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("((3 + 4) * (2 + 6))");
            MD_C_Expr *left  = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("3"), AtomExpr("4"));
            MD_C_Expr *right = BinOpExpr(MD_C_ExprKind_Add, AtomExpr("2"), AtomExpr("6"));
            MD_C_Expr *expr  = BinOpExpr(MD_C_ExprKind_Multiply, left, right);
            TestResult(MatchParsedWithExpr(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(1*2+3)");
            MD_C_Expr *left  = BinOpExpr(MD_C_ExprKind_Multiply, AtomExpr("1"), AtomExpr("2"));
            MD_C_Expr *expr  = BinOpExpr(MD_C_ExprKind_Add, left, AtomExpr("3"));
            TestResult(MatchParsedWithExpr(string, expr));
        }
    }
    
    Test("Type Parsing")
    {
        {
            MD_String8 string = MD_S8Lit("(i32)");
            MD_C_Expr *expr = AtomExpr("i32");
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(*i32)");
            MD_C_Expr *expr = TypeExpr(MD_C_ExprKind_Pointer, AtomExpr("i32"));
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(**i32)");
            MD_C_Expr *expr = TypeExpr(MD_C_ExprKind_Pointer, TypeExpr(MD_C_ExprKind_Pointer, AtomExpr("i32")));
            TestResult(MatchParsedWithType(string, expr));
        }
        {
            MD_String8 string = MD_S8Lit("(*void)");
            MD_C_Expr *expr = TypeExpr(MD_C_ExprKind_Pointer, AtomExpr("void"));
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
        MD_String8List list = MD_StringListFromNodeFlags(MD_NodeFlag_StringLiteral | MD_NodeFlag_ParenLeft | MD_NodeFlag_BeforeSemicolon);
        MD_b32 match = 1;
        for(MD_String8Node *node = list.first; node; node = node->next)
        {
            if(!MD_StringMatch(node->string, MD_S8Lit("StringLiteral"), 0)     &&
               !MD_StringMatch(node->string, MD_S8Lit("ParenLeft"), 0)       &&
               !MD_StringMatch(node->string, MD_S8Lit("BeforeSemicolon"), 0))
            {
                match = 0;
                break;
            }
        }
        TestResult(match);
    }
    
    Test("Node Comments")
    {
        
        // NOTE(rjf): Pre-Comments:
        {
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("/*foobar*/ (a b c)"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_before, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("// foobar\n(a b c)"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_before, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("// foobar\n\n(a b c)"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_before, MD_S8Lit(""), 0));
            }
        }
        
        // NOTE(rjf): Post-Comments:
        {
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a b c) /*foobar*/"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_after, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a b c) // foobar"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_after, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a b c)\n// foobar"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_after, MD_S8Lit(""), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("(a b c)\n\n// foobar"));
                TestResult(parse.node->kind == MD_NodeKind_Label &&
                           MD_StringMatch(parse.node->comment_after, MD_S8Lit(""), 0));
            }
        }
    }
    
    Test("Errors")
    {
        struct { char *s; int columns[2]; } tests[] = {
            {"{", {1}},
            {"}", {1}},
            {"'", {1}},
            {"a:'''\nmulti-line text literal", {3}},
            {"/* foo", {1}},
            {"label:@tag {1, 2, 3} /* /* unterminated comment */", {8, 22}},
            {"@\"tag\" node", {2}},
            {"{a,,#b,}", {4, 5}},
            {"foo""\x80""bar", {4}},
        };
        
        int max_error_count = MD_ArrayCount(tests[0].columns);
        
        for(int i_test = 0; i_test < MD_ArrayCount(tests); ++i_test)
        {
            MD_ParseResult parse = MD_ParseWholeString(MD_S8Lit("test.md"), MD_S8CString(tests[i_test].s));
            
            MD_b32 columns_match = 1;
            {
                MD_Error *e = parse.first_error;
                for(int i_error = 0; i_error < max_error_count && tests[i_test].columns[i_error]; ++i_error)
                {
                    if(!e || MD_CodeLocFromNode(e->node).column != tests[i_test].columns[i_error])
                    {
                        columns_match = 0;
                        break;
                    }
                    e = e->next;
                }
                
                if(e && e->next)
                {
                    columns_match = 0;
                }
            }
            TestResult(columns_match);
        }
        
        {
            MD_ParseResult parse = MD_ParseWholeFile(MD_S8Lit("__does_not_exist.md"));
            TestResult(parse.node->kind == MD_NodeKind_File && parse.first_error);
        }
        
    }
    
    Test("Hash maps")
    {
        MD_String8 key_strings[] = 
        {
            MD_S8Lit("\xed\x80\x73\x71\x78\xba\xff\xd6\x87\x83\xcd\x20\x28\xf7\x1c\xc1\x5f\xca\x98\x9c\x5a\xab\x0c\xae\x9a\x60\x57\x03\xeb\x1f\xde\x99"),
            MD_S8Lit("\x4c\x80\xb7\x8b\xbf\x65\x5a\x4b\xc1\x2a\xc3\x5f\xe1\x66\xfb\x0d\x72\x83\x1c\x63\xba\xb5\x97\x02\x3f\x6a\xe0\x2a\x1b\x82\x07\x76"),
            MD_S8Lit("\xd8\xfd\x11\x4b\x04\xdf\xe5\x20\x5b\xd6\x4f\x87\x00\x70\x6a\xc8\xde\xed\xc7\x79\xdb\x87\x24\x36\xa8\x7a\x31\x41\x00\x57\xbd\x8d"),
        };
        
        MD_MapKey keys[MD_ArrayCount(key_strings)*2];
        for (MD_u64 i = 0; i < MD_ArrayCount(key_strings); i += 1){
            keys[i] = MD_MapKeyStr(key_strings[i]);
        }
        for (MD_u64 i = MD_ArrayCount(key_strings); i < MD_ArrayCount(keys); i += 1){
            keys[i] = MD_MapKeyPtr((void *)i);
        }
        
        {
            MD_Map map = MD_MapMake();
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1){
                MD_MapInsert(&map, keys[i], (void *)i);
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1){
                MD_MapSlot *slot = MD_MapLookup(&map, keys[i]);
                TestResult(slot && slot->val == (void *)i);
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1){
                MD_MapOverwrite(&map, keys[i], (void *)(i + 10));
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1){
                MD_MapSlot *slot = MD_MapLookup(&map, keys[i]);
                TestResult(slot && slot->val == (void *)(i + 10));
            }
        }
    }
    
    Test("String Inner & Outer")
    {
        MD_String8 samples[6] = {
            MD_S8Lit("'foo-bar'"),
            MD_S8Lit("'''foo-bar'''"),
            MD_S8Lit("\"foo-bar\""),
            MD_S8Lit("\"\"\"foo-bar\"\"\""),
            MD_S8Lit("`foo-bar`"),
            MD_S8Lit("```foo-bar```"),
        };
        
        MD_Node *nodes[MD_ArrayCount(samples)];
        for (int i = 0; i < MD_ArrayCount(samples); i += 1){
            MD_ParseResult result = MD_ParseOneNode(MD_S8Lit(""), samples[i]);
            nodes[i] = result.node;
        }
        
        TestResult(MD_StringMatch(nodes[0]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_StringMatch(nodes[1]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_StringMatch(nodes[2]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_StringMatch(nodes[3]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_StringMatch(nodes[4]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_StringMatch(nodes[5]->string, MD_S8Lit("foo-bar"), 0));
        
        TestResult(MD_StringMatch(nodes[0]->whole_string, samples[0], 0));
        TestResult(MD_StringMatch(nodes[1]->whole_string, samples[1], 0));
        TestResult(MD_StringMatch(nodes[2]->whole_string, samples[2], 0));
        TestResult(MD_StringMatch(nodes[3]->whole_string, samples[3], 0));
        TestResult(MD_StringMatch(nodes[4]->whole_string, samples[4], 0));
        TestResult(MD_StringMatch(nodes[5]->whole_string, samples[5], 0));
    }
    
    Test("String escaping")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("`\\``"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("\\`"), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("``` \\``` ```"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit(" \\``` "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("`````\\````"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("``\\`"), 0));
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("`\\'`"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("\\'"), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("''' \\''' '''"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit(" \\''' "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("'''''\\''''"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("''\\'"), 0));
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("`\\\"`"));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("\\\""), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("\"\"\" \\\"\"\" \"\"\""));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit(" \\\"\"\" "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("\"\"\"\"\"\\\"\"\"\""));
            TestResult(MD_StringMatch(parse.node->string, MD_S8Lit("\"\"\\\""), 0));
        }
    }
    
    Test("Node-With-Flags Seeking")
    {
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("foo:{x y z; a b c}"));
            MD_Node *node = parse.node;
            MD_Node *group_first = node->first_child;
            MD_Node *group_last = MD_SeekNodeWithFlags(group_first, MD_NodeFlag_AfterSemicolon);
            
            TestResult(MD_StringMatch(group_first->string,       MD_S8Lit("x"), 0));
            TestResult(MD_StringMatch(group_first->next->string, MD_S8Lit("y"), 0));
            TestResult(MD_StringMatch(group_last->string,        MD_S8Lit("z"), 0));
            
            group_first = group_last->next;
            group_last = MD_SeekNodeWithFlags(group_first, MD_NodeFlag_AfterSemicolon);
            
            TestResult(MD_StringMatch(group_first->string,       MD_S8Lit("a"), 0));
            TestResult(MD_StringMatch(group_first->next->string, MD_S8Lit("b"), 0));
            TestResult(MD_StringMatch(group_last->string,        MD_S8Lit("c"), 0));
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("foo:{a b c , d e f , g h i}"));
            MD_Node *node = parse.node;
            MD_Node *group_first = 0;
            MD_Node *group_last = 0;
            
            group_first = node->first_child;
            group_last = MD_SeekNodeWithFlags(group_first, MD_NodeFlag_AfterComma);
            TestResult(MD_StringMatch(group_first->string,       MD_S8Lit("a"), 0));
            TestResult(MD_StringMatch(group_first->next->string, MD_S8Lit("b"), 0));
            TestResult(MD_StringMatch(group_last->string,        MD_S8Lit("c"), 0));
            
            group_first = group_last->next;
            group_last = MD_SeekNodeWithFlags(group_first, MD_NodeFlag_AfterComma);
            TestResult(MD_StringMatch(group_first->string,       MD_S8Lit("d"), 0));
            TestResult(MD_StringMatch(group_first->next->string, MD_S8Lit("e"), 0));
            TestResult(MD_StringMatch(group_last->string,        MD_S8Lit("f"), 0));
            
            group_first = group_last->next;
            group_last = MD_SeekNodeWithFlags(group_first, MD_NodeFlag_AfterComma);
            TestResult(MD_StringMatch(group_first->string,       MD_S8Lit("g"), 0));
            TestResult(MD_StringMatch(group_first->next->string, MD_S8Lit("h"), 0));
            TestResult(MD_StringMatch(group_last->string,        MD_S8Lit("i"), 0));
        }
        
    }
    
    Test("Scoped in Unscoped")
    {
        // TODO(allen): This test is to reveal a strange phenomenon in the current
        // grammar; it should be eliminated if we decide to disallow this
        MD_String8 file_name = MD_S8Lit("raw_text");
        {
            MD_String8 text = MD_S8Lit("foo: bar {\n"
                                       "fiz, baz\n"
                                       "} end\n");
            MD_ParseResult result = MD_ParseWholeString(file_name, text);
            MD_Node *foo_node = MD_ChildFromString(result.node, MD_S8Lit("foo"));
            MD_Node *end_node = MD_ChildFromString(foo_node, MD_S8Lit("end"));
            TestResult(!MD_NodeIsNil(end_node));
        }
    }
    
    Test("Tagged & Unlabeled")
    {
        // TODO(allen): these tests checking for rules that I find fishy; maybe instead
        // of trying to pass these tests, adjust the rules so we don't have this odd
        // expected behavior to begin with? Not sure.
        MD_String8 file_name = MD_S8Lit("raw_text");
        {
            MD_ParseResult result = MD_ParseWholeString(file_name, MD_S8Lit("foo:{@tag {bar}}\n"));
            TestResult(result.first_error == 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(file_name, MD_S8Lit("foo:@tag bar\n"));
            TestResult(result.first_error == 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(file_name, MD_S8Lit("foo:bar @tag {bar}\n"));
            TestResult(result.first_error == 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(file_name, MD_S8Lit("foo:@tag {bar}\n"));
            TestResult(result.first_error != 0);
        }
    }
    
    return 0;
}