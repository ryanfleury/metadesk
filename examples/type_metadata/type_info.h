/*
** Example: type metadata
**
** This is a hand written header to be included into the final program. It
** defines the types used to layout the generated tables of metadata.
**
** This file *does not* get included into the generator itself.
**
*/

#ifndef TYPE_INFO_H
#define TYPE_INFO_H

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
    TypeKind kind;
    char *name;
    int name_length;
    union
    {
        // basic
        int size;
        // struct,enum
        int child_count;
    };
    // struct: TypeInfoMember*
    // enum: TypeInfoMember*
    void *children;
    // enum
    TypeInfo *underlying_type;
};

typedef struct TypeInfoMember TypeInfoMember;
struct TypeInfoMember
{
    char *name;
    int name_length;
    int array_count_member_index;
    TypeInfo *type;
};

typedef struct TypeInfoEnumerant TypeInfoEnumerant;
struct TypeInfoEnumerant
{
    char *name;
    int name_length;
    int value;
};

#endif //TYPE_INFO_H
