// LICENSE AT END OF FILE (MIT).

#include <dirent.h>
#include <sys/stat.h>

#define MD_IMPL_FileIterIncrement MD_POSIX_FileIterIncrement
typedef struct MD_POSIX_FileIter MD_POSIX_FileIter;
struct MD_POSIX_FileIter
{
    DIR *dir;
};
MD_StaticAssert(sizeof(MD_POSIX_FileIter) <= sizeof(MD_FileIter), file_iter_size_check);

static MD_b32
MD_POSIX_FileIterIncrement(MD_FileIter *opaque_it, MD_String8 path, MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    MD_POSIX_FileIter *it = (MD_POSIX_FileIter *)opaque_it;
    if(it->dir == 0)
    {
        it->dir = opendir((char*)path.str);
    }
    
    if(it->dir)
    {
        struct dirent *dir_entry = readdir(it->dir);
        if(dir_entry)
        {
            out_info->filename = MD_S8Fmt("%s", dir_entry->d_name);
            out_info->flags = 0;
            
            if(path.size > 1 && path.str[path.size-1] == '/')
            {
                path.size -= 1;
            }
            
            struct stat st;
            MD_String8 cfile_path = MD_S8Fmt("%.*s/%s", MD_S8VArg(path), dir_entry->d_name);
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
            closedir(it->dir);
            it->dir = 0;
        }
    }
    
    return result;
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
