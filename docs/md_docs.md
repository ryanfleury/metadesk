////////////////////////////////
//~ Basic Unicode string types.

@struct MD_String8: {
 str: *MD_u8,
 size: MD_u64,
};

@struct MD_String16: {
 str: *MD_u16,
 size: MD_u64,
};

@struct MD_String32: {
 str: *MD_u32,
 size: MD_u64,
};

@struct MD_String8Node: {
 next: MD_String8Node,
 string: MD_String8,
};

@struct MD_String8List: {
 node_count: MD_u64,
 total_size: MD_u64,
 first: *MD_String8Node,
 last: *MD_String8Node,
};

@prefix(MD_StringMatchFlag)
@base_type(MD_u32)
@flags MD_StringMatchFlags: {
 CaseInsensitive,
 RightSideSloppy,
 FindLast,
 SlashInsensitive,
};

@struct MD_UnicodeConsume: {
 codepoint: MD_u32,
 advance: MD_u32,
};

@enum MD_WordStyle: {
 UpperCamelCase,
 LowerCamelCase,
 UpperCase,
 LowerCase,
};

////////////////////////////////
//~ Node types that are used to build all ASTs.

@enum MD_NodeKind: {
    Nil,
    File,
    Namespace,
    Label,
    Tag,
    MAX,
};

@prefix(MD_NodeFlag)
@base_type(MD_u32)
@flags MD_NodeFlags: {
 ParenLeft,
 ParenRight,
 BracketLeft,
 BracketRight,
 BraceLeft,
 BraceRight,

 BeforeSemicolon,
 BeforeComma,

 AfterSemicolon,
 AfterComma,

 Numeric,
 Identifier,
 StringLiteral,
 CharLiteral,
};

@prefix(MD_NodeMatchFlag)
@base_type(MD_u32)
@flags MD_NodeMatchFlags: {
 MD_NodeMatchFlag_Tags,
 MD_NodeMatchFlag_TagArguments,
};

@struct MD_Node: {
 next: *MD_Node,
 prev: *MD_Node,
 parent: *MD_Node,
 first_child: *MD_Node,
 last_child: *MD_Node,

 // Tag list.
 first_tag: *MD_Node,
 last_tag: *MD_Node,

 // Node info.
 kind: MD_NodeKind,
 flags: MD_NodeFlags,
 string: MD_String8,
 whole_string: MD_String8,
 string_hash: MD_u64,

 // Comments.
 comment_before: MD_String8,
 comment_after: MD_String8,
 
 // Source code location information.
 filename: MD_String8,
 file_contents: *MD_u8,
 at: *MD_u8,
};

////////////////////////////////
//~ Code Location Info.

@struct MD_CodeLoc: {
 filename: MD_String8,
 line: MD_u32,
 column: MD_u32,
};

////////////////////////////////
//~ Message Levels

@enum MD_MessageKind: {
 None,
 Warning,
 Error,
}

////////////////////////////////
//~ String-To-Node table

@enum MD_MapCollisionRule: {
 Chain,
 Overwrite,
}

@struct MD_MapSlot: {
 next: *MD_MapSlot,
 hash: MD_u64,
 key: *void;
 value: *void;
};

@struct MD_Map: {
 table_size: MD_u64,
 table: **MD_MapSlot,
};

////////////////////////////////
//~ Tokens

@enum MD_TokenKind: {
 Nil,

 RegularMin,

 // A group of characters that begins with an underscore or alphabetic character,
 // and consists of numbers, alphabetic characters, or underscores after that.
 Identifier,

 // A group of characters beginning with a numeric character or a '-', and then
 // consisting of only numbers, alphabetic characters, or '.'s after that.
 NumericLiteral,

 // A group of arbitrary characters, grouped together by a " character, OR by a
 // """ symbol at the beginning and end of the group. String literals beginning with
 // " are to only be specified on a single line, but """ strings can exist across
 // many lines.
 StringLiteral,

 // A group of arbitrary characters, grouped together by a ' character at the
 // beginning, and a ' character at the end.
 CharLiteral,

 // A group of symbolic characters. The symbolic characters are:
 // ~!@#$%^&*()-+=[{]}:;<>,./?|\
 //
 // Groups of multiple characters are only allowed in specific circumstances. Most of these
 // are only 1 character long, but some groups are allowed:
 //
 // "<<", ">>", "<=", ">=", "+=", "-=", "*=", "/=", "::", ":=", "==", "&=", "|=", "->"
 Symbol,

 RegularMax,

 Comment,

 WhitespaceMin,
 Whitespace,
 Newline,
 WhitespaceMax,

 MD_TokenKind_BadCharacter,

 MAX,
};

@struct MD_Token: {
 kind: MD_TokenKind,
 string: MD_String8,
 outer_string: MD_String8,
};

@prefix(MD_TokenGroup)
@base_type(MD_u32)
@flags MD_TokenGroups: {
 Comment,
 Whitespace,
 Regular,
};

////////////////////////////////
//~ Parsing State

@struct MD_Error: {
 next: *MD_Error,
 string: MD_String8,
 filename: MD_String8,
 node: *MD_Node,
 catastrophic: MD_b32,
 location: MD_CodeLoc,
};

@struct MD_ParseCtx: {
 first_root: *MD_Node,
 last_root: *MD_Node,
 first_error: *MD_Error,
 last_error: *MD_Error,
 at: *MD_u8,
 filename: MD_String8,
 file_contents: MD_String8,
 namespace_table: MD_Map,
 selected_namespace: *MD_Node,
 catastrophic_error: MD_b32,
};

@struct MD_ParseResult: {
 node: *MD_Node;
 first_error: *MD_Error;
 bytes_parse: MD_u64;
};

////////////////////////////////
//~ Expression and Type-Expression parser helper types.

// VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
// it is kept in-sync with the MD_ExprPrecFromExprKind function.

@enum MD_ExprKind: {
 Nil,

 // NOTE(rjf): Atom
 Atom,

 // NOTE(rjf): Access
 Dot,
 Arrow,
 Call,
 Subscript,
 Dereference,
 Reference,

 // NOTE(rjf): Arithmetic
 Add,
 Subtract,
 Multiply,
 Divide,
 Mod,

 // NOTE(rjf): Comparison
 IsEqual,
 IsNotEqual,
 LessThan,
 GreaterThan,
 LessThanEqualTo,
 GreaterThanEqualTo,

 // NOTE(rjf): Bools
 BoolAnd,
 BoolOr,
 BoolNot,

 // NOTE(rjf): Bitwise
 BitAnd,
 BitOr,
 BitNot,
 BitXor,
 LeftShift,
 RightShift,

 // NOTE(rjf): Unary numeric
 Negative,

 // NOTE(rjf): Type
 Pointer,
 Array,
 Volatile,
 Const,
 
 MAX,
};

@typedef(MD_i32) MD_ExprPrec;

@struct MD_Expr: {
 node: *MD_Node,
 kind: MD_ExprKind,
 parent: *MD_Expr,
 sub: [2]*MD_Expr,
};

////////////////////////////////
//~ Command line parsing helper types.

@struct MD_CommandLine: {
 arguments: *MD_String8;
 argument_count: MD_u32;
};

////////////////////////////////
//~ File system access types.

@prefix(MD_FileFlag)
@base_type(MD_u32)
@flags MD_FileFlags: {
 Directory,
};

@struct MD_FileInfo: {
 flags: MD_FileFlags;
 filename: MD_String8;
 file_size: MD_u64;
};

@opaque
@struct MD_FileIter: {};

////////////////////////////////
//~ Basic Utilities

@macro MD_Assert: {
 c,
};

@macro MD_StaticAssert: {
 c,
};

@macro MD_ArrayCount: {
 a,
};

////////////////////////////////
//~ Characters

@func MD_CharIsAlpha: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsAlphaUpper: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsAlphaLower: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsDigit: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsSymbol: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsReservedSymbol: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharIsSpace: {
 c: MD_u8,
 return: MD_b32,
};

@func MD_CharToUpper: {
 c: MD_u8,
 return: MD_u8,
};

@func MD_CharToLower: {
 c: MD_u8,
 return: MD_u8,
};

@func MD_CorrectSlash: {
 c: MD_u8,
 return: MD_u8,
};

////////////////////////////////
//~ Strings

@func MD_S8: {
 str: *MD_u8,
 size: MD_u64,
 return: MD_String8,
};

@macro MD_S8CString: {
 s,
};

@macro MD_S8Lit: {
 s,
};

@func MD_S8Range: {
 str: *MD_u8,
 opl: *MD_u8,
 return: MD_String8,
};

@func MD_StringSubstring: {
 str: MD_String8,
 min: MD_u64,
 max: MD_u64
 return: MD_String8,
};

@func MD_StringSkip: {
 str: MD_String8,
 min: MD_u64,
 return: MD_String8,
};

@func MD_StringChop: {
 str: MD_String8,
 nmax: MD_u64,
 return: MD_String8,
};

@func MD_StringPrefix: {
 str: MD_String8,
 size: MD_u64,
 return: MD_String8,
};

@func MD_StringSuffix: {
 str: MD_String8,
 size: MD_u64,
 return: MD_String8,
};

@func MD_StringMatch: {
 a: MD_String8,
 b: MD_String8,
 flags: MD_StringMatchFlags,
 return: MD_b32,
};

@func MD_FindSubstring: {
 str: MD_String8,
 substring: MD_String8,
 start_pos: MD_u64,
 flags: MD_StringMatchFlags,
 return: MD_u64,
};

@func MD_FindLastSubstring: {
 str: MD_String8,
 substring: MD_String8,
 flags: MD_StringMatchFlags,
 return: MD_u64,
};

@func MD_ChopExtension: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_SkipFolder: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_ExtensionFromPath: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_FolderFromPath: {
 string: MD_String8,
 return: MD_String8,
};


@func MD_PushStringCopy: {
 string: MD_String8,
 return: MD_String8,
};

@func MD_PushStringFV: {
 fmt: *char,
 args: va_list,
 return: MD_String8,
};

@func MD_PushStringF: {
 fmt: *char,
 "...",
 return: MD_String8,
};

@macro MD_StringExpand: { s, }


@func MD_PushStringToList: {
 list: *MD_String8List,
 string: MD_String8,
};

@func MD_PushStringListToList: {
 list: *MD_String8List,
 to_push: *MD_String8List,
};

@func MD_SplitString: {
 string: MD_String8,
 split_count: MD_u32,
 splits: *MD_String8,
 return: MD_String8List
};

@func MD_JoinStringList: {
 list: MD_String8List,
 return: MD_String8,
};

@func MD_JoinStringListWithSeparator: {
 list: MD_String8List,
 separator: MD_String8
 return: MD_String8,
};

@func MD_I64FromString: {
 string: MD_String8,
 radix: MD_u32,
 return: MD_i64,
};

@func MD_F64FromString: {
 string: MD_String8,
 return: MD_f64,
};

@func MD_HashString: {
 string: MD_String8,
 return: MD_u64,
};

@func MD_CalculateCStringLength: {
 cstr: *char,
 return: MD_u64,
};

@func MD_StyledStringFromString: {
 string: MD_String8,
 word_style: MD_WordStyle,
 separator: MD_String8,
 return: MD_String8
};

////////////////////////////////
//~ Enum/Flag Strings

@func MD_StringFromNodeKind: {
 kind: MD_NodeKind,
 return: MD_String8,
};

@func MD_StringListFromNodeFlags: {
 flags: MD_NodeFlags,
 return: MD_String8List,
};

////////////////////////////////
//~ Unicode Conversions

@func MD_CodepointFromUtf8: {
 str: MD_u8,
 max: MD_u64,
 return: MD_UnicodeConsume,
};

@func MD_CodepointFromUtf16: {
 str: *MD_u16,
 max: MD_u64,
 return: MD_UnicodeConsume,
};

@func MD_Utf8FromCodepoint: {
 out: *MD_u8,
 codepoint: MD_u32,
 return: MD_u32,
};

@func MD_Utf16FromCodepoint: {
 out: *MD_u16,
 codepoint: MD_u32,
 return: MD_u32,
};

@func MD_S8FromS16: {
 str: MD_String16,
 return: MD_String8,
};

@func MD_S16FromS8: {
 str: MD_String8,
 return: MD_String16,
};

@func MD_S8FromS32: {
 str: MD_String32,
 return: MD_String8,
};

@func MD_S32FromS8: {
 str: MD_String8,
 return: MD_String32,
};

////////////////////////////////
//~ String-To-Node-List Table

@func MD_Map_Lookup: {
 table: *MD_Map,
 string: MD_String8,
 return: *MD_MapSlot,
};

@func MD_Map_Insert: {
 table: *MD_Map,
 collision_rule: MD_MapCollisionRule,
 string: MD_String8,
 node: *MD_Node,
 return: MD_b32,
};

////////////////////////////////
//~ Parsing

@func MD_TokenKindIsWhitespace: {
 kind: MD_TokenKind,
 return: MD_b32,
};

@func MD_TokenKindIsComment: {
 kind: MD_TokenKind,
 return: MD_b32,
};

@func MD_TokenKindIsRegular: {
 kind: MD_TokenKind,
 return: MD_b32,
};

@func MD_Parse_InitializeCtx: {
 filename: MD_String8,
 contents: MD_String8,
 return: MD_ParseCtx,
};

@func MD_Parse_Bump: {
 ctx: *MD_ParseCtx,
 token: MD_Token,
};

@func MD_Parse_BumpNext: {
 ctx: *MD_ParseCtx,
};

@func MD_Parse_LexNext: {
 ctx: *MD_ParseCtx,
 return: MD_Token,
};

@func MD_Parse_PeekSkipSome: {
 ctx: *MD_ParseCtx,
 skip_groups: MD_TokenGroups,
 return: MD_Token,
};

@func MD_Parse_TokenMatch: {
 token: MD_Token,
 string: MD_String8,
 flags: MD_StringMatchFlags,
 return: MD_b32,
};

@func MD_Parse_Require: {
 ctx: *MD_ParseCtx,
 string: MD_String8,
 return: MD_b32,
};

@func MD_Parse_RequireKind: {
 ctx: *MD_ParseCtx,
 kind: MD_TokenKind,
 out_token: *MD_Token,
 return: MD_b32,
};

@func MD_ParseOneNode: {
 filename: MD_String8,
 contents: MD_String8,
 return: MD_ParseResult,
};

@func MD_ParseWholeString: {
 filename: MD_String8,
 contents: MD_String8,
 return: MD_ParseResult,
};

@func MD_ParseWholeFile: {
 filename: MD_String8,
 return: MD_ParseResult,
};

////////////////////////////////
//~ Tree/List Building

@func MD_NodeIsNil: {
 node: *MD_Node,
 return: MD_b32,
};

@func MD_NilNode: {
 return: *MD_Node,
};

@func MD_MakeNodeFromToken: {
 kind: MD_NodeKind,
 filename: MD_String8,
 file: *MD_u8,
 at: *MD_u8,
 token: MD_Token,
 return: *MD_Node,
};

@func MD_MakeNodeFromString: {
 kind: MD_NodeKind,
 filename: MD_String8,
 file: *MD_u8,
 at: *MD_u8,
 string: MD_String8
 return: *MD_Node,
};

@func MD_PushSibling: {
 first: **MD_Node,
 last: **MD_Node,
 parent: *MD_Node,
 new_sibling: *MD_Node,
};

@func MD_PushChild: {
 parent: *MD_Node,
 new_child: *MD_Node,
};

@func MD_PushTag: {
 node: *MD_Node,
 tag: *MD_Node,
};

////////////////////////////////
//~ Introspection Helpers

@func MD_NodeFromString: {
 first: *MD_Node,
 last: *MD_Node,
 string: MD_String8,
 return: *MD_Node,
};

@func MD_NodeFromIndex: {
 first: *MD_Node,
 last: *MD_Node,
 n: int,
 return: *MD_Node,
};

@func MD_IndexFromNode: {
 node: *MD_Node,
 return: int,
};

@func MD_NextNodeSibling: {
 last: *MD_Node,
 string: MD_String8,
 return: *MD_Node,
};

@func MD_ChildFromString: {
 node: *MD_Node,
 child_string: MD_String8,
 return: *MD_Node,
};

@func MD_TagFromString: {
 node: *MD_Node,
 tag_string: MD_String8,
 return: *MD_Node,
};

@func MD_ChildFromIndex: {
 node: *MD_Node,
 n: int,
 return: *MD_Node,
};

@func MD_TagFromIndex: {
 node: *MD_Node,
 n: int,
 return: *MD_Node,
};

@func MD_TagArgFromIndex: {
 node: *MD_Node,
 tag_string: MD_String8,
 n: int,
 return: *MD_Node,
};

@func MD_NodeHasTag: {
 node: *MD_Node,
 tag_string: MD_String8,
 return: MD_b32,
};

@func MD_CodeLocFromNode: {
 node: *MD_Node,
 return: MD_CodeLoc,
};

@func MD_ChildCountFromNode: {
 node: *MD_Node,
 return: MD_i64,
};

@func MD_TagCountFromNode: {
 node: *MD_Node,
 return: MD_i64,
};

@macro MD_EachNode: { it, first, };

////////////////////////////////
//~ Error/Warning Helpers

@func MD_NodeMessage: {
 node: *MD_Node,
 kind: MD_MessageKind,
 str: MD_String8,
};

@func MD_NodeMessageF: {
 node: *MD_Node,
 kind: MD_MessageKind,
 fmt: *char,
 "..."
};

////////////////////////////////
//~ Tree Comparison/Verification

@func MD_NodeMatch: {
 a: *MD_Node,
 b: *MD_Node,
 str_flags: MD_StringMatchFlags,
 node_flags: MD_NodeMatchFlags,
 return: MD_b32,
};

@func MD_NodeDeepMatch: {
 a: *MD_Node,
 b: *MD_Node,
 str_flags: MD_StringMatchFlags,
 node_flags: MD_NodeMatchFlags,
 return: MD_b32,
};

////////////////////////////////
//~ Expression and Type-Expression Helper

@func MD_NilExpr: {
 return: *MD_Expr,
};

@func MD_ExprIsNil: {
 expr: *MD_Expr,
 return: MD_b32,
};

@func MD_PreUnaryExprKindFromNode: {
 node: *MD_Node,
 return: MD_ExprKind,
};

@func MD_BinaryExprKindFromNode: {
 node: *MD_Node,
 return: MD_ExprKind,
};

@func MD_ExprPrecFromExprKind: {
 kind: MD_ExprKind,
 return: MD_ExprPrec,
};

@func MD_MakeExpr: {
 node: *MD_Node,
 kind: MD_ExprKind,
 left: *MD_Expr,
 right: *MD_Expr,
 return: *MD_Expr,
};

@func MD_ParseAsExpr: {
 first: *MD_Node,
 last: *MD_Node,
 return: *MD_Expr,
};

@func MD_ParseAsType: {
 first: *MD_Node,
 last: *MD_Node,
 return: *MD_Expr,
};

@func MD_EvaluateExpr_I64: {
 expr: *MD_Expr,
 return: MD_i64,
};

@func MD_EvaluateExpr_F64: {
 expr: *MD_Expr,
 return: MD_f64,
};

@func MD_ExprMatch: {
 a: *MD_Expr,
 b: *MD_Expr,
 str_flags: MD_StringMatchFlags,
 return: MD_b32,
};

@func MD_ExprDeepMatch: {
 a: *MD_Expr,
 b: *MD_Expr,
 str_flags: MD_StringMatchFlags,
 return: MD_b32,
};

////////////////////////////////
//~ Generation

@func MD_OutputTree: {
 file: *FILE,
 node: *MD_Node,
};

////////////////////////////////
//~ C Language Generation

@func MD_OutputTree_C_String: {
 file: *FILE,
 node: *MD_Node,
};

@func MD_OutputTree_C_Struct: {
 file: *FILE,
 node: *MD_Node,
};

@func MD_OutputTree_C_Decl: {
 file: *FILE,
 node: *MD_Node,
};

@func MD_Output_C_DeclByNameAndType: {
 file: *FILE,
 name: MD_String8,
 type: *MD_Expr,
};

@func MD_OutputExpr: {
 file: *FILE,
 expr: *MD_Expr,
};

@func MD_OutputExpr_C: {
 file: *FILE,
 expr: *MD_Expr,
};

@func MD_OutputType: {
 file: *FILE,
 type: *MD_Expr,
};

@func MD_OutputType_C_LHS: {
 file: *FILE,
 type: *MD_Expr,
};

@func MD_OutputType_C_RHS: {
 file: *FILE,
 type: *MD_Expr,
};

////////////////////////////////
//~ Command Line Argument Helper

@func MD_CommandLine_Start: {
 argument_count: int,
 arguments: **char,
 return: MD_CommandLine,
};

@func MD_CommandLine_Flag: {
 cmdln: *MD_CommandLine,
 string: MD_String8,
 return: MD_b32,
};

@func MD_CommandLine_FlagStrings: {
 cmdln: *MD_CommandLine,
 string: MD_String8,
 out_count: int,
 out: *MD_String8,
 return: MD_b32,
};

@func MD_CommandLine_FlagIntegers: {
 cmdln: *MD_CommandLine,
 string: MD_String8,
 out_count: int,
 out: *MD_i64,
 return: MD_b32,
};

@func MD_CommandLine_FlagString: {
 cmdln: *MD_CommandLine,
 string: MD_String8,
 out: *MD_String8,
 return: MD_b32,
};

@func MD_CommandLine_FlagInteger: {
 cmdln: *MD_CommandLine,
 string: MD_String8,
 out: *MD_i64,
 return: MD_b32,
};

@func MD_CommandLine_Increment: {
 cmdln: *MD_CommandLine,
 string_ptr: **MD_String8,
 return: MD_b32,
};

////////////////////////////////
//~ File System

@func MD_LoadEntireFile: {
 filename: MD_String8,
 return: MD_String8,
};

@func MD_FileIterIncrement: {
 it: *MD_FileIter,
 path: MD_String8,
 out_info: *MD_FileInfo,
 return: MD_b32,
};



