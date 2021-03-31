//~ Metadesk Library
// LICENSE AT END OF FILE (MIT).

// TODO List
//
// - Expression/Type helper
//   - Parsing things as a type, getting basic type info
// - Freeing calls, allow hooking the allocator with an arena or
//   something so that someone can use this for work at an application/game's
//   runtime
// - Outputting anything to MD or C code ideally would do something
//   smart with auto-indentation, since some people have wanted readable
//   layout (if they aren't using 4coder with virtual whitespace basically)
// - Split out C-related stuff into helper language layers
// - Helpers for parsing NodeFlags, figuring out which nodes in a set are
//   separated by a semicolon, something like MD_SeekNodeWithFlags(node) -> node ?
// - Escaping characters from strings

// NOTE(allen): "Plugin" functionality
//
// MD_b32     MD_IMPL_FileIterIncrement(MD_FileIter*, MD_String8, MD_FileInfo*) - optional
// void*      MD_IMPL_Alloc(MD_u64)                                             - required
//
// TODO(allen): Commentary about this system somewhere easy to discover when
// you go digging.

#ifndef MD_H
#define MD_H

// NOTE(rjf): Compiler cracking from the 4th dimension

#if defined(_MSC_VER)

# define MD_COMPILER_CL 1

# if defined(_WIN32)
#  define MD_OS_WINDOWS 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(_M_AMD64)
#  define MD_ARCH_X64 1
# elif defined(_M_IX86)
#  define MD_ARCH_X86 1
# elif defined(_M_ARM64)
#  define MD_ARCH_ARM64 1
# elif defined(_M_ARM)
#  define MD_ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#if _MSC_VER >= 1920
#define MD_COMPILER_CL_YEAR 2019
#elif _MSC_VER >= 1910
#define MD_COMPILER_CL_YEAR 2017
#elif _MSC_VER >= 1900
#define MD_COMPILER_CL_YEAR 2015
#elif _MSC_VER >= 1800
#define MD_COMPILER_CL_YEAR 2013
#elif _MSC_VER >= 1700
#define MD_COMPILER_CL_YEAR 2012
#elif _MSC_VER >= 1600
#define MD_COMPILER_CL_YEAR 2010
#elif _MSC_VER >= 1500
#define MD_COMPILER_CL_YEAR 2008
#elif _MSC_VER >= 1400
#define MD_COMPILER_CL_YEAR 2005
#else
#define MD_COMPILER_CL_YEAR 0
#endif

#elif defined(__clang__)

# define MD_COMPILER_CLANG 1

# if defined(__APPLE__) && defined(__MACH__)
#  define MD_OS_MAC 1
# elif defined(__gnu_linux__)
#  define MD_OS_LINUX 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define MD_ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define MD_ARCH_X86 1
# elif defined(__aarch64__)
#  define MD_ARCH_ARM64 1
# elif defined(__arm__)
#  define MD_ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#elif defined(__GNUC__) || defined(__GNUG__)

# define MD_COMPILER_GCC 1

# if defined(__gnu_linux__)
#  define MD_OS_LINUX 1
# else
#  error This compiler/platform combo is not supported yet
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define MD_ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define MD_ARCH_X86 1
# elif defined(__aarch64__)
#  define MD_ARCH_ARM64 1
# elif defined(__arm__)
#  define MD_ARCH_ARM32 1
# else
#  error architecture not supported yet
# endif

#else
# error This compiler is not supported yet
#endif

#if defined(MD_ARCH_X64)
# define MD_ARCH_64BIT 1
#elif defined(MD_ARCH_X86)
# define MD_ARCH_32BIT 1
#endif

// NOTE(allen): Review @rjf; Building in C++
// Added language cracking. Handy for a few pesky problems that can't be solved
// strictly within the intersection of C & C++
#if defined(__cplusplus)
# define MD_LANG_CPP 1
#else
# define MD_LANG_C 1
#endif

// zeroify

#if !defined(MD_ARCH_32BIT)
#define MD_ARCH_32BIT 0
#endif
#if !defined(MD_ARCH_64BIT)
#define MD_ARCH_64BIT 0
#endif
#if !defined(MD_ARCH_X64)
#define MD_ARCH_X64 0
#endif
#if !defined(MD_ARCH_X86)
#define MD_ARCH_X86 0
#endif
#if !defined(MD_ARCH_ARM64)
#define MD_ARCH_ARM64 0
#endif
#if !defined(MD_ARCH_ARM32)
#define MD_ARCH_ARM32 0
#endif
#if !defined(MD_COMPILER_CL)
#define MD_COMPILER_CL 0
#endif
#if !defined(MD_COMPILER_GCC)
#define MD_COMPILER_GCC 0
#endif
#if !defined(MD_COMPILER_CLANG)
#define MD_COMPILER_CLANG 0
#endif
#if !defined(MD_OS_WINDOWS)
#define MD_OS_WINDOWS 0
#endif
#if !defined(MD_OS_LINUX)
#define MD_OS_LINUX 0
#endif
#if !defined(MD_OS_MAC)
#define MD_OS_MAC 0
#endif
#if !defined(MD_LANG_C)
#define MD_LANG_C 0
#endif
#if !defined(MD_LANG_CPP)
#define MD_LANG_CPP 0
#endif

#define MD_FUNCTION
#define MD_GLOBAL static

#if MD_LANG_CPP
# define MD_ZERO_STRUCT {}
#else
# define MD_ZERO_STRUCT {0}
#endif

// NOTE(allen): Review @rjf; Building in C++
// In order to link to C functions from C++ code, we need to mark them as using
// C linkage. In particular I mean FindFirstFileA, FindNextFileA right now.
// We don't necessarily need to apply this to the DD functions if the user is
// building from source, so I haven't done that.

#if MD_LANG_C
# define MD_C_LINKAGE_BEGIN
# define MD_C_LINKAGE_END
#else
# define MD_C_LINKAGE_BEGIN extern "C"{
# define MD_C_LINKAGE_END }
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
// NOTE(allen): Review @rjf; Building in C++
// In C++ compiler I have to include this to get memset and memcpy to compile
#include <string.h>

typedef int8_t   MD_i8;
typedef int16_t  MD_i16;
typedef int32_t  MD_i32;
typedef int64_t  MD_i64;
typedef uint8_t  MD_u8;
typedef uint16_t MD_u16;
typedef uint32_t MD_u32;
typedef uint64_t MD_u64;
typedef int8_t   MD_b8;
typedef int16_t  MD_b16;
typedef int32_t  MD_b32;
typedef int64_t  MD_b64;
typedef float    MD_f32;
typedef double   MD_f64;

//~ Basic UTF-8 string types.

typedef struct MD_String8 MD_String8;
struct MD_String8
{
    MD_u8 *str;
    MD_u64 size;
};

typedef struct MD_String16 MD_String16;
struct MD_String16
{
    MD_u16 *str;
    MD_u64 size;
};

typedef struct MD_String32 MD_String32;
struct MD_String32
{
    MD_u32 *str;
    MD_u64 size;
};

typedef struct MD_String8Node MD_String8Node;
struct MD_String8Node
{
    MD_String8Node *next;
    MD_String8 string;
};

typedef struct MD_String8List MD_String8List;
struct MD_String8List
{
    MD_u64 node_count;
    MD_u64 total_size;
    MD_String8Node *first;
    MD_String8Node *last;
};

typedef MD_u32 MD_StringMatchFlags;
enum
{
    MD_StringMatchFlag_CaseInsensitive  = (1<<0),
    MD_StringMatchFlag_RightSideSloppy  = (1<<1),
    MD_StringMatchFlag_FindLast         = (1<<2),
    MD_StringMatchFlag_SlashInsensitive = (1<<3),
};

typedef struct MD_UnicodeConsume MD_UnicodeConsume;
struct MD_UnicodeConsume
{
    MD_u32 codepoint;
    MD_u32 advance;
};

typedef enum MD_WordStyle
{
    MD_WordStyle_UpperCamelCase,
    MD_WordStyle_LowerCamelCase,
    MD_WordStyle_UpperCase,
    MD_WordStyle_LowerCase,
}
MD_WordStyle;

//~ Node types that are used to build all ASTs.

typedef enum MD_NodeKind
{
    MD_NodeKind_Nil,
    MD_NodeKind_File,
    MD_NodeKind_List,
    MD_NodeKind_Reference,
    MD_NodeKind_Label,
    MD_NodeKind_Tag,
    MD_NodeKind_ErrorMarker,
    MD_NodeKind_MAX,
}
MD_NodeKind;

typedef MD_u32 MD_NodeFlags;
enum
{
    MD_NodeFlag_ParenLeft        = (1<<0),
    MD_NodeFlag_ParenRight       = (1<<1),
    MD_NodeFlag_BracketLeft      = (1<<2),
    MD_NodeFlag_BracketRight     = (1<<3),
    MD_NodeFlag_BraceLeft        = (1<<4),
    MD_NodeFlag_BraceRight       = (1<<5),
    
    MD_NodeFlag_BeforeSemicolon  = (1<<6),
    MD_NodeFlag_BeforeComma      = (1<<7),
    
    MD_NodeFlag_AfterSemicolon   = (1<<8),
    MD_NodeFlag_AfterComma       = (1<<9),
    
    MD_NodeFlag_Numeric          = (1<<10),
    MD_NodeFlag_Identifier       = (1<<11),
    MD_NodeFlag_StringLiteral    = (1<<12),
    MD_NodeFlag_CharLiteral      = (1<<13),
};

typedef MD_u32 MD_NodeMatchFlags;
enum
{
    MD_NodeMatchFlag_Tags         = (1<<0),
    MD_NodeMatchFlag_TagArguments = (1<<1),
};

typedef struct MD_Node MD_Node;
struct MD_Node
{
    // Tree relationship data.
    MD_Node *next;
    MD_Node *prev;
    MD_Node *parent;
    MD_Node *first_child;
    MD_Node *last_child;
    
    // Tag list.
    MD_Node *first_tag;
    MD_Node *last_tag;
    
    // Node info.
    MD_NodeKind kind;
    MD_NodeFlags flags;
    MD_String8 string;
    MD_String8 whole_string;
    MD_u64 string_hash;
    
    // Comments.
    MD_String8 comment_before;
    MD_String8 comment_after;
    
    // Source code location information.
    MD_String8 filename;
    MD_u8 *file_contents;
    MD_u8 *at;
    
    // Reference.
    MD_Node *ref_target;
};

//~ Code Location Info.

typedef struct MD_CodeLoc MD_CodeLoc;
struct MD_CodeLoc
{
    MD_String8 filename;
    int line;
    int column;
};

//~ String-To-Ptr and Ptr-To-Ptr tables

typedef enum MD_MapCollisionRule
{
    MD_MapCollisionRule_Chain,
    MD_MapCollisionRule_Overwrite,
}
MD_MapCollisionRule;

typedef struct MD_MapSlot MD_MapSlot;
struct MD_MapSlot
{
    MD_MapSlot *next;
    MD_u64 hash;
    void *key;
    void *value;
};

typedef struct MD_Map MD_Map;
struct MD_Map
{
    MD_u64 table_size;
    MD_MapSlot **table;
};

//~ Token kinds.

typedef enum MD_TokenKind
{
    MD_TokenKind_Nil,
    
    MD_TokenKind_RegularMin,
    
    // A group of characters that begins with an underscore or alphabetic character,
    // and consists of numbers, alphabetic characters, or underscores after that.
    MD_TokenKind_Identifier,
    
    // A group of characters beginning with a numeric character or a '-', and then
    // consisting of only numbers, alphabetic characters, or '.'s after that.
    MD_TokenKind_NumericLiteral,
    
    // A group of arbitrary characters, grouped together by a " character, OR by a
    // """ symbol at the beginning and end of the group. String literals beginning with
    // " are to only be specified on a single line, but """ strings can exist across
    // many lines.
    MD_TokenKind_StringLiteral,
    
    // A group of arbitrary characters, grouped together by a ' character at the beginning,
    // and a ' character at the end.
    MD_TokenKind_CharLiteral,
    
    // A group of symbolic characters, where symbolic characters means any of the following:
    // ~!@#$%^&*()-+=[{]}:;<>,./?|\
   //
    // Groups of multiple characters are only allowed in specific circumstances. Most of these
    // are only 1 character long, but some groups are allowed:
    //
    // "<<", ">>", "<=", ">=", "+=", "-=", "*=", "/=", "::", ":=", "==", "&=", "|=", "->"
    MD_TokenKind_Symbol,
    
    MD_TokenKind_RegularMax,
    
    MD_TokenKind_Comment,
    
    MD_TokenKind_WhitespaceMin,
    MD_TokenKind_Whitespace,
    MD_TokenKind_Newline,
    MD_TokenKind_WhitespaceMax,
    
    MD_TokenKind_BadCharacter,
    // Character outside currently supported encodings
    
    MD_TokenKind_MAX,
}
MD_TokenKind;

//~ Token type.
typedef struct MD_Token MD_Token;
struct MD_Token
{
    MD_TokenKind kind;
    MD_String8 string;
    MD_String8 outer_string;
};

//~ Token groups.
typedef MD_u32 MD_TokenGroups;
enum{
    MD_TokenGroup_Comment    = (1 << 0),
    MD_TokenGroup_Whitespace = (1 << 1),
    MD_TokenGroup_Regular    = (1 << 2)
};

//~ Parsing State

typedef enum MD_MessageKind
{
    MD_MessageKind_None,
    MD_MessageKind_Warning,
    MD_MessageKind_Error,
    MD_MessageKind_CatastrophicError,
}
MD_MessageKind;

typedef struct MD_Error MD_Error;
struct MD_Error
{
    MD_Error *next;
    MD_String8 string;
    MD_Node *node;
    MD_MessageKind kind;
};

typedef struct MD_ParseCtx MD_ParseCtx;
struct MD_ParseCtx
{
    MD_Node *first_root;
    MD_Node *last_root;
    MD_Error *first_error;
    MD_Error *last_error;
    MD_u8 *at;
    MD_String8 filename;
    MD_String8 file_contents;
    MD_MessageKind error_level;
};

typedef struct MD_ParseResult MD_ParseResult;
struct MD_ParseResult
{
    MD_Node *node;
    MD_Error *first_error;
    MD_u64 bytes_parsed;
    MD_Node *namespaces;
};

//~ Expression and Type-Expression parser helper types.

typedef enum MD_ExprKind
{
    // VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
    // it is kept in-sync with the _MD_MetadataFromExprKind function.
    
    MD_ExprKind_Nil,
    
    // NOTE(rjf): Atom
    MD_ExprKind_Atom,
    
    // NOTE(rjf): Access
    MD_ExprKind_Dot,
    MD_ExprKind_Arrow,
    MD_ExprKind_Call,
    MD_ExprKind_Subscript,
    MD_ExprKind_Dereference,
    MD_ExprKind_Reference,
    
    // NOTE(rjf): Arithmetic
    MD_ExprKind_Add,
    MD_ExprKind_Subtract,
    MD_ExprKind_Multiply,
    MD_ExprKind_Divide,
    MD_ExprKind_Mod,
    
    // NOTE(rjf): Comparison
    MD_ExprKind_IsEqual,
    MD_ExprKind_IsNotEqual,
    MD_ExprKind_LessThan,
    MD_ExprKind_GreaterThan,
    MD_ExprKind_LessThanEqualTo,
    MD_ExprKind_GreaterThanEqualTo,
    
    // NOTE(rjf): Bools
    MD_ExprKind_BoolAnd,
    MD_ExprKind_BoolOr,
    MD_ExprKind_BoolNot,
    
    // NOTE(rjf): Bitwise
    MD_ExprKind_BitAnd,
    MD_ExprKind_BitOr,
    MD_ExprKind_BitNot,
    MD_ExprKind_BitXor,
    MD_ExprKind_LeftShift,
    MD_ExprKind_RightShift,
    
    // NOTE(rjf): Unary numeric
    MD_ExprKind_Negative,
    
    // NOTE(rjf): Type
    MD_ExprKind_Pointer,
    MD_ExprKind_Array,
    MD_ExprKind_Volatile,
    MD_ExprKind_Const,
    
    MD_ExprKind_MAX,
}
MD_ExprKind;

typedef MD_i32 MD_ExprPrec;

typedef struct MD_Expr MD_Expr;
struct MD_Expr
{
    MD_Node *node;
    MD_ExprKind kind;
    MD_Expr *parent;
    MD_Expr *sub[2];
};

//~ Command line parsing helper types.
typedef struct MD_CommandLine MD_CommandLine;
struct MD_CommandLine
{
    // TODO(rjf): Linked-list vs. array?
    MD_String8 *arguments;
    int argument_count;
};

//~ File system access types.

typedef MD_u32 MD_FileFlags;
enum
{
    MD_FileFlag_Directory = (1<<0),
};

typedef struct MD_FileInfo MD_FileInfo;
struct MD_FileInfo
{
    MD_FileFlags flags;
    MD_String8 filename;
    MD_u64 file_size;
};

typedef struct MD_FileIter MD_FileIter;
struct MD_FileIter
{
    // This is opaque state to store OS-specific file-system iteration data.
    MD_u64 state[2];
};

//~ Basic Utilities
#define MD_Assert(c) if (!(c)) { *(volatile MD_u64 *)0 = 0; }
#define MD_StaticAssert(c,label) MD_u8 MD_static_assert_##label[(c)?(1):(-1)]
#define MD_ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

//~ Characters
MD_FUNCTION MD_b32 MD_CharIsAlpha(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsAlphaUpper(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsAlphaLower(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsDigit(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsSymbol(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsReservedSymbol(MD_u8 c);
MD_FUNCTION MD_b32 MD_CharIsSpace(MD_u8 c);
MD_FUNCTION MD_u8  MD_CharToUpper(MD_u8 c);
MD_FUNCTION MD_u8  MD_CharToLower(MD_u8 c);
MD_FUNCTION MD_u8  MD_CorrectSlash(MD_u8 c);

//~ Strings
MD_FUNCTION MD_String8     MD_S8(MD_u8 *str, MD_u64 size);
#define MD_S8CString(s)    MD_S8((MD_u8 *)(s), MD_CalculateCStringLength(s))

#if MD_LANG_C
#define MD_S8Lit(s)        (MD_String8){(MD_u8 *)(s), sizeof(s)-1}
#elif MD_LANG_CPP
#define MD_S8Lit(s)        MD_S8((MD_u8*)(s), sizeof(s) - 1)
#endif

MD_FUNCTION MD_String8     MD_S8Range(MD_u8 *str, MD_u8 *opl);

MD_FUNCTION MD_String8     MD_StringSubstring(MD_String8 str, MD_u64 min, MD_u64 max);
MD_FUNCTION MD_String8     MD_StringSkip(MD_String8 str, MD_u64 min);
MD_FUNCTION MD_String8     MD_StringChop(MD_String8 str, MD_u64 nmax);
MD_FUNCTION MD_String8     MD_StringPrefix(MD_String8 str, MD_u64 size);
MD_FUNCTION MD_String8     MD_StringSuffix(MD_String8 str, MD_u64 size);

MD_FUNCTION MD_b32         MD_StringMatch(MD_String8 a, MD_String8 b, MD_StringMatchFlags flags);
MD_FUNCTION MD_u64         MD_FindSubstring(MD_String8 str, MD_String8 substring,
                                            MD_u64 start_pos, MD_StringMatchFlags flags);

MD_FUNCTION MD_String8     MD_ChopExtension(MD_String8 string);
MD_FUNCTION MD_String8     MD_SkipFolder(MD_String8 string);
MD_FUNCTION MD_String8     MD_ExtensionFromPath(MD_String8 string);
MD_FUNCTION MD_String8     MD_FolderFromPath(MD_String8 string);

MD_FUNCTION MD_String8     MD_PushStringCopy(MD_String8 string);
MD_FUNCTION MD_String8     MD_PushStringFV(char *fmt, va_list args);
MD_FUNCTION MD_String8     MD_PushStringF(char *fmt, ...);

#define MD_StringExpand(s) (int)(s).size, (s).str

MD_FUNCTION void           MD_PushStringToList(MD_String8List *list, MD_String8 string);
MD_FUNCTION void           MD_PushStringListToList(MD_String8List *list, MD_String8List *to_push);
MD_FUNCTION MD_String8List MD_SplitString(MD_String8 string, int split_count, MD_String8 *splits);
// TODO(allen): single joiner with MD_StringJoin optional parameter
MD_FUNCTION MD_String8     MD_JoinStringList(MD_String8List list);
MD_FUNCTION MD_String8     MD_JoinStringListWithSeparator(MD_String8List list, MD_String8 separator);
MD_FUNCTION MD_i64         MD_I64FromString(MD_String8 string, MD_u32 radix);
MD_FUNCTION MD_f64         MD_F64FromString(MD_String8 string);
MD_FUNCTION MD_u64         MD_HashString(MD_String8 string);
MD_FUNCTION MD_u64         MD_CalculateCStringLength(char *cstr);

MD_FUNCTION MD_String8     MD_StyledStringFromString(MD_String8 string, MD_WordStyle word_style, MD_String8 separator);

//~ Enum/Flag Strings
MD_FUNCTION MD_String8      MD_StringFromNodeKind(MD_NodeKind kind);
MD_FUNCTION MD_String8List  MD_StringListFromNodeFlags(MD_NodeFlags flags);

//~ Unicode Conversions
MD_FUNCTION MD_UnicodeConsume MD_CodepointFromUtf8(MD_u8 *str, MD_u64 max);
MD_FUNCTION MD_UnicodeConsume MD_CodepointFromUtf16(MD_u16 *str, MD_u64 max);
MD_FUNCTION MD_u32 MD_Utf8FromCodepoint(MD_u8 *out, MD_u32 codepoint);
MD_FUNCTION MD_u32 MD_Utf16FromCodepoint(MD_u16 *out, MD_u32 codepoint);
MD_FUNCTION MD_String8     MD_S8FromS16(MD_String16 str);
MD_FUNCTION MD_String16    MD_S16FromS8(MD_String8 str);
MD_FUNCTION MD_String8     MD_S8FromS32(MD_String32 str);
MD_FUNCTION MD_String32    MD_S32FromS8(MD_String8 str);

//~ String-To-Pointer Table
MD_FUNCTION MD_MapSlot       *MD_StringMap_Lookup(MD_Map *table, MD_String8 string);
MD_FUNCTION MD_b32            MD_StringMap_Insert(MD_Map *table, MD_MapCollisionRule collision_rule, MD_String8 string, void *value);

//~ Pointer-To-Pointer Table
MD_FUNCTION MD_MapSlot       *MD_PtrMap_Lookup(MD_Map *map, void *key);
MD_FUNCTION MD_b32            MD_PtrMap_Insert(MD_Map *map, MD_MapCollisionRule collision_rule, void *key, void *value);

//~ Parsing
MD_FUNCTION MD_b32         MD_TokenKindIsWhitespace(MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_TokenKindIsComment(MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_TokenKindIsRegular(MD_TokenKind kind);
MD_FUNCTION MD_ParseCtx    MD_Parse_InitializeCtx(MD_String8 filename, MD_String8 contents);

MD_FUNCTION void           MD_Parse_Bump(MD_ParseCtx *ctx, MD_Token token);
MD_FUNCTION void           MD_Parse_BumpNext(MD_ParseCtx *ctx);
MD_FUNCTION MD_Token       MD_Parse_LexNext(MD_ParseCtx *ctx);
MD_FUNCTION MD_Token       MD_Parse_PeekSkipSome(MD_ParseCtx *ctx, MD_TokenGroups skip_groups);
MD_FUNCTION MD_b32         MD_Parse_TokenMatch(MD_Token token, MD_String8 string, MD_StringMatchFlags flags);
MD_FUNCTION MD_b32         MD_Parse_Require(MD_ParseCtx *ctx, MD_String8 string, MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_Parse_RequireKind(MD_ParseCtx *ctx, MD_TokenKind kind, MD_Token *out_token);
MD_FUNCTION MD_ParseResult MD_ParseOneNode     (MD_String8 filename, MD_String8 contents);
MD_FUNCTION MD_ParseResult MD_ParseWholeString (MD_String8 filename, MD_String8 contents);
MD_FUNCTION MD_ParseResult MD_ParseWholeFile   (MD_String8 filename);

//~ Location Conversion
MD_FUNCTION MD_CodeLoc MD_CodeLocFromFileOffset(MD_String8 filename, MD_u8 *base, MD_u8 *off);
MD_FUNCTION MD_CodeLoc MD_CodeLocFromNode(MD_Node *node);

//~ Tree/List Building
MD_FUNCTION MD_b32   MD_NodeIsNil(MD_Node *node);
MD_FUNCTION MD_Node *MD_NilNode(void);
MD_FUNCTION MD_Node *MD_MakeNodeFromToken(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_Token token);
MD_FUNCTION MD_Node *MD_MakeNodeFromString(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_String8 string);
MD_FUNCTION void     MD_PushSibling(MD_Node **first, MD_Node **last, MD_Node *parent, MD_Node *new_sibling);
MD_FUNCTION void     MD_PushChild(MD_Node *parent, MD_Node *new_child);
MD_FUNCTION void     MD_PushTag(MD_Node *node, MD_Node *tag);
MD_FUNCTION void     MD_InsertToNamespace(MD_Node *ns, MD_Node *node);

//~ Introspection Helpers
MD_FUNCTION MD_Node *  MD_NodeFromString(MD_Node *first, MD_Node *last, MD_String8 string);
MD_FUNCTION MD_Node *  MD_NodeFromIndex(MD_Node *first, MD_Node *last, int n);
MD_FUNCTION int        MD_IndexFromNode(MD_Node *node);
MD_FUNCTION MD_Node *  MD_NextNodeSibling(MD_Node *last, MD_String8 string);
MD_FUNCTION MD_Node *  MD_ChildFromString(MD_Node *node, MD_String8 child_string);
MD_FUNCTION MD_Node *  MD_TagFromString(MD_Node *node, MD_String8 tag_string);
MD_FUNCTION MD_Node *  MD_ChildFromIndex(MD_Node *node, int n);
MD_FUNCTION MD_Node *  MD_TagFromIndex(MD_Node *node, int n);
MD_FUNCTION MD_Node *  MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, int n);
MD_FUNCTION MD_b32     MD_NodeHasTag(MD_Node *node, MD_String8 tag_string);
MD_FUNCTION MD_i64     MD_ChildCountFromNode(MD_Node *node);
MD_FUNCTION MD_i64     MD_TagCountFromNode(MD_Node *node);
MD_FUNCTION MD_Node *  MD_Deref(MD_Node *node);
// NOTE(rjf): For-Loop Helpers
#define MD_EachNode(it, first) MD_Node *it = (first); !MD_NodeIsNil(it); it = it->next
#define MD_EachNodeRef(it, first) MD_Node *it##_r = (first), *it = MD_Deref(it##_r); \
!MD_NodeIsNil(it##_r); \
it##_r = it##_r->next, it = MD_Deref(it##_r)

//~ Error/Warning Helpers
MD_FUNCTION void MD_NodeMessage(MD_Node *node, MD_MessageKind kind, MD_String8 str);
MD_FUNCTION void MD_NodeMessageF(MD_Node *node, MD_MessageKind kind, char *fmt, ...);

//~ Tree Comparison/Verification
MD_FUNCTION MD_b32 MD_NodeMatch(MD_Node *a, MD_Node *b, MD_StringMatchFlags str_flags, MD_NodeMatchFlags node_flags);
MD_FUNCTION MD_b32 MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_StringMatchFlags str_flags, MD_NodeMatchFlags node_flags);

//~ Expression and Type-Expression Helper
MD_FUNCTION MD_Expr *     MD_NilExpr(void);
MD_FUNCTION MD_b32        MD_ExprIsNil(MD_Expr *expr);
MD_FUNCTION MD_ExprKind   MD_PreUnaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_ExprKind   MD_BinaryExprKindFromNode(MD_Node *node);
MD_FUNCTION MD_ExprPrec   MD_ExprPrecFromExprKind(MD_ExprKind kind);
MD_FUNCTION MD_Expr *     MD_MakeExpr(MD_Node *node, MD_ExprKind kind, MD_Expr *left, MD_Expr *right);
MD_FUNCTION MD_Expr *     MD_ParseAsExpr(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_Expr *     MD_ParseAsType(MD_Node *first, MD_Node *last);
MD_FUNCTION MD_i64        MD_EvaluateExpr_I64(MD_Expr *expr);
MD_FUNCTION MD_f64        MD_EvaluateExpr_F64(MD_Expr *expr);
MD_FUNCTION MD_b32        MD_ExprMatch(MD_Expr *a, MD_Expr *b, MD_StringMatchFlags str_flags);
MD_FUNCTION MD_b32        MD_ExprDeepMatch(MD_Expr *a, MD_Expr *b, MD_StringMatchFlags str_flags);

//~ Generation
MD_FUNCTION void MD_OutputTree(FILE *file, MD_Node *node);

//~ C Language Generation
MD_FUNCTION void MD_OutputTree_C_String(FILE *file, MD_Node *node);
MD_FUNCTION void MD_OutputTree_C_Struct(FILE *file, MD_Node *node);
MD_FUNCTION void MD_OutputTree_C_Decl(FILE *file, MD_Node *node);
MD_FUNCTION void MD_Output_C_DeclByNameAndType(FILE *file, MD_String8 name, MD_Expr *type);
MD_FUNCTION void MD_OutputExpr(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputExpr_C(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputType(FILE *file, MD_Expr *expr);
MD_FUNCTION void MD_OutputType_C_LHS(FILE *file, MD_Expr *type);
MD_FUNCTION void MD_OutputType_C_RHS(FILE *file, MD_Expr *type);

// TODO(allen): needs another pass
//~ Command Line Argument Helper
MD_FUNCTION MD_CommandLine MD_CommandLine_Start(int argument_count, char **arguments);
MD_FUNCTION MD_b32         MD_CommandLine_Flag(MD_CommandLine *cmdln, MD_String8 string);
MD_FUNCTION MD_b32         MD_CommandLine_FlagStrings(MD_CommandLine *cmdln, MD_String8 string, int out_count, MD_String8 *out);
MD_FUNCTION MD_b32         MD_CommandLine_FlagIntegers(MD_CommandLine *cmdln, MD_String8 string, int out_count, MD_i64 *out);
MD_FUNCTION MD_b32         MD_CommandLine_FlagString(MD_CommandLine *cmdln, MD_String8 string, MD_String8 *out);
MD_FUNCTION MD_b32         MD_CommandLine_FlagInteger(MD_CommandLine *cmdln, MD_String8 string, MD_i64 *out);
MD_FUNCTION MD_b32         MD_CommandLine_Increment(MD_CommandLine *cmdln, MD_String8 **string_ptr);

//~ File System
MD_FUNCTION MD_String8  MD_LoadEntireFile(MD_String8 filename);
MD_FUNCTION MD_b32      MD_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info);

#endif // MD_H

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
