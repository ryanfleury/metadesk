// LICENSE AT END OF FILE (MIT).

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 #include <sys/syscall.h>

// NOTE(mal): To get these constants I need to #define _GNU_SOURCE, which invites non-POSIX behavior I'd rather avoid
#ifndef O_PATH
#define O_PATH                 010000000
#endif
#define AT_NO_AUTOMOUNT        0x800
#define AT_SYMLINK_NOFOLLOW    0x100

#define MD_IMPL_FileIterIncrement MD_LINUX_FileIterIncrement
typedef struct MD_LINUX_FileIter MD_LINUX_FileIter;
struct MD_LINUX_FileIter
{
    int dir_fd;
    DIR *dir;
};
MD_StaticAssert(sizeof(MD_LINUX_FileIter) <= sizeof(MD_FileIter), file_iter_size_check);

static MD_b32
MD_LINUX_FileIterIncrement(MD_FileIter *opaque_it, MD_String8 path, MD_FileInfo *out_info)
{
    MD_b32 result = 0;

    MD_LINUX_FileIter *it = (MD_LINUX_FileIter *)opaque_it;
    if(it->dir == 0)
    {
        it->dir = opendir((char*)path.str);
        it->dir_fd = open((char *)path.str, O_PATH|O_CLOEXEC);
    }

    if(it->dir != 0 && it->dir_fd != -1)
    {
        struct dirent *dir_entry = readdir(it->dir);
        if(dir_entry)
        {
            out_info->filename = MD_PushStringF("%s", dir_entry->d_name);
            out_info->flags = 0;

            struct stat st; 
            if(fstatat(it->dir_fd, dir_entry->d_name, &st, AT_NO_AUTOMOUNT|AT_SYMLINK_NOFOLLOW) == 0)
            {
                if((st.st_mode & S_IFMT) == S_IFDIR)
                {
                    out_info->flags |= MD_FileFlag_Directory;
                }
                out_info->file_size = st.st_size;
            }
            result = 1;
        }
    }

    if(result == 0)
    {
        if(it->dir != 0)
        {
            closedir(it->dir);
            it->dir = 0;
        }
        if(it->dir_fd != -1)
        {
            close(it->dir_fd);
            it->dir_fd = -1;
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
