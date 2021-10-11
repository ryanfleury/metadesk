/*
** Example: overrides
**
** This example shows using the overrides system in Metadesk to plug in a
** custom memory allocator and file loading routine. There are more options
** in the overrides than are presented here. A full list of the overrides
** options is kept in md.c 'Overrides & Options Macros'
**
** A few of the reasons one might want to use the Metadesk overrides are:
**  1. Plugging in a custom allocator to control the memory allocations
**  2. Plugging in a custom arena implementation for more seamless 
**     interoperation between the codebase and library
**  3. Provide implementation for unsupported OSes without having to modify
 **     md.h or md.c
**  4. Remove dependency on CRT
**  5. Remove dependency on OS headers
**
*/

//~ example allocator /////////////////////////////////////////////////////////

// @notes This isn't really "the example" but we need something to play the
//  role of a custom allocator, imagine this is any alloc & free style
//  allocator you might already have in a codebase.

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

// @notes Disabling print helpers removes any APIs from the library that depend
//  on FILE from stdio.h.
#define MD_DISABLE_PRINT_HELPERS 1

// @notes Here is also a good place to disable the default implementations of 
//  anything that is overriden to avoid extra includes.
#define MD_DEFAULT_MEMORY 0
#define MD_DEFAULT_FILE_LOAD 0

// @notes We can also disable default implementations for "optional" parts,
//  here we disable the default file iterator without replacing it, which gets
//  this example off of direct OS header dependencies.
#define MD_DEFAULT_FILE_ITER 0

// @notes We include the metadesk header before we define the overrides because
//  some overrides require that metadesk base types be visible. There are
//  exceptions to this pattern, in particular overrides for types need to be
//  defined before including md.h, we aren't going that far here.
#include "md.h"


//~ set metadesk overrides ////////////////////////////////////////////////////

// override memory to use malloc/free

// @notes A common practice in setting up allocator overrides is to use a pass
//  through opaque user context pointer. Metadesk does something different.
//  We recommend passing the context pointer with a global in single threaded
//  cases, and with a pointer in thread local storage in multi-threaded cases.
ExampleAllocator* md_example_allocator = 0;

// @notes In this example the allocator only provides alloc & free, but the
//  Metadesk override group we want to plug into has reserve commit, decommit &
//  release. This is okay though, we can turn commit & decommit into no-ops and
//  reserve & release as equivalent to alloc & free.

void* md_reserve_by_example_allocator(unsigned long long size);
void  md_release_by_example_allocator(void *ptr, unsigned long long ignore);

#define MD_IMPL_Reserve       md_reserve_by_example_allocator
#define MD_IMPL_Commit(p,z)   (1)
#define MD_IMPL_Decommit(p,z) ((void)0)
#define MD_IMPL_Release       md_release_by_example_allocator

// @notes Since we are turning commit & decommit into no-ops it doesn't make
//  sense for the Metadesk arena to have a reserve size larger than it's
//  commit size anymore. The default for reserve size is 64 megabytes, which
//  is usually too large of an alloc block size, and the default commit size
//  is 64 kilabytes, which is usually too small. So we set both to 1 megabyte.
//
// Pro-Tip: (N << 20) is a nice shorthand for N megabytes, and
//          (N << 10) is N kilabytes.

#define MD_DEFAULT_ARENA_RES_SIZE (1 << 20)
#define MD_DEFAULT_ARENA_CMT_SIZE (1 << 20)


// override file loading

// @notes We'll also demonstrate another override, this time one that relies
//  on Metadesk-provided types. The actual override here is pointless, as it's
//  just another implementation of "LoadEntireFile" on stdio.h, which is what
//  the default provided by the library is as well.

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

#include <stdio.h>

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
    // ... where ever program init stuff is happening ...
    
    // initialize the example allocator
    ExampleAllocator allocator = {0};
    
    // metadesk allocator context gets setup before a call to MD_ArenaAlloc
    md_example_allocator = &allocator;
    
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    
    // ... any normal metadesk usage may now happen ...
    
    
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

//~ final notes ///////////////////////////////////////////////////////////////

// @notes The Metadesk override system uses macro overriding. This means it
//  does not provide a mechanism for dynamically dispatching to different
//  implementations of the overridable operations. But you can always plug in
//  your own dynamic dispatch into the macro if you really need it. We
//  recommend against trying to instantiate the library twice in the same
//  program with different overrides, because that will lead to separate
//  instances of our thread local context variables which will make Metadesk
//  more resource intensive than it needs to be, and may lead to surprising
//  behavior.

