// Sample code to demonstrate errors being reported for certain nodes.

#include "md.h"
#include "md.c"

int main(int argument_count, char **arguments)
{
    // NOTE(rjf): Parse all the files passed in via command line.
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i]));
        MD_PushSibling(&first, &last, MD_NilNode(), root);
    }
    
    // NOTE(rjf): Put errors on every single node.
    for(MD_EachNode(root, first))
    {
        for(MD_EachNode(node, root->first_child))
        {
            MD_NodeErrorF(node, "This node has an error!");
        }
    }
    
    return 0;
}
