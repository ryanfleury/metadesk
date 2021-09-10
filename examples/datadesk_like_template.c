/* 
** Example: datadesk-like-template
**
 ** This example is setup as a copy-pastable template for creating metadesk
** based metaprograms that have the same structure as datadesk.
**
** Datadesk was a precursor language to metadesk. This example is mostly meant
** to help datadesk users understand metadesk and migrate onto it.
** 
** A "datadesk-like" metaprogram is passed the input metacode files on the
** command line. These files are parsed. Then a set of three user-defined
 ** functions that form the "custom layer" are called. The "custom layer"
 ** defines all the additional analysis and code generation.
**
*/

//~ Includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;


//~ Declare user defined functions (the datadesk "custom layer") //////////////

// Called when the program starts.
static void Initialize(void);
// Called for every top-level node that is parsed from passed files.
static void TopLevel(MD_Node *node);
// Runs before the program ends.
static void CleanUp(void);


//~ main //////////////////////////////////////////////////////////////////////

int main(int argument_count, char **arguments)
{
    // simple single-threaded one-run memory management setup
    arena = MD_ArenaAlloc(1ull << 40);
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(arena, MD_S8CString(arguments[i])).node;
        MD_PushNewReference(arena, list, root);
    }
    
    // calls to the "custom layer"
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


//~ The "custom layer" definitions ////////////////////////////////////////////


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

