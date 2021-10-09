/*
** Example: type metadata
**
** This file shows including the generated types and metadata in the a final
** program and using it to unpack a buffer of data.
**
*/

//~ setup base types //////////////////////////////////////////////////////////
#include <stdint.h>
typedef uint8_t U8;
typedef uint16_t U16;
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

U8 raw_buffer[] = {
    0x03,0x00,0x00,0x00,0x00,0x00,0x90,0x40,0x06,0x00,0x00,0x00,
    0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,
    0x2D,0xB2,0x5D,0x3F,0x00,0x00,0x00,0xBF,0x2D,0xB2,0x5D,0x3F,
    0x00,0x00,0x80,0xBF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBF,
    0x2D,0xB2,0x5D,0xBF,0x00,0x00,0x00,0x3F,0x2D,0xB2,0x5D,0xBF,
    0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,
    0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0xC0,
    0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,
    0x00,0x00,0x00,0xC0,0x00,0x00,0x80,0x40,0x00,0x00,0x00,0x00,
};

#include <stdio.h>
#include <assert.h>

int
print_data_from_type_info(U8 **ptr_inout, U8 *opl, TypeInfo *type_info, int indent)
{
    static char spaces[] = 
        "                                                                ";
    
    int result = 1;
    
    switch (type_info->kind){
        case TypeKind_Basic:
        {
            // special case each basic type
            
            if (type_info == &U32_type_info)
            {
                if (*ptr_inout + sizeof(U32) > opl)
                {
                    result = 0;
                    goto finish;
                }
                U32 x = *(U32*)*ptr_inout;
                *ptr_inout += sizeof(x);
                fprintf(stdout, "%u\n", x);
            }
            
            if (type_info == &F32_type_info)
            {
                if (*ptr_inout + sizeof(F32) > opl)
                {
                    result = 0;
                    goto finish;
                }
                F32 x = *(F32*)*ptr_inout;
                *ptr_inout += sizeof(x);
                fprintf(stdout, "%f\n", x);
            }
            
            if (type_info == &V2F32_type_info)
            {
                if (*ptr_inout + sizeof(V2F32) > opl)
                {
                    result = 0;
                    goto finish;
                }
                V2F32 x = *(V2F32*)*ptr_inout;
                *ptr_inout += sizeof(x);
                fprintf(stdout, "(%f,%f)\n", x.x, x.y);
            }
        }break;
        
        case TypeKind_Struct:
        {
            U32 member_value_memory[20] = {0};
            
            // iterate members
            TypeInfoMember *first_member = (TypeInfoMember*)type_info->children;
            TypeInfoMember *member = first_member;
            int child_count = type_info->child_count;
            for (int i = 0; i < child_count; i += 1, member += 1)
            {
                assert(i < 20);
                TypeInfo *member_type_info = member->type;
                
                // decode single members
                if (member->array_count_member_index == -1)
                {
                    if (member_type_info == &U32_type_info && *ptr_inout + 4 <= opl)
                    {
                        member_value_memory[i] = *(U32*)*ptr_inout;
                    }
                    fprintf(stdout, "%.*s%.*s: ", indent, spaces, member->name_length, member->name);
                    if (!print_data_from_type_info(ptr_inout, opl, member_type_info, indent + 1))
                    {
                        result = 0;
                        goto finish;
                    }
                }
                
                // decode arrays
                else
                {
                    assert(member->array_count_member_index < i);
                    U32 count = member_value_memory[member->array_count_member_index];
                    fprintf(stdout, "%.*s%.*s:\n", indent, spaces, member->name_length, member->name);
                    indent += 1;
                    for (U32 j = 0; j < count; j += 1)
                    {
                        fprintf(stdout, "%.*s[%d]: ", indent, spaces, j);
                        if (!print_data_from_type_info(ptr_inout, opl, member_type_info, indent + 1))
                        {
                            result = 0;
                            goto finish;
                        }
                    }
                    indent -= 1;
                }
            }
        }break;
        
        case TypeKind_Enum:
        {
            // rely on the underlying type (default to U32) to print enums
            TypeInfo *underlying_type = type_info->underlying_type;
            if (underlying_type == 0)
            {
                underlying_type = &U32_type_info;
            }
            fprintf(stdout, "(%.*s)", type_info->name_length, type_info->name);
            if (!print_data_from_type_info(ptr_inout, opl, underlying_type, indent))
            {
                result = 0;
                goto finish;
            }
        }break;
        
        default:
        {
            result = 0;
            goto finish;
        }break;
    }
    
    finish:;
    
    return(result);
}

int
main(int argc, char **argv)
{
    // decode the raw buffer of shape data
    U8 *ptr = raw_buffer;
    U8 *opl = ptr + sizeof(raw_buffer);
    for (;ptr < opl;)
    {
        // decode a shape discriminator
        if (ptr + sizeof(Shape) > opl)
        {
            fprintf(stdout, "Could not decode shape discriminator\n");
            break;
        }
        Shape shape = *(Shape*)ptr;
        ptr += sizeof(shape);
        
        // get type info
        TypeInfo *type_info = type_info_from_shape(shape);
        if (type_info == 0)
        {
            fprintf(stdout, "Unrecognized shape discriminator\n");
            break;
        }
        
        // print type name
        fprintf(stdout, "%.*s:\n", type_info->name_length, type_info->name);
        
        // print contents
        if (!print_data_from_type_info(&ptr, opl, type_info, 1))
        {
            fprintf(stdout, "Decoding error\n");
            break;
        }
    }
    
    return(0);
}
