// Sample program that takes C-like information specified in the Metadesk format
// and generates valid C code from it.

#include "md.h"
#include "md_c_helpers.h"

#include "md.c"
#include "md_c_helpers.c"

int main(int argument_count, char **arguments)
{
    MD_String8 example_code = MD_S8Lit("@struct Foo:\n"
                                       "{\n"
                                       "  a: S32,\n"
                                       "  b: *S32,\n"
                                       "  c: **void,\n"
                                       "  d: F32,\n"
                                       "  e: *[100]F32,\n"
                                       "  f: ([4 + 5]S32),\n"
                                       "  g: ([FOO + BAR]I32),\n"
                                       "}\n\n");
    MD_Node *code = MD_ParseWholeString(MD_S8Lit("Generated Test Code"), example_code).node;
    
    printf("Source Metadesk Code:\n");
    printf("%.*s\n\n", MD_StringExpand(example_code));
    
    printf("Generated C Code:\n");
    for(MD_EachNode(node, code->first_child))
    {
        if(MD_NodeHasTag(node, MD_S8Lit("struct")))
        {
            MD_OutputTree_C_Struct(stdout, node);
        }
    }
    printf("\n\n");
    
    return 0;
}