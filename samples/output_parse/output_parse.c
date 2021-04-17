// Sample code to print out the data from MD_ParseWholeFile
#include "md.h"
#include "md.c"

#define INDENT_SPACES 2
static void Print(FILE* file, int indent_count, char* fmt, ...) {
    for(int i = 0; i < indent_count*INDENT_SPACES; i += 1)
    {
        fprintf(file, " ");
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
}

static void PrintNode(MD_Node* node, FILE* file, int indent_count) {
    Print(file, indent_count, "Node {\n");
    Print(file, indent_count+1, "Kind: %.*s,\n", MD_StringExpand(MD_StringFromNodeKind(node->kind)));
    
    int flags_bits = sizeof(node->flags)*8;
    char binary_flags[sizeof(node->flags)*8+1];
    binary_flags[flags_bits] = '\0';
    int flag_index = 0;
    MD_NodeFlags flags = node->flags;
    for (int i = 0; i < flags_bits; i++) {
        binary_flags[i] = (flags&1) ? '1' : '0';
        flag_index++;
        flags >>= 1;
    }
    
    Print(file, indent_count+1, "Flags: %s,\n", binary_flags);
    Print(file, indent_count+1, "Flag Names: ", binary_flags);
    MD_String8List flags_list = MD_StringListFromNodeFlags(node->flags);
    MD_String8 flag_names = MD_JoinStringList(flags_list, MD_S8CString(", "));
    fprintf(file, "%.*s,\n", MD_StringExpand(flag_names));
    
    if(node->string.size > 0) Print(file, indent_count+1, "String: %.*s,\n", MD_StringExpand(node->string));
    if(node->whole_string.size > 0) Print(file, indent_count+1, "Whole String: %.*s,\n", MD_StringExpand(node->whole_string));
    if (node->first_tag->kind != MD_NodeKind_Nil) {
        for (MD_EachNode(tag, node->first_tag)) {
            Print(file, indent_count+1, "Tag: @%.*s\n", MD_StringExpand(tag->string));
            if (tag->first_child->kind != MD_NodeKind_Nil) {
                Print(file, indent_count+2, "Tag Children{\n");
                for (MD_EachNode(arg, tag->first_child)) {
                    PrintNode(arg, file, indent_count+3);
                }
                Print(file, indent_count+2, "}\n");
            }
        }
    }
    
    for(MD_EachNode(child, node->first_child)) {
        PrintNode(child, file, indent_count+1);
    }
    Print(file, indent_count, "}\n");
}

int main(int argument_count, char **arguments)
{
    // NOTE(pmh): Parse all the files passed in via command line.
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_Node *root = MD_ParseWholeFile(MD_S8CString(arguments[i])).node;
        MD_PushSibling(&first, &last, root);
    }
    
    for(MD_EachNode(root, first))
    {
        MD_String8 code_filename = MD_ChopExtension(MD_SkipFolder(root->filename));
        MD_String8 info_filename = MD_PushStringF("parsed_%.*s.txt", MD_StringExpand(code_filename));
        printf("Parse Input -> Output: %.*s -> %.*s\n", MD_StringExpand(code_filename), MD_StringExpand(info_filename));
        
        FILE* file = fopen((char *)info_filename.str, "wb");
        for(MD_EachNode(node, root->first_child))
        {
            PrintNode(node, file, 0);
            fprintf(file, "\n");
        }
        fclose(file);
    }
    
    return 0;
}