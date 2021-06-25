//~ Metadesk Library
// LICENSE AT END OF FILE (MIT).

// TODO List
//
// [ ] Freeing calls, allow hooking the allocator with an arena or
//     something so that someone can use this for work at an application/game's
//     runtime
// [ ] Outputting anything to MD or C code ideally would do something
//     smart with auto-indentation, since some people have wanted readable
//     layout (if they aren't using 4coder with virtual whitespace basically)
// [ ] Split out C-related stuff into helper language layers
// [ ] Helpers for parsing NodeFlags, figuring out which nodes in a set are
//     separated by a semicolon, something like MD_SeekNodeWithFlags(node) -> node ?
// [ ] Escaping characters from strings
// [ ] Fill in more String -> Integer helpers
// [ ] Memory Management Strategy
//     [ ] Gather map of current memory management situation
//     [ ] Reset all memory operation?
//     [ ] Thread context?
//     [ ] Scratch memory?
// [ ] Reference Manual

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

# if _MSC_VER >= 1920
#  define MD_COMPILER_CL_YEAR 2019
# elif _MSC_VER >= 1910
#  define MD_COMPILER_CL_YEAR 2017
# elif _MSC_VER >= 1900
#  define MD_COMPILER_CL_YEAR 2015
# elif _MSC_VER >= 1800
#  define MD_COMPILER_CL_YEAR 2013
# elif _MSC_VER >= 1700
#  define MD_COMPILER_CL_YEAR 2012
# elif _MSC_VER >= 1600
#  define MD_COMPILER_CL_YEAR 2010
# elif _MSC_VER >= 1500
#  define MD_COMPILER_CL_YEAR 2008
# elif _MSC_VER >= 1400
#  define MD_COMPILER_CL_YEAR 2005
# else
#  define MD_COMPILER_CL_YEAR 0
# endif

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

#if defined(__cplusplus)
# define MD_LANG_CPP 1
#else
# define MD_LANG_C 1
#endif

// zeroify

#if !defined(MD_ARCH_32BIT)
# define MD_ARCH_32BIT 0
#endif
#if !defined(MD_ARCH_64BIT)
# define MD_ARCH_64BIT 0
#endif
#if !defined(MD_ARCH_X64)
# define MD_ARCH_X64 0
#endif
#if !defined(MD_ARCH_X86)
# define MD_ARCH_X86 0
#endif
#if !defined(MD_ARCH_ARM64)
# define MD_ARCH_ARM64 0
#endif
#if !defined(MD_ARCH_ARM32)
# define MD_ARCH_ARM32 0
#endif
#if !defined(MD_COMPILER_CL)
# define MD_COMPILER_CL 0
#endif
#if !defined(MD_COMPILER_GCC)
# define MD_COMPILER_GCC 0
#endif
#if !defined(MD_COMPILER_CLANG)
# define MD_COMPILER_CLANG 0
#endif
#if !defined(MD_OS_WINDOWS)
# define MD_OS_WINDOWS 0
#endif
#if !defined(MD_OS_LINUX)
# define MD_OS_LINUX 0
#endif
#if !defined(MD_OS_MAC)
# define MD_OS_MAC 0
#endif
#if !defined(MD_LANG_C)
# define MD_LANG_C 0
#endif
#if !defined(MD_LANG_CPP)
# define MD_LANG_CPP 0
#endif

#define MD_FUNCTION
#define MD_GLOBAL static

#if MD_LANG_CPP
# define MD_ZERO_STRUCT {}
#else
# define MD_ZERO_STRUCT {0}
#endif

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

typedef MD_u32 MD_MatchFlags;
enum
{
    MD_MatchFlag_CaseInsensitive  = (1<<0),
    MD_MatchFlag_RightSideSloppy  = (1<<1),
    MD_MatchFlag_FindLast         = (1<<2),
    MD_MatchFlag_SlashInsensitive = (1<<3),
    MD_MatchFlag_Tags             = (1<<4),
    MD_MatchFlag_TagArguments     = (1<<5),
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
    // NOTE(rjf): Must be kept in sync with MD_StringFromNodeKind.
    
    MD_NodeKind_Nil,
    MD_NodeKind_File,
    MD_NodeKind_List,
    MD_NodeKind_Reference,
    // TODO(allen): Proposal names for "Label"
    //  String, Value, Content, Center, Trunk, Branch,
    //  Cell, Entity, Node, Code
    MD_NodeKind_Label,
    MD_NodeKind_Tag,
    MD_NodeKind_ErrorMarker,
    MD_NodeKind_COUNT,
}
MD_NodeKind;

typedef MD_u64 MD_NodeFlags;
#define MD_NodeFlag_AfterFromBefore(f) ((f) << 1)
enum
{
    // NOTE(rjf): Must be kept in sync with MD_StringListFromNodeFlags.
    
    // NOTE(rjf): Because of MD_NodeFlag_AfterFromBefore, it is *required* that
    // every single pair of "Before*" or "After*" flags be in the correct order
    // which is that the Before* flag comes first, and the After* flag comes
    // immediately after (After* being the more significant bit).
    
    MD_NodeFlag_ParenLeft               = (1<<0),
    MD_NodeFlag_ParenRight              = (1<<1),
    MD_NodeFlag_BracketLeft             = (1<<2),
    MD_NodeFlag_BracketRight            = (1<<3),
    MD_NodeFlag_BraceLeft               = (1<<4),
    MD_NodeFlag_BraceRight              = (1<<5),
    
    MD_NodeFlag_BeforeSemicolon         = (1<<6),
    MD_NodeFlag_AfterSemicolon          = (1<<7),
    
    MD_NodeFlag_BeforeComma             = (1<<8),
    MD_NodeFlag_AfterComma              = (1<<9),
    
    MD_NodeFlag_StringSingleQuote       = (1<<10),
    MD_NodeFlag_StringDoubleQuote       = (1<<13),
    MD_NodeFlag_StringTick              = (1<<15),
    MD_NodeFlag_StringTripletSingleQuote= (1<<16),
    MD_NodeFlag_StringTripletDoubleQuote= (1<<18),
    MD_NodeFlag_StringTripletTick       = (1<<20),
    
    MD_NodeFlag_Numeric                 = (1<<22),
    MD_NodeFlag_Identifier              = (1<<23),
    MD_NodeFlag_StringLiteral           = (1<<24),
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

typedef struct MD_MapKey MD_MapKey;
struct MD_MapKey
{
    MD_u64 hash;
    MD_u64 size;
    void *ptr;
};

typedef struct MD_MapSlot MD_MapSlot;
struct MD_MapSlot
{
    MD_MapSlot *next;
    MD_MapKey key;
    void *val;
};

typedef struct MD_MapBucket MD_MapBucket;
struct MD_MapBucket
{
    MD_MapSlot *first;
    MD_MapSlot *last;
};

typedef struct MD_Map MD_Map;
struct MD_Map
{
    MD_MapBucket *buckets;
    MD_u64 bucket_count;
};

//~ Token kinds.

typedef enum MD_TokenKind
{
    MD_TokenKind_Nil,
    
    MD_TokenKind_RegularMin,
    MD_TokenKind_Identifier,
    MD_TokenKind_NumericLiteral,
    MD_TokenKind_StringLiteralSingleQuote,
    MD_TokenKind_StringLiteralSingleQuoteTriplet,
    MD_TokenKind_StringLiteralDoubleQuote,
    MD_TokenKind_StringLiteralDoubleQuoteTriplet,
    MD_TokenKind_StringLiteralTick,
    MD_TokenKind_StringLiteralTickTriplet,
    MD_TokenKind_Symbol,
    MD_TokenKind_RegularMax,
    
    MD_TokenKind_Comment,
    
    MD_TokenKind_WhitespaceMin,
    MD_TokenKind_Whitespace,
    MD_TokenKind_Newline,
    MD_TokenKind_WhitespaceMax,
    
    // Character outside currently supported encodings
    MD_TokenKind_BadCharacter,
    
    MD_TokenKind_COUNT,
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

typedef MD_u32 MD_ParseSetFlags;
enum
{
    MD_ParseSetFlag_Paren    = (1<<0),
    MD_ParseSetFlag_Brace    = (1<<1),
    MD_ParseSetFlag_Bracket  = (1<<2),
    MD_ParseSetFlag_Implicit = (1<<3),
};

typedef struct MD_ParseCtx MD_ParseCtx;
struct MD_ParseCtx
{
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
};

//~ Command line parsing helper types.

typedef struct MD_CommandLineOption MD_CommandLineOption;
struct MD_CommandLineOption
{
    MD_CommandLineOption *next;
    MD_String8 name;
    MD_String8List values;
};

typedef struct MD_CommandLine MD_CommandLine;
struct MD_CommandLine
{
    MD_String8List arguments;
    MD_String8List inputs;
    MD_CommandLineOption *first_option;
    MD_CommandLineOption *last_option;
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

//~ Memory Operations
MD_FUNCTION void MD_MemoryZero(void *memory, MD_u64 size);
MD_FUNCTION void MD_MemoryCopy(void *dst, void *src, MD_u64 size);

MD_FUNCTION void* MD_AllocZero(MD_u64 size);
#define MD_PushArray(T,c) (T*)MD_AllocZero(sizeof(T)*(c))
// NOTE(rjf): Right now, both calls just automatically zero their memory,
// but I'm explicitly splitting this out to ensure that we don't accidentally
// assume that we have zeroed memory incorrectly in the future (when our
// allocation approach changes).
#define MD_PushArrayZero(T,c) (T*)MD_AllocZero(sizeof(T)*(c))

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
# define MD_S8Lit(s)        (MD_String8){(MD_u8 *)(s), sizeof(s)-1}
#elif MD_LANG_CPP
# define MD_S8Lit(s)        MD_S8((MD_u8*)(s), sizeof(s) - 1)
#endif

MD_FUNCTION MD_String8     MD_S8Range(MD_u8 *str, MD_u8 *opl);

MD_FUNCTION MD_String8     MD_StringSubstring(MD_String8 str, MD_u64 min, MD_u64 max);
MD_FUNCTION MD_String8     MD_StringSkip(MD_String8 str, MD_u64 min);
MD_FUNCTION MD_String8     MD_StringChop(MD_String8 str, MD_u64 nmax);
MD_FUNCTION MD_String8     MD_StringPrefix(MD_String8 str, MD_u64 size);
MD_FUNCTION MD_String8     MD_StringSuffix(MD_String8 str, MD_u64 size);

MD_FUNCTION MD_b32         MD_StringMatch(MD_String8 a, MD_String8 b, MD_MatchFlags flags);
MD_FUNCTION MD_u64         MD_FindSubstring(MD_String8 str, MD_String8 substring,
                                            MD_u64 start_pos, MD_MatchFlags flags);

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
MD_FUNCTION MD_String8     MD_JoinStringList(MD_String8List list, MD_String8 separator);
MD_FUNCTION MD_i64         MD_I64FromString(MD_String8 string, MD_u32 radix);
MD_FUNCTION MD_f64         MD_F64FromString(MD_String8 string);
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

//~ Map Table Data Structure
MD_FUNCTION MD_u64 MD_HashString(MD_String8 string);
MD_FUNCTION MD_u64 MD_HashPointer(void *p);

MD_FUNCTION MD_Map      MD_MapMakeBucketCount(MD_u64 bucket_count);
MD_FUNCTION MD_Map      MD_MapMake(void);
MD_FUNCTION MD_MapKey   MD_MapKeyStr(MD_String8 string);
MD_FUNCTION MD_MapKey   MD_MapKeyPtr(void *ptr);
MD_FUNCTION MD_MapSlot* MD_MapLookup(MD_Map *map, MD_MapKey key);
MD_FUNCTION MD_MapSlot* MD_MapScan(MD_MapSlot *first_slot, MD_MapKey key);
MD_FUNCTION MD_MapSlot* MD_MapInsert(MD_Map *map, MD_MapKey key, void *val);
MD_FUNCTION MD_MapSlot* MD_MapOverwrite(MD_Map *map, MD_MapKey key, void *val);

//~ Parsing

MD_FUNCTION MD_NodeFlags   MD_NodeFlagsFromTokenKind(MD_TokenKind kind);

MD_FUNCTION MD_b32         MD_TokenKindIsWhitespace(MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_TokenKindIsComment(MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_TokenKindIsRegular(MD_TokenKind kind);

MD_FUNCTION void           MD_PushNodeError(MD_ParseCtx *ctx, MD_Node *node, MD_MessageKind kind, MD_String8 str);
MD_FUNCTION void           MD_PushNodeErrorF(MD_ParseCtx *ctx, MD_Node *node, MD_MessageKind kind, char *fmt, ...);
MD_FUNCTION void           MD_PushTokenError(MD_ParseCtx *ctx, MD_Token token, MD_MessageKind kind, MD_String8 str);
MD_FUNCTION void           MD_PushTokenErrorF(MD_ParseCtx *ctx, MD_Token token, MD_MessageKind kind, char *fmt, ...);

MD_FUNCTION MD_ParseCtx    MD_Parse_InitializeCtx(MD_String8 filename, MD_String8 contents);
MD_FUNCTION void           MD_Parse_Bump(MD_ParseCtx *ctx, MD_Token token);
MD_FUNCTION void           MD_Parse_BumpNext(MD_ParseCtx *ctx);
MD_FUNCTION MD_Token       MD_Parse_LexNext(MD_ParseCtx *ctx);
MD_FUNCTION MD_Token       MD_Parse_PeekSkipSome(MD_ParseCtx *ctx, MD_TokenGroups skip_groups);
MD_FUNCTION MD_b32         MD_Parse_Require(MD_ParseCtx *ctx, MD_String8 string, MD_TokenKind kind);
MD_FUNCTION MD_b32         MD_Parse_RequireKind(MD_ParseCtx *ctx, MD_TokenKind kind, MD_Token *out_token);

MD_FUNCTION void           MD_Parse_Set(MD_ParseCtx *ctx, MD_Node *root,
                                        MD_ParseSetFlags flags);

MD_FUNCTION MD_ParseResult MD_ParseOneNodeFromCtx(MD_ParseCtx *ctx);

MD_FUNCTION MD_ParseResult MD_ParseOneNode(MD_String8 filename, MD_String8 contents);
MD_FUNCTION MD_ParseResult MD_ParseWholeString(MD_String8 filename, MD_String8 contents);
MD_FUNCTION MD_ParseResult MD_ParseWholeFile(MD_String8 filename);

//~ Location Conversion
MD_FUNCTION MD_CodeLoc MD_CodeLocFromFileBaseOff(MD_String8 filename, MD_u8 *base, MD_u8 *off);
MD_FUNCTION MD_CodeLoc MD_CodeLocFromNode(MD_Node *node);

//~ Tree/List Building
MD_FUNCTION MD_b32   MD_NodeIsNil(MD_Node *node);
MD_FUNCTION MD_Node *MD_NilNode(void);
MD_FUNCTION MD_Node *MD_MakeNode(MD_NodeKind kind, MD_String8 string,
                                 MD_String8 whole_string, MD_u8 *at);
MD_FUNCTION void     MD_PushSibling(MD_Node **first, MD_Node **last, MD_Node *new_sibling);
MD_FUNCTION void     MD_PushChild(MD_Node *parent, MD_Node *new_child);
MD_FUNCTION void     MD_PushTag(MD_Node *node, MD_Node *tag);
MD_FUNCTION MD_Node *MD_PushReference(MD_Node *list, MD_Node *target);

//~ Introspection Helpers
MD_FUNCTION MD_Node *  MD_NodeFromString(MD_Node *first, MD_Node *last, MD_String8 string);
MD_FUNCTION MD_Node *  MD_NodeFromIndex(MD_Node *first, MD_Node *last, int n);
MD_FUNCTION int        MD_IndexFromNode(MD_Node *node);
MD_FUNCTION MD_Node *  MD_RootFromNode(MD_Node *node);
MD_FUNCTION MD_Node *  MD_NextNodeSibling(MD_Node *last, MD_String8 string);
MD_FUNCTION MD_Node *  MD_ChildFromString(MD_Node *node, MD_String8 child_string);
MD_FUNCTION MD_Node *  MD_TagFromString(MD_Node *node, MD_String8 tag_string);
MD_FUNCTION MD_Node *  MD_ChildFromIndex(MD_Node *node, int n);
MD_FUNCTION MD_Node *  MD_TagFromIndex(MD_Node *node, int n);
MD_FUNCTION MD_Node *  MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, int n);
MD_FUNCTION MD_Node *  MD_TagArgFromString(MD_Node *node, MD_String8 tag_string, MD_String8 arg_string);
MD_FUNCTION MD_b32     MD_NodeHasTag(MD_Node *node, MD_String8 tag_string);
MD_FUNCTION MD_i64     MD_ChildCountFromNode(MD_Node *node);
MD_FUNCTION MD_i64     MD_TagCountFromNode(MD_Node *node);
MD_FUNCTION MD_Node *  MD_Deref(MD_Node *node);
MD_FUNCTION MD_Node *  MD_SeekNodeWithFlags(MD_Node *start, MD_NodeFlags one_past_last_flags);

// NOTE(rjf): For-Loop Helpers
#define MD_EachNode(it, first) MD_Node *it = (first); !MD_NodeIsNil(it); it = it->next
#define MD_EachNodeRef(it, first) MD_Node *it##_r = (first), *it = MD_Deref(it##_r); \
!MD_NodeIsNil(it##_r); \
it##_r = it##_r->next, it = MD_Deref(it##_r)

//~ Error/Warning Helpers
MD_FUNCTION void MD_Message(FILE *out, MD_CodeLoc loc, MD_MessageKind kind, MD_String8 str);
MD_FUNCTION void MD_MessageF(FILE *out, MD_CodeLoc loc, MD_MessageKind kind, char *fmt, ...);
MD_FUNCTION void MD_NodeMessage(FILE *out, MD_Node *node, MD_MessageKind kind, MD_String8 str);
MD_FUNCTION void MD_NodeMessageF(FILE *out, MD_Node *node, MD_MessageKind kind, char *fmt, ...);

//~ Tree Comparison/Verification
MD_FUNCTION MD_b32 MD_NodeMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags);
MD_FUNCTION MD_b32 MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_MatchFlags node_flags);

//~ Generation
MD_FUNCTION void MD_OutputTree(FILE *file, MD_Node *node);

//~ Command Line Argument Helper
MD_FUNCTION MD_String8List MD_StringListFromArgCV(int argument_count, char **arguments);
MD_FUNCTION MD_CommandLine MD_CommandLineFromOptions(MD_String8List options);
MD_FUNCTION MD_String8List MD_CommandLineOptionValues(MD_CommandLine cmdln, MD_String8 name);
MD_FUNCTION MD_b32 MD_CommandLineOptionPassed(MD_CommandLine cmdln, MD_String8 name);
MD_FUNCTION MD_i64 MD_CommandLineOptionI64(MD_CommandLine cmdln, MD_String8 name);

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
