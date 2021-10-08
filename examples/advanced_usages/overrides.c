/*
** Example: overrides
**
** TODO full commentary
**
*/


//~ example allocator /////////////////////////////////////////////////////////

typedef struct ExampleAllocatorNode{
    struct ExampleAllocatorNode *next;
    struct ExampleAllocatorNode *prev;
    int size_after_node;
} ExampleAllocatorNode;

typedef struct ExampleAllocator{
    ExampleAllocatorNode *first;
    ExampleAllocatorNode *last;
} ExampleAllocator;

void* examp_alloc(ExampleAllocator *a, int size);
void  examp_free(ExampleAllocator *a, void *ptr);


//~ include metadesk header ///////////////////////////////////////////////////

#include "md.h"


//~ set metadesk overrides ////////////////////////////////////////////////////

// override memory to use malloc/free

ExampleAllocator* md_example_allocator = 0;

void* md_reserve_by_example_allocator(unsigned long long size);
void  md_release_by_example_allocator(void *ptr, unsigned long long ignore);

#define MD_IMPL_Reserve       md_reserve_by_example_allocator
#define MD_IMPL_Commit(p,z)   (1)
#define MD_IMPL_Decommit(p,z) ((void)0)
#define MD_IMPL_Release(p,z)  md_release_by_example_allocator


// override file loading

MD_String8 md_load_entire_file_by_stdio(MD_Arena *arena, MD_String8 filename);

#define MD_IMPL_LoadEntireFile md_load_entire_file_by_stdio


//~ metadesk source, global arena /////////////////////////////////////////////

#include "md.c"
static MD_Arena *arena = 0;


//~ implement overrides ///////////////////////////////////////////////////////

// override memory to use malloc/free

#include <stdlib.h>
#include <assert.h>

void*
md_reserve_by_example_allocator(unsigned long long size)
{
    assert(md_example_allocator != 0);
    void *result = examp_alloc(md_example_allocator, (int)size);
    return(result);
}

void
md_release_by_example_allocator(void *ptr, unsigned long long ignore)
{
    assert(md_example_allocator != 0);
    examp_free(md_example_allocator, ptr);
}

// override file loading

MD_String8
md_load_entire_file_by_stdio(MD_Arena *arena, MD_String8 filename)
{
    MD_String8 result = {0};
    MD_ArenaTemp scratch = MD_GetScratch(&arena, 1);
    MD_String8 filename_copy = MD_S8Copy(scratch.arena, filename);
    char *filename_cstr = (char*)filename_copy.str;
    FILE *file = fopen(filename_cstr, "rb");
    if (file != 0)
    {
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        fread(result.str, result.size, 1, file);
        fclose(file);
    }
    MD_ReleaseScratch(scratch);
    return(result);
}


//~ main //////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // initialize the example allocator
    ExampleAllocator allocator = {0};
    
    // configure the allocator context
    md_example_allocator = &allocator;
    
    
    // setup the global arena
    arena = MD_ArenaAlloc();
    // ... any normal metadesk usage may go here ...
    
    
    return 0;
}


//~ implement the example allocator ///////////////////////////////////////////

void*
examp_alloc(ExampleAllocator *a, int size)
{
    ExampleAllocatorNode *node = (ExampleAllocatorNode*)malloc(size + sizeof(*node));
    node->size_after_node = size;
    if (a->first == 0)
    {
        a->first = a->last = node;
        node->next = node->prev = 0;
    }
    else
    {
        node->prev = a->last;
        node->next = 0;
        a->last->next = node;
        a->last = node;
    }
    
    void *result = (node + 1);
    return(result);
}

void
examp_free(ExampleAllocator *a, void *ptr)
{
    ExampleAllocatorNode *node = ((ExampleAllocatorNode*)ptr) - 1;
    if (node->next != 0)
    {
        node->next->prev = node->prev;
    }
    if (node->prev != 0)
    {
        node->prev->next = node->next;
    }
    if (a->first == node)
    {
        a->first = node->next;
    }
    if (a->last == node)
    {
        a->last = node->prev;
    }
    free(node);
}

