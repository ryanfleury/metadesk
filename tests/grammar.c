#include "md.h"
#include "md.c"
#include <stdlib.h>     

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

typedef enum AllowedOperationFlags AllowedOperationFlags;
enum AllowedOperationFlags
{
    AllowedOperationFlag_Fill   = 1<<0,
    AllowedOperationFlag_Tag    = 1<<1,
};

static void ExpandProduction(MD_Node *production, MD_String8List *out, MD_Node *cur_node, AllowedOperationFlags allowed);

static void Extend(MD_String8 *s, char c)
{
    *s = MD_PushStringF("%.*s%c", MD_StringExpand(*s), c);
}

static void ExpandRule(MD_Node *rule, MD_String8List *out_strings, MD_Node *cur_node, AllowedOperationFlags allowed)
{
    for(MD_EachNode(rule_element, rule->first_child))
    {
        MD_b32 expand = 1;
        if(MD_NodeHasTag(rule_element, MD_S8Lit(OPTIONAL_TAG)))
        {
            expand = rand_U32(globals.random_series)%2;
        }

        if(expand)
        {
            MD_Node *node_to_tag = 0;
            MD_b32 is_markup = 0;
            AllowedOperationFlags new_flags = 0;
            for(MD_EachNode(tag_node, rule_element->first_tag)){
                if(MD_StringMatch(tag_node->string, MD_S8Lit("child"), 0))
                {
                    cur_node = NewChild(cur_node);
                    allowed &= ~AllowedOperationFlag_Tag; // NOTE(mal): Tag parameters are not tags
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("sibling"), 0))
                {
                    cur_node = NewChild(cur_node->parent);
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("fill"), 0))
                {
                    new_flags |= AllowedOperationFlag_Fill;
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("tag"), 0))
                {
                    new_flags |= AllowedOperationFlag_Tag;
                    node_to_tag = cur_node;
                    cur_node = NewChild(0);
                    cur_node->kind = MD_NodeKind_Tag;
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit(OPTIONAL_TAG), 0))
                {
                }
                else if(MD_StringMatch(tag_node->string, MD_S8Lit("markup"), 0))
                {
                    is_markup = 1;
                }
                else
                {
                    MD_Assert(!"Not implemented");
                }
            }

            allowed |= new_flags;

            MD_b32 has_children = !MD_NodeIsNil(rule_element->first_child);
            if(has_children)
            {
                ExpandRule(rule_element, out_strings, cur_node, allowed);
            }
            else
            {
                if(rule_element->flags & MD_NodeFlag_CharLiteral)   // NOTE(mal): Terminal production
                {
                    char c = 0;
                    if(rule_element->string.size == 2 && rule_element->string.str[0] == '\\')
                    {
                        switch(rule_element->string.str[1]){
                          case '\\': c = '\\';  break;
                          case '\'': c = '\'';  break;
                          case '"':  c = '\"';  break;
                        }
                    }
                    else
                    {
                        c = rule_element->string.str[0];
                    }

                    MD_String8 character = MD_PushStringF("%c", c);
                    MD_PushStringToList(out_strings, character);

                    if(allowed & (AllowedOperationFlag_Fill|AllowedOperationFlag_Tag))
                    {
                        Extend(&cur_node->whole_string, c);
                        if(!is_markup)
                        {
                            Extend(&cur_node->string, c);
                        }
                    }
                }
                else        // NOTE(mal): Non-terminal production
                {

                    MD_Node * production = MD_NodeTable_Lookup(globals.production_table, rule_element->string)->node;
                    MD_Assert(production);
                    ExpandProduction(production, out_strings, cur_node, allowed);
                }
            }

            if(node_to_tag)
            {
                MD_PushTag(node_to_tag, cur_node);
                cur_node = node_to_tag;
            }

            allowed &= ~new_flags;
        }
    }
}

static void ExpandProduction(MD_Node *production, MD_String8List *out, MD_Node *cur_node, AllowedOperationFlags allowed)
{
    MD_i64 rule_count = MD_ChildCountFromNode(production);
    int rule_number = rand_U32(globals.random_series)%rule_count;
    MD_Node *rule = MD_ChildFromIndex(production, rule_number);
    ExpandRule(rule, out, cur_node, allowed);
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

    while(!MD_NodeIsNil(a) && !MD_NodeIsNil(b))
    {
        if(!EqualTrees(a, b))
        {
            result = 0;
        }
        a = a->next;
        b = b->next;
    }

    return result;
}
static MD_b32 EqualTrees(MD_Node *a, MD_Node *b)
{
    MD_b32 result = (a->kind == b->kind && MD_StringMatch(a->string, b->string, 0) && 
                     MD_StringMatch(a->string, b->string, 0));
    result &= EqualList(a->first_tag, b->first_tag);
    result &= EqualList(a->first_child, b->first_child);
    return result;
}

int main(int argument_count, char **arguments)
{
    MD_Node *grammar = MD_ParseWholeFile(MD_S8Lit("tests/grammar.md"));

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

    // NOTE(mal): Debug rules after transformations
#if 0
    for(MD_EachNode(production, productions->first_child))
    {
        printf("%.*s: \n", MD_StringExpand(production->string));

        for(MD_EachNode(rule, production->first_child))
        {
            printf("    ");
            PrintRule(rule);
            printf("\n");
        }
    }
#endif

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
    MD_Node *non_terminal_production = FindNonTerminalProduction(file_production, &visited_productions);
    if(non_terminal_production)
    {
        fprintf(stderr, "Error: Non-terminal production \"%.*s\"\n", MD_StringExpand(non_terminal_production->string));
        goto error;
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

    RandomSeries random_series = rand_seed(0, 0);  // NOTE(mal): Reproduceable
    globals.random_series = &random_series;

    MD_Node* node = MD_NodeTable_Lookup(globals.production_table, MD_S8Lit("file"))->node;
    MD_u32 test_count = 1000;
    for(int i = 0; i < test_count; ++i)
    {
        MD_String8List expanded_list = {0};

        //if(i == 25) BP;

        // NOTE(mal): Generate a random MD file
        MD_Node *file_control_node = NewChild(0);
        file_control_node->kind = MD_NodeKind_File;
        ExpandProduction(node, &expanded_list, file_control_node, 0);
        MD_String8 expanded = MD_JoinStringList(expanded_list);

        MD_Node *file_node = MD_ParseWholeString(MD_S8Lit(""), expanded);
        file_node->string = file_node->whole_string = (MD_String8){0};

        printf("Test %d: ", i);
        printf("> %.*s <\n", MD_StringExpand(expanded));
        if(!EqualTrees(file_node, file_control_node))
        {
            printf("\nFailed test %d\n", i);
            printf("MD:\n");
            MD_OutputTree(stdout, file_node);
            printf("Grammar:\n");
            MD_OutputTree(stdout, file_control_node); printf("\n");
            return -1;
        }
    }

    printf("Passed %d tests\n", test_count);
    
    error:
    
    return 0;
}
