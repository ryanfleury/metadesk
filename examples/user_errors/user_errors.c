/*
** Example: user errors
**
** This example shows how to print custom error messages.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;


//~ main //////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argc; i += 1)
    {
        // parse the file
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        // print metadesk errors
        for (MD_Message *message = parse_result.errors.first;
             message != 0;
             message = message->next)
        {
            MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(stderr, code_loc, message->kind, message->string);
        }
        
        // save to parse results list
        MD_PushNewReference(arena, list, parse_result.node);
    }
    
    // check for custom errors
    for(MD_EachNode(ref, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(ref);
        for(MD_EachNode(node, root->first_child))
        {
            // top level node should have one or zero tags.
            MD_Node *tag_2 = MD_TagFromIndex(node, 1);
            if (!MD_NodeIsNil(tag_2))
            {
                MD_CodeLoc loc = MD_CodeLocFromNode(tag_2); 
                MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                                MD_S8Lit("Not supposed to have multiple tags."));
            }
            
            // top level sets with brackets should not have names
            if ((node->flags & MD_NodeFlag_HasBracketLeft) ||
                (node->flags & MD_NodeFlag_HasBracketRight))
            {
                if (node->string.size > 0)
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(node); 
                    MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                                    MD_S8Lit("Nodes with brackets should not have names."));
                }
            }
            
        }
    }
    
    return 0;
}
