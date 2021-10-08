/*
** Example: user errors
**
** This example shows how to print custom error messages.
**
** Pass user_errors.mdesk to see the custom messages in action.
**
** We will be adding two custom rules that check for certain properties of the
** top level nodes in a metadesk file:
**  1. No top level node should have multiple tags
**  2. Top level sets with brackets '[]' are expected to be unnamed
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
        
        // @notes Here we are printing out all the messages that came back from
        //  the parser. We could try to wait and print all the errors at once,
        //  or perform the custom checks in this block to put all the error
        //  output in a single place. But all that work deferring tasks or
        //  analyzing can quickly make things complicated. Out preferred method
        //  is to process the Metadesk file in stages. This is the parsing
        //  stage so this is where the parsing errors are reported. Custom
        //  errors get added in later stages whenever the analyzer finds them.
        
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
        
        // @notes The custom rules in this example apply to the top level nodes
        //  so we can check for them by visiting each node from the root.
        //  The example "type metadata" includes more advanced custom errors
        //  that are only discovered after more analysis.
        for(MD_EachNode(node, root->first_child))
        {
            
            // top level node should have one or zero tags.
            // @notes This helper gives us a way to treat the tags on `node`
            //  as an array and access it by index. If there's a tag at index 1
            //  then we know there are at least two tags, which is against
            //  the first custom rule.
            MD_Node *tag_2 = MD_TagFromIndex(node, 1);
            if (!MD_NodeIsNil(tag_2))
            {
                // @notes The MD_PrintMessage helper formats a message for us
                //  with a filename and line number, and prints it to a FILE.
                //  We provide it with the coordinates with a MD_CodeLoc, which
                //  we can get from any metadesk node in the parse tree with
                //  the helper MD_CodeLoc. In this case we will get an error 
                //  message pointed right at the offending tag.
                //
                //  Messages don't have to be printed with MD_PrintMessage.
                //  In cases where the errors have a final destination other
                //  than a FILE, the message can be saved as a string with
                //  MD_FormatMessage.
                MD_CodeLoc loc = MD_CodeLocFromNode(tag_2); 
                MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                                MD_S8Lit("Not supposed to have multiple tags."));
            }
            
            // top level sets with brackets should not have names
            // @notes The second custom rule applies to sets with "[]"
            //  delimiters. We can easily detect nodes for those sets by
            //  checking the node flags:
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
