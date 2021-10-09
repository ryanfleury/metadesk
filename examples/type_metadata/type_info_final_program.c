/*
** Example: type metadata
**
** This file shows including the generated type information into a final
** program and using that type info to unpack a buffer of data.
**
*/

//~ setup base types //////////////////////////////////////////////////////////
#include <stdint.h>
typedef uint32_t U32;
typedef float F32;
typedef struct V2F32 V2F32;
struct V2F32
{
    F32 x;
    F32 y;
};

//~ include types for type info ///////////////////////////////////////////////
#include "type_info.h"

//~ include generated type info ///////////////////////////////////////////////
#include "generated/meta_types.h"
#include "generated/meta_types.c"

//~ main //////////////////////////////////////////////////////////////////////

#include <stdio.h>

int
main(int argc, char **argv)
{
    // TODO(allen): write example final program
    
    return(0);
}
