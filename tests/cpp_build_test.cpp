#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

int main(void)
{
    MD_ThreadContext tctx;
    MD_ThreadInit(&tctx);
    
    arena = MD_ArenaNew(1ull << 40);
    
    printf("%d\n", MD_CPP_VERSION);
    
    MD_String8 foo = "foo"_md;
    MD_String8 bar = "bar"_md;
    MD_String8 str = MD_S8Fmt(arena, "%S%S", foo, bar);
    printf("%.*s", MD_S8VArg(str));
    return 0;
}
