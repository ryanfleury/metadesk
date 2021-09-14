// Sample code to demonstrate errors being reported for certain nodes.

#define MD_ENABLE_PRINT_HELPERS 1

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

int main(int argument_count, char **arguments)
{
    arena = MD_ArenaAlloc(1ull << 40);
    
    // NOTE(rjf): Parse all the files passed in via command line.
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(arena, MD_S8CString(arguments[i])).node;
        MD_PushNewReference(arena, list, root);
    }
    
    // NOTE(rjf): Print errors on every single node.
    for(MD_EachNode(ref, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(ref);
        for(MD_EachNode(node, root->first_child))
        {
            MD_PrintNodeMessageFmt(stderr, node, MD_MessageKind_Error, "This node has an error!");
        }
    }
    
    return 0;
}
