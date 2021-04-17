#include "md.h"
#include "md_c_helpers.h"

#include "md.c"
#include "md_c_helpers.c"

static MD_Map MapFromChildren(MD_Node *node)
{
    MD_Map result = {0};
    for(MD_EachNodeRef(child, node->first_child))
    {
        MD_StringMap_Insert(&result, MD_MapCollisionRule_Chain, child->string, child);
    }
    return result;
}

static void OutputType_C_LHS_Namespace(FILE *file, MD_Map *user_defined_types, MD_String8 prefix, MD_Expr *type)
{
    switch (type->kind)
    {
        case MD_ExprKind_Atom:
        {
            MD_Node *node = type->node;
            
            if(MD_StringMap_Lookup(user_defined_types, type->node->string))
            {
                fprintf(file, "%.*s", MD_StringExpand(prefix));
            }
            fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
        }break;
        
        case MD_ExprKind_Pointer:
        {
            OutputType_C_LHS_Namespace(file, user_defined_types, prefix, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
            fprintf(file, "*");
        }break;
        
        case MD_ExprKind_Array:
        {
            OutputType_C_LHS_Namespace(file, user_defined_types, prefix, type->sub[0]);
            if (_MD_OutputType_C_NeedsParens(type))
            {
                fprintf(file, "(");
            }
        }break;
        
        case MD_ExprKind_Volatile: { fprintf(file, "volatile "); }break;
        case MD_ExprKind_Const:    { fprintf(file, "const "); }break;
        
        default:
        {
            fprintf(file, "{ unexpected MD_ExprKind (%i) in type info for node \"%.*s\" }",
                    type->kind,
                    MD_StringExpand(type->node->whole_string));
        }break;
    }
}

static void OutputPrefixedType(FILE *f, MD_Map *user_defined_types, MD_String8 prefix, MD_Node *node, MD_u32 depth)
{
#define I() fprintf(f, "%*s", 4*depth, "");
    if(MD_NodeHasTag(node, MD_S8Lit("enum")))
    {
        I(); fprintf(f, "typedef enum\n");
        I(); fprintf(f, "{\n");
        for(MD_EachNode(enumerand, node->first_child))
        {
            I(); fprintf(f, "    %.*s%.*s_%.*s,\n", MD_StringExpand(prefix), 
                         MD_StringExpand(node->string), MD_StringExpand(enumerand->string));
        }
        I(); fprintf(f, "} %.*s%.*s;\n\n", MD_StringExpand(prefix), MD_StringExpand(node->string));
    }
    else if(MD_NodeHasTag(node, MD_S8Lit("flags")))
    {
        I(); fprintf(f, "typedef enum\n");
        I(); fprintf(f, "{\n");
        
        MD_String8 singular_flag_type_name = node->string;
        if(MD_StringMatch(MD_StringSuffix(singular_flag_type_name, 5), MD_S8Lit("Flags"), 
                          MD_MatchFlag_CaseInsensitive))
        {
            singular_flag_type_name = MD_StringChop(singular_flag_type_name, 1);
        }
        MD_u32 v = 0;
        for(MD_EachNode(enumerand, node->first_child))
        {
            I(); fprintf(f, "    %.*s%.*s_%.*s = (1<<%d),\n", MD_StringExpand(prefix), 
                         MD_StringExpand(singular_flag_type_name), MD_StringExpand(enumerand->string), v);
            v += 1;
        }
        I(); fprintf(f, "} %.*s%.*s;\n\n", MD_StringExpand(prefix), MD_StringExpand(node->string));
    }
    else if(MD_NodeHasTag(node, MD_S8Lit("union")) || MD_NodeHasTag(node, MD_S8Lit("struct")))
    {
        MD_String8 aggregate_kind = MD_NodeHasTag(node, MD_S8Lit("union")) ? MD_S8Lit("union") : MD_S8Lit("struct");
        
        if(depth == 0)
        {
            fprintf(f, "typedef %.*s %.*s%.*s %.*s%.*s;\n",
                    MD_StringExpand(aggregate_kind),
                    MD_StringExpand(prefix), MD_StringExpand(node->string),
                    MD_StringExpand(prefix), MD_StringExpand(node->string));
        }
        
        if(depth == 0)
        {
            I(); fprintf(f, "%.*s %.*s%.*s\n", MD_StringExpand(aggregate_kind),
                         MD_StringExpand(prefix), MD_StringExpand(node->string));
        }
        else
        {
            I(); fprintf(f, "%.*s\n", MD_StringExpand(aggregate_kind));
        }
        
        I(); fprintf(f, "{\n");
        for(MD_EachNode(member, node->first_child))
        {
            OutputPrefixedType(f, user_defined_types, prefix, member, depth+1);
        }
        if(depth == 0)
        {
            I(); fprintf(f, "};\n\n");
        }
        else
        {
            I(); fprintf(f, "} %.*s;\n", MD_StringExpand(node->string));
        }
    }
    else 
    {
        MD_Expr *type = MD_ParseAsType(node->first_child, node->last_child);
        
        I(); 
        OutputType_C_LHS_Namespace(f, user_defined_types, prefix, type);
        fprintf(f, " %.*s", MD_StringExpand(node->string));
        MD_OutputType_C_RHS(f, type);
        
        fprintf(f, ";\n");
    }
#undef I
}

static void AppendConversionCode(FILE *f, MD_Map *user_defined_types, MD_Node *new_element, MD_Node *old_element, MD_String8 member_path)
{
    MD_Map new_element_map = MapFromChildren(new_element);
    
    if(MD_NodeHasTag(new_element, MD_S8Lit("struct")))
    {
        MD_String8 extended_member_path = MD_PushStringF("%.*s.%.*s", MD_StringExpand(member_path), 
                                                         MD_StringExpand(old_element->string));
        
        for(MD_EachNode(member, old_element->first_child))
        {
            MD_MapSlot *slot = MD_StringMap_Lookup(&new_element_map, member->string);
            if(slot)
            {
                AppendConversionCode(f, user_defined_types, member, slot->value, extended_member_path);
            }
            else
            {
                fprintf(f, "    // TODO: What to do with %.*s\n", MD_StringExpand(old_element->string));
            }
        }
    }
    else if(MD_NodeIsNil(new_element->first_tag))
    {
        if(MD_ChildCountFromNode(new_element) == 1)
        {
            MD_String8 type_name = new_element->first_child->string;
            if(MD_NodeDeepMatch(new_element, old_element, 0))
            {
                MD_String8 extended_member_path = MD_PushStringF("%.*s.%.*s", MD_StringExpand(member_path), 
                                                                 MD_StringExpand(old_element->string));
                
                if(MD_StringMap_Lookup(user_defined_types, type_name))
                {
                    fprintf(f, "    result%.*s = %.*sFromV1(v%.*s);\n", 
                            MD_StringExpand(extended_member_path), MD_StringExpand(type_name), 
                            MD_StringExpand(extended_member_path));
                }
                else
                {   // NOTE(mal): Assumes unknown types are base types
                    fprintf(f, "    result%.*s = v%.*s;\n", 
                            MD_StringExpand(extended_member_path), MD_StringExpand(extended_member_path));
                }
            }
            else
            {
                fprintf(f, "    // TODO: What to do with %.*s %.*s\n", 
                        MD_StringExpand(old_element->string), MD_StringExpand(type_name));
            }
        }
        else
        {
            fprintf(f, "    // TODO: What to do with %.*s\n", MD_StringExpand(old_element->string));
        }
    }
    else
    {
        fprintf(stderr, "Missing code to process tag @%.*s\n", MD_StringExpand(new_element->first_tag->string));
        exit(1);
    }
}

int main(int argument_count, char **arguments)
{
    if(argument_count != 3)
    {
        fprintf(stderr, "USAGE: %s spec_file_name.md out_file_name.c\n", arguments[0]);
        return -1;
    }
    
    MD_ParseResult spec = MD_ParseWholeFile(MD_S8CString(arguments[1]));
    if(spec.first_error)
    {
        for(MD_Error *error = spec.first_error; error; error = error->next)
        {
            MD_NodeMessage(error->node, error->kind, error->string);
        }
        return -1;
    }
    
    FILE *f = fopen(arguments[2], "wb");
    if(f == 0)
    {
        fprintf(stderr, "Unable to open destination file \"%s\"\n", arguments[2]);
        return -1;
    }
    
    MD_Map ns_map = MapFromChildren(spec.namespaces);
    
    // NOTE(mal): Old types get "v1_" as a prefix
    fprintf(f, "// V1\n");
    MD_Node *v1 = MD_StringMap_Lookup(&ns_map, MD_S8Lit("v1"))->value;
    MD_Map v1_map = MapFromChildren(v1);
    for(MD_EachNodeRef(node, v1->first_child))
    {
        OutputPrefixedType(f, &v1_map, MD_S8Lit("v1_"), node, 0);
    }
    
    // NOTE(mal): New types don't get a prefix
    fprintf(f, "// V2\n");
    MD_Map empty_map = {0};
    
    MD_Node *v2 = MD_StringMap_Lookup(&ns_map, MD_S8Lit("v2"))->value;
    for(MD_EachNodeRef(node, v2->first_child))
    {
        OutputPrefixedType(f, &empty_map, MD_S8Lit(""), node, 0);
    }
    
    // NOTE(mal): Routines that map old into new
    fprintf(f, "// V1->V2\n");
    MD_Map v2_map = MapFromChildren(v2);
    for(MD_EachNodeRef(node, v1->first_child))
    {
        MD_MapSlot *slot =  MD_StringMap_Lookup(&v2_map, node->string);
        MD_Node *v2_type = slot->value;
        MD_Map children_map = MapFromChildren(v2_type);
        
        fprintf(f, "static %.*s %.*sFromV1(v1_%.*s v)\n{\n",
                MD_StringExpand(node->string), MD_StringExpand(node->string), MD_StringExpand(node->string));
        
        if(MD_NodeHasTag(node, MD_S8Lit("enum")))
        {
            fprintf(f, "    %.*s result = 0;\n", MD_StringExpand(node->string));
            fprintf(f, "    switch(v)\n    {\n");
            
            for(MD_EachNode(enumerand, node->first_child))
            {
                fprintf(f, "        case v1_%.*s_%.*s: ", 
                        MD_StringExpand(node->string), MD_StringExpand(enumerand->string));
                MD_MapSlot *slot = MD_StringMap_Lookup(&children_map, enumerand->string);
                if(slot)
                {
                    MD_Node *v2_enumerand = slot->value;
                    fprintf(f, "result = %.*s_%.*s; break;\n", 
                            MD_StringExpand(node->string), MD_StringExpand(v2_enumerand->string));
                }
                else
                {
                    fprintf(f, "assert(!\"Enumerand v1_%.*s_%.*s is no longer allowed\\n\");\n", 
                            MD_StringExpand(node->string), MD_StringExpand(enumerand->string));
                }
            }
            fprintf(f, "        default: assert(!\"Illegal value for enum v1_%.*s\\n\"); break;\n", MD_StringExpand(node->string));
            fprintf(f, "    }\n");
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("flags")))
        {
            fprintf(f, "    %.*s result = 0;\n", MD_StringExpand(node->string));
            
            MD_String8 singular_flag_type_name = node->string;
            if(MD_StringMatch(MD_StringSuffix(singular_flag_type_name, 5), MD_S8Lit("Flags"), 
                              MD_MatchFlag_CaseInsensitive))
            {
                singular_flag_type_name = MD_StringChop(singular_flag_type_name, 1);
            }
            
            for(MD_EachNode(flag, node->first_child))
            {
                fprintf(f, "    if(v & v1_%.*s_%.*s) ", 
                        MD_StringExpand(singular_flag_type_name), MD_StringExpand(flag->string));
                MD_MapSlot *slot = MD_StringMap_Lookup(&children_map, flag->string);
                if(slot)
                {
                    fprintf(f, "result |= %.*s_%.*s;\n", 
                            MD_StringExpand(singular_flag_type_name), MD_StringExpand(flag->string));
                }
                else
                {
                    fprintf(f, "assert(!\"Flag v1_%.*s is no longer allowed\\n\");\n", MD_StringExpand(flag->string));
                }
            }
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("union")))
        {
            fprintf(f, "    %.*s result = {0};\n", MD_StringExpand(node->string));
            
            MD_Node *authoritative_member = v2_type->first_child;
            for(MD_EachNode(member, v2_type->first_child))
            {
                if(MD_NodeHasTag(member, MD_S8Lit("authoritative")))
                {
                    authoritative_member = member;
                    break;
                }
            }
            
            MD_Map v1_children_map = MapFromChildren(node);
            MD_MapSlot *v1_member_slot = MD_StringMap_Lookup(&v1_children_map, authoritative_member->string);
            if(v1_member_slot)
            {
                AppendConversionCode(f, &v2_map, authoritative_member, v1_member_slot->value, MD_S8Lit(""));
            }
            else
            {
                fprintf(stderr, "Can't generate conversion for union %.*s", MD_StringExpand(node->string));
            }
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("struct")))
        {
            fprintf(f, "    %.*s result = {0};\n", MD_StringExpand(node->string));
            MD_Map v1_children_map = MapFromChildren(node);
            for(MD_EachNode(member, v2_type->first_child))
            {
                MD_MapSlot *v1_member_slot = MD_StringMap_Lookup(&v1_children_map, member->string);
                if(v1_member_slot)
                {
                    AppendConversionCode(f, &v2_map, member, v1_member_slot->value, MD_S8Lit(""));
                }
                else
                {
                    fprintf(stderr, "Can't generate conversion for struct member %.*s", MD_StringExpand(node->string));
                }
            }
        }
        else
        {
            fprintf(stderr, "Missing code to process tag @%.*s\n", MD_StringExpand(node->first_tag->string));
            return -1;
        }
        fprintf(f, "    return result;\n");
        fprintf(f, "}\n\n");
        fflush(f);
    }
    
    fclose(f);
    return 0;
}
