//$ exe //

#include "md.h"
#include "md.c"

MD_Arena *arena = 0;

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
    int spaces = 20 - test_ctx.number_of_tests;
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
    return MD_MakeNode(arena, kind, string, string, 0);
}

static MD_b32
MatchParsedWithNode(MD_String8 string, MD_Node *tree)
{
    MD_ParseResult parse = MD_ParseOneNode(arena, string, 0);
    return MD_NodeDeepMatch(tree, parse.node, MD_NodeMatchFlag_Tags | MD_NodeMatchFlag_TagArguments);
}

static MD_b32
TokenMatch(MD_Token token, MD_String8 string, MD_TokenKind kind)
{
    return MD_S8Match(string, token.string, 0) && token.kind == kind;
}

int main(void)
{
    arena = MD_ArenaAlloc();
    
    Test("Lexer")
    {
        MD_String8 string = MD_S8Lit("abc def 123 456 123_456 abc123 123abc +-*");
        MD_Token tokens[100];
        
        MD_Token *token = tokens;
        MD_Token *token_opl = tokens + MD_ArrayCount(tokens);
        MD_u64 pos = 0;
        for (; pos < string.size && token < token_opl; )
        {
            *token = MD_TokenFromString(MD_S8Skip(string, pos));
            pos += token->raw_string.size;
            token += 1;
        }
        
        TestResult(TokenMatch(tokens[0], MD_S8Lit("abc"), MD_TokenKind_Identifier));
        TestResult(TokenMatch(tokens[1], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[2], MD_S8Lit("def"), MD_TokenKind_Identifier));
        TestResult(TokenMatch(tokens[3], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[4], MD_S8Lit("123"), MD_TokenKind_Numeric));
        TestResult(TokenMatch(tokens[5], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[6], MD_S8Lit("456"), MD_TokenKind_Numeric));
        TestResult(TokenMatch(tokens[7], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[8], MD_S8Lit("123_456"), MD_TokenKind_Numeric));
        TestResult(TokenMatch(tokens[9], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[10], MD_S8Lit("abc123"), MD_TokenKind_Identifier));
        TestResult(TokenMatch(tokens[11], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[12], MD_S8Lit("123abc"), MD_TokenKind_Numeric));
        TestResult(TokenMatch(tokens[13], MD_S8Lit(" "), MD_TokenKind_Whitespace));
        TestResult(TokenMatch(tokens[14], MD_S8Lit("+-*"), MD_TokenKind_Symbol));
    }
    
    Test("Empty Sets")
    {
        TestResult(MatchParsedWithNode(MD_S8Lit("{}"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("()"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("[]"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("[)"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""))));
        TestResult(MatchParsedWithNode(MD_S8Lit("(]"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""))));
    }
    
    Test("Simple Unnamed Sets")
    {
        {
            MD_String8 string = MD_S8Lit("{a, b, c}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("a")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("b")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("c")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        {
            MD_String8 string = MD_S8Lit("(1 2 3 4 5)");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("1")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("2")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("3")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("4")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("5")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        {
            MD_String8 string = MD_S8Lit("{a}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("a")));
            TestResult(MatchParsedWithNode(string, tree));
        }
    }
    
    Test("Simple Named Sets")
    {
        MD_String8 string = MD_S8Lit("simple_set: {a, b, c}");
        MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit("simple_set"));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("a")));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("b")));
        MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("c")));
        TestResult(MatchParsedWithNode(string, tree));
    }
    
    Test("Nested Sets")
    {
        {
            MD_String8 string = MD_S8Lit("{a b:{1 2 3} c}");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("a")));
            {
                MD_Node *sub = MakeTestNode(MD_NodeKind_Main, MD_S8Lit("b"));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("1")));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("2")));
                MD_PushChild(sub, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("3")));
                MD_PushChild(tree, sub);
            }
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("c")));
            TestResult(MatchParsedWithNode(string, tree));
        }
        
        {
            MD_String8 string = MD_S8Lit("foo: { (size: u64) -> *void }");
            MD_Node *tree = MakeTestNode(MD_NodeKind_Main, MD_S8Lit("foo"));
            MD_Node *params = MakeTestNode(MD_NodeKind_Main, MD_S8Lit(""));
            MD_Node *size = MakeTestNode(MD_NodeKind_Main, MD_S8Lit("size"));
            MD_PushChild(size, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("u64")));
            MD_PushChild(params, size);
            MD_PushChild(tree, params);
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("->")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("*")));
            MD_PushChild(tree, MakeTestNode(MD_NodeKind_Main, MD_S8Lit("void")));
            TestResult(MatchParsedWithNode(string, tree));
        }
    }
    
    Test("Non-Sets")
    {
        TestResult(MatchParsedWithNode(MD_S8Lit("foo"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit("foo"))));
        TestResult(MatchParsedWithNode(MD_S8Lit("123"), MakeTestNode(MD_NodeKind_Main, MD_S8Lit("123"))));
        TestResult(MatchParsedWithNode(MD_S8Lit("+"),   MakeTestNode(MD_NodeKind_Main, MD_S8Lit("+"))));
    }
    
    Test("Set Border Flags")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(0, 100)"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_HasParenLeft &&
                       parse.node->flags & MD_NodeFlag_HasParenRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(0, 100]"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_HasParenLeft &&
                       parse.node->flags & MD_NodeFlag_HasBracketRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("[0, 100)"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_HasBracketLeft &&
                       parse.node->flags & MD_NodeFlag_HasParenRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("[0, 100]"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_HasBracketLeft &&
                       parse.node->flags & MD_NodeFlag_HasBracketRight);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("{0, 100}"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_HasBraceLeft &&
                       parse.node->flags & MD_NodeFlag_HasBraceRight);
        }
    }
    
    Test("Node Separator Flags")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a, b)"), 0);
            TestResult(parse.node->first_child->flags & MD_NodeFlag_IsBeforeComma);
            TestResult(parse.node->first_child->next->flags & MD_NodeFlag_IsAfterComma);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a; b)"), 0);
            TestResult(parse.node->first_child->flags & MD_NodeFlag_IsBeforeSemicolon);
            TestResult(parse.node->first_child->next->flags & MD_NodeFlag_IsAfterSemicolon);
        }
    }
    
    Test("Node Text Flags")
    {
        TestResult(MD_ParseOneNode(arena, MD_S8Lit("123"), 0).node->flags &
                   MD_NodeFlag_Numeric);
        TestResult(MD_ParseOneNode(arena, MD_S8Lit("123_456_789"), 0).node->flags &
                   MD_NodeFlag_Numeric);
        TestResult(MD_ParseOneNode(arena, MD_S8Lit("abc"), 0).node->flags &
                   MD_NodeFlag_Identifier);
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("\"foo\""), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringDoubleQuote);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("'foo'"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringSingleQuote);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("`foo`"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringTick);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("\"\"\"foo\"\"\""), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringDoubleQuote &&
                       parse.node->flags & MD_NodeFlag_StringTriplet);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("'''foo'''"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringSingleQuote &&
                       parse.node->flags & MD_NodeFlag_StringTriplet);
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("```foo```"), 0);
            TestResult(parse.node->flags & MD_NodeFlag_StringLiteral &&
                       parse.node->flags & MD_NodeFlag_StringTick &&
                       parse.node->flags & MD_NodeFlag_StringTriplet);
        }
    }
    
    Test("Style Strings")
    {
        {
            MD_String8 str = MD_S8Stylize(arena, MD_S8Lit("THIS_IS_A_TEST"),
                                          MD_IdentifierStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_S8Match(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_S8Stylize(arena, MD_S8Lit("this_is_a_test"),
                                          MD_IdentifierStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_S8Match(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_S8Stylize(arena, MD_S8Lit("ThisIsATest"),
                                          MD_IdentifierStyle_UpperCamelCase, MD_S8Lit(" "));
            TestResult(MD_S8Match(str, MD_S8Lit("This Is A Test"), 0));
        }
        {
            MD_String8 str = MD_S8Stylize(arena, MD_S8Lit("Here is another test."),
                                          MD_IdentifierStyle_UpperCamelCase, MD_S8Lit(""));
            TestResult(MD_S8Match(str, MD_S8Lit("HereIsAnotherTest."), 0));
        }
    }
    
    Test("Enum Strings")
    {
        TestResult(MD_S8Match(MD_StringFromNodeKind(MD_NodeKind_Main), MD_S8Lit("Main"), 0));
        TestResult(MD_S8Match(MD_StringFromNodeKind(MD_NodeKind_Main), MD_S8Lit("Main"), 0));
        MD_String8List list = MD_StringListFromNodeFlags(arena, 
                                                         MD_NodeFlag_StringLiteral |
                                                         MD_NodeFlag_HasParenLeft |
                                                         MD_NodeFlag_IsBeforeSemicolon);
        MD_b32 match = 1;
        for(MD_String8Node *node = list.first; node; node = node->next)
        {
            if(!MD_S8Match(node->string, MD_S8Lit("StringLiteral"), 0)     &&
               !MD_S8Match(node->string, MD_S8Lit("HasParenLeft"), 0)       &&
               !MD_S8Match(node->string, MD_S8Lit("IsBeforeSemicolon"), 0))
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
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("/*foobar*/ (a b c)"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->prev_comment, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("// foobar\n(a b c)"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->prev_comment, MD_S8Lit(" foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("// foobar\n\n(a b c)"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->prev_comment, MD_S8Lit(""), 0));
            }
        }
        
        // NOTE(rjf): Post-Comments:
        {
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a b c) /*foobar*/"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->next_comment, MD_S8Lit("foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a b c) // foobar"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->next_comment, MD_S8Lit(" foobar"), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a b c)\n// foobar"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->next_comment, MD_S8Lit(""), 0));
            }
            {
                MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("(a b c)\n\n// foobar"), 0);
                TestResult(parse.node->kind == MD_NodeKind_Main &&
                           MD_S8Match(parse.node->next_comment, MD_S8Lit(""), 0));
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
            {"label: {1, 2, 3} /* /* unterminated comment */", {18}},
            {"{a,,#b,}", {4, 5}},
            {"foo""\x80""bar", {4}},
        };
        
        int max_error_count = MD_ArrayCount(tests[0].columns);
        
        for(int i_test = 0; i_test < MD_ArrayCount(tests); ++i_test)
        {
            MD_ParseResult parse = MD_ParseWholeString(arena, MD_S8Lit("test.mdesk"), MD_S8CString(tests[i_test].s));
            
            MD_b32 columns_match = 1;
            {
                MD_Message *e = parse.errors.first;
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
            MD_ParseResult parse = MD_ParseWholeFile(arena, MD_S8Lit("__does_not_exist.mdesk"));
            TestResult(parse.node->kind == MD_NodeKind_File && parse.errors.first != 0);
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
        for (MD_u64 i = 0; i < MD_ArrayCount(key_strings); i += 1)
        {
            keys[i] = MD_MapKeyStr(key_strings[i]);
        }
        for (MD_u64 i = MD_ArrayCount(key_strings); i < MD_ArrayCount(keys); i += 1)
        {
            keys[i] = MD_MapKeyPtr((void *)i);
        }
        
        {
            MD_Map map = MD_MapMake(arena);
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1)
            {
                MD_MapInsert(arena, &map, keys[i], (void *)i);
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1)
            {
                MD_MapSlot *slot = MD_MapLookup(&map, keys[i]);
                TestResult(slot && slot->val == (void *)i);
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1)
            {
                MD_MapOverwrite(arena, &map, keys[i], (void *)(i + 10));
            }
            for (MD_u64 i = 0; i < MD_ArrayCount(keys); i += 1)
            {
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
        for (int i = 0; i < MD_ArrayCount(samples); i += 1)
        {
            MD_ParseResult result = MD_ParseOneNode(arena, samples[i], 0);
            nodes[i] = result.node;
        }
        
        TestResult(MD_S8Match(nodes[0]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_S8Match(nodes[1]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_S8Match(nodes[2]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_S8Match(nodes[3]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_S8Match(nodes[4]->string, MD_S8Lit("foo-bar"), 0));
        TestResult(MD_S8Match(nodes[5]->string, MD_S8Lit("foo-bar"), 0));
        
        TestResult(MD_S8Match(nodes[0]->raw_string, samples[0], 0));
        TestResult(MD_S8Match(nodes[1]->raw_string, samples[1], 0));
        TestResult(MD_S8Match(nodes[2]->raw_string, samples[2], 0));
        TestResult(MD_S8Match(nodes[3]->raw_string, samples[3], 0));
        TestResult(MD_S8Match(nodes[4]->raw_string, samples[4], 0));
        TestResult(MD_S8Match(nodes[5]->raw_string, samples[5], 0));
    }
    
    Test("String escaping")
    {
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("`\\``"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("\\`"), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("``` \\``` ```"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit(" \\``` "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("`````\\````"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("``\\`"), 0));
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("`\\'`"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("\\'"), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("''' \\''' '''"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit(" \\''' "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("'''''\\''''"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("''\\'"), 0));
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("`\\\"`"), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("\\\""), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("\"\"\" \\\"\"\" \"\"\""), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit(" \\\"\"\" "), 0));
        }
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("\"\"\"\"\"\\\"\"\"\""), 0);
            TestResult(MD_S8Match(parse.node->string, MD_S8Lit("\"\"\\\""), 0));
        }
    }
    
    Test("Node-With-Flags Seeking")
    {
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("foo:{x y z; a b c}"), 0);
            MD_Node *node = parse.node;
            MD_Node *group_first = node->first_child;
            MD_Node *group_opl = MD_FirstNodeWithFlags(group_first->next, MD_NodeFlag_IsAfterSemicolon);
            
            TestResult(MD_S8Match(group_first->string,                    MD_S8Lit("x"), 0));
            TestResult(MD_S8Match(group_first->next->string,              MD_S8Lit("y"), 0));
            TestResult(MD_S8Match(group_first->next->next->string,        MD_S8Lit("z"), 0));
            TestResult(group_opl == group_first->next->next->next);
            
            group_first = group_opl;
            group_opl = MD_FirstNodeWithFlags(group_first->next, MD_NodeFlag_IsAfterSemicolon);
            
            TestResult(MD_S8Match(group_first->string,                    MD_S8Lit("a"), 0));
            TestResult(MD_S8Match(group_first->next->string,              MD_S8Lit("b"), 0));
            TestResult(MD_S8Match(group_first->next->next->string,        MD_S8Lit("c"), 0));
            TestResult(group_opl == group_first->next->next->next);
        }
        
        {
            MD_ParseResult parse = MD_ParseOneNode(arena, MD_S8Lit("foo:{a b c , d e f , g h i}"), 0);
            MD_Node *node = parse.node;
            MD_Node *group_first = 0;
            MD_Node *group_opl = 0;
            
            group_first = node->first_child;
            group_opl = MD_FirstNodeWithFlags(group_first->next, MD_NodeFlag_IsAfterComma);
            TestResult(MD_S8Match(group_first->string,                    MD_S8Lit("a"), 0));
            TestResult(MD_S8Match(group_first->next->string,              MD_S8Lit("b"), 0));
            TestResult(MD_S8Match(group_first->next->next->string,        MD_S8Lit("c"), 0));
            TestResult(group_opl == group_first->next->next->next);
            
            group_first = group_opl;
            group_opl = MD_FirstNodeWithFlags(group_first->next, MD_NodeFlag_IsAfterComma);
            TestResult(MD_S8Match(group_first->string,                    MD_S8Lit("d"), 0));
            TestResult(MD_S8Match(group_first->next->string,              MD_S8Lit("e"), 0));
            TestResult(MD_S8Match(group_first->next->next->string,        MD_S8Lit("f"), 0));
            TestResult(group_opl == group_first->next->next->next);
            
            group_first = group_opl;
            group_opl = MD_FirstNodeWithFlags(group_first->next, MD_NodeFlag_IsAfterComma);
            TestResult(MD_S8Match(group_first->string,                    MD_S8Lit("g"), 0));
            TestResult(MD_S8Match(group_first->next->string,              MD_S8Lit("h"), 0));
            TestResult(MD_S8Match(group_first->next->next->string,        MD_S8Lit("i"), 0));
            TestResult(group_opl == group_first->next->next->next);
        }
        
    }
    
    Test("Unscoped Subtleties")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        // finished unscoped set
        {
            MD_String8 text = MD_S8Lit("a:\nb:\nc");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first == 0);
            TestResult(result.node->first_child == result.node->last_child);
        }
        
        // unfinished unscoped set
        {
            MD_String8 text = MD_S8Lit("a:\nb:\n\n");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first != 0);
        }
        {
            MD_String8 text = MD_S8Lit("a:\nb:\n");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first != 0);
        }
        {
            MD_String8 text = MD_S8Lit("a:\nb:");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first != 0);
        }
        
        // labeled scoped set in unscoped set
        {
            MD_String8 text = MD_S8Lit("a: b: {\nx\n} c");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first == 0);
            TestResult(MD_ChildCountFromNode(result.node) == 1);
            TestResult(MD_ChildCountFromNode(result.node->first_child) == 2);
        }
        {
            MD_String8 text = MD_S8Lit("a: b: {\nx\n}\nc");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first == 0);
            TestResult(MD_ChildCountFromNode(result.node) == 2);
            TestResult(MD_ChildCountFromNode(result.node->first_child) == 1);
        }
        
        // scoped set is not unscoped
        {
            MD_String8 text = MD_S8Lit("a: {\nx\ny\n} c");
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, text);
            TestResult(result.errors.first == 0);
            TestResult(result.node->first_child != result.node->last_child);
        }
    }
    
    Test("Tags")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("@foo bar"));
            TestResult(MD_NodeHasTag(result.node->first_child, MD_S8Lit("foo"), 0));
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("@+ bar"));
            TestResult(MD_NodeHasTag(result.node->first_child, MD_S8Lit("+"), 0));
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("@'a b c' bar"));
            TestResult(MD_NodeHasTag(result.node->first_child, MD_S8Lit("a b c"), 0));
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("@100 bar"));
            TestResult(MD_NodeHasTag(result.node->first_child, MD_S8Lit("100"), 0));
        }
    }
    
    Test("Tagged & Unlabeled")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        // tagged in scoped set always legal
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("foo:{@tag {bar}}\n"));
            TestResult(result.errors.first == 0);
        }
        
        // tagged label in unscoped set legal
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("foo:@tag bar\n"));
            TestResult(result.errors.first == 0);
        }
        
        // unlabeled scoped set in unscoped set illegal
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("foo:bar {bar}\n"));
            TestResult(result.errors.first != 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("foo:bar @tag {bar}\n"));
            TestResult(result.errors.first != 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, MD_S8Lit("foo:@tag {bar}\n"));
            TestResult(result.errors.first != 0);
        }
    }
    
    Test("Integer Lexing")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        MD_String8 test_strings[] = {
            MD_S8Lit("0765"),
            MD_S8Lit("0xABC"),
            MD_S8Lit("0x123"),
            MD_S8Lit("0b010"),
        };
        
        MD_String8 *string = test_strings;
        for (int i = 0; i < MD_ArrayCount(test_strings); i += 1, string += 1)
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, *string);
            TestResult((result.errors.first == 0) &&
                       (result.node->first_child == result.node->last_child) &&
                       (result.node->first_child->flags & MD_NodeFlag_Numeric));
        }
    }
    
    Test("Float Lexing")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        MD_String8 test_strings[] = {
            MD_S8Lit("0"),
            MD_S8Lit("1"),
            MD_S8Lit("0.5"),
            MD_S8Lit("1e2"),
            MD_S8Lit("1e+2"),
            MD_S8Lit("1e-2"),
            MD_S8Lit("1.5e2"),
            MD_S8Lit("1.5e+2"),
            MD_S8Lit("1.5e-2"),
        };
        
        MD_String8 *string = test_strings;
        for (int i = 0; i < MD_ArrayCount(test_strings); i += 1, string += 1)
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name, *string);
            TestResult((result.errors.first == 0) &&
                       (result.node->first_child == result.node->last_child) &&
                       (result.node->first_child->flags & MD_NodeFlag_Numeric));
        }
    }
    
    Test("Labels are Not Reserved")
    {
        MD_String8 file_name = MD_S8Lit("raw_text");
        
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name,
                                                        MD_S8Lit("foo: '(' )"));
            TestResult(result.errors.first != 0);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name,
                                                        MD_S8Lit("foo ':' ( )"));
            TestResult(result.errors.first == 0);
            TestResult(MD_ChildCountFromNode(result.node) == 3);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name,
                                                        MD_S8Lit("'@'bar foo"));
            TestResult(result.errors.first == 0);
            TestResult(MD_ChildCountFromNode(result.node) == 3);
        }
        {
            MD_ParseResult result = MD_ParseWholeString(arena, file_name,
                                                        MD_S8Lit("foo: '(' ')'"));
            TestResult(result.errors.first == 0);
            TestResult(MD_ChildCountFromNode(result.node) == 1);
            TestResult(MD_ChildCountFromNode(result.node->first_child) == 2);
        }
    }
    
    Test("Debug Strings")
    {
        {
            MD_String8 code     = MD_S8Lit("@foo @bar @baz a: { b c d e f }");
            MD_String8 expected = MD_S8Lit("@foo\n@bar\n@baz\na:\n{\n b,\n c,\n d,\n e,\n f,\n}");
            MD_ParseResult parse = MD_ParseOneNode(arena, code, 0);
            MD_String8List actual_strings = {0};
            MD_DebugDumpFromNode(arena, &actual_strings, parse.node,
                                 0, MD_S8Lit(" "), MD_GenerateFlags_Tree);
            MD_String8 actual = MD_S8ListJoin(arena, actual_strings, 0);
            TestResult(MD_S8Match(expected, actual, 0));
        }
        {
            MD_String8 code     = MD_S8Lit("@foo(1, 2, 3) a: { x y }");
            MD_String8 expected = MD_S8Lit("@foo(1,\n     2,\n     3)\na:\n{\n x,\n y,\n}");
            MD_ParseResult parse = MD_ParseOneNode(arena, code, 0);
            MD_String8List actual_strings = {0};
            MD_DebugDumpFromNode(arena, &actual_strings, parse.node,
                                 0, MD_S8Lit(" "), MD_GenerateFlags_Tree);
            MD_String8 actual = MD_S8ListJoin(arena, actual_strings, 0);
            TestResult(MD_S8Match(expected, actual, 0));
        }
        {
            MD_String8 code     = MD_S8Lit("// foo\na");
            MD_String8 expected = MD_S8Lit("/*\n foo\n*/\na");
            MD_ParseResult parse = MD_ParseOneNode(arena, code, 0);
            MD_String8List actual_strings = {0};
            MD_DebugDumpFromNode(arena, &actual_strings, parse.node, 0, MD_S8Lit(" "),
                                 MD_GenerateFlags_Tree|MD_GenerateFlag_Comments);
            MD_String8 actual = MD_S8ListJoin(arena, actual_strings, 0);
            TestResult(MD_S8Match(expected, actual, 0));
        }
        {
            MD_String8 code = MD_S8Lit("@foo @bar @baz a: { b c d e f }");
            MD_ParseResult parse1 = MD_ParseOneNode(arena, code, 0);
            MD_String8List reconstruction_strs = {0};
            MD_ReconstructionFromNode(arena, &reconstruction_strs, parse1.node, 0, MD_S8Lit(" "));
            MD_String8 reconstruction = MD_S8ListJoin(arena, reconstruction_strs, 0);
            MD_ParseResult parse2 = MD_ParseOneNode(arena, reconstruction, 0);
            TestResult(MD_NodeDeepMatch(parse1.node, parse2.node, MD_NodeMatchFlag_TagArguments|MD_NodeMatchFlag_NodeFlags));
        }
        {
            MD_String8 code = MD_S8Lit("@foo(x: y: z) @bar(a: b: c) @baz(1: 2: 3) abcdefg: { b: 4, c d: 5; e; f, }");
            MD_ParseResult parse1 = MD_ParseOneNode(arena, code, 0);
            MD_String8List reconstruction_strs = {0};
            MD_ReconstructionFromNode(arena, &reconstruction_strs, parse1.node, 0, MD_S8Lit(" "));
            MD_String8 reconstruction = MD_S8ListJoin(arena, reconstruction_strs, 0);
            MD_ParseResult parse2 = MD_ParseOneNode(arena, reconstruction, 0);
            TestResult(MD_NodeDeepMatch(parse1.node, parse2.node, MD_NodeMatchFlag_TagArguments|MD_NodeMatchFlag_NodeFlags));
        }
        {
            MD_String8 code = MD_S8Lit("@foo(x: y: z)\n"
                                       "@bar(a: b: c)\n"
                                       "@baz(1: 2: 3)\n"
                                       "abcdefg:\n"
                                       "{\n"
                                       "  b: 4,\n"
                                       "  c\n"
                                       "  d: 5;\n"
                                       "  e;\n"
                                       "  f,\n"
                                       "}\n");
            MD_ParseResult parse1 = MD_ParseOneNode(arena, code, 0);
            MD_String8List reconstruction_strs = {0};
            MD_ReconstructionFromNode(arena, &reconstruction_strs, parse1.node, 0, MD_S8Lit(" "));
            MD_String8 reconstruction = MD_S8ListJoin(arena, reconstruction_strs, 0);
            MD_ParseResult parse2 = MD_ParseOneNode(arena, reconstruction, 0);
            TestResult(MD_NodeDeepMatch(parse1.node, parse2.node, MD_NodeMatchFlag_TagArguments|MD_NodeMatchFlag_NodeFlags));
        }
    }
    
    return 0;
}
