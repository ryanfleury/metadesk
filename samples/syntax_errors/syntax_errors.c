// Sample code to demonstrate syntax error detection and reporting

#include "md.h"
#include "md.c"

int main(int argument_count, char **arguments)
{
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_ParseResult parse = MD_ParseWholeFile(MD_S8CString(arguments[i]));
        if(parse.first_error)
        {
            for(MD_Error *error = parse.first_error; error; error = error->next)
            {
                MD_OutputError(stderr, error);
            }
        }
        else
        {
            printf("No error in file %s\n", arguments[i]);
        }
    }
    
    return 0;
}
