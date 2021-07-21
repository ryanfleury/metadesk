// Sample code for old Data Desk style custom layer, adapted to Metadesk.

#include "md.h"
#include "md.c"

static void
Initialize(void)
{
    // Called when the program starts.
}

static void
TopLevel(MD_Node *node)
{
    // Called for every top-level node that is parsed from passed files.
}

static void
CleanUp(void)
{
    // Runs before the program ends.
}

int main(int argument_count, char **arguments)
{
    MD_Arena *arena = MD_ArenaNew(1ull << 40);
    
    // NOTE(rjf): Parse all the files passed in via command line.
    MD_Node *list = MD_MakeList();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(arena, MD_S8CString(arguments[i])).node;
        MD_PushNewReference(list, root);
    }
    
    // NOTE(rjf): Call "custom layer" back.
    Initialize();
    for(MD_EachNode(ref, list->first_child))
    {
        MD_Node *root = MD_NodeFromReference(ref);
        for(MD_EachNode(node, root->first_child))
        {
            TopLevel(node);
        }
    }
    CleanUp();
    
    return 0;
}