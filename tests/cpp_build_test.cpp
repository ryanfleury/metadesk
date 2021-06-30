#include "md.h"
#include "md.c"

int main(void)
{
    MD_String8 str = "foobar"_md;
    printf("%.*s", MD_StringExpand(str));
    
    return 0;
}
