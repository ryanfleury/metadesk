// Sample code to demonstrate errors being reported for certain nodes.

#include "md.h"
#include "md.c"

int main(int argument_count, char **arguments)
{
    // NOTE(rjf): Parse all the files passed in via command line.
    MD_Node *list = MD_MakeList();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i])).node;
        MD_PushReference(list, root);
    }
    
    // NOTE(rjf): Print errors on every single node.
    for(MD_EachNode(ref, list->first_child))
    {
        MD_Node *root = MD_Deref(ref);
        for(MD_EachNode(node, root->first_child))
        {
            MD_NodeMessageF(stderr, node, MD_MessageKind_Error, "This node has an error!");
        }
    }
    
    return 0;
}
