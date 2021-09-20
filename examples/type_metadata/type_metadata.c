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
    
    // member list
    struct TypeMember *first_member;
    struct TypeMember *last_member;
    int member_count;
    
    // enumerant list
    struct TypeEnumerant *first_enumerant;
    struct TypeEnumerant *last_enumerant;
    int enumerant_count;
    
};

typedef struct TypeMember TypeMember;
struct TypeMember
{
    TypeMember *next;
    MD_Node *node;
    TypeInfo *type;
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
};


//~ node maps /////////////////////////////////////////////////////////////////

TypeInfo *first_type = 0;
TypeInfo *last_type = 0;
MD_Map type_map = {0};

MapInfo *first_map = 0;
MapInfo *last_map = 0;
MD_Map map_map = {0};


//~ main //////////////////////////////////////////////////////////////////////

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
    FILE *fstream_errors = stderr;
    
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
            MD_PrintMessage(fstream_errors, code_loc, message->kind, message->string);
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
                if (MD_S8Match(tag_arg_str, MD_S8Lit("basic"), 0)){
                    kind = TypeKind_Basic;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("struct"), 0)){
                    kind = TypeKind_Struct;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("enum"), 0)){
                    kind = TypeKind_Enum;
                }
                
                if (kind == TypeKind_Null){
                    MD_CodeLoc loc = MD_CodeLocFromNode(node);
                    MD_PrintMessageFmt(fstream_errors, loc, MD_MessageKind_Error,
                                       "Unrecognized type kind '%.*s'\n",
                                       MD_S8VArg(tag_arg_str));
                }
                else{
                    TypeInfo *type_info = MD_PushArrayZero(arena, TypeInfo, 1);
                    type_info->kind = kind;
                    type_info->node = node;
                    
                    MD_QueuePush(first_type, last_type, type_info);
                    MD_MapInsert(arena, &type_map, MD_MapKeyStr(node->string), type_info);
                }
            }
            
            // gather map
            MD_Node *map_tag =  MD_TagFromString(node, MD_S8Lit("map"), 0);
            
            if (!MD_NodeIsNil(map_tag))
            {
                MapInfo *map_info = MD_PushArray(arena, MapInfo, 1);
                map_info->node = node;
                
                MD_QueuePush(first_map, last_map, map_info);
                MD_MapInsert(arena, &map_map, MD_MapKeyStr(node->string), map_info);
            }
        }
    }
    
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
                    MD_PrintMessage(fstream_errors, loc, MD_MessageKind_Error,
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
                    MD_PrintMessageFmt(fstream_errors, loc, MD_MessageKind_Error,
                                       "Could not resolve type name '%.*s'", MD_S8VArg(type_name));
                    got_list = 0;
                    goto skip_member;
                }
                
                // resolved type:
                if (got_list)
                {
                    TypeInfo *type_info = (TypeInfo*)type_info_slot->val;
                    
                    // TODO(allen): handle the array tag
                    
                    TypeMember *member = MD_PushArray(arena, TypeMember, 1);
                    member->node = member_node;
                    member->type = type_info;
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
    
    // TODO check types & build member lists
    // TODO check maps & build case lists
    // TODO generate type definitions
    // TODO generate function declarations
    // TODO generate metadata tables
    // TODO generate function definitions
    
    // print state
    for (TypeInfo *type = first_type;
         type != 0;
         type = type->next){
        char *kind_string = "ERROR";
        switch (type->kind){
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
             member = member->next){
            printf("  %.*s: %.*s\n",
                   MD_S8VArg(member->node->string),
                   MD_S8VArg(member->type->node->string));
        }
        
        // print enumerant lists
        for (TypeEnumerant *enumerant = type->first_enumerant;
             enumerant != 0;
             enumerant = enumerant->next){
            printf("  %.*s: %d\n",
                   MD_S8VArg(enumerant->node->string),
                   enumerant->value);
        }
    }
    
    for (MapInfo *map = first_map;
         map != 0;
         map = map->next){
        MD_Node *node = map->node;
        printf("%.*s: map\n", MD_S8VArg(node->string));
    }
}
