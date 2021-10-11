/* 
** Example: data desk like template
**
 ** This example is setup as a copy-pastable template for creating metadesk
** based metaprograms that have the same structure as data desk metaprograms.
**
** Data Desk was a precursor language to metadesk. This example is mostly meant
** to help Data Desk users understand metadesk and migrate onto it.
** 
** A "data-desk-like" metaprogram is passed the input metacode files on the
** command line. These files are parsed. Then a set of three user-defined
 ** functions that form the "custom layer" are called. The "custom layer"
 ** defines all the additional analysis and code generation.
**
*/

//~ Includes and globals //////////////////////////////////////////////////////

#define MD_ENABLE_PRINT_HELPERS 1

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;


//~ Declare user defined functions (the data desk "custom layer") //////////////

static void Initialize(void);        // Runs at the beginning of generation.
static void TopLevel(MD_Node *node); // Runs once for each top-level node from each file.
static void CleanUp(void);           // Runs at the end of generation.


//~ main //////////////////////////////////////////////////////////////////////

int main(int argument_count, char **arguments)
{
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_b32 failed_parse = 0;
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argument_count; i += 1)
    {
        
        // parse the file
        MD_String8 file_name = MD_S8CString(arguments[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        // print metadesk errors
        for (MD_Message *message = parse_result.errors.first;
             message != 0;
             message = message->next)
        {
            MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(stdout, code_loc, message->kind, message->string);
        }
        
        // save to parse results list
        MD_PushNewReference(arena, list, parse_result.node);
        
        // mark failure state
        if (parse_result.errors.max_message_kind >= MD_MessageKind_Error)
        {
            failed_parse = 1;
        }
    }
    
    // call the "custom layer"
    if (!failed_parse)
    {
        Initialize();
        for(MD_EachNode(ref, list->first_child))
        {
            MD_Node *root = MD_ResolveNodeFromReference(ref);
            for(MD_EachNode(node, root->first_child))
            {
                TopLevel(node);
            }
        }
        CleanUp();
    }
    
    return 0;
}


//~ The "custom layer" ////////////////////////////////////////////////////////

static void
Initialize(void)
{
    /*TODO*/
}

static void
TopLevel(MD_Node *node)
{
    /*TODO*/
}

static void
CleanUp(void)
{
    /*TODO*/
}

