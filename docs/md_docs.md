////////////////////////////////
//~ Basic Unicode string types.

@doc("A pointer and size representing a UTF-8 string. Also used for any range of untyped data range.")
@see(MD_String8List)
@struct MD_String8: {
 str: *MD_u8,
 size: MD_u64,
};

@doc("A pointer and size representing a UTF-16 string.")
@struct MD_String16: {
 str: *MD_u16,
 size: MD_u64,
};

@doc("A pointer and size representing a UTF-32 string.")
@struct MD_String32: {
 str: *MD_u32,
 size: MD_u64,
};

@doc("A node in an MD_String8List")
@struct MD_String8Node: {
 next: *MD_String8Node,
 string: MD_String8,
};

@doc("A list of multiple possibly discontiguous strings (in MD_String8). Stored as a singly linked list.")
@struct MD_String8List: {
 node_count: MD_u64,
 total_size: MD_u64,
 first: *MD_String8Node,
 last: *MD_String8Node,
};

@doc("Controls matching rules in routines that perform string matching.")
@prefix(MD_StringMatchFlag)
@base_type(MD_u32)
@flags MD_StringMatchFlags: {
 @doc("Consider lower case letters equivalent to upper case equivalents in the ASCII range.")
 CaseInsensitive,

 @doc("Do not require the strings to be the same length. If one of the strings is a prefix of another, the two strings will count as a match.")
 RightSideSloppy,

 @doc("On routines returning the location of a substring, alters the behavior to return the last match instead of the first match.")
 FindLast,

 @doc("Consider forward slash and backward slash as equivalents.")
 SlashInsensitive,
};

@doc("Information gathered from consuming one unicode character from some unicode encoded stream.")
@struct MD_UnicodeConsume: {
 @doc("The codepoint of the consumed character.")
 codepoint: MD_u32,

 @doc("The size of the character in the encoded stream, measured in 'units'. A unit is one byte in UTF-8, two bytes in UTF-16, and four bytes in UTF-32.")
 advance: MD_u32,
};

@doc("Styles of identifier spacing and capitalization.")
@enum MD_WordStyle: {
 UpperCamelCase,
 LowerCamelCase,
 UpperCase,
 LowerCase,
};

////////////////////////////////
//~ Node types that are used to build all ASTs.

@doc("The basic kinds of nodes in the abstract syntax tree parsed from metadesk.")
@see(MD_Node)
@enum MD_NodeKind: {
 @doc("The Nil node is a unique node representing the lack of information, for example iterating off the end of a list, or up to the parent of a root node results in Nil.")
 Nil,

 @doc("A File node represents parsed metadesk source text.")
 File,

 @doc("A List node serves as the root of an externally chained list of nodes. It's children are nodes with the MD_NodeKind_Reference kind.")
 List,

 @doc("A Reference node is an indirection to another node. The node field 'ref_target' contains a pointer to the referenced node. These nodes are typically used for creating externally chained linked lists that gather nodes from a parse tree.")
 Reference,

 @doc("A Namespace node represents a namespace created by the '#namespace' reserved keyword.")
 Namespace,

 @doc("A Label node represents the main structure of the metadesk abstract syntax tree. Some labels have children which will also be labels. Labels can be given their text by identifiers, numerics, string and character literals, and operator symbols.")
 @see(MD_TokenKind)
 Label,

 @doc("A Tag node represents a tag attached to a label node with the '@identifer' syntax. The children of a tag node represent the arguments placed in the tag.")
 Tag,
 
 @doc("Not a real kind value given to nodes, this is always one larger than the largest enum value that can be given to a node.")
 MAX,
};

@doc("Flags put on nodes to indicate extra information about specific details about the strings that were parsed to create the node.")
@see(MD_Node)
@see(MD_TokenKind)
@prefix(MD_NodeFlag)
@base_type(MD_u32)
@flags MD_NodeFlags: {
 @doc("The node has children in an open/close symbol pair and '(' is the open symbol.")
 ParenLeft,
 @doc("The node has children in an open/close symbol pair and ')' is the close .")
 ParenRight,
 @doc("The node has children in an open/close symbol pair and '[' is the open symbol.")
 BracketLeft,
 @doc("The node has children in an open/close symbol pair and ']' is the close symbol.")
 BracketRight,
 @doc("The node has children in an open/close symbol pair and '{' is the close symbol.")
 BraceLeft,
 @doc("The node has children in an open/close symbol pair and '}' is the close symbol.")
 BraceRight,

 @doc("The delimiter between this node and it's next sibling is a ';'")
 BeforeSemicolon,
 @doc("The delimiter between this node and it's next sibling is a ','")
 BeforeComma,

 @doc("The delimiter between this node and it's previous sibling is a ';'")
 AfterSemicolon,
 @doc("The delimiter between this node and it's previous sibling is a ','")
 AfterComma,

 @doc("The label on this node comes from a token with the MD_TokenKind_NumericLiteral kind.")
 Numeric,
 @doc("The label on this node comes from a token with the MD_TokenKind_Identifier kind.")
 Identifier,
 @doc("The label on this node comes from a token with the MD_TokenKind_StringLiteral kind.")
 StringLiteral,
 @doc("The label on this node comes from a token with the MD_TokenKind_CharLiteral kind.")
 CharLiteral,
};

@doc("Controls matching rules in routines that compare MD_Node trees")
@prefix(MD_NodeMatchFlag)
@base_type(MD_u32)
@flags MD_NodeMatchFlags: {
 @doc("When this flag is set, differences in the order and names of tags on a node count as differences in the input nodes. Without this flag tags are ignored in tree comparisons.")
 Tags,

 @doc("When this flag is set in addition to MD_NodeMatchFlag_Tags, the differences in the arguments of each tag (the tag's children in the tree) are count as differences in the input nodes. Tag arguments are compared with fully recursive compares, whether or not the compare routine would be recursive or not.")
 TagArguments,
};

@doc("The main 'lego-brick' for modeling the result of a metadesk parse. Also used in some auxiliary data structures.")
@struct MD_Node: {
 @doc("The next sibling in the hierarchy, or the next tag in a list of tags, or next node in an externally chained linked list.")
 next: *MD_Node,
 @doc("The previous sibling in the hierarchy, or the previous tag in a list of tags, or previous node in an externally chained linked list.")
 prev: *MD_Node,
 @doc("The parent in the hierarchy, or root node of an externally chained linked list.")
 parent: *MD_Node,
 @doc("The first child in the hierarchy, or the first node in an externally chained linked list.")
 first_child: *MD_Node,
 @doc("The last child in the hierarchy, or the last node in an externally chained linked list.")
 last_child: *MD_Node,

 @doc("The first tag attached to a node.")
 first_tag: *MD_Node,
 @doc("The last tag attached to a node.")
 last_tag: *MD_Node,

 @doc("Indicates the role that the node plays in metadesk node graph.")
 kind: MD_NodeKind,
 @doc("Extra information about the source that generated this node in the parse.")
 flags: MD_NodeFlags,
 @doc("The string of the token labeling this node, after processing. Processing removing quote marks that delimits string literals and character literals")
 string: MD_String8,
 @doc("The raw string of the token labeling this node.")
 whole_string: MD_String8,
 @doc("A hash of the string field using the metadesk built in hash function.")
 string_hash: MD_u64,

 @doc("The raw string of the comment token before this node, if there is one.")
 comment_before: MD_String8,
 @doc("The raw string of the comment token after this node, if there is one.")
 comment_after: MD_String8,
 
 @doc("The name of the file from which this node was parsed; or the name that was passed to the parse call.")
 filename: MD_String8,
 @doc("The pointer to the base of the raw string from which this node was parsed.")
 file_contents: *MD_u8,
 @doc("A pointer into the raw string from which this was parsed indicating the beginning of the text that generated this node.")
 at: *MD_u8,

 @doc("The external pointer from an MD_NodeKind_Reference kind node in an externally linked list.")
 ref_target: *MD_Node,
};

////////////////////////////////
//~ Code Location Info.

@doc("Source code location using file, line, column coordinates")
@struct MD_CodeLoc: {
 filename: MD_String8,
 @doc("Line numbers are 1 based, the lowest valid location is on line number 1.")
 line: MD_u32,
 @doc("Column numbers are 1 based, the lowest valid location is on column number 1.")
 column: MD_u32,
};

////////////////////////////////
//~ String-To-Node table

@doc("Controls the behavior of routines that write into maps when the written key was already in the map.")
@enum MD_MapCollisionRule: {
 @doc("When the key written was already in the map, a new key value pair is attached to the same chain always. Leaving multiple values associated to the same key.")
 Chain,
 @doc("When the key written was already in the map, the existing value is replaced with the newly inserted value.")
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

@enum MD_MessageKind: {
 None,
 Warning,
 Error,
 CatastrophicError,
}

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
//~ Location Conversion

@func MD_CodeLocFromFileOffset: {
 filename: MD_String8,
 base: *MD_u8,
 off: *MD_u8,
 return: MD_CodeLoc,
};

@func MD_CodeLocFromNode: {
 node: *MD_Node,
 return: MD_CodeLoc,
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



