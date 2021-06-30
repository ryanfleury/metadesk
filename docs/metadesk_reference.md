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
@doc("These constants control the @code `MD_StyledStringFromString` function.")
@enum MD_WordStyle: {
    @doc("Creates identifiers that look like this @code `ExampleIdentifier`")
        UpperCamelCase,
    @doc("Creates identifiers that look like this @code `exampleIdentifier`")
        LowerCamelCase,
    @doc("Creates identifiers that look like this @code `Example_Identifier`")
        UpperCase,
    @doc("Creates identifiers that look like this @code `example_identifier`")
        LowerCase,
};

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

////////////////////////////////
//~ Node types that are used to build all ASTs.

@send(Nodes)
@doc("These constants distinguish major roles of @code `MD_Node` in the Metadesk abstract syntax tree data structure.")
@enum MD_NodeKind: {
    @doc("The Nil node is a unique node representing the lack of information, for example iterating off the end of a list, or up to the parent of a root node results in Nil.")
        Nil,
    
    @doc("A File node represents parsed metadesk source text.")
        File,
    
    @doc("A List node serves as the root of an externally chained list of nodes. Its children are nodes with the @code 'MD_NodeKind_Reference' kind.")
        List,
    
    @doc("A Reference node is an indirection to another node. The node field @code 'ref_target' contains a pointer to the referenced node. These nodes are typically used for creating externally chained linked lists that gather nodes from a parse tree.")
        Reference,
    
    @doc("A Label node represents the main structure of the metadesk abstract syntax tree. Some labels have children which will also be labels. Labels can be given their text by identifiers, numerics, string and character literals, and operator symbols.")
        @see(MD_TokenKind)
        Label,
    
    @doc("A Tag node represents a tag attached to a label node with the @code '@identifer' syntax. The children of a tag node represent the arguments placed in the tag.")
        Tag,
    
    @doc("Not a real kind value given to nodes, this is always one larger than the largest enum value that can be given to a node.")
        MAX,
};

@send(Nodes)
@doc("These flags are set on @code `MD_Node` to indicate particular details about the strings that were parsed to create the node.")
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
    @doc("The delimiter between this node and its next sibling is a @code ','")
        BeforeComma,
    
    @doc("The delimiter between this node and its previous sibling is a @code ';'")
        AfterSemicolon,
    @doc("The delimiter between this node and its previous sibling is a @code ','")
        AfterComma,
    
    @doc("The label on this node comes from a token with the @code MD_TokenKind_NumericLiteral kind.")
        Numeric,
    @doc("The label on this node comes from a token with the @code MD_TokenKind_Identifier kind.")
        Identifier,
    @doc("The label on this node comes from a token with the @code MD_TokenKind_StringLiteral kind.")
        StringLiteral,
    @doc("The label on this node comes from a token with the @code MD_TokenKind_CharLiteral kind.")
        CharLiteral,
};

@send(Nodes)
@doc("The @code `MD_Node` is the main 'lego-brick' for modeling the result of a metadesk parse. Also used in some auxiliary data structures.")
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
//~ String-To-Node table

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
    @doc("For slots with a string key, the hash of the key.")
        hash: MD_u64,
    @doc("The key for slots with a pointer key.")
        key: *void;
    @doc("The value part of the pair.")
        value: *void;
};

@send(Map)
@doc("The map is a chained hash table data structure. Data written to the map is a key-value pair. The key of a pair may either be a pointer, or a string. Both types may be mixed inside a single map. Keys stored with one type never match keys of the other type. The values of the pairs are pointers.")
@struct MD_Map: {
    table_size: MD_u64,
    table: **MD_MapSlot,
};

////////////////////////////////
//~ Tokens

@send(Tokens)
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
    Whitespace,
    Newline,
    BadCharacter,
    
    MAX,
};

@send(Tokens)
@struct MD_Token: {
    kind: MD_TokenKind,
    string: MD_String8,
    outer_string: MD_String8,
};

@send(Tokens)
@prefix(MD_TokenGroup)
@base_type(MD_u32)
@flags MD_TokenGroups: {
    Comment,
    Whitespace,
    Regular,
};

////////////////////////////////
//~ Parsing State

@send(Parsing)
@enum MD_MessageKind: {
    None,
    Warning,
    Error,
    CatastrophicError,
}

@send(Parsing)
@struct MD_Error: {
    next: *MD_Error,
    string: MD_String8,
    filename: MD_String8,
    node: *MD_Node,
    catastrophic: MD_b32,
    location: MD_CodeLoc,
};

@send(Parsing)
@struct MD_ParseCtx: {
    first_error: *MD_Error,
    last_error: *MD_Error,
    at: *MD_u8,
    filename: MD_String8,
    file_contents: MD_String8,
    catastrophic_error: MD_b32,
};

@send(Parsing)
@struct MD_ParseResult: {
    node: *MD_Node;
    first_error: *MD_Error;
    bytes_parse: MD_u64;
};

////////////////////////////////
//~ Command line parsing helper types.

@send(CommandLineHelper)
@struct MD_CommandLine: {
    arguments: *MD_String8;
    argument_count: MD_u32;
};

////////////////////////////////
//~ File system access types.

@send(FileSystemHelper)
@prefix(MD_FileFlag)
@base_type(MD_u32)
@flags MD_FileFlags: {
    Directory,
};

@send(FileSystemHelper)
@struct MD_FileInfo: {
    flags: MD_FileFlags;
    filename: MD_String8;
    file_size: MD_u64;
};

@send(FileSystemHelper)
@opaque
@struct MD_FileIter: {};

////////////////////////////////
//~ Basic Utilities

@send(HelperMacros)
@macro MD_Assert: {
    c,
};

@send(HelperMacros)
@macro MD_StaticAssert: {
    c,
};

@send(HelperMacros)
@macro MD_ArrayCount: {
    a,
};

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
     Returns whether an ASCII character is a within the set of characters that Metadesk considers to be symbols: @code '~', @code '!', @code '@', @code '#', @code '$', @code '%', @code '^', @code '&', @code '*', @code '(', @code ')', @code '-', @code '=', @code '+', @code '[', @code ']', @code '{', @code '}', @code ':', @code ';', @code ',', @code '<', @code '.', @code '>', @code '/', @code '?', @code '|', or @code '\\'.
     ```)
@func MD_CharIsSymbol: {
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
@doc("Constructs an MD_String8 from a C-string literal by using @code 'sizeof'.")
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
@doc("Allocates a new string, with the contents of the string being determined by a standard C formatting string passed in @code 'fmt', with a variable-argument list being passed in @code 'args'. Used when composing variable argument lists at multiple levels, and when you need to pass a @code 'va_list'. Before this call, it is expected that you call @code 'va_start' on the passed @code 'va_list', and also that you call @code 'va_end' after the function returns. If you just want to pass variable arguments yourself (instead of a @code 'va_list'), then see MD_PushStringF.")
@see(MD_PushStringF)
@see(MD_PushStringCopy)
@func MD_PushStringFV: {
    fmt: *char,
    args: va_list,
    return: MD_String8,
};

@send(Strings)
@doc("Allocates a new string, with the contents of the string being determined by a standard C formatting string passed in @code 'fmt', with variable arguments fitting the expected ones in @code 'fmt' being passed in after. If you are composing this with your own variable-arguments call, use MD_PushStringF instead.")
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
@doc("Returns a new MD_String8 that contains the contents of each string in @code 'list', in order, with no separator character.")
@see(MD_String8)
@see(MD_String8List)
@see(MD_String8Node)
@see(MD_SplitString)
@see(MD_JoinStringListWithSeparator)
@func MD_JoinStringList: {
    list: MD_String8List,
    return: MD_String8,
};

@send(Strings)
@func MD_JoinStringListWithSeparator: {
    list: MD_String8List,
    separator: MD_String8
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
@func MD_U64FromString: {
    string: MD_String8,
    radix: MD_u32,
    return: MD_u64,
};

@send(Strings)
@func MD_CStyleIntFromString: {
    string: MD_String8,
    return: MD_i64,
};

@send(Strings)
@func MD_F64FromString: {
    string: MD_String8,
    return: MD_f64,
};


////////////////////////////////
//~ Enum/Flag Strings

@send(Nodes)
@func MD_StringFromNodeKind: {
    kind: MD_NodeKind,
    return: MD_String8,
};

@send(Nodes)
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
//~ String-To-Pointer Table

// TODO(allen): update reference for map

@send(Map)
@func MD_StringMap_Lookup: {
    table: *MD_Map,
    string: MD_String8,
    return: *MD_MapSlot,
};

@send(Map)
@func MD_StringMap_Insert: {
    table: *MD_Map,
    collision_rule: MD_MapCollisionRule,
    string: MD_String8,
    node: *MD_Node,
    return: MD_b32,
};

////////////////////////////////
//~ Pointer-To-Pointer Table

@send(Map)
@func MD_PtrMap_Lookup: {
    table: *MD_Map,
    key: *void,
    return: *MD_MapSlot,
};

@send(Map)
@func MD_PtrMap_Insert: {
    table: *MD_Map,
    collision_rule: MD_MapCollisionRule,
    key: *void,
    node: *MD_Node,
    return: MD_b32,
};

////////////////////////////////
//~ Parsing

@send(Tokens)
@func MD_TokenKindIsWhitespace: {
    kind: MD_TokenKind,
    return: MD_b32,
};

@send(Tokens)
@func MD_TokenKindIsComment: {
    kind: MD_TokenKind,
    return: MD_b32,
};

@send(Tokens)
@func MD_TokenKindIsRegular: {
    kind: MD_TokenKind,
    return: MD_b32,
};

@send(Parsing)
@func MD_PushNodeError: {
    ctx: *MD_ParseCtx,
    node: *MD_Node,
    kind: MD_MessageKind,
    str: MD_String8,
};

@send(Parsing)
@func MD_PushNodeErrorF: {
    ctx: *MD_ParseCtx,
    node: *MD_Node,
    kind: MD_MessageKind,
    fmt: *char,
    "..."
};

@send(Parsing)
@func MD_PushTokenError: {
    ctx: *MD_ParseCtx,
    token: MD_Token,
    kind: MD_MessageKind,
    str: MD_String8,
};

@send(Parsing)
@func MD_PushTokenErrorF: {
    ctx: *MD_ParseCtx,
    token: MD_Token,
    kind: MD_MessageKind,
    fmt: *char,
    "..."
};

@send(Parsing)
@func MD_Parse_InitializeCtx: {
    filename: MD_String8,
    contents: MD_String8,
    return: MD_ParseCtx,
};

@send(Parsing)
@func MD_Parse_Bump: {
    ctx: *MD_ParseCtx,
    token: MD_Token,
};

@send(Parsing)
@func MD_Parse_BumpNext: {
    ctx: *MD_ParseCtx,
};

@send(Parsing)
@func MD_Parse_LexNext: {
    ctx: *MD_ParseCtx,
    return: MD_Token,
};

@send(Parsing)
@func MD_Parse_PeekSkipSome: {
    ctx: *MD_ParseCtx,
    skip_groups: MD_TokenGroups,
    return: MD_Token,
};

@send(Parsing)
@func MD_Parse_Require: {
    ctx: *MD_ParseCtx,
    string: MD_String8,
    return: MD_b32,
};

@send(Parsing)
@func MD_Parse_RequireKind: {
    ctx: *MD_ParseCtx,
    kind: MD_TokenKind,
    out_token: *MD_Token,
    return: MD_b32,
};

@send(Parsing)
@func MD_ParseOneNode: {
    filename: MD_String8,
    contents: MD_String8,
    return: MD_ParseResult,
};

@send(Parsing)
@func MD_ParseWholeString: {
    filename: MD_String8,
    contents: MD_String8,
    return: MD_ParseResult,
};

@send(Parsing)
@func MD_ParseWholeFile: {
    filename: MD_String8,
    return: MD_ParseResult,
};

////////////////////////////////
//~ Location Conversion

@send(CodeLoc)
@func MD_CodeLocFromFileOffset: {
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
