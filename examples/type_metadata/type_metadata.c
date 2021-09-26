/*
** Example: type-metadata
**
** This example shows how one might use metadesk to define types with a
** metadesk file and generate the types along with metadata such as field
** layouts, string tables, and maps.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

static MD_Arena *arena = 0;

//~ types /////////////////////////////////////////////////////////////////////

typedef enum TypeKind
{
    TypeKind_Null,
    TypeKind_Basic,
    TypeKind_Struct,
    TypeKind_Enum,
} TypeKind;

typedef struct TypeInfo TypeInfo;
struct TypeInfo
{
    TypeInfo *next;
    TypeKind kind;
    MD_Node *node;
    
    // basic
    int size;
    
    // structs
    struct TypeMember *first_member;
    struct TypeMember *last_member;
    int member_count;
    
    // enums
    struct TypeEnumerant *first_enumerant;
    struct TypeEnumerant *last_enumerant;
    int enumerant_count;
    TypeInfo *underlying_type;
};

typedef struct TypeMember TypeMember;
struct TypeMember
{
    TypeMember *next;
    MD_Node *node;
    TypeInfo *type;
    MD_Node *array_count;
};

typedef struct TypeEnumerant TypeEnumerant;
struct TypeEnumerant
{
    TypeEnumerant *next;
    MD_Node *node;
    int value;
};

typedef struct MapInfo MapInfo;
struct MapInfo
{
    MapInfo *next;
    MD_Node *node;
    
    MD_Node *in;
    MD_Node *out;
    
    int is_complete;
    MD_Node *default_val;
    MD_Node *auto_val;
};


//~ node maps /////////////////////////////////////////////////////////////////

TypeInfo *first_type = 0;
TypeInfo *last_type = 0;
MD_Map type_map = {0};

MapInfo *first_map = 0;
MapInfo *last_map = 0;
MD_Map map_map = {0};


//~ feature generators ////////////////////////////////////////////////////////

void
generate_type_definitions_from_types(FILE *out, TypeInfo *first_type)
{
    MD_PrintGenNoteCComment(out);
    
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        switch (type->kind)
        {
            default:break;
            
            case TypeKind_Struct:
            {
                MD_String8 struct_name = type->node->string;
                fprintf(out, "typedef struct %.*s %.*s;\n", MD_S8VArg(struct_name), MD_S8VArg(struct_name));
                fprintf(out, "struct %.*s\n", MD_S8VArg(struct_name));
                fprintf(out, "{\n");
                for (TypeMember *member = type->first_member;
                     member != 0;
                     member = member->next)
                {
                    MD_String8 type_name = member->type->node->string;
                    MD_String8 member_name = member->node->string;
                    int is_array = (!MD_NodeIsNil(member->array_count));
                    if (is_array)
                    {
                        fprintf(out, "%.*s *%.*s;\n", MD_S8VArg(type_name), MD_S8VArg(member_name));
                    }
                    else
                    {
                        fprintf(out, "%.*s %.*s;\n", MD_S8VArg(type_name), MD_S8VArg(member_name));
                    }
                }
                fprintf(out, "};\n");
            }break;
            
            case TypeKind_Enum:
            {
                MD_String8 enum_name = type->node->string;
                fprintf(out, "typedef enum %.*s\n", MD_S8VArg(enum_name));
                fprintf(out, "{\n");
                for (TypeEnumerant *enumerant = type->first_enumerant;
                     enumerant != 0;
                     enumerant = enumerant->next)
                {
                    MD_String8 member_name = enumerant->node->string;
                    fprintf(out, "%.*s_%.*s = %d,\n",
                            MD_S8VArg(enum_name), MD_S8VArg(member_name), enumerant->value);
                }
                fprintf(out, "} %.*s;\n", MD_S8VArg(enum_name));
            }break;
        }
    }
    
    fprintf(out, "\n");
}

void
generate_function_declarations_from_maps(FILE *out, MapInfo *first_map)
{
    MD_PrintGenNoteCComment(out);
    
    for (MapInfo *map = first_map;
         map != 0;
         map = map->next)
    {
        MD_Node *node = map->node;
        
        MD_String8 in_type = map->in->string;
        MD_String8 out_type = map->out->string;
        if (MD_S8Match(out_type, MD_S8Lit("$Type"), 0))
        {
            out_type = MD_S8Lit("TypeInfo*");
        }
        
        fprintf(out, "%.*s %.*s(%.*s v);\n",
                MD_S8VArg(out_type), MD_S8VArg(node->string), MD_S8VArg(in_type));
    }
    
    fprintf(out, "\n");
}

void
generate_type_info_declarations_from_types(FILE *out, TypeInfo *first_type)
{
    MD_PrintGenNoteCComment(out);
    
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        MD_String8 name = type->node->string;
        fprintf(out, "extern TypeInfo %.*s_type_info;\n", MD_S8VArg(name));
    }
    
    fprintf(out, "\n");
}

void
generate_struct_member_tables_from_types(FILE *out, TypeInfo *first_type)
{
    MD_PrintGenNoteCComment(out);
    
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == TypeKind_Struct)
        {
            MD_String8 type_name = type->node->string;
            int member_count = type->member_count;
            
            fprintf(out, "TypeInfoMember %.*s_members[%d] = {\n", MD_S8VArg(type_name), member_count);
            
            for (TypeMember *member = type->first_member;
                 member != 0;
                 member = member->next)
            {
                MD_String8 member_name = member->node->string;
                MD_String8 member_type_name = member->type->node->string;
                int array_count_member_index = -1;
                if (!MD_NodeIsNil(member->array_count))
                {
                    array_count_member_index = MD_IndexFromNode(member->array_count);
                }
                fprintf(out, "{\"%.*s\", %d, %d, &%.*s_type_info},\n",
                        MD_S8VArg(member_name), (int)member_name.size,
                        array_count_member_index, MD_S8VArg(member_type_name));
            }
            
            fprintf(out, "};\n");
        }
    }
    
    fprintf(out, "\n");
}

void
generate_enum_member_tables_from_types(FILE *out, TypeInfo *first_type)
{
    MD_PrintGenNoteCComment(out);
    
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == TypeKind_Enum)
        {
            MD_String8 type_name = type->node->string;
            int enumerant_count = type->enumerant_count;
            
            fprintf(out, "TypeInfoEnumerant %.*s_members[%d] = {\n",
                    MD_S8VArg(type_name), enumerant_count);
            for (TypeEnumerant *enumerant = type->first_enumerant;
                 enumerant != 0;
                 enumerant = enumerant->next)
            {
                MD_String8 enumerant_name = enumerant->node->string;
                int value = enumerant->value;
                fprintf(out, "{\"%.*s\", %d, %d},\n",
                        MD_S8VArg(enumerant_name), (int)enumerant_name.size,
                        value);
            }
            
            fprintf(out, "};\n");
        }
    }
    
    fprintf(out, "\n");
}

void
generate_type_info_definitions_from_types(FILE *out, TypeInfo *first_type)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    
    MD_PrintGenNoteCComment(out);
    
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        MD_String8 type_name = type->node->string;
        
        switch (type->kind)
        {
            default:break;
            
            case TypeKind_Basic:
            {
                int size = type->size;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Basic, \"%.*s\", %d, %d, 0, 0};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, size);
            }break;
            
            case TypeKind_Struct:
            {
                int child_count = type->member_count;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Struct, \"%.*s\", %d, %d, %.*s_members, 0};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name));
            }break;
            
            case TypeKind_Enum:
            {
                MD_String8 underlying_type_ptr_expression = MD_S8Lit("0");
                if (type->underlying_type != 0)
                {
                    MD_String8 underlying_type_name = type->underlying_type->node->string;
                    underlying_type_ptr_expression = MD_S8Fmt(scratch.arena, "&%.*s_type_info",
                                                              MD_S8VArg(underlying_type_name));
                }
                
                int child_count = type->enumerant_count;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Enum, \"%.*s\", %d, %d, %.*s_members, %.*s};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name),
                        MD_S8VArg(underlying_type_ptr_expression));
            }break;
        }
    }
    
    fprintf(out, "\n");
    
    MD_ReleaseScratch(scratch);
}


//~ main //////////////////////////////////////////////////////////////////////

MD_Node*
get_md_child_value(MD_Node *parent, MD_String8 child_name)
{
    MD_Node *child = MD_ChildFromString(parent, child_name, 0);
    MD_Node *result = child->first_child;
    return(result);
}

int
main(int argc, char **argv)
{
    char *argv_dummy[2] = {
        0,
        "W:/metadesk/examples/type_metadata/types.mdesk"
    };
    argc = 2;
    argv = argv_dummy;
    
    // setup the global arena
    arena = MD_ArenaAlloc();
    
    // output stream routing
    FILE *error_file = stderr;
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for(int i = 1; i < argc; i += 1)
    {
        // parse the file
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        // print metadesk errors
        for (MD_Message *message = parse_result.errors.first;
             message != 0;
             message = message->next)
        {
            MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(error_file, code_loc, message->kind, message->string);
        }
        
        // save to parse results list
        MD_PushNewReference(arena, list, parse_result.node);
    }
    
    // init maps
    type_map = MD_MapMake(arena);
    map_map = MD_MapMake(arena);
    
    // gather types & maps
    for(MD_EachNode(ref, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(ref);
        for(MD_EachNode(node, root->first_child))
        {
            // gather type
            MD_Node *type_tag =  MD_TagFromString(node, MD_S8Lit("type"), 0);
            
            if (!MD_NodeIsNil(type_tag))
            {
                TypeKind kind = TypeKind_Null;
                MD_Node   *tag_arg_node = type_tag->first_child;
                MD_String8 tag_arg_str = tag_arg_node->string;
                if (MD_S8Match(tag_arg_str, MD_S8Lit("basic"), 0))
                {
                    kind = TypeKind_Basic;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("struct"), 0))
                {
                    kind = TypeKind_Struct;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("enum"), 0))
                {
                    kind = TypeKind_Enum;
                }
                
                if (kind == TypeKind_Null)
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(node);
                    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                       "Unrecognized type kind '%.*s'\n",
                                       MD_S8VArg(tag_arg_str));
                }
                else
                {
                    TypeInfo *type_info = MD_PushArrayZero(arena, TypeInfo, 1);
                    type_info->kind = kind;
                    type_info->node = node;
                    
                    MD_QueuePush(first_type, last_type, type_info);
                    MD_MapInsert(arena, &type_map, MD_MapKeyStr(node->string), type_info);
                }
            }
            
            // gather map
            MD_Node *map_tag = MD_TagFromString(node, MD_S8Lit("map"), 0);
            
            if (!MD_NodeIsNil(map_tag))
            {
                // NOTE we could use an expression parser here to make this fancier
                // and check for the 'In -> Out' semicolon delimited syntax more
                // carefully, this isn't checking it very rigorously. But there are
                // no other cases we need to expect so far so being a bit sloppy
                // buys us a lot of simplicity.
                MD_Node *in = map_tag->first_child;
                MD_Node *arrow = in->next;
                MD_Node *out = arrow->next;
                {
                    MD_Node *error_at = 0;
                    if (MD_NodeIsNil(in))
                    {
                        error_at = map_tag;
                    }
                    else if (!MD_S8Match(arrow->string, MD_S8Lit("->"), 0) ||
                             MD_NodeIsNil(out))
                    {
                        error_at = in;
                    }
                    if (error_at != 0)
                    {
                        MD_CodeLoc loc = MD_CodeLocFromNode(error_at);
                        MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                        MD_S8Lit("Map's type should be specified like: `In -> Out`"));
                    }
                }
                
                // check for named children in the map tag
                int is_complete = MD_NodeHasChild(map_tag, MD_S8Lit("complete"), 0);
                MD_Node *default_val = get_md_child_value(map_tag, MD_S8Lit("default"));
                MD_Node *auto_val = get_md_child_value(map_tag, MD_S8Lit("auto"));
                
                // save a new map
                MapInfo *map_info = MD_PushArrayZero(arena, MapInfo, 1);
                map_info->node = node;
                map_info->in = in;
                map_info->out = out;
                map_info->is_complete = is_complete;
                map_info->default_val = default_val;
                map_info->auto_val = auto_val;
                
                MD_QueuePush(first_map, last_map, map_info);
                MD_MapInsert(arena, &map_map, MD_MapKeyStr(node->string), map_info);
            }
        }
    }
    
    // TODO no duplicate member names check
    
    // TODO basic type sizes
    
    // build member lists
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == TypeKind_Struct)
        {
            // build the list
            MD_b32 got_list = 1;
            TypeMember *first_member = 0;
            TypeMember *last_member = 0;
            int member_count = 0;
            
            MD_Node *type_root_node = type->node;
            for (MD_EachNode(member_node, type_root_node->first_child))
            {
                MD_Node *type_name_node = member_node->first_child;
                
                // missing type node?
                if (MD_NodeIsNil(type_name_node))
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(member_node);
                    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                    MD_S8Lit("Missing type name for member"));
                    got_list = 0;
                    goto skip_member;
                }
                
                // has type node:
                MD_String8 type_name = type_name_node->string;
                MD_MapSlot *type_info_slot = (MD_MapSlot*)MD_MapLookup(&type_map, MD_MapKeyStr(type_name));
                
                // could not resolve type?
                if (type_info_slot == 0)
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(type_name_node);
                    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                       "Could not resolve type name '%.*s'", MD_S8VArg(type_name));
                    got_list = 0;
                    goto skip_member;
                }
                
                // resolved type:
                if (got_list)
                {
                    TypeInfo *type_info = (TypeInfo*)type_info_slot->val;
                    
                    MD_Node *array_count = MD_NilNode();
                    MD_Node *array_tag = MD_TagFromString(type_name_node, MD_S8Lit("array"), 0);
                    if (!MD_NodeIsNil(array_tag))
                    {
                        MD_Node *array_count_referencer = array_tag->first_child;
                        if (array_count_referencer->string.size == 0)
                        {
                            MD_CodeLoc loc = MD_CodeLocFromNode(array_tag);
                            MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                            MD_S8Lit("array tags must specify a parameter for their count"));
                        }
                        else
                        {
                            array_count = MD_ChildFromString(type_root_node, array_count_referencer->string, 0);
                            if (MD_NodeIsNil(array_count))
                            {
                                MD_CodeLoc loc = MD_CodeLocFromNode(array_count_referencer);
                                MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                                   "'%.*s' is not a member of %.*s",
                                                   MD_S8VArg(array_count_referencer->string), MD_S8VArg(type_name));
                            }
                        }
                    }
                    
                    TypeMember *member = MD_PushArray(arena, TypeMember, 1);
                    member->node = member_node;
                    member->type = type_info;
                    member->array_count = array_count;
                    MD_QueuePush(first_member, last_member, member);
                    member_count += 1;
                }
                
                skip_member:;
            }
            
            // save the list
            if (got_list)
            {
                type->first_member = first_member;
                type->last_member = last_member;
                type->member_count = member_count;
            }
        }
    }
    
    // TODO check enum base types
    
    // build enumerant lists
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == TypeKind_Enum)
        {
            
            // build the list
            MD_b32 got_list = 1;
            TypeEnumerant *first_enumerant = 0;
            TypeEnumerant *last_enumerant = 0;
            int enumerant_count = 0;
            
            int next_implicit_value = 0;
            
            MD_Node *type_root_node = type->node;
            for (MD_EachNode(enumerant_node, type_root_node->first_child))
            {
                MD_Node *value_node = enumerant_node->first_child;
                int value = 0;
                
                // missing value node?
                if (MD_NodeIsNil(value_node))
                {
                    value = next_implicit_value;
                    next_implicit_value += 1;
                }
                
                // has value node
                else
                {
                    MD_String8 value_string = value_node->string;
                    if (!MD_StringIsCStyleInt(value_string))
                    {
                        got_list = 0;
                        goto skip_enumerant;
                    }
                    value = (int)MD_CStyleIntFromString(value_string);
                }
                
                // set next implicit value
                next_implicit_value = value + 1;
                
                // save enumerant
                if (got_list)
                {
                    TypeEnumerant *enumerant = MD_PushArray(arena, TypeEnumerant, 1);
                    enumerant->node = enumerant_node;
                    enumerant->value = value;
                    MD_QueuePush(first_enumerant, last_enumerant, enumerant);
                    enumerant_count += 1;
                }
                
                skip_enumerant:;
            }
            
            // save the list
            if (got_list)
            {
                type->first_enumerant = first_enumerant;
                type->last_enumerant = last_enumerant;
                type->enumerant_count = enumerant_count;
            }
        }
    }
    
    // TODO check maps & build case lists
    
    // generate meta types header
    {
        FILE *h = fopen("meta_types.h", "wb");
        fprintf(h, "#if !defined(META_TYPES_H)\n");
        fprintf(h, "#define META_TYPES_H\n");
        
        // generate type definitions
        generate_type_definitions_from_types(h, first_type);
        
        // generate function declarations
        generate_function_declarations_from_maps(h, first_map);
        
        // generate metadata declarations
        generate_type_info_declarations_from_types(h, first_type);
        
        fprintf(h, "#endif // META_TYPES_H\n");
        fclose(h);
    }
    
    // generate meta types code
    {
        // open output file
        FILE *c = fopen("meta_types.c", "wb");
        
        // generate metadata tables
        generate_struct_member_tables_from_types(c, first_type);
        generate_enum_member_tables_from_types(c, first_type);
        generate_type_info_definitions_from_types(c, first_type);
        
        // TODO generate function definitions
        
        // close output file
        fclose(c);
    }
    
    // print state
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next)
    {
        char *kind_string = "ERROR";
        switch (type->kind)
        {
            default:break;
            case TypeKind_Basic:  kind_string = "basic"; break;
            case TypeKind_Struct: kind_string = "struct"; break;
            case TypeKind_Enum:   kind_string = "enum"; break;
        }
        
        MD_Node *node = type->node;
        printf("%.*s: %s\n", MD_S8VArg(node->string), kind_string);
        
        // print member lists
        for (TypeMember *member = type->first_member;
             member != 0;
             member = member->next)
        {
            printf("  %.*s: %.*s\n",
                   MD_S8VArg(member->node->string),
                   MD_S8VArg(member->type->node->string));
        }
        
        // print enumerant lists
        for (TypeEnumerant *enumerant = type->first_enumerant;
             enumerant != 0;
             enumerant = enumerant->next)
        {
            printf("  %.*s: %d\n",
                   MD_S8VArg(enumerant->node->string),
                   enumerant->value);
        }
    }
    
    for (MapInfo *map = first_map;
         map != 0;
         map = map->next)
    {
        MD_Node *node = map->node;
        printf("%.*s: map\n", MD_S8VArg(node->string));
    }
}
