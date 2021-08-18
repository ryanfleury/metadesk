#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

int main(int argument_count, char **arguments)
{
    arena = MD_ArenaAlloc(1ull << 40);
    
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(arena, MD_S8CString(arguments[i])).node;
        MD_PushNewReference(arena, list, root);
    }
    
    for(MD_EachNodeRef(root, list->first_child))
    {
        for(MD_EachNode(node, root->first_child))
        {
            MD_String8List strs = MD_DebugStringListFromNode(arena, node, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree);
            MD_String8 str = MD_S8ListJoin(arena, &strs, 0);
            fprintf(stdout, "%.*s\n", MD_S8VArg(str));
        }
    }
    
    return 0;
}
