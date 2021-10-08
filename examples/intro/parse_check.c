/* 
** Example: parse check
**
 ** This example shows how to use the metadesk library to parse metadesk files,
** print errors, and dump verbose feedback on the resulting metadesk trees.
** This is also a nice utility for checking and inspecting your metadesk files.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

// @notes For simple single-threaded memory management in a run-once-and-exit
//  utility, a single global arena is our recommended approach.
static MD_Arena *arena = 0;


//~ main //////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for (int i = 1; i < argc; i += 1)
    {
        
        // parse the file
        // @notes Here we rely on MD_ParseWholeFile which loads the file itself
        //  and then does the whole parse. In a simple utility program like
        //  this metadesk's default implementations for the overrides make
        //  this work.
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        // print metadesk errors
        for (MD_Message *message = parse_result.errors.first;
             message != 0;
             message = message->next)
        {
            // @notes To print a message from the parse, technically we can do
            //  whatever we want. But we'll use the message format suggested
            //  by the metadesk library. First we get the code location - which
            //  means the file name and line number - for the node on this
            //  message. Then we pass the details of this message to the
            //  MD_PrintMessage helper function.
            MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(stdout, code_loc, message->kind, message->string);
        }
        
        // save to parse results list
        // @notes Metadesk message kinds are sorted in order of severity. So we
        //  can easily check the severity of a entire parse by looking at the
        //  `max_message_kind` field on the errors from the parse result. Here
        //  only push the parse result onto our list if there were no errors.
        if (parse_result.errors.max_message_kind < MD_MessageKind_Error)
        {
            MD_PushNewReference(arena, list, parse_result.node);
        }
    }
    
    // print the verbose parse results
    // @notes The Metadesk library provides a macros for iterating chains of
    //  MD_Node as shown here. The first parameter to the macro names an
    //  MD_Node pointer in the for loop that iterates through each node in the
    //  list. The second parameter is a pointer to the first node of the list.
    //  Generally we past the `first_child` of a list or parent MD_Node, but we
    //  don't always have to.
    for (MD_EachNode(root_it, list->first_child))
    {
        
        // @notes The `list` we have been building does not contain a normal
        //  MD_Node chain like in the trees returned from the parser. Instead
        //  it contains a chain of 'reference' nodes, which can point to any
        //  metadesk node from a previous parse. So our `root_it` is iterating
        //  a list of reference nodes. Here we resolve the reference node to
        //  get the root of the parse tree.
        MD_Node *root = MD_ResolveNodeFromReference(root_it);
        
        for (MD_EachNode(node, root->first_child))
        {
            // @notes The Metadesk library likes to use MD_String8List for
            //  functions that build and return big strings. This simplifies
            //  memory management for big strings, and means a series of string
            //  builders can be called back to back and gathered into one list.
            //  When the string needs to be finalized into a single contiguous
            //  block a user can just call `MD_S8ListJoin` as shown here.
            MD_String8List stream = {0};
            MD_DebugDumpFromNode(arena, &stream, node, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree);
            MD_String8 str = MD_S8ListJoin(arena, stream, 0);
            fwrite(str.str, str.size, 1, stdout);
            fwrite("\n", 1, 1, stdout);
        }
    }
    
    return 0;
}
