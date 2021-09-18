#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

void run_test_on_string(MD_String8 string)
{
    MD_String16 s16 = MD_S16FromS8(arena, string);
    MD_String8  s8_ts16 = MD_S8FromS16(arena, s16);
    MD_Assert(MD_S8Match(s8_ts16, string, 0));
    
    MD_String32 s32 = MD_S32FromS8(arena, string);
    MD_String8  s8_ts32 = MD_S8FromS32(arena, s32);
    MD_Assert(MD_S8Match(s8_ts32, string, 0));
}

int main(void)
{
    arena = MD_ArenaAlloc();
    
    // TODO(allen): throw more at this.
    
    char test_string_c[] = "Foo bar; test the unicode\n\t\0Etc";
    MD_String8 test_string = MD_S8(test_string_c, sizeof(test_string_c) - 1);
    
    run_test_on_string(test_string);
    
    return 0;
}

