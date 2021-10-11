/* 
** Example: hello world
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

//~ main //////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // parse a string
    MD_String8 name = MD_S8Lit("<name>");
    MD_String8 hello_world = MD_S8Lit("hello world");
    MD_ParseResult parse = MD_ParseWholeString(arena, name, hello_world);
    
    // print the results
    MD_PrintDebugDumpFromNode(stdout, parse.node, MD_GenerateFlags_All);
}
