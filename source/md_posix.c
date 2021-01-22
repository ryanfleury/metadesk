// LICENSE AT END OF FILE (MIT).

#include <dirent.h>
#include <sys/stat.h>

#define MD_IMPL_FileIterIncrement MD_POSIX_FileIterIncrement

static MD_b32
MD_POSIX_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    DIR *dir = (DIR *)(it->state);
    if(dir == 0)
    {
        dir = opendir((char*)path.str);
    }
    
    struct dirent *dir_entry = 0;
    if(dir && (dir_entry = readdir(dir)))
    {
        out_info->filename = MD_PushStringF("%s", dir_entry->d_name);
        out_info->flags = 0;
        
        if(path.size > 1 && path.str[path.size-1] == '/')
        {
            path.size -= 1;
        }
        MD_String8 cfile_path = MD_PushStringF("%.*s/%s", MD_StringExpand(path), dir_entry->d_name);
        
        // NOTE(mal): On Linux, fstatat(2) would save us from doing path manipulation and avoid some race
        //            conditions but we would need to make space for an extra directory file descriptor
        //            inside MD_FileIter struct. We would also stop being POSIX-compliant.
        struct stat st;
        if(stat((char *)cfile_path.str, &st) == 0)
        {
            if((st.st_mode & S_IFMT) == S_IFDIR)
            {
                out_info->flags |= MD_FileFlag_Directory;
            }
            out_info->file_size = st.st_size;
        }
        result = 1;
    }
    else
    {
        closedir(dir);
        dir = 0;
        result = 0;
    }
    
    it->state = *(MD_u64 *)(&dir);
    
    return result;
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
