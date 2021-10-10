/* 
** Example: memory management
**
** TODO
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

//~ pretend config file ///////////////////////////////////////////////////////

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
    ConfigFile *result = MD_PushArrayZero(arena, ConfigFile, 1);
    
    MD_String8 file_name = MD_S8Copy(arena, MD_S8CString(file_name_cstr));
    MD_String8 contents = MD_LoadEntireFile(arena, file_name);
    MD_ParseResult parse = MD_ParseWholeString(arena, file_name, contents);
    
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
    if (file != 0)
    {
        MD_ArenaRelease(file->arena);
    }
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
