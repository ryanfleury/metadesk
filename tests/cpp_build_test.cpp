#include "md.h"
#include "md.c"

#include <stdio.h>

static MD_Arena *arena = 0;

int main(void)
{
    arena = MD_ArenaAlloc();
    
    printf("%d\n", MD_CPP_VERSION);
    
#if MD_CPP_VERSION != 0 && MD_CPP_VERSION != 98 // anything C++11 and up
    MD_String8 foo = "foo"_md;
    MD_String8 bar = "bar"_md;
    MD_String8 str = MD_S8Fmt(arena, "%S%S", foo, bar);
    printf("%.*s", MD_S8VArg(str));
#endif

    return 0;
}
