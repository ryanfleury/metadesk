#include "md.h"
#include "md.c"

int main(void)
{
    MD_String8 foo = "foo"_md;
    MD_String8 bar = "bar"_md;
    MD_String8 str = MD_PushStringF("%S%S", foo, bar);
    printf("%.*s", MD_StringExpand(str));
    return 0;
}
