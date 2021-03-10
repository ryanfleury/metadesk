#include "md.h"
#include "md.c"
#include <stdlib.h>     

#define DEBUG_RULES_AFTER_TRANSFORMATIONS 0
#define DEBUG_PRINT_GENERATED_TESTS 1

// NOTE: https://www.pcg-random.org/download.html
typedef struct RandomSeries{
    MD_u64 state; // NOTE: RNG state.  All values are possible.
    MD_u64 inc;   //       Controls which RNG sequence (stream) is selected. Odd at all times.
} RandomSeries;

static MD_u32 rand_U32(RandomSeries* rng)
{
    MD_u64 oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    MD_u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    MD_u32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static RandomSeries rand_seed(MD_u64 initstate, MD_u64 initseq)
{
    RandomSeries result = {0};

    result.state = 0U;
    result.inc = (initseq << 1u) | 1u;        // NOTE: inc must be odd
    rand_U32(&result);
    result.state += initstate;
    rand_U32(&result);

    return result;
}

MD_GLOBAL struct
{
    RandomSeries *random_series;
    MD_NodeTable *production_table;
} globals = {0};

static void TagSquareBracketSetsAsOptional(MD_Node *node, MD_Node *optional_tag)
{
    if(node->kind == MD_NodeKind_Label && 
       (node->flags & MD_NodeFlag_BracketLeft) && (node->flags & MD_NodeFlag_BracketRight))
    {
        if(node->string.size) // NOTE(mal): Production. Tag belongs to the first child
        {
            MD_Assert(!MD_NodeIsNil(node->first_child));
            MD_PushTag(node->first_child, optional_tag);
        }
        else
        {
            MD_PushTag(node, optional_tag);
        }
    }

    for(MD_EachNode(child, node->first_child))
    {
        TagSquareBracketSetsAsOptional(child, optional_tag);
    }
}

static MD_Node * NewChildLabel(MD_Node *parent, MD_String8 label)
{
    MD_Node *result = 0;

    result = MD_MakeNodeFromString(MD_NodeKind_Label, MD_S8Lit(""), 0, 0, label);
    if(parent)
    {
        MD_PushChild(parent, result);
    }
    return result;
}

static MD_Node * NewChild(MD_Node *parent)
{
    MD_Node *result = NewChildLabel(parent, MD_S8Lit(""));
    return result;
}

#define OPTIONAL_TAG "optional"

// TODO(mal): Use data table instead of smuggling depth as file_contents
#define GET_DEPTH(node) ((MD_u64)((node)->file_contents))
#define SET_DEPTH(node, v) (node)->file_contents = (MD_u8 *)(v);
static void PrintRule(MD_Node *rule)
{
    MD_b32 is_literal_char = rule->flags & MD_NodeFlag_CharLiteral;

    MD_b32 optional = MD_NodeHasTag(rule, MD_S8Lit(OPTIONAL_TAG));

    if(optional)
    {
        printf("[");
    }

    for(MD_EachNode(tag, rule->first_tag))
    {
        if(!MD_StringMatch(tag->string, MD_S8Lit(OPTIONAL_TAG), 0))
        {
            printf("@%.*s ", MD_StringExpand(tag->string));
        }
    }

    if(is_literal_char)
    {
        printf("'");
    }

    MD_b32 has_children = !MD_NodeIsNil(rule->first_child);
    if(has_children)
    {
        MD_Assert(rule->string.size == 0);

        for(MD_EachNode(rule_element, rule->first_child))
        {
            PrintRule(rule_element);
            printf(" (%lu) ", (unsigned long)GET_DEPTH(rule_element));

            if(!MD_NodeIsNil(rule_element->next))
            {
                printf(" ");
            }
        }
    }
    else
    {
        MD_Assert(rule->string.size > 0);
        printf("%.*s", MD_StringExpand(rule->string));
    }

    if(is_literal_char)
    {
        printf("'");
    }

    if(optional)
    {
        printf("]");
    }
}

typedef enum OperationFlags
{
    OperationFlag_Fill          = 1<<0,
    OperationFlag_Markup        = 1<<1,
    OperationFlag_Tag           = 1<<2,
} OperationFlags;

static void Extend(MD_String8 *s, char c)
{
    *s = MD_PushStringF("%.*s%c", MD_StringExpand(*s), c);
}

static void ExpandProduction(MD_Node *production, MD_String8List *out, MD_Node *cur_node, 
                             OperationFlags op_flags, MD_u32 max_depth, MD_u32 depth);

static void ExpandRule(MD_Node *rule, MD_String8List *out_strings, MD_Node *cur_node, OperationFlags op_flags,
                       MD_u32 max_depth, MD_u32 depth)
{
    for(MD_EachNode(rule_element, rule->first_child))
    {
        MD_b32 expand = 1;
        if(MD_NodeHasTag(rule_element, MD_S8Lit(OPTIONAL_TAG)))
        {
            expand = rand_U32(globals.random_series)%2;

            if(expand)
            {
                if(GET_DEPTH(rule_element) + depth >= max_depth)
                {
                    expand = 0;
                }
            }
        }

        if(expand)
        {
            MD_Node *node_to_tag = 0;
            OperationFlags old_op_flags = op_flags;
            for(MD_EachNode(tag_node, rule_element->first_tag)){
                if(MD_StringMatch(tag_node->string, MD_S8Lit("child"), 0))
                {
                    cur_node = NewChild(cur_node);
                    op_flags &= ~OperationFlag_Tag; // NOTE(mal): Tag parameters are not tags
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("sibling"), 0))
                {
                    cur_node = NewChild(cur_node->parent);
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("fill"), 0))
                {
                    op_flags |= OperationFlag_Fill;
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("tag"), 0))
                {
                    op_flags |= OperationFlag_Tag;
                    node_to_tag = cur_node;
                    cur_node = NewChild(0);
                    cur_node->kind = MD_NodeKind_Tag;
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("markup"), 0))
                {
                    op_flags |= OperationFlag_Markup;
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit(OPTIONAL_TAG), 0))
                {
                }
                else
                {
                    MD_Assert(!"Not implemented");
                }
            }

            MD_b32 has_children = !MD_NodeIsNil(rule_element->first_child);
            if(has_children)
            {
                ExpandRule(rule_element, out_strings, cur_node, op_flags, max_depth, depth+1);
            }
            else
            {
                if(rule_element->flags & MD_NodeFlag_CharLiteral)   // NOTE(mal): Terminal production
                {
                    char c = 0;
                    if(rule_element->string.size == 2 && rule_element->string.str[0] == '\\')
                    {
                        switch(rule_element->string.str[1]){
                          case '\\': c = '\\'; break;
                          case '\'': c = '\''; break;
                          case '"':  c = '\"'; break;
                          case 'n':  c = '\n'; break;
                        }
                    }
                    else
                    {
                        c = rule_element->string.str[0];
                    }

                    MD_String8 character = MD_PushStringF("%c", c);
                    MD_PushStringToList(out_strings, character);

                    if(op_flags & OperationFlag_Fill)
                    {
                        Extend(&cur_node->whole_string, c);
                        if(!(op_flags & OperationFlag_Markup))
                        {
                            Extend(&cur_node->string, c);
                        }
                    }
                }
                else        // NOTE(mal): Non-terminal production
                {
                    MD_Node * production = MD_NodeTable_Lookup(globals.production_table, rule_element->string)->node;
                    MD_Assert(production);
                    ExpandProduction(production, out_strings, cur_node, op_flags, max_depth, depth+1);
                }
            }

            if(node_to_tag)
            {
                MD_PushTag(node_to_tag, cur_node);
                cur_node = node_to_tag;
            }

            op_flags = old_op_flags;
        }
    }
}

static void ExpandProduction(MD_Node *production, MD_String8List *out, MD_Node *cur_node, 
                             OperationFlags op_flags, MD_u32 max_depth, MD_u32 depth)
{
    MD_i64 rule_count = MD_ChildCountFromNode(production);

    MD_Assert(GET_DEPTH(production)+depth <= max_depth);

    MD_Node *rule = 0;
    do{
        int rule_number = rand_U32(globals.random_series)%rule_count;
        rule = MD_ChildFromIndex(production, rule_number);
    }while(GET_DEPTH(rule)+depth > max_depth);

    ExpandRule(rule, out, cur_node, op_flags, max_depth, depth);
}

static MD_Node * FindNonTerminalProduction(MD_Node *node, MD_NodeTable *visited)
{
    MD_Node *result = 0;

    MD_b32 should_visit = 1;
    if(!MD_NodeIsNil(node->first_child) && node->string.size)
    {
        if(MD_NodeTable_Lookup(visited, node->string))
        {
            should_visit = 0;
        }
        else
        {
            MD_b32 inserted = MD_NodeTable_Insert(visited, MD_NodeTableCollisionRule_Overwrite, node->string, node);
            MD_Assert(inserted);
        }
    }

    if(should_visit)
    {
        if(MD_NodeIsNil(node->first_child))
        {
            if(node->flags & MD_NodeFlag_CharLiteral)
            {
            }
            else
            {
                MD_NodeTableSlot *slot = MD_NodeTable_Lookup(globals.production_table, node->string);
                if(slot)
                {
                    MD_Node *production = slot->node;
                    result = FindNonTerminalProduction(production, visited);
                }
                else
                {
                    result = node;
                }
            }
        }
        else
        {
            for(MD_EachNode(child, node->first_child))
            {
                result = FindNonTerminalProduction(child, visited);
                if(result)
                {
                    break;
                }
            }
        }
    }

    return result;
}

static MD_b32 EqualTrees(MD_Node *a, MD_Node *b);
static MD_b32 EqualList(MD_Node *a, MD_Node *b)
{
    MD_b32 result = 1;
    while(!MD_NodeIsNil(a) || !MD_NodeIsNil(b))
    {
        if(!EqualTrees(a, b))
        {
            result = 0;
            break;
        }
        a = a->next;
        b = b->next;
    }
    return result;
}
static MD_b32 EqualTrees(MD_Node *a, MD_Node *b)
{
    MD_b32 result = (a->kind == b->kind && 
                     MD_StringMatch(a->string, b->string, 0) && 
                     MD_StringMatch(a->whole_string, b->whole_string, 0));
    result &= EqualList(a->first_tag, b->first_tag);
    result &= EqualList(a->first_child, b->first_child);
    result &= MD_StringMatch(a->comment_before, b->comment_before, 0);
    return result;
}

static MD_String8 EscapeNewlines(MD_String8 s)
{
    MD_String8 result = s;

    MD_u32 newline_count = 0;
    for(MD_u8 *c = s.str; c < s.str + s.size; ++c)
    {
        newline_count += (*c == '\n');
    }

    if(newline_count)
    {
        result.size = s.size + newline_count;
        result.str = calloc(result.size+1, sizeof(MD_u8));
        for(MD_u8 *dest = result.str, *orig = s.str; orig < s.str + s.size; ++orig, ++dest)
        {
            if(*orig == '\n')
            {
                *dest++ = '\\';
                *dest = 'n';
            }
            else
            {
                *dest = *orig;
            }
        }
    }

    return result;
}

typedef struct Test Test;
struct Test
{
    MD_String8 input;
    MD_Node *expected_output;
    Test *next;
};

// TODO(mal): Use data table instead of smuggling depth as file_contents
#define GET_DEPTH(node) ((MD_u64)((node)->file_contents))
#define SET_DEPTH(node, v) (node)->file_contents = (MD_u8 *)(v);
static void ComputeElementDepth(MD_Node *re)
{
    MD_u64 result = 0;
    MD_b32 has_children = !MD_NodeIsNil(re->first_child);
    if(has_children)
    {
        for(MD_EachNode(sub_re, re->first_child))
        {
            ComputeElementDepth(sub_re);
            MD_u64 depth = GET_DEPTH(sub_re);
            if(result < depth)
            {
                result = depth;
            }
        }
    }
    else
    {
        if(re->flags & MD_NodeFlag_CharLiteral)   // NOTE(mal): Terminal production
        {
            result = 1;
        }
        else
        {
            MD_Node * production = MD_NodeTable_Lookup(globals.production_table, re->string)->node;
            result = GET_DEPTH(production)+1;
        }
    }

    SET_DEPTH(re, result);
}

// NOTE(mal): Compares first by size then alphabetically
static int StringCompare(MD_String8 a, MD_String8 b)
{
    int result = 0;
    if(a.size < b.size)
    {
        result = -1;
    }
    else if(a.size > b.size)
    {
        result = 1;
    }
    else
    {
        for(MD_u64 i = 0; i < a.size; ++i)
        {
            if(a.str[i] < b.str[i])
            {
                result = -1;
                break;
            }
            else if(a.str[i] > b.str[i])
            {
                result = 1;
                break;
            }
        }
    }
    return result;
}

int main(int argument_count, char **arguments)
{
    MD_Node *grammar = MD_ParseWholeFile(MD_S8Lit("tests/grammar.md")).node;

    // NOTE(mal): In order to get a BNF-like syntax, I feed the MD output through two transformations:
    //            1) Tag []-style sets as optional 
    MD_Node *optional_tag = 0;
    {
        MD_ParseResult parse_result = MD_ParseOneNode(MD_S8Lit(""), MD_S8Lit("@"OPTIONAL_TAG" is_a_tag"));
        optional_tag = parse_result.node->first_tag;
    }
    TagSquareBracketSetsAsOptional(grammar, optional_tag);

    // NOTE(mal): 2) Divide rules around '|' operators and transplant them into a new tree structure
    //               productions > production > rules > rule > rule_element
    MD_Node *productions = NewChild(0);
    for(MD_EachNode(symbol, grammar->first_child))
    {
        MD_Node *production = NewChildLabel(productions, symbol->string);
        MD_Node *rule = NewChild(production);
        for(MD_Node *rule_element = symbol->first_child; !MD_NodeIsNil(rule_element); )
        {
            MD_Node *next = rule_element->next;

            MD_b32 has_children = !MD_NodeIsNil(rule_element->first_child);
            if(has_children)
            {
                MD_PushChild(rule, rule_element);
            }
            else
            {
                if(MD_StringMatch(rule_element->string, MD_S8Lit("|"), 0) && 
                   !(rule_element->flags & MD_NodeFlag_CharLiteral))
                {
                    rule = NewChild(production);
                }
                else
                {
                    MD_PushChild(rule, rule_element);
                }
            }

            rule_element = next;
        }
    }

    // NOTE(mal): Build production hash table
    MD_NodeTable production_table_ = {0};
    globals.production_table = &production_table_;
    for(MD_EachNode(production, productions->first_child))
    {
        MD_b32 inserted = MD_NodeTable_Insert(globals.production_table, MD_NodeTableCollisionRule_Overwrite, 
                                              production->string, production);
        MD_Assert(inserted);
    }

    // NOTE(mal): Check for root production
    MD_Node* file_production = 0;
    {
        MD_NodeTableSlot *file_production_slot = MD_NodeTable_Lookup(globals.production_table, MD_S8Lit("file"));
        if(!file_production_slot)
        {
            fprintf(stderr, "Error: Grammar file does not specify \"file\" production\n");
            goto error;
        }
        file_production = file_production_slot->node;
    }

    // NOTE(mal): Check that all branches lead to terminal nodes
    MD_NodeTable visited_productions = {0};

    for(MD_EachNode(production, productions->first_child))
    {
        MD_Node *non_terminal_production = FindNonTerminalProduction(production, &visited_productions);
        if(non_terminal_production)
        {
            fprintf(stderr, "Error: Non-terminal production \"%.*s\"\n", MD_StringExpand(non_terminal_production->string));
            goto error;
        }
    }

    // NOTE(mal): Check that all productions are reachable
    for(MD_EachNode(production, productions->first_child))
    {
        MD_NodeTableSlot *slot = MD_NodeTable_Lookup(&visited_productions, production->string);
        if(!slot)
        {
            fprintf(stderr, "Warning: Unreachable production \"%.*s\"\n", MD_StringExpand(production->string));
        }
    }

    // NOTE(mal): Compute depth of productions, rules, rule elements
    MD_b32 progress = 1;
    while(progress)
    {
        progress = 0;
        for(MD_EachNode(production, productions->first_child))
        {
            MD_u64 min_rule_depth = 0xffffffffffffffff;
            for(MD_EachNode(rule, production->first_child))
            {
                MD_u64 max_rule_element_depth = 0;
                MD_u64 max_mandatory_rule_element_depth = 0;
                for(MD_EachNode(rule_element, rule->first_child))
                {
                    ComputeElementDepth(rule_element);
                    MD_u64 depth = GET_DEPTH(rule_element);

                    if(!MD_NodeHasTag(rule_element, MD_S8Lit(OPTIONAL_TAG)))
                    {
                        MD_u64 depth = 0;
                        MD_Assert(MD_NodeIsNil(rule_element->first_child));
                        if(!(rule_element->flags & MD_NodeFlag_CharLiteral))
                        {
                            MD_Node * production = MD_NodeTable_Lookup(globals.production_table, rule_element->string)->node;
                            depth = GET_DEPTH(production);
                        }
                        depth += 1;

                        if(depth > max_mandatory_rule_element_depth)
                        {
                            max_mandatory_rule_element_depth = depth;
                        }
                    }

                    if(depth > max_rule_element_depth)
                    {
                        max_rule_element_depth = depth;
                    }
                }

                if(max_mandatory_rule_element_depth > GET_DEPTH(rule))
                {
                    progress = 1;
                    SET_DEPTH(rule, max_mandatory_rule_element_depth);
                }

                if(max_mandatory_rule_element_depth < min_rule_depth)
                {
                    min_rule_depth = max_mandatory_rule_element_depth;
                }
            }

            if(min_rule_depth > GET_DEPTH(production))
            {
                progress = 1;
                SET_DEPTH(production, min_rule_depth);
            }
        }
    }

#if DEBUG_RULES_AFTER_TRANSFORMATIONS
    for(MD_EachNode(production, productions->first_child))
    {
        printf("%.*s (min %lu): \n", MD_StringExpand(production->string), GET_DEPTH(production));
        for(MD_EachNode(rule, production->first_child))
        {
            printf("    ");
            PrintRule(rule);
            printf(" (max %lu)", GET_DEPTH(rule));
            printf("\n");
        }
    }
#endif

    RandomSeries random_series = rand_seed(0, 0);  // NOTE(mal): Reproduceable
    globals.random_series = &random_series;
    MD_Node* file_production_node = MD_NodeTable_Lookup(globals.production_table, MD_S8Lit("file"))->node;

    // NOTE(mal): Generate test_count unique tests, sorted by complexity
    MD_u32 test_count = 1000;
    MD_u32 max_production_depth = 30;
    Test *first_test = calloc(1, sizeof(Test));
    first_test->expected_output = NewChild(0);
    first_test->expected_output->kind = MD_NodeKind_File;
    for(MD_u32 i_test = 1; i_test < test_count;)
    {
        Test test = {0};
        test.expected_output = NewChild(0);
        test.expected_output->kind = MD_NodeKind_File;

        MD_String8List string_list = {0};
        // NOTE(mal): Generate a random MD file
        ExpandProduction(file_production_node, &string_list, test.expected_output, 0, max_production_depth, 0);
        test.input = MD_JoinStringList(string_list);

        Test *prev = 0;
        for(Test *cur = first_test; cur; cur = cur->next)
        {
            int comp = StringCompare(test.input, cur->input);
            if(comp > 0)
            {
                prev = cur;
            }
            else if(comp < 0)
            {
                break;
            }
            else
            {
                prev = 0;           // NOTE(mal): Repeat node
            }
        }

        if(prev)
        {
            Test *stored_test = calloc(1, sizeof(Test));
            *stored_test = test;
            stored_test->next = prev->next;
            prev->next = stored_test;
            ++i_test;
        }
    }

    MD_u32 i_test = 0;
    for(Test *test = first_test; test; test = test->next)
    {
        MD_Node *file_node = MD_ParseWholeString(MD_S8Lit(""), test->input).node;
        file_node->string = file_node->whole_string = (MD_String8){0};

#if DEBUG_PRINT_GENERATED_TESTS
        printf("> %.*s <\n", MD_StringExpand(EscapeNewlines(test->input)));
#endif
        if(!EqualTrees(file_node, test->expected_output))
        {
            printf("\nFailed test %d\n", i_test);
            printf("> %.*s <\n", MD_StringExpand(test->input));
            printf("MD:\n");
            MD_OutputTree(stdout, file_node);
            printf("Grammar:\n");
            MD_OutputTree(stdout, test->expected_output); printf("\n");
            return -1;
        }
        ++i_test;
    }

    printf("Passed %d tests\n", test_count);
    
    error:
    
    return 0;
}
