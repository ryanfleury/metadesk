title: "Metadesk Reference"

////////////////////////////////
//~ Layout for docs.

@def Strings: {}
@def Nodes: {}
@def CodeLoc: {}
@def Map: {}
@def Tokens: {}
@def Parsing: {}
@def ExpressionParsingHelper: {}
@def CommandLineHelper: {}
@def FileSystemHelper: {}
@def HelperMacros: {}
@def Characters: {}
@def Output: {}

main:
{
    
    @title "Strings",
    @paste Strings,
    
    @title "Nodes",
    @paste Nodes,
    
    @title "Source Code Locations",
    @paste CodeLoc,
    
    @title "Map",
    @paste Map,
    
    @title "Tokenization",
    @paste Tokens,
    
    @title "Metadesk Format Parsing",
    @paste Parsing,
    
    @title "Expression Parsing Helper",
    @paste ExpressionParsingHelper,
    
    @title "Command Line Parsing Helper",
    @paste CommandLineHelper,
    
    @title "File System Helper",
    @paste FileSystemHelper,
    
    @title "Helper Macros",
    @paste HelperMacros,
    
    @title "Memory Operations",
    @paste MemoryOperations,
    
    @title "Character Helpers",
    @paste Characters,
    
    @title "Generation Helpers",
    @paste Output,
    
}

////////////////////////////////
//~ Basic Unicode string types.

@send(Strings)
@doc("This type is used as the fundamental string type in Metadesk, and as the type for byte granularity data blobs. Strings of this type are encoded in UTF-8.")
@see(MD_String8List)
@struct MD_String8: {
    str: *MD_u8,
    size: MD_u64,
};

@send(Strings)
@doc("This type represents a string encoded in UTF-16.")
@struct MD_String16: {
    str: *MD_u16,
    size: MD_u64,
};

@send(Strings)
@doc("This type represents a string encoded in UTF-32.")
@struct MD_String32: {
    str: *MD_u32,
    size: MD_u64,
};

@send(Strings)
@doc("MD_String8Node forms one node in a linked list of strings. Generally used as a part of an MD_String8List data structure.")
@struct MD_String8Node: {
    @doc("The next node in the list, or null if this is the last node.")
        next: *MD_String8Node,
    @doc("The string value stored at this node.")
        string: MD_String8,
};

@send(Strings)
@doc("This type is implemented as a singly linked list with an MD_String8 at each node.")
@see(MD_PushStringToList)
@see(MD_SplitString)
@see(MD_JoinStringList)
@struct MD_String8List: {
    @doc("The number of nodes in the list.")
        node_count: MD_u64,
    @doc("The size of all strings in the list summed together.")
        total_size: MD_u64,
    first: *MD_String8Node,
    last: *MD_String8Node,
};

@send(Strings)
{
    """
        A sample loop over MD_String8List:
    """
        
        @code
        """
        MD_String8List list;
    for (MD_String8Node *node = list.first; node != 0; node = node->next)
    {
        MD_String8 string = node->string;
        // ... work on string ...
    }
    """
        
        """
        The @code node_count and @code total_size are automatically maintained by the helpers for inserting to the list, and are used in the string joining functions. Hand rolled code for building string lists should take those fields into consideration if the join functions need to work.
        """
}

@send(Strings)
@doc("These flags control matching rules in routines that perform matching on strings and Metadesk AST nodes.")
@see(MD_Node)
@prefix(MD_MatchFlag)
@base_type(MD_u32)
@flags MD_MatchFlags: {
    @doc("When comparing strings, consider lower case letters equivalent to upper case equivalents in the ASCII range.")
        CaseInsensitive,
    
    @doc("When comparing strings, do not require the strings to be the same length. If one of the strings is a prefix of another, the two strings will count as a match.")
        RightSideSloppy,
    
    @doc("For routines returning the location of a substring, alters the behavior to return the last match instead of the first match.")
        FindLast,
    
    @doc("When comparing strings, consider forward slash and backward slash to be equivalents.")
        SlashInsensitive,
    
    @doc("When comparing nodes with this flag set, differences in the order and names of tags on a node count as differences in the input nodes. Without this flag tags are ignored in tree comparisons.")
        Tags,
    
    @doc("When comparing nodes with this flag set in addition to @code 'MD_NodeMatchFlag_Tags', the differences in the arguments of each tag (the tag's children in the tree) are count as differences in the input nodes. Tag arguments are compared with fully recursive compares, whether or not the compare routine would be recursive or not.")
        TagArguments,
};

@send(Strings)
@doc("This type is used to report the results of consuming one character from a unicode encoded stream.")
@see(MD_CodepointFromUtf8)
@see(MD_CodepointFromUtf16)
@struct MD_UnicodeConsume: {
    @doc("The codepoint of the consumed character.")
        codepoint: MD_u32,
    
    @doc("The size of the character in the encoded stream, measured in 'units'. A unit is one byte in UTF-8, two bytes in UTF-16, and four bytes in UTF-32.")
        advance: MD_u32,
};

@send(Strings)
@doc("These constants control how MD_StyledStringFromString forms strings.")
@enum MD_WordStyle: {
    @doc("Also known as @code 'PascalCase'. Creates identifiers that look like: @code `ExampleIdentifier`")
        UpperCamelCase,
    @doc("Creates identifiers that look like: @code `exampleIdentifier`")
        LowerCamelCase,
    @doc("Creates identifiers that look like: @code `Example_Identifier`")
        UpperCase,
    @doc("Creates identifiers that look like: @code `example_identifier`")
        LowerCase,
};

////////////////////////////////
//~ Node types that are used to build all ASTs.

@send(Nodes)
@doc("These constants distinguish major roles of MD_Node in the Metadesk abstract syntax tree data structure.")
@enum MD_NodeKind: {
    @doc("The @code 'Nil' node is a unique node representing the lack of information, for example iterating off the end of a list, or up to the parent of a root node results in Nil.")
        Nil,
    
    @doc("A @code 'File' node represents parsed Metadesk source text.")
        File,
    
    @doc("A @code 'List' node serves as the root of an externally chained list of nodes. Its children are nodes with the @code 'MD_NodeKind_Reference' kind.")
        List,
    
    @doc("A @code 'Reference' node is an indirection to another node. The node field @code 'ref_target' contains a pointer to the referenced node. These nodes are typically used for creating externally chained linked lists that gather nodes from a parse tree.")
        Reference,
    
    @doc("A @code 'Label' node represents the main structure of the metadesk abstract syntax tree. Some labels have children which will also be labels. Labels can be given their text by identifiers, numerics, string and character literals, and operator symbols.")
        @see(MD_TokenKind)
        Label,
    
    @doc("A @code 'Tag' node represents a tag attached to a label node with the @code '@identifer' syntax. The children of a tag node represent the arguments placed in the tag.")
        Tag,
    
    @doc("An @code 'ErrorMarker' node is generated when reporting errors. It is used to record the location of an error that occurred in the lexing phase of a parse.")
        ErrorMarker,
    
    @doc("Not a real kind value given to nodes, this is always one larger than the largest enum value that can be given to a node.")
        COUNT,
};

@send(Nodes)
@doc("These flags are set on MD_Node to indicate particular details about the strings that were parsed to create the node.")
@see(MD_Node)
@see(MD_TokenKind)
@prefix(MD_NodeFlag)
@base_type(MD_u32)
@flags MD_NodeFlags: {
    @doc("This node's children open with @code '('")
        ParenLeft,
    @doc("This node's children close with @code ')'")
        ParenRight,
    @doc("This node's children open with @code '['")
        BracketLeft,
    @doc("This node's children close with @code ']'")
        BracketRight,
    @doc("This node's children open with @code '{'")
        BraceLeft,
    @doc("This node's children close with @code '}'")
        BraceRight,
    
    @doc("The delimiter between this node and its next sibling is a @code ';'")
        BeforeSemicolon,
    @doc("The delimiter between this node and its previous sibling is a @code ';'")
        AfterSemicolon,
    
    @doc("The delimiter between this node and its next sibling is a @code ','")
        BeforeComma,
    @doc("The delimiter between this node and its previous sibling is a @code ','")
        AfterComma,
    
    @doc("This is a string literal, with @code `'` character(s) marking the boundaries.")
        StringSingleQuote,
    @doc(```This is a string literal, with @code '"' character(s) marking the boundaries.```)
        StringDoubleQuote,
    @doc("This is a string literal, with @code '`' character(s) marking the boundaries." "\"")
        StringTick,
    @doc("This is a string literal that used triplets (three of its boundary characters in a row, on either side) to mark its boundaries, making it multiline.")
        StringTriplet,
    
    @doc("The label on this node comes from a token with the @code MD_TokenKind_NumericLiteral kind.")
        Numeric,
    @doc("The label on this node comes from a token with the @code MD_TokenKind_Identifier kind.")
        Identifier,
    @doc("The label on this node comes from a token with the @code MD_TokenKind_StringLiteral kind.")
        StringLiteral,
};

@send(Nodes)
@doc("The @code `MD_Node` is the main 'lego-brick' for modeling the result of a Metadesk parse. Also used in some auxiliary data structures.")
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
    
    @doc("The byte-offset into the string from which this node was parsed. Used for producing data for an MD_CodeLoc.")
        offset: MD_u64,
    
    @doc("The external pointer from an @code 'MD_NodeKind_Reference' kind node in an externally linked list.")
        ref_target: *MD_Node,
};

////////////////////////////////
//~ Code Location Info.

@send(CodeLoc)
@doc("This type encodes source code locations using file, line, column coordinates.")
@struct MD_CodeLoc: {
    filename: MD_String8,
    @doc("Line numbers are 1 based, the lowest valid location is on line number 1.")
        line: MD_u32,
    @doc("Column numbers are 1 based, the lowest valid location is on column number 1.")
        column: MD_u32,
};

////////////////////////////////
//~ String-To-Ptr and Ptr-To-Ptr tables

@send(Map)
@doc("An abstraction over the types of keys used in a MD_Map and the work of hashing those keys, can be constructed from an MD_String8 or an void*.")
@struct MD_MapKey: {
    @doc("The hash of the key. The hash function used is determined from the key type.")
        hash: MD_u64,
    @doc("For a non-empty MD_String8, the size of the string data. For a void*, zero.")
        size: MD_u64,
    @doc("For a non-empty MD_String8, points to the string data of the key. For a void*, the direct pointer value.")
        ptr: *void,
};

@send(Map)
@doc("A slot containing one (key,value) pair in a MD_Map.")
@struct MD_MapSlot: {
    @doc("The next slot in the same bucket of the MD_Map.")
        next: *MD_MapSlot,
    @doc("The key that maps to this slot.")
        key: MD_MapKey;
    @doc("The value part of the pair.")
        value: *void;
};

@send(Map)
@doc("The data used to form a table in an MD_Map. Stores pointers that form a linked list of all MD_MapSlot instances that mapped to this bucket.")
@struct MD_MapBucket:
{
    first: *MD_MapSlot,
    last: *MD_MapSlot,
}

@send(Map)
@doc("The map is a chained hash table data structure. Data written to the map is a key-value pair. The key of a pair may either be a pointer, or a string. Both types may be mixed inside a single map. Keys stored with one type never match keys of the other type. The values of the pairs are pointers.")
@struct MD_Map: {
    buckets: *MD_MapBucket,
    bucket_count: MD_u64,
};

////////////////////////////////
//~ Tokens

@send(Tokens)
@doc("Flags encoding the kind of a token produced by the lexer.")
@see(MD_TokenFromString)
@flags MD_TokenKind:
{
    @doc("When this bit is set, the token follows C-like identifier rules. It may start with an alphabetic character or an underscore, and can contain alphanumeric characters or underscores inside it.")
        Identifier,
    
    @doc("When this bit is set, the token follows C-like numeric literal rules.")
        NumericLiteral,
    
    @doc("When this bit is set, the token was recognized as a string literal. These may be formed with C-like rules, with a single-quote or double-quote around the string contents. They may also be formed with Metadesk's additional rules. These rules allow using @code '`' characters to mark the boundaries of the string, and also using triplets of any of these characters (@code '```This is a string```') to allow newlines within the string's contents.")
        StringLiteral,
    
    @doc("When this bit is set, the token was recognized as a symbolic character. Whether a character is considered symbolic is determined by the MD_CharIsSymbol function.")
        Symbol,
    
    @doc("When this bit is set, the token is reserved for special uses by the Metadesk parser.")
        Reserved,
    
    @doc("When this bit is set, the token was recognized as a comment. Comments can be formed in the traditional C-like ways, using @code '//' for single-line, or @code '/*' and @code '*/' for multiline. Metadesk differs, slightly, in that it allows nested multiline comments. So, every @code '/*' must be matched by a @code '*/'.")
        Comment,
    
    @doc("When this bit is set, the token contains only whitespace.")
        Whitespace,
    
    @doc("When this bit is set, the token is a newline character.")
        Newline,
    
    @doc("When this bit is set, the token is a comment that was malformed syntactically.")
        BrokenComment,
    
    @doc("When this bit is set, the token is a string literal that was malformed syntactically.")
        BrokenStringLiteral,
    
    @doc("When this bit is set, the token contains a character in an encoding that is not supported by the parser Metadesk.")
        BadCharacter,
}

@send(Tokens)
@doc("The type used for encoding data about any token produced by the lexer.")
@struct MD_Token: {
    kind: MD_TokenKind;
    @doc("Flags that should be attached to an MD_Node that uses this token to define its string. Only includes flags that can be understood by the lexer; is not the comprehensive set of node flags that a node needs.")
        node_flags: MD_NodeFlags;
    @doc("The contents of this token, not including any boundary characters.")
        string: MD_String8;
    @doc("The full contents of the string used to form this token, including all boundary characters.")
        outer_string: MD_String8;
};

////////////////////////////////
//~ Parsing State

@send(Parsing)
@doc("This type distinguishes the roles of messages, including errors and warnings.")
@enum MD_MessageKind: {
    @doc("The message does not have a particular role.")
        Null,
    @doc("The message is a warning.")
        Warning,
    @doc("The message has information about a non-catastrophic error. Reasonable results may still have been produced, but something illegal was encountered.")
        Error,
    @doc("The message has information about a catastrophic error, meaning that the output of whatever the error was for cannot be trusted, and should be treated as a complete failure.")
        CatastrophicError,
}

@send(Parsing)
@doc("This type encodes information about messages.")
@struct MD_Error: {
    @doc("A pointer to the next error, in a chain of errors. This is @code '0' when it is the last error in a chain.")
        next: *MD_Error,
    @doc("The node that this message refers to.")
        node: *MD_Node,
    @doc("This message's kind.")
        kind: MD_MessageKind,
    @doc("The message contents.")
        string: MD_String8,
};

@send(Parsing)
@doc("This type is for a chain of error messages, with data about the entire list.")
@struct MD_ErrorList:
{
    @doc("""The "worst" message kind in this chain, where a message kind is "worse" than another if it has a higher numeric value (if it is defined later in MD_MessageKind) than another.""")
        max_error_kind: MD_MessageKind;
    @doc("The number of errors in this list.")
        node_count: MD_u64;
    @doc("The first error in the list.")
        first: *MD_Error;
    @doc("The last error in the list.")
        last: *MD_Error;
}

@send(Parsing)
@struct MD_ParseResult: {
    node: *MD_Node;
    first_error: *MD_Error;
    bytes_parse: MD_u64;
};

@send(Parsing)
@doc("This type is used for encoding the rule for delimiting the end of a set of nodes, when parsing.")
@see(MD_ParseNodeSet)
@enum MD_ParseSetRule:
{
    @doc("The set needs to end when an appropriate delimiter, matching the opening delimiter, is encountered. If the opening delimiter is a @code '{' character, then the set will end with a @code '}' character. If the opening delimiter is a @code '(' or @code '[' character, then the set will end with either a @code ')' or @code ']' delimiter. If the set has no opening delimiter, then the set will end with either one newline (if children have been specified), or two newlines (if children have not been specified), or a separator character (@code ',' or @code ';').")
        EndOnDelimiter,
    @doc("The set is occurring at the top-level, and so it should not end until the entire file has been parsed.")
        Global,
}

@send(Parsing)
@doc("This type is used for the results of calls that do Metadesk parsing.")
@see(MD_ParseWholeFile)
@see(MD_ParseWholeString)
@see(MD_ParseOneNode)
@see(MD_ParseNodeSet)
@see(MD_ParseTagList)
@struct MD_ParseResult:
{
    @doc("The node produced by the parser.")
        node: *MD_Node;
    @doc("The last node, in a chain, produced by the parser, when the parsing call is capable of returning many nodes.")
        last_node: *MD_Node;
    @doc("The number of bytes processed by the parser. Any bytes after this number, in the string that was passed to the parser, were not considered.")
        bytes_parsed: MD_u64;
    @doc("The list of errors produced by the parser when parsing the provided string.")
        errors: MD_ErrorList;
}

////////////////////////////////
//~ Command line parsing helper types.

@send(CommandLineHelper)
@doc("A type used to encode parsed command line options, that can have values associated with them.")
@see(MD_CommandLine)
@see(MD_CommandLineFromOptions)
@struct MD_CommandLineOption:
{
    @doc("A pointer to the next option, if this is within a chain of options. Will be @code '0' if this is the last option in a chain.")
        next: *MD_CommandLineOption;
    @doc("The name of this option.")
        name: MD_String8;
    @doc("The values associated with this option.")
        values: MD_String8List;
};

@send(CommandLineHelper)
@doc("The type encoding a fully parsed set of command line options.")
@see(MD_CommandLineFromOptions)
@struct MD_CommandLine:
{
    @doc("The list of all command line arguments, as an unstructured list of strings.")
        arguments: MD_String8List;
    @doc("The list of arguments that were not parsed as structured options (which are a name, with an optional set of values).")
        inputs: MD_String8List;
    @doc("The first option that was parsed, forming the head of a chain of options.")
        first_option: *MD_CommandLineOption;
    @doc("The last option that was parsed.")
        last_option: *MD_CommandLineOption;
};

////////////////////////////////
//~ File system access types.

@send(FileSystemHelper)
@prefix(MD_FileFlag)
@base_type(MD_u32)
@doc("Flags encoding certain properties of a file.")
@see(MD_FileInfo)
@see(MD_FileIterIncrement)
@flags MD_FileFlags: {
    @doc("The associated file is a directory.")
        Directory,
};

@send(FileSystemHelper)
@doc("Encodes general metadata about a file.")
@see(MD_FileIterIncrement)
@struct MD_FileInfo: {
    flags: MD_FileFlags;
    filename: MD_String8;
    file_size: MD_u64;
};

@send(FileSystemHelper)
@doc("An opaque iterator type used to store operating-system-specific state, when iterating files.")
@see(MD_FileIterIncrement)
@opaque
@struct MD_FileIter: {};

////////////////////////////////
//~ Basic Utilities

@send(HelperMacros)
@doc("Will cause a crash when @code 'c' is false.")
@macro MD_Assert: {
    c,
};

@send(HelperMacros)
@doc("Will cause a compilation failure when @code 'c' is false.")
@macro MD_StaticAssert: {
    c,
    @doc("A label for this static assertion. Necessary for static asserts in C.")
        label,
};

@send(HelperMacros)
@doc("Treats @code 'a' as a static array to calculate the number of elements in the array. Does not work on pointers used to point at a number of elements.")
@macro MD_ArrayCount: {
    a,
};

////////////////////////////////
//~ Linked List Macros

@send(HelperMacros) @macro MD_CheckNull: { p }
@send(HelperMacros) @macro MD_SetNull: { p }
@send(HelperMacros) @macro MD_CheckNil: { p }
@send(HelperMacros) @macro MD_SetNil: { p }

@send(HelperMacros) @macro MD_QueuePush_NZ:
{
    f,
    l,
    n,
    next,
    zchk,
    zset,
}

@send(HelperMacros) @macro MD_QueuePop_NZ:
{
    f,
    l,
    next,
    zset,
}

@send(HelperMacros) @macro MD_StackPush_N:
{
    f,
    n,
    next,
}

@send(HelperMacros) @macro MD_StackPop_NZ:
{
    f,
    next,
    zchk,
}

@send(HelperMacros) @macro MD_DblPushBack_NPZ:
{
    f,
    l,
    n,
    next,
    prev,
    zchk,
    zset,
}

@send(HelperMacros) @macro MD_DblRemove_NPZ:
{
    f,
    l,
    n,
    next,
    prev,
    zset,
}

@send(HelperMacros) @macro MD_QueuePush: { f, l, n }
@send(HelperMacros) @macro MD_QueuePop:  { f, l }
@send(HelperMacros) @macro MD_StackPush: { f, n }
@send(HelperMacros) @macro MD_StackPop:  { f }
@send(HelperMacros) @macro MD_DblPushBack: { f, l, n }
@send(HelperMacros) @macro MD_DblPushFront: { f, l, n }
@send(HelperMacros) @macro MD_DblRemove: { f, l, n }
@send(HelperMacros) @macro MD_NodeDblPushBack: { f, l, n }
@send(HelperMacros) @macro MD_NodeDblPushFront: { f, l, n }
@send(HelperMacros) @macro MD_NodeDblRemove: { f, l, n }

////////////////////////////////
//~ Memory Operations

@send(MemoryOperations)
@doc("Zeroes @code 'size' bytes at the address stored in @code 'memory'.")
@func MD_MemoryZero:
{
    memory: *void,
    size: MD_u64,
}

@send(MemoryOperations)
@doc("Copies @code 'size' bytes from the address in @code 'src', to the address in @code 'dst'.")
@func MD_MemoryCopy:
{
    dst: *void,
    src: *void,
    size: MD_u64,
}

@send(MemoryOperations)
@doc("Allocates @code 'size' bytes. This memory cannot be freed.")
@func MD_AllocZero:
{
    size: MD_u64,
}

@send(MemoryOperations)
@macro MD_PushArray: { T, c }

@send(MemoryOperations)
@macro MD_PushArrayZero: { T, c }

////////////////////////////////
//~ Characters

@send(Characters)
@doc("Returns whether an ASCII character is alphabetic.")
@func MD_CharIsAlpha: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc("Returns whether an ASCII character is alphabetic and upper-case.")
@func MD_CharIsAlphaUpper: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc("Returns whether an ASCII character is alphabetic and lower-case.")
@func MD_CharIsAlphaLower: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc("Returns whether an ASCII character is numeric.")
@func MD_CharIsDigit: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc(```
     Returns whether an ASCII character is a non-reserved symbol as defined by the Metadesk grammar: @code '~', @code '!', @code '$', @code '%', @code '^', @code '&', @code '*', @code '-', @code '=', @code '+', @code '<', @code '.',  @code '>', @code '/', @code '?', @code '|', or @code '\\'.
     ```)
@func MD_CharIsUnreservedSymbol: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc(```
     Returns whether an ASCII character is a reserved symbol as defined by the Metadesk grammar: @code '{', @code '}', @code '(', @code ')', @code '\\', @code '[', @code ']', @code '#', @code ',', @code ';', @code ':', or @code '@'.
     ```)
@func MD_CharIsReservedSymbol: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc(```
     Returns whether an ASCII character is within the set of reserved symbolic characters by the Metadesk language: @code '{', @code '}', @code '(', @code ')', @code '\\', @code '[', @code ']', @code '#', @code ',', @code ';',
     @code ':', or @code '@'.
     ```)
@func MD_CharIsReservedSymbol: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc("Return whether an ASCII character is whitespace.")
@func MD_CharIsSpace: {
    c: MD_u8,
    return: MD_b32,
};

@send(Characters)
@doc("Return the alphabetic uppercase equivalent of an ASCII character, or just returns the passed-in character if it is not alphabetic.")
@func MD_CharToUpper: {
    c: MD_u8,
    return: MD_u8,
};

@send(Characters)
@doc("Return the alphabetic lowercase equivalent of an ASCII character, or just returns the passed-in character if it is not alphabetic.")
@func MD_CharToLower: {
    c: MD_u8,
    return: MD_u8,
};

@send(Characters)
@doc("Return a @code '/' if @code '\\' is passed in, otherwise just returns the passed character.")
@func MD_CorrectSlash: {
    c: MD_u8,
    return: MD_u8,
};

////////////////////////////////
//~ Strings

@send(Strings)
@doc("Constructs an MD_String8.")
@func MD_S8: {
    @doc("The base pointer of the returned MD_String8.")
        str: *MD_u8,
    @doc("The size of the returned MD_String8.")
        size: MD_u64,
    return: MD_String8,
};

@send(Strings)
@doc("Constructs an MD_String8 from a C-string by calling an equivalent to @code 'strlen'.")
@macro MD_S8CString: {
    s,
};

@send(Strings)
@doc("Constructs an MD_String8 from a C-string literal by using @code 'sizeof'. In C++, the equivalent can be done with the user-defined literal code provided in the library, which uses @code '_md' as a suffix on a string literal.")
@macro MD_S8Lit: {
    s,
};

@send(Strings)
@doc("Constructs an MD_String8 from two pointers into the same buffer, corresponding to the beginning and one past the last byte of a string.")
@func MD_S8Range: {
    str: *MD_u8,
    opl: *MD_u8,
    return: MD_String8,
};

@send(Strings)
@doc("Returns an MD_String8 encoding a sub-range of the passed MD_String8.")
@func MD_StringSubstring: {
    @doc("The string for which the substring is returned.")
        str: MD_String8,
    @doc("The offset, from the passed string's base, of the first character of the returned substring.")
        min: MD_u64,
    @doc("The offset, from the passed string's base, of one byte past the last character of the returned substring.")
        max: MD_u64
        return: MD_String8,
};

@send(Strings)
@doc("Returns a sub-range of the passed MD_String8, skipping the first @code 'min' bytes.")
@func MD_StringSkip: {
    @doc("The string for which the substring is returned.")
        str: MD_String8,
    @doc("The new minimum offset, relative to the base of @code 'str'. Also the number of bytes to skip at the beginning of @code 'str'.")
        min: MD_u64,
    return: MD_String8,
};

@send(Strings)
@doc("Returns a sub-range of the passed MD_String8, chopping off the last @code 'min' bytes.")
@func MD_StringChop: {
    @doc("The string for which the substring is returned.")
        str: MD_String8,
    @doc("The number of bytes to chop off at the end of @code 'str'.")
        nmax: MD_u64,
    return: MD_String8,
};

@send(Strings)
@doc("Returns a prefix of the passed MD_String8.")
@func MD_StringPrefix: {
    @doc("The string for which the substring is returned.")
        str: MD_String8,
    @doc("The desired size of the returned prefix.")
        size: MD_u64,
    return: MD_String8,
};

@send(Strings)
@doc("Returns a suffix of the passed MD_String8.")
@func MD_StringSuffix: {
    @doc("The string for which the substring is returned.")
        str: MD_String8,
    @doc("The desired size of the returned suffix.")
        size: MD_u64,
    return: MD_String8,
};

@send(Strings)
@doc("Compares the passed strings @code 'a' and @code 'b', and determines whether or not the two strings match. The passed MD_MatchFlags argument will modify the string matching algorithm; for example, allowing case insensitivity. Return @code '1' if the strings are found to match, and @code '0' otherwise.")
@func MD_StringMatch: {
    @doc("The first string to compare.")
        a: MD_String8,
    @doc("The second string to compare.")
        b: MD_String8,
    @doc("Controls for the string matching algorithm.")
        flags: MD_MatchFlags,
    @doc("@code '1' if the strings match, or @code '0' otherwise.")
        return: MD_b32,
};

@send(Strings)
@doc("Searches @code 'str' for an occurrence of @code 'substring'. The passed @code 'flags' can be used to modify the matching rules. Returns the position at which the search ended; if the return value is equivalent to @code 'str.size', then the substring was not found.")
@func MD_FindSubstring: {
    @doc("The string to search within for the substring.")
        str: MD_String8,
    @doc("The 'needle' string to find within @code 'str'.")
        substring: MD_String8,
    @doc("The first position, in bytes relative to the base of @code 'str', to conduct the search from.")
        start_pos: MD_u64,
    @doc("Controls for the string matching algorithm.")
        flags: MD_MatchFlags,
    @doc("The last position at which the substring was searched for. Will be equal to @code 'str.size' if the substring was not found. If it is less than that, the substring was found at the returned offset.")
        return: MD_u64,
};

@send(Strings)
@doc("Searches @code 'string' for the last @code '.' character occurring in the string, and chops the @code '.' and anything following after it off of the returned string.")
@see(MD_StringChop)
@func MD_ChopExtension: {
    string: MD_String8,
    return: MD_String8,
};

@send(Strings)
@doc("Searches @code 'string' for the last @code '/' or @code '\' character occurring in the string, and skips it and anything before it in the returned string.")
@see(MD_StringSkip)
@func MD_SkipFolder: {
    string: MD_String8,
    return: MD_String8,
};

@send(Strings)
@doc("Searches @code 'string' for the last @code '.' character, and returns the substring starting at the character after it, to the end of the string. For usual file naming schemes where the extension of a file is encoded by any characters following the last @code '.' of a filename, this will return the extension.")
@see(MD_FolderFromPath)
@see(MD_StringSuffix)
@see(MD_StringSubstring)
@see(MD_StringChop)
@func MD_ExtensionFromPath: {
    string: MD_String8,
    return: MD_String8,
};

@send(Strings)
@doc("Searches @code 'string' for the last @code '/' or @code '\' character, and returns the substring that ends with that character. For usual file naming schemes where folders are encoded with @code '/' or @code '\' characters, this will return the entire path to the passed filename, not including the filename itself.")
@see(MD_ExtensionFromPath)
@see(MD_StringPrefix)
@see(MD_StringSubstring)
@see(MD_StringSkip)
@func MD_FolderFromPath: {
    string: MD_String8,
    return: MD_String8,
};

@send(Strings)
@doc("Copies @code 'string' by allocating an entirely new portion of memory and copying the passed string's memory to the newly allocated memory. Returns the copy of @code 'string' using the new memory.")
@func MD_PushStringCopy: {
    string: MD_String8,
    return: MD_String8,
};

@send(Strings)
@doc("Allocates a new string, with the contents of the string being determined by a mostly-standard C formatting string passed in @code 'fmt', with a variable-argument list being passed in @code 'args'. Used when composing variable argument lists at multiple levels, and when you need to pass a @code 'va_list'. The format string is non-standard because it allows @code '%S' as a specifier for MD_String8 arguments. Before this call, it is expected that you call @code 'va_start' on the passed @code 'va_list', and also that you call @code 'va_end' after the function returns. If you just want to pass variable arguments yourself (instead of a @code 'va_list'), then see MD_PushStringF.")
@see(MD_PushStringF)
@see(MD_PushStringCopy)
@func MD_PushStringFV: {
    fmt: *char,
    args: va_list,
    return: MD_String8,
};

@send(Strings)
@doc("Allocates a new string, with the contents of the string being determined by a mostly-standard C formatting string passed in @code 'fmt', with variable arguments fitting the expected ones in @code 'fmt' being passed in after. The format string is non-standard because it allows @code '%S' as a specifier for MD_String8 arguments. If you are composing this with your own variable-arguments call, use MD_PushStringFV instead.")
@see(MD_PushStringFV)
@see(MD_PushStringCopy)
@func MD_PushStringF: {
    fmt: *char,
    "...",
    return: MD_String8,
};

@send(Strings)
@doc("This is a helper macro that is normally used with passing an MD_String8 into a @code 'printf' like function, usually used in combination with the @code '%.*s' format specifier. Metadesk uses length-based strings, not null-terminated (like many C functions expect), so this is often convenient when interacting with C-like APIs. This will expand to passing the size of the passed string first, a comma, and the pointer to the base of the string being passed immediately after.")
@see(MD_String8)
@macro MD_StringExpand: { s, }

@send(Strings)
@doc("Pushes a new MD_String8 to an MD_String8List by allocating a new MD_String8Node, filling it with @code 'string', and modifying the existing list elements in @code 'list' to end with the newly allocated node.")
@see(MD_String8List)
@see(MD_String8Node)
@see(MD_String8)
@func MD_PushStringToList: {
    list: *MD_String8List,
    string: MD_String8,
};

@send(Strings)
@doc("Pushes a MD_String8List to another MD_String8List. This will zero all memory in @code 'to_push', so you cannot expect @code 'to_push' to retain any of the list elements it had before this call. This is because no strings nor nodes are copied, so the nodes in @code 'to_push' are repurposed for being a part of @code 'list'.")
@see(MD_String8List)
@see(MD_String8Node)
@see(MD_String8)
@see(MD_PushStringToList)
@func MD_PushStringListToList: {
    list: *MD_String8List,
    to_push: *MD_String8List,
};

@send(Strings)
@doc("Divides @code 'string' into an MD_String8List, each node of which corresponds to a substring of @code 'string' that was separated by an occurrence of one of the 'splitter's passed in @code 'splits'.")
@see(MD_String8)
@see(MD_String8List)
@see(MD_String8Node)
@see(MD_JoinStringList)
@func MD_SplitString: {
    @doc("The string to search for splitting strings, and to subdivide.")
        string: MD_String8,
    @doc("The number of splitting strings to search for.")
        split_count: MD_u32,
    @doc("A pointer to an array of strings stored as MD_String8 objects, each corresponding to a string pattern that should split @code 'string'.")
        splits: *MD_String8,
    @doc("A list containing all of the strings separated by the passed splitter strings. None of the strings will contain the splitter strings themselves.")
        return: MD_String8List
};

@send(Strings)
@doc("Returns a new MD_String8 that contains the contents of each string in @code 'list', in order, with the @code 'separator' string inserted between each string.")
@see(MD_String8)
@see(MD_String8List)
@see(MD_String8Node)
@see(MD_SplitString)
@func MD_JoinStringList: {
    list: MD_String8List,
    separator: MD_String8,
    return: MD_String8,
};

@send(Strings)
@func MD_CalculateCStringLength: {
    cstr: *char,
    return: MD_u64,
};

@send(Strings)
@func MD_StyledStringFromString: {
    string: MD_String8,
    word_style: MD_WordStyle,
    separator: MD_String8,
    return: MD_String8
};

////////////////////////////////
//~ Numeric Strings

@send(Strings)
@doc("Parses @code 'string' as an integer with a base defined by @code 'radix'. Returns the parsed value.")
@func MD_U64FromString: {
    string: MD_String8,
    radix: MD_u32,
    return: MD_u64,
};

@send(Strings)
@doc("Parses @code 'string' as an integer with a base defined by C-like rules: If the numeric part of the string begins with @code '0x', then it will be parsed as base-16. If it begins with @code '0b', it will be parsed as base-2. Otherwise, it will be parsed as base 10. Returns the parsed value.")
@func MD_CStyleIntFromString: {
    string: MD_String8,
    return: MD_i64,
};

@send(Strings)
@doc("Parses @code 'string' as a floating point number, and returns the parsed value.")
@func MD_F64FromString: {
    string: MD_String8,
    return: MD_f64,
};

////////////////////////////////
//~ Enum/Flag Strings

@send(Nodes)
@doc("Returns a string that contains a name matching @code 'kind'.")
@func MD_StringFromNodeKind: {
    kind: MD_NodeKind,
    return: MD_String8,
};

@send(Nodes)
@doc("Builds a string list for all bits set in @code 'flags', with each string being the name of one of the flags that is set.")
@func MD_StringListFromNodeFlags: {
    flags: MD_NodeFlags,
    return: MD_String8List,
};

////////////////////////////////
//~ Unicode Conversions

@send(Strings)
@func MD_CodepointFromUtf8: {
    str: MD_u8,
    max: MD_u64,
    return: MD_UnicodeConsume,
};

@send(Strings)
@func MD_CodepointFromUtf16: {
    str: *MD_u16,
    max: MD_u64,
    return: MD_UnicodeConsume,
};

@send(Strings)
@func MD_Utf8FromCodepoint: {
    out: *MD_u8,
    codepoint: MD_u32,
    return: MD_u32,
};

@send(Strings)
@func MD_Utf16FromCodepoint: {
    out: *MD_u16,
    codepoint: MD_u32,
    return: MD_u32,
};

@send(Strings)
@func MD_S8FromS16: {
    str: MD_String16,
    return: MD_String8,
};

@send(Strings)
@func MD_S16FromS8: {
    str: MD_String8,
    return: MD_String16,
};

@send(Strings)
@func MD_S8FromS32: {
    str: MD_String32,
    return: MD_String8,
};

@send(Strings)
@func MD_S32FromS8: {
    str: MD_String8,
    return: MD_String32,
};

////////////////////////////////
//~ Map Table Data Structure

@send(Map)
@func MD_HashString: {
    string: MD_String8,
    return: MD_u64,
};

@send(Map)
@func MD_HashPointer: {
    p: *void,
};

@send(Map)
MD_MapMakeBucketCount: {
    bucket_count: MD_u64,
    return: MD_Map,
};

@send(Map)
MD_MapMake: {
    return: MD_Map,
};

@send(Map)
MD_MapKeyStr: {
    string: MD_String8,
    return: MD_MapKey,
};

@send(Map)
MD_MapKeyPtr: {
    ptr: *void,
    return: MD_MapKey,
};

@send(Map)
MD_MapLookup: {
    map: *MD_Map,
    key: MD_MapKey,
    return: *MD_MapSlot,
};

@send(Map)
MD_MapScan: {
    first_slot: *MD_MapSlot,
    key: MD_MapKey,
    return: *MD_MapSlot,
};

@send(Map)
MD_MapInsert: {
    map: *MD_Map,
    key: MD_MapKey,
    val: *void,
    return: *MD_MapSlot,
};

@send(Map)
MD_MapOverwrite: {
    map: *MD_Map,
    key: MD_MapKey,
    val: *void,
    return: *MD_MapSlot,
};

////////////////////////////////
//~ Parsing

@send(Parsing) @func
@doc("Produces a single token, given some input string.")
@see(MD_Token)
@see(MD_TokenKind)
MD_TokenFromString:
{
    string: MD_String8;
    return: MD_Token;
}

@send(Parsing) @func
@doc("Returns the number of bytes that can be skipped, when skipping over certain token kinds.")
@see(MD_Token)
@see(MD_TokenKind)
MD_LexAdvanceFromSkips:
{
    string: MD_String8;
    skip_kinds: MD_TokenKind;
    return: MD_u64;
}

@send(Parsing) @func
@doc("Allocates and initializes an MD_Error associated with a particular MD_Node.")
@see(MD_Error)
MD_MakeNodeError:
{
    @doc("The node associated with the message.")
        node: *MD_Node;
    @doc("The message kind, encoding its severity.")
        kind: MD_MessageKind;
    @doc("The string for the message.")
        str: MD_String8;
    return: *MD_Error
}

@send(Parsing) @func
@doc("Allocates and initializes an MD_Error associated with a particular MD_Token.")
@see(MD_Error)
MD_MakeTokenError:
{
    @doc("The entire string that is being parsed. The parser used a substring of this string to form @code 'token'.")
        parse_contents: MD_String8;
    @doc("The token associated with this message.")
        token: MD_Token;
    @doc("The message kind, encoding its severity.")
        kind: MD_MessageKind;
    @doc("The string for the message.")
        str: MD_String8;
    return: *MD_Error;
}

@send(Parsing) @func
@doc("Pushes a constructed MD_Error into an MD_ErrorList.")
@see(MD_Error)
@see(MD_ErrorList)
MD_PushErrorToList:
{
    list: *MD_ErrorList;
    error: *MD_Error;
}

@send(Parsing) @func
@see(MD_Error)
@see(MD_ErrorList)
@doc("Pushes the contents of @code 'to_push' into @code 'list'. Zeroes @code 'to_push'; the memory used in forming @code 'to_push' will be used in @code 'list', and nothing will be copied.")
MD_PushErrorListToList:
{
    list: *MD_ErrorList;
    to_push: *MD_ErrorList;
}

@send(Parsing) @func
@doc("Constructs a default MD_ParseResult, which indicates that nothing was parsed.")
@see(MD_ParseResult)
MD_ParseResultZero:
{
    return: MD_ParseResult;
}

@send(Parsing) @func
@doc("Parses a single Metadesk node set, starting at @code 'offset' bytes into @code 'string'. Parses the associated set delimiters in accordance with @code 'rule'.")
@see(MD_ParseSetRule)
MD_ParseNodeSet:
{
    @doc("The string containing the source text to parse.")
        string: MD_String8;
    @doc("The offset into @code 'string' where this function should start parsing.")
        offset: MD_u64;
    @doc("The parent node for which the set's children are being parsed.")
        parent: *MD_Node;
    @doc("The rule to use for determining the end of the set.")
        rule: MD_ParseSetRule;
    return: MD_ParseResult;
}

@send(Parsing) @func
@doc("Parses a tag list, starting at @code 'offset' bytes into @code 'string'.")
MD_ParseTagList:
{
    @doc("The string containing the source text to parse.")
        string: MD_String8;
    @doc("The offset into @code 'string' where this function should start parsing.")
        offset: MD_u64;
    return: MD_ParseResult;
}

@send(Parsing) @func
@doc("Parses a single Metadesk subtree, starting at @code 'offset' bytes into @code 'string'.")
MD_ParseOneNode:
{
    @doc("The string containing the source text to parse.")
        string: MD_String8;
    @doc("The offset into @code 'string' where this function should start parsing.")
        offset: MD_u64;
    return: MD_ParseResult;
}

@send(Parsing) @func
@doc("Parses an entire string encoding Metadesk. Parents all parsed nodes with a node with @code 'MD_NodeKind_File' set as its kind.")
@see(MD_NodeKind)
MD_ParseWholeString:
{
    @doc("The filename to associate with the parse.")
        filename: MD_String8;
    @doc("The string that contains the text to parse.")
        contents: MD_String8;
    return: MD_ParseResult;
}

@send(Parsing) @func
@doc("Uses the C standard library to load the file associated with @code 'filename', and parses all of it to return a single tree for the whole file.")
MD_ParseWholeFile:
{
    @doc("The filename for the file to be loaded and parsed.")
        filename: MD_String8;
    return: MD_ParseResult;
}

////////////////////////////////
//~ Location Conversion

@send(CodeLoc)
@func MD_CodeLocFromFileBaseOff: {
    filename: MD_String8,
    base: *MD_u8,
    off: *MD_u8,
    return: MD_CodeLoc,
};

@send(CodeLoc)
@func MD_CodeLocFromNode: {
    node: *MD_Node,
    return: MD_CodeLoc,
};

////////////////////////////////
//~ Tree/List Building

@send(Nodes)
@func MD_NodeIsNil: {
    node: *MD_Node,
    return: MD_b32,
};

@send(Nodes)
@func MD_NilNode: {
    return: *MD_Node,
};

@send(Nodes)
@func MD_PushChild: {
    parent: *MD_Node,
    new_child: *MD_Node,
};

@send(Nodes)
@func MD_PushTag: {
    node: *MD_Node,
    tag: *MD_Node,
};

@send(Nodes)
@func MD_PushReference: {
    list: *MD_Node,
    target: *MD_Node,
    return: *MD_Node,
};

////////////////////////////////
//~ Introspection Helpers

@send(Nodes)
@func MD_NodeFromString: {
    first: *MD_Node,
    last: *MD_Node,
    string: MD_String8,
    return: *MD_Node,
};

@send(Nodes)
@func MD_NodeFromIndex: {
    first: *MD_Node,
    last: *MD_Node,
    n: int,
    return: *MD_Node,
};

@send(Nodes)
@func MD_IndexFromNode: {
    node: *MD_Node,
    return: int,
};

@send(Nodes)
@func MD_NextNodeSibling: {
    last: *MD_Node,
    string: MD_String8,
    return: *MD_Node,
};

@send(Nodes)
@func MD_ChildFromString: {
    node: *MD_Node,
    child_string: MD_String8,
    return: *MD_Node,
};

@send(Nodes)
@func MD_TagFromString: {
    node: *MD_Node,
    tag_string: MD_String8,
    return: *MD_Node,
};

@send(Nodes)
@func MD_ChildFromIndex: {
    node: *MD_Node,
    n: int,
    return: *MD_Node,
};

@send(Nodes)
@func MD_TagFromIndex: {
    node: *MD_Node,
    n: int,
    return: *MD_Node,
};

@send(Nodes)
@func MD_TagArgFromIndex: {
    node: *MD_Node,
    tag_string: MD_String8,
    n: int,
    return: *MD_Node,
};

@send(Nodes)
@func MD_NodeHasTag: {
    node: *MD_Node,
    tag_string: MD_String8,
    return: MD_b32,
};

@send(Nodes)
@func MD_ChildCountFromNode: {
    node: *MD_Node,
    return: MD_i64,
};

@send(Nodes)
@func MD_TagCountFromNode: {
    node: *MD_Node,
    return: MD_i64,
};

@send(Nodes)
@macro MD_EachNode: { it, first, };

////////////////////////////////
//~ Error/Warning Helpers

@send(Nodes)
@func MD_Message: {
    out: *FILE,
    loc: MD_CodeLoc,
    kind: MD_MessageKind,
    str: MD_String8,
};

@send(Nodes)
@func MD_Message: {
    out: *FILE,
    loc: MD_CodeLoc,
    kind: MD_MessageKind,
    fmt: *char,
    "..."
};

@send(Nodes)
@func MD_NodeMessage: {
    out: *FILE,
    node: *MD_Node,
    kind: MD_MessageKind,
    str: MD_String8,
};

@send(Nodes)
@func MD_NodeMessageF: {
    out: *FILE,
    node: *MD_Node,
    kind: MD_MessageKind,
    fmt: *char,
    "..."
};

////////////////////////////////
//~ Tree Comparison/Verification

@send(Nodes)
@func MD_NodeMatch: {
    a: *MD_Node,
    b: *MD_Node,
    str_flags: MD_StringMatchFlags,
    node_flags: MD_NodeMatchFlags,
    return: MD_b32,
};

@send(Nodes)
@func MD_NodeDeepMatch: {
    a: *MD_Node,
    b: *MD_Node,
    str_flags: MD_StringMatchFlags,
    node_flags: MD_NodeMatchFlags,
    return: MD_b32,
};

////////////////////////////////
//~ Generation

@send(Output)
@func MD_OutputTree: {
    file: *FILE,
    node: *MD_Node,
};

////////////////////////////////
//~ Command Line Argument Helper

@send(CommandLineHelper)
@func MD_CommandLine_Start: {
    argument_count: int,
    arguments: **char,
    return: MD_CommandLine,
};

@send(CommandLineHelper)
@func MD_CommandLine_Flag: {
    cmdln: *MD_CommandLine,
    string: MD_String8,
    return: MD_b32,
};

@send(CommandLineHelper)
@func MD_CommandLine_FlagStrings: {
    cmdln: *MD_CommandLine,
    string: MD_String8,
    out_count: int,
    out: *MD_String8,
    return: MD_b32,
};

@send(CommandLineHelper)
@func MD_CommandLine_FlagIntegers: {
    cmdln: *MD_CommandLine,
    string: MD_String8,
    out_count: int,
    out: *MD_i64,
    return: MD_b32,
};

@send(CommandLineHelper)
@func MD_CommandLine_FlagString: {
    cmdln: *MD_CommandLine,
    string: MD_String8,
    out: *MD_String8,
    return: MD_b32,
};

@send(CommandLineHelper)
@func MD_CommandLine_FlagInteger: {
    cmdln: *MD_CommandLine,
    string: MD_String8,
    out: *MD_i64,
    return: MD_b32,
};

@send(CommandLineHelper)
@func MD_CommandLine_Increment: {
    cmdln: *MD_CommandLine,
    string_ptr: **MD_String8,
    return: MD_b32,
};

////////////////////////////////
//~ File System

@send(FileSystemHelper)
@func MD_LoadEntireFile: {
    filename: MD_String8,
    return: MD_String8,
};

@send(FileSystemHelper)
@func MD_FileIterIncrement: {
    it: *MD_FileIter,
    path: MD_String8,
    out_info: *MD_FileInfo,
    return: MD_b32,
};

////////////////////////////////
//~ C Helper
////////////////////////////////


////////////////////////////////
//~ Expression and Type-Expression parser helper types.

// VERY_IMPORTANT_NOTE(rjf): If this enum is ever changed, ensure that
// it is kept in-sync with the MD_ExprPrecFromExprKind function.

@send(ExpressionParsingHelper)
@enum MD_C_ExprKind: {
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

@send(ExpressionParsingHelper)
@enum MD_C_ExprKindGroup: {
    Nil,
    Atom,
    Binary,
    PreUnary,
    PostUnary,
    Type,
};

@send(ExpressionParsingHelper)
@typedef(MD_i32) MD_C_ExprPrec;

@send(ExpressionParsingHelper)
@struct MD_C_Expr: {
    node: *MD_Node,
    kind: MD_ExprKind,
    parent: *MD_C_Expr,
    sub: ([2]*MD_C_Expr),
};

////////////////////////////////
//~ Expression and Type-Expression Helper

@send(ExpressionParsingHelper)
@func MD_C_NilExpr: {
    return: *MD_C_Expr,
};

@send(ExpressionParsingHelper)
@func MD_C_ExprIsNil: {
    expr: *MD_C_Expr,
    return: MD_b32,
};

@send(ExpressionParsingHelper)
@func MD_C_PreUnaryExprKindFromNode: {
    node: *MD_Node,
    return: MD_ExprKind,
};

@send(ExpressionParsingHelper)
@func MD_C_BinaryExprKindFromNode: {
    node: *MD_Node,
    return: MD_ExprKind,
};

@send(ExpressionParsingHelper)
@func MD_C_ExprPrecFromExprKind: {
    kind: MD_ExprKind,
    return: MD_ExprPrec,
};

@send(ExpressionParsingHelper)
@func MD_C_MakeExpr: {
    node: *MD_Node,
    kind: MD_ExprKind,
    left: *MD_C_Expr,
    right: *MD_C_Expr,
    return: *MD_C_Expr,
};

@send(ExpressionParsingHelper)
@func MD_C_ParseAsExpr: {
    first: *MD_Node,
    last: *MD_Node,
    return: *MD_C_Expr,
};

@send(ExpressionParsingHelper)
@func MD_C_ParseAsType: {
    first: *MD_Node,
    last: *MD_Node,
    return: *MD_C_Expr,
};

@send(ExpressionParsingHelper)
@func MD_C_EvaluateExpr_I64: {
    expr: *MD_C_Expr,
    return: MD_i64,
};

@send(ExpressionParsingHelper)
@func MD_C_EvaluateExpr_F64: {
    expr: *MD_C_Expr,
    return: MD_f64,
};

@send(ExpressionParsingHelper)
@func MD_C_ExprMatch: {
    a: *MD_C_Expr,
    b: *MD_C_Expr,
    str_flags: MD_StringMatchFlags,
    return: MD_b32,
};

@send(ExpressionParsingHelper)
@func MD_C_ExprDeepMatch: {
    a: *MD_C_Expr,
    b: *MD_C_Expr,
    str_flags: MD_StringMatchFlags,
    return: MD_b32,
};

////////////////////////////////
//~ C Language Generation

@send(Output)
@func MD_C_Generate_String: {
    file: *FILE,
    node: *MD_Node,
};

@send(Output)
@func MD_C_Generate_Struct: {
    file: *FILE,
    node: *MD_Node,
};

@send(Output)
@func MD_C_Generate_Decl: {
    file: *FILE,
    node: *MD_Node,
};

@send(Output)
@func MD_C_Generate_DeclByNameAndType: {
    file: *FILE,
    name: MD_String8,
    type: *MD_C_Expr,
};

@send(Output)
@func MD_C_Generate_Expr: {
    file: *FILE,
    expr: *MD_C_Expr,
};

@send(Output)
@func MD_C_Generate_TypeLHS: {
    file: *FILE,
    type: *MD_C_Expr,
};

@send(Output)
@func MD_C_Generate_TypeRHS: {
    file: *FILE,
    type: *MD_C_Expr,
};
