#include "md.h"
#include "md.c"

int main(int argument_count, char **arguments)
{
    MD_Node *list = MD_MakeList();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i])).node;
        MD_PushReference(list, root);
    }
    
    for(MD_EachNodeRef(root, list->first_child))
    {
        for(MD_EachNode(node, root->first_child))
        {
            MD_OutputTree(stdout, node, 0);
        }
    }
    
    return 0;
}