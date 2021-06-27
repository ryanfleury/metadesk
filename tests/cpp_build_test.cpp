#include "md.h"
#include "md.c"

MD_String8 operator "" _md(const char *s, size_t size)
{
    MD_String8 str = MD_S8((MD_u8 *)s, (MD_u64)size);
    return str;
}

int main(void)
{
    MD_String8 str = "foobar"_md;
    printf("%.*s", MD_StringExpand(str));
    
    return 0;
}
