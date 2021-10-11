/* 
** Example: memory management
**
** This example shows how to use Metadesk in a program where memory management
** is important. For example if Metadesk is used as the basis for a config file
** system in an application, you may need to do things like reloading a config,
** or managing multiple configs with different arbitrary lifetimes.
**
** In the case of the simple metaprogram we manage memory by just having one
** global arena, and we never free anything. In this case we'll start using
** multiple arenas to create distinct lifetime "buckets" or "groups".
**
** Comments in this example explains a little about how Metadesk arenas work,
 ** and tips for using them effectively.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

// @notes In this example we're just using the default implementations of
//  the low level memory allocators. There's nothing wrong with doing it this
//  way but in a codebase where something like a config file matters there
//  may very well already be some custom allocators. Check out the overrides
//  example for details on how to plug in custom allocators. 
//
//  Everything shown here about the Metadesk arena remains true either way, 
//  because arenas are implemented on top of the low level memory allocator.
//  Basically you can think of the arena as performing allocation batching with
//  whatever low level memory allocator is available.

#include "md.h"
#include "md.c"


//~ pretend config file ///////////////////////////////////////////////////////

// @notes In this example we'll pretend we have a config file system, but we
//  only show the part up to finishing the Metadesk parse. Each ConfigFile will
//  carry a Metadesk arena, which handles the memory, and every version of the
//  file data at each stage of processing.
//
//  In a real system there would likely be at least one more stage of 
//  processing where a more processed version of the config that is made from 
//  analyzing the Metadesk tree.
//
//  Here we can release a ConfigFile by simply releasing the arena because we
//  have followed the simple rule that everything in the ConfigFile is
//  *allocated on the arena*. If we were doing the analysis phase, we could
//  keep this working by just writing the analyzer to allocate on the arena
//  too.
//
//  An alternative approach here that could save memory is to use the arena
//  as an allocator for temporary intermediates. In this approach during the
//  analysis stage the final data structure would be allocated outside the
//  arena used for parsing, and everything that the final structure needs would
//  be copied out. Then the parse could be thrown away. This approach saves
//  memory at the cost of making problems harder to trouble shoot, and
//  sometimes more time in analysis to copy things out of the temporary arena.

typedef struct ConfigFile{
    MD_Arena *arena;
    MD_String8 file_name;
    MD_String8 contents;
    MD_Node *root;
    MD_MessageList errors;
} ConfigFile;

ConfigFile*
new_config_file_from_file_name(char *file_name_cstr)
{
    MD_Arena *arena = MD_ArenaAlloc();
    
    // @notes MD_PushArray and MD_PushArrayZero are the fundamental allocation
    //  operations to do with a Metadesk arena. Metadesk APIs that take an
    //  MD_Arena* parameter are APIs that need to do allocation. When we pass
    //  an arena in one of these APIs the returned data is allocated using that
    //  arena and we say that the data is "allocated on the arena".
    ConfigFile *result = MD_PushArrayZero(arena, ConfigFile, 1);
    
    // @notes We explicitly copy the file name onto the arena so that we can
    //  be totally sure that it has the same lifetime as everything else in 
    //  the ConfigFile.
    MD_String8 file_name = MD_S8Copy(arena, MD_S8CString(file_name_cstr));
    
    // @notes Here we break down MD_ParseWholeFile into it's two stages
    //  explicitly so that we can save the contents and the parse in the
    //  ConfigFile.
    MD_String8 contents = MD_LoadEntireFile(arena, file_name);
    MD_ParseResult parse = MD_ParseWholeString(arena, file_name, contents);
    
    // @notes This part can be a little bit subtle. First we allocated the
    //  arena with MD_ArenaAlloc. Then we used the arena to allocate a 
    //  ConfigFile. Now we are storing a pointer to the arena in the config.
    //  The subtle part is that if you're used to thinking in terms of
    //  'ownership' it seems like the config owns the arena, but if we release
    //  the arena, we also release the config.
    //
    //  A different way to think about it is that the arena is a handle that
    //  manages allocation lifetimes. The ConfigFile and all of the data it
    //  holds share the same lifetime, so they are all allocated on the same
    //  "lifetime handle" (i.e. the same arena). The ConfigFile is the root of
    //  all that data so it also holds the handle for releasing later.
    result->arena = arena;
    result->file_name = file_name;
    result->contents = contents;
    result->root = parse.node;
    result->errors = parse.errors;
    return(result);
}

void
release_config_file(ConfigFile *file)
{
    MD_ArenaRelease(file->arena);
}


//~ just to simulate new config files coming from somewhere ///////////////////

int    in_files_count = 0;
char** in_file_names = 0;
int    in_file_iter = 0;

ConfigFile*
new_config_file(void)
{
    ConfigFile *result = new_config_file_from_file_name(in_file_names[in_file_iter]);
    in_file_iter = (in_file_iter + 1)%in_files_count;
    return(result);
}

//~ main //////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
    // make sure we have something to parse
    if (argc <= 1)
    {
        fprintf(stderr, "pass at least one input file");
        exit(1);
    }
    
    // setup the the source of files
    in_files_count = argc - 1;
    in_file_names = argv + 1;
    
    
    // @notes The idea here is to simulate a situation where an allocate and
    //  never free strategy would lead to growing memory usage over time
    //  (i.e. a memory leak).
    
    // pretend there are unpredictable lifetimes tied to real-time events
    {
        // first we get three config files
        ConfigFile *files[3];
        files[0] = new_config_file();
        files[1] = new_config_file();
        files[2] = new_config_file();
        
        // then we chaotically replace the slots for a while
        for (MD_u32 i = 10000; i < 20000; i += 1)
        {
            MD_u32 x = (i >> (i&3)) ^ (i << (16 + (i&3)));
            MD_u32 slot_index = x%3;
            
            release_config_file(files[slot_index]);
            files[slot_index] = new_config_file();
        }
        
        // then we're done with all the config files
        release_config_file(files[0]);
        release_config_file(files[1]);
        release_config_file(files[2]);
    }
}

// @notes A final note on "scratch arenas".
//
//  Often it is useful to use an arena for a temporary allocation that will be
//  thrown away by the end of the current scope. One could create a new arena
//  with MD_ArenaAlloc and later release it with MD_ArenaRelease, but this
//  sort of case is perfect for using the thread-local scratch pool.
//
//  To get an arena for scratch work from the pool one uses:
//   MD_ArenaTemp scratch = MD_GetScratch(0, 0);
//  To allocate with it:
//   MD_WhateverArenaApi(scratch.arena, ...);
//  And to release it:
//   MD_ReleaseScratch(scratch);
//
//  If an arena is being used for allocating something to return to the caller
//  it is important that it not also be the scratch, or else when the scratch
//  release happens, all of the memory that was supposed to stay allocated
//  for the caller to see will also be released (or worse, marked as released
//  but still valid looking for some time).
//
//  To avoid this when getting a scratch arena the API MD_GetScratch allows you
//  to specify arenas you are already using, to force it to pick one you are
//  not using. In 99.99% of cases a call to this API either looks like:
//   MD_GetScratch(0, 0);
//  Or
//   MD_GetScratch(&arena, 1);
//
//  If there are more than one arena already in use when a new scratch is 
//  needed they can all be specified by packing them into an array:
//   MD_Arena *arena_conflicts[2] = {arena1, arena2};
//   MD_GetScratch(arena_conflicts, 2);
//
//  Watch out! If the scratch pool doesn't have a non-conflicting arena then
//  it will return a null handle, likely leading to a crash. This can be 
//  avoided by defining a higher value for #define MD_IMPL_ScratchCount.
//  But it's generally possible and a lot better to avoid this path.

