/* 
** Example: hello world
**
 ** 
**
*/

//~ Includes and globals //////////////////////////////////////////////////////

// @notes Metadesk is a source-include library. So we include the
//  file "md.c" into the usage code directly. The library 

#include "md.h"
#include "md.c"

// @notes For simple single-threaded memory management in a run-once-and-exit
//  utility, a single global arena is our prefered setup.
static MD_Arena *arena = 0;

//~ main //////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv){
    // setup the global arena
    // @notes Metadesk arenas do linear reserve-and-commit allocation. This
    //  code makes an arena with a 1 terabyte reserve which works so long as
    //  we're only doing one or a few arenas.
    arena = MD_ArenaAlloc(1ull << 40);
    
    // parse a string
    MD_String8 name = MD_S8Lit("<name>");
    MD_String8 hello_world = MD_S8Lit("hello world");
    MD_ParseResult parse = MD_ParseWholeString(arena, hello_world_name, hello_world);
    
    // print the results
    MD_PrintDebugDumpFromNode(stdout, parse.node, MD_GenerateFlags_All);
}