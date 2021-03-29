#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "generated/converter.c"

int main(void)
{
    v1_Entry v1_entry = {0};

    v1_entry.to_remove = 12;

    v1_entry.kind = v1_EntryKind_C;

    v1_entry.flags = v1_EntryFlag_B | v1_EntryFlag_C;

    v1_entry.color.components.r = 255;
    v1_entry.color.components.g = 128;
    v1_entry.color.components.b = 1;
    v1_entry.color.components.a = 42;

    v1_entry.p.x = 2;
    v1_entry.p.y = 3;

    Entry entry = EntryFromV1(v1_entry);

    assert(entry.kind == EntryKind_C);
    assert(entry.flags == (EntryFlag_B | EntryFlag_C));
    assert(entry.color.components.r == 255);
    assert(entry.color.components.g == 128);
    assert(entry.color.components.b == 1);
    assert(entry.color.components.a == 42);
    assert(v1_entry.p.x == 2);
    assert(v1_entry.p.y == 3);

    printf("Conversion test success!\n");

    return 0;
}
