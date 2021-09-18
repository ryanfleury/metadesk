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
    // setup the global arena
    arena = MD_ArenaAlloc();
    
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
            MD_PrintMessage(stderr, code_loc, message->kind, message->string);
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
                    MD_PrintMessageFmt(stderr, loc, MD_MessageKind_Error,
                                       "Unrecognized type kind '%.*s'\n",
                                       MD_S8VArg(tag_arg_str));
                }
                else{
                    TypeInfo *type_info = MD_PushArray(arena, TypeInfo, 1);
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
    }
    
    for (MapInfo *map = first_map;
         map != 0;
         map = map->next){
        MD_Node *node = map->node;
        printf("%.*s: map\n", MD_S8VArg(node->string));
    }
    
    // TODO metadata hand-written portion
    // TODO check types & build member lists
    // TODO check maps & build case lists
    // TODO generate type definitions
    // TODO generate function declarations
    // TODO generate metadata tables
    // TODO generate function definitions
}
