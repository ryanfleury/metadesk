#include "md.h"
#include "md.c"

int main(void)
{
    MD_String8 foo = "foo"_md;
    MD_String8 bar = "bar"_md;
    MD_String8 str = MD_S8Fmt("%S%S", foo, bar);
    printf("%.*s", MD_S8VArg(str));
    return 0;
}
