// LICENSE AT END OF FILE (MIT).

////////////////////////////////
// TODO(allen): Write commentary for all of this.

#define MAX_PATH 260
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef int BOOL;
typedef void *HANDLE;
typedef char CHAR;
typedef const CHAR *LPCSTR;

typedef struct _FILETIME _FILETIME;
struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};
typedef _FILETIME FILETIME;
typedef _FILETIME *PFILETIME;
typedef _FILETIME *LPFILETIME;

typedef struct _WIN32_FIND_DATAA _WIN32_FIND_DATAA;
struct _WIN32_FIND_DATAA
{
#define FILE_ATTRIBUTE_DIRECTORY 0x10
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    CHAR     cFileName[MAX_PATH];
    CHAR     cAlternateFileName[14];
    DWORD    dwFileType;
    DWORD    dwCreatorType;
    WORD     wFinderFlags;
};

typedef _WIN32_FIND_DATAA WIN32_FIND_DATAA;
typedef _WIN32_FIND_DATAA *PWIN32_FIND_DATAA;
typedef _WIN32_FIND_DATAA *LPWIN32_FIND_DATAA;

MD_C_LINKAGE_BEGIN

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

MD_C_LINKAGE_END


#pragma comment(lib, "User32.lib")

#define MD_IMPL_FileIterIncrement MD_WIN32_FileIterIncrement

static MD_b32
MD_WIN32_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    WIN32_FIND_DATAA find_data = MD_ZERO_STRUCT;
    HANDLE state = *(HANDLE *)(&it->state[0]);
    if(state == 0)
    {
        MD_b32 need_star = 0;
        if(path.str[path.size-1] == '/' ||
           path.str[path.size-1] == '\\')
        {
            need_star = 1;
        }
        MD_String8 cpath = need_star ? MD_PushStringF("%.*s*", MD_StringExpand(path)) : path;
        state = FindFirstFileA((char*)cpath.str, &find_data);
        result = !!state;
    }
    else
    {
        result = !!FindNextFileA(state, &find_data);
    }
    
    it->state[0] = *(MD_u64 *)(&state);
    if(result)
    {
        out_info->flags = 0;
        if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            out_info->flags |= MD_FileFlag_Directory;
        }
        out_info->filename = MD_PushStringF("%s", find_data.cFileName);
        out_info->file_size = ((((MD_u64)find_data.nFileSizeHigh) << 32) |
                               ((MD_u64)find_data.nFileSizeLow));
    }
    
    return result;
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/