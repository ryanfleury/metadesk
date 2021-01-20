//~ Metadesk Library

// TODO(allen): Test including this after <Windows.h>

// NOTE(allen): Include the OS specific functions
#if MD_OS_WINDOWS
# include "md_win32.c"
#elif MD_OS_LINUX
# include "md_posix.c"
#else
# error No default implementation for this OS
#endif

#include "md_malloc.c"

#include "md_impl.c"
