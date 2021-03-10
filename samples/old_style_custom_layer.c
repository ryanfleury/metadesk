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
    // NOTE(rjf): Parse all the files passed in via command line.
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i])).node;
        MD_PushSibling(&first, &last, MD_NilNode(), root);
    }
    
    // NOTE(rjf): Call "custom layer" back.
    Initialize();
    for(MD_EachNode(root, first))
    {
        for(MD_EachNode(node, root->first_child))
        {
            TopLevel(node);
        }
    }
    CleanUp();
    
    return 0;
}