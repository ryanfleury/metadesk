/* date = September 25th 2021 8:06 pm */

#ifndef TYPE_METADATA_H
#define TYPE_METADATA_H

//~ types /////////////////////////////////////////////////////////////////////

typedef enum GEN_TypeKind
{
    GEN_TypeKind_Null,
    GEN_TypeKind_Basic,
    GEN_TypeKind_Struct,
    GEN_TypeKind_Enum,
} GEN_TypeKind;

typedef struct GEN_TypeInfo GEN_TypeInfo;
struct GEN_TypeInfo
{
    GEN_TypeInfo *next;
    GEN_TypeKind kind;
    MD_Node *node;
    
    // basic
    int size;
    
    // structs
    struct GEN_TypeMember *first_member;
    struct GEN_TypeMember *last_member;
    int member_count;
    
    // enums
    struct GEN_TypeEnumerant *first_enumerant;
    struct GEN_TypeEnumerant *last_enumerant;
    int enumerant_count;
    GEN_TypeInfo *underlying_type;
};

typedef struct GEN_TypeMember GEN_TypeMember;
struct GEN_TypeMember
{
    GEN_TypeMember *next;
    MD_Node *node;
    GEN_TypeInfo *type;
    MD_Node *array_count;
};

typedef struct GEN_TypeEnumerant GEN_TypeEnumerant;
struct GEN_TypeEnumerant
{
    GEN_TypeEnumerant *next;
    MD_Node *node;
    int value;
};

typedef struct GEN_MapInfo GEN_MapInfo;
struct GEN_MapInfo
{
    GEN_MapInfo *next;
    MD_Node *node;
    
    MD_Node *in;
    MD_Node *out;
    
    int is_complete;
    MD_Node *default_val;
    MD_Node *auto_val;
};


//~ helpers ///////////////////////////////////////////////////////////////////
MD_Node* gen_get_md_child_value(MD_Node *parent, MD_String8 child_name);

//~ analyzers /////////////////////////////////////////////////////////////////
void gen_gather_types_and_maps(MD_Node *list);
void gen_check_duplicate_member_names(void);
void gen_equip_basic_type_size(void);
void gen_equip_struct_members(void);
void gen_equip_enum_underlying_type(void);
void gen_equip_enum_members(void);

//~ generators ////////////////////////////////////////////////////////////////
void gen_type_definitions_from_types(FILE *out);
void gen_function_declarations_from_maps(FILE *out);
void gen_type_info_declarations_from_types(FILE *out);
void gen_struct_member_tables_from_types(FILE *out);
void gen_enum_member_tables_from_types(FILE *out);
void gen_type_info_definitions_from_types(FILE *out);


#endif //TYPE_METADATA_H
