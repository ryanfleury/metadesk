// LICENSE AT END OF FILE (MIT).

#define MD_FUNCTION_IMPL MD_FUNCTION
#define MD_PRIVATE_FUNCTION_IMPL MD_FUNCTION_IMPL
#define MD_UNTERMINATED_TOKEN_LEN_CAP 20

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) md_stbsp_##name
#include "md_stb_sprintf.h"

//~ Nil Node Definition

static MD_Node _md_nil_node =
{
    &_md_nil_node,         // next
    &_md_nil_node,         // prev
    &_md_nil_node,         // parent
    &_md_nil_node,         // first_child
    &_md_nil_node,         // last_child
    &_md_nil_node,         // first_tag
    &_md_nil_node,         // last_tag
    MD_NodeKind_Nil,       // kind
    0,                     // flags
    MD_ZERO_STRUCT,        // string
    MD_ZERO_STRUCT,        // whole_string
    0xdeadffffffffffull,   // string_hash
    MD_ZERO_STRUCT,        // comment_before
    MD_ZERO_STRUCT,        // comment_after
    0,                     // at
    &_md_nil_node,         // ref_target
};

//~ Memory Operations

MD_FUNCTION_IMPL void
MD_MemoryZero(void *memory, MD_u64 size)
{
    memset(memory, 0, size);
}

MD_FUNCTION_IMPL void
MD_MemoryCopy(void *dest, void *src, MD_u64 size)
{
    memcpy(dest, src, size);
}

MD_FUNCTION_IMPL void *
MD_AllocZero(MD_u64 size)
{
#if !defined(MD_IMPL_Alloc)
# error Missing implementation detail MD_IMPL_Alloc
#else
    void *result = MD_IMPL_Alloc(size);
    MD_MemoryZero(result, size);
    return(result);
#endif
}

//~ Characters

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlpha(MD_u8 c)
{
    return MD_CharIsAlphaUpper(c) || MD_CharIsAlphaLower(c);
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaUpper(MD_u8 c)
{
    return c >= 'A' && c <= 'Z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaLower(MD_u8 c)
{
    return c >= 'a' && c <= 'z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsDigit(MD_u8 c)
{
    return (c >= '0' && c <= '9');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSymbol(MD_u8 c)
{
    return (c == '~' || c == '!' || c == '@' || c == '#' || c == '$' ||
            c == '%' || c == '^' || c == '&' || c == '*' || c == '(' ||
            c == ')' || c == '-' || c == '=' || c == '+' || c == '[' ||
            c == ']' || c == '{' || c == '}' || c == ':' || c == ';' ||
            c == ',' || c == '<' || c == '.' || c == '>' || c == '/' ||
            c == '?' || c == '|' || c == '\\');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsReservedSymbol(MD_u8 c)
{
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '\\' ||
            c == '[' || c == ']' || c == '#' || c == ',' || c == ';'  ||
            c == ':' || c == '@');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSpace(MD_u8 c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v';
}

MD_FUNCTION_IMPL MD_u8
MD_CharToUpper(MD_u8 c)
{
    return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CharToLower(MD_u8 c)
{
    return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CorrectSlash(MD_u8 c)
{
    return (c == '\\' ? '/' : c);
}

//~ Strings

MD_FUNCTION_IMPL MD_String8
MD_S8(MD_u8 *str, MD_u64 size)
{
    MD_String8 string;
    string.str = str;
    string.size = size;
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_S8Range(MD_u8 *str, MD_u8 *opl)
{
    MD_String8 string;
    string.str = str;
    string.size = (MD_u64)(opl - str);
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSubstring(MD_String8 str, MD_u64 min, MD_u64 max)
{
    if(max > str.size)
    {
        max = str.size;
    }
    if(min > str.size)
    {
        min = str.size;
    }
    if(min > max)
    {
        MD_u64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSkip(MD_String8 str, MD_u64 min)
{
    return MD_StringSubstring(str, min, str.size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringChop(MD_String8 str, MD_u64 nmax)
{
    return MD_StringSubstring(str, 0, str.size - nmax);
}

MD_FUNCTION_IMPL MD_String8
MD_StringPrefix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, 0, size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringSuffix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, str.size - size, str.size);
}

MD_FUNCTION_IMPL MD_b32
MD_StringMatch(MD_String8 a, MD_String8 b, MD_MatchFlags flags)
{
    int result = 0;
    if(a.size == b.size || flags & MD_MatchFlag_RightSideSloppy)
    {
        result = 1;
        for(MD_u64 i = 0; i < a.size; i += 1)
        {
            MD_b32 match = (a.str[i] == b.str[i]);
            if(flags & MD_MatchFlag_CaseInsensitive)
            {
                match |= (MD_CharToLower(a.str[i]) == MD_CharToLower(b.str[i]));
            }
            if(flags & MD_MatchFlag_SlashInsensitive)
            {
                match |= (MD_CorrectSlash(a.str[i]) == MD_CorrectSlash(b.str[i]));
            }
            if(match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_u64
MD_FindSubstring(MD_String8 str, MD_String8 substring, MD_u64 start_pos, MD_MatchFlags flags)
{
    MD_b32 found = 0;
    MD_u64 found_idx = str.size;
    for(MD_u64 i = start_pos; i < str.size; i += 1)
    {
        if(i + substring.size <= str.size)
        {
            MD_String8 substr_from_str = MD_StringSubstring(str, i, i+substring.size);
            if(MD_StringMatch(substr_from_str, substring, flags))
            {
                found_idx = i;
                found = 1;
                if(!(flags & MD_MatchFlag_FindLast))
                {
                    break;
                }
            }
        }
    }
    return found_idx;
}

MD_FUNCTION_IMPL MD_String8
MD_ChopExtension(MD_String8 string)
{
    MD_u64 period_pos = MD_FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.size = period_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_SkipFolder(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindSubstring(string, MD_S8Lit("/"), 0,
                                        MD_MatchFlag_SlashInsensitive|
                                        MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.str += slash_pos+1;
        string.size -= slash_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_ExtensionFromPath(MD_String8 string)
{
    MD_u64 period_pos = MD_FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.str += period_pos+1;
        string.size -= period_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_FolderFromPath(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindSubstring(string, MD_S8Lit("/"), 0,
                                        MD_MatchFlag_SlashInsensitive|
                                        MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.size = slash_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringCopy(MD_String8 string)
{
    MD_String8 res;
    res.size = string.size;
    res.str = MD_PushArray(MD_u8, string.size + 1);
    MD_MemoryCopy(res.str, string.str, string.size);
    return(res);
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringFV(char *fmt, va_list args)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args2;
    va_copy(args2, args);
    MD_u64 needed_bytes = md_stbsp_vsnprintf(0, 0, fmt, args)+1;
    result.str = MD_PushArray(MD_u8, needed_bytes);
    result.size = needed_bytes - 1;
    md_stbsp_vsnprintf((char*)result.str, needed_bytes, fmt, args2);
    return result;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringF(char *fmt, ...)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args;
    va_start(args, fmt);
    result = MD_PushStringFV(fmt, args);
    va_end(args);
    return result;
}

MD_FUNCTION_IMPL void
MD_PushStringToList(MD_String8List *list, MD_String8 string)
{
    MD_String8Node *node = MD_PushArray(MD_String8Node, 1);
    node->string = string;
    
    MD_QueuePush(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += string.size;
}

MD_FUNCTION_IMPL void
MD_PushStringListToList(MD_String8List *list, MD_String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MD_MemoryZero(to_push, sizeof(*to_push));
}

MD_FUNCTION_IMPL MD_String8List
MD_SplitString(MD_String8 string, int split_count, MD_String8 *splits)
{
    MD_String8List list = MD_ZERO_STRUCT;
    
    MD_u64 split_start = 0;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        MD_b32 was_split = 0;
        for(int split_idx = 0; split_idx < split_count; split_idx += 1)
        {
            MD_b32 match = 0;
            if(i + splits[split_idx].size <= string.size)
            {
                match = 1;
                for(MD_u64 split_i = 0; split_i < splits[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splits[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                MD_String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_PushStringToList(&list, split_string);
                split_start = i + splits[split_idx].size;
                i += splits[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            MD_String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_PushStringToList(&list, split_string);
            break;
        }
    }
    
    return list;
}

MD_FUNCTION_IMPL MD_String8
MD_JoinStringList(MD_String8List list, MD_String8 separator)
{
    if (list.node_count == 0)
    {
        return MD_S8Lit("");
    }
    MD_String8 string = MD_ZERO_STRUCT;
    string.size = list.total_size + (list.node_count - 1)*separator.size;
    string.str = MD_PushArray(MD_u8, string.size);
    MD_u64 write_pos = 0;
    for(MD_String8Node *node = list.first; node; node = node->next)
    {
        MD_MemoryCopy(string.str + write_pos, node->string.str, node->string.size);
        write_pos += node->string.size;
        if (node != list.last){
            MD_MemoryCopy(string.str + write_pos, separator.str, separator.size);
            write_pos += separator.size;
        }
    }
    return string;
}

MD_FUNCTION_IMPL MD_u64
MD_CalculateCStringLength(char *cstr)
{
    MD_u64 i = 0;
    for(; cstr[i]; i += 1);
    return i;
}

MD_FUNCTION_IMPL MD_u64
MD_U64FromString(MD_String8 string, MD_u32 radix)
{
    MD_Assert(2 <= radix && radix <= 16);
    static MD_u8 char_to_value[] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };
    MD_u64 value = 0;
    for (MD_u64 i = 0; i < string.size; i += 1){
        value *= radix;
        MD_u8 c = string.str[i];
        value += char_to_value[(c - 0x30)&0x1F];
    }
    return(value);
}

MD_FUNCTION_IMPL MD_i64
MD_CStyleIntFromString(MD_String8 string)
{
    MD_u64 p = 0;
    
    // consume sign
    MD_i64 sign = +1;
    if (p < string.size){
        MD_u8 c = string.str[p];
        if (c == '-'){
            sign = -1;
            p += 1;
        }
        else if (c == '+'){
            p += 1;
        }
    }
    
    // radix from prefix
    MD_u64 radix = 10;
    if (p < string.size){
        MD_u8 c0 = string.str[p];
        if (c0 == '0'){
            p += 1;
            radix = 8;
            if (p < string.size){
                MD_u8 c1 = string.str[p];
                if (c1 == 'x'){
                    p += 1;
                    radix = 16;
                }
                else if (c1 == 'b'){
                    p += 1;
                    radix = 2;
                }
            }
        }
    }
    
    // consume integer "digits"
    MD_String8 digits_substr = MD_StringSkip(string, p);
    MD_u64 n = MD_U64FromString(digits_substr, radix);
    
    // combine result
    MD_i64 result = sign*n;
    return(result);
}

MD_FUNCTION_IMPL MD_f64
MD_F64FromString(MD_String8 string)
{
    char str[64];
    MD_u64 str_size = string.size;
    if (str_size > sizeof(str) - 1){
        str_size = sizeof(str) - 1;
    }
    MD_MemoryCopy(str, string.str, str_size);
    str[str_size] = 0;
    return(atof(str));
}

MD_FUNCTION_IMPL MD_String8
MD_StyledStringFromString(MD_String8 string, MD_WordStyle word_style, MD_String8 separator)
{
    MD_String8 result = MD_ZERO_STRUCT;
    
    MD_String8List words = MD_ZERO_STRUCT;
    
    MD_b32 break_on_uppercase = 0;
    {
        break_on_uppercase = 1;
        for(MD_u64 i = 0; i < string.size; i += 1)
        {
            if(!MD_CharIsAlpha(string.str[i]) && !MD_CharIsDigit(string.str[i]))
            {
                break_on_uppercase = 0;
                break;
            }
        }
    }
    
    MD_b32 making_word = 0;
    MD_String8 word = MD_ZERO_STRUCT;
    
    for(MD_u64 i = 0; i < string.size;)
    {
        if(making_word)
        {
            if((break_on_uppercase && MD_CharIsAlphaUpper(string.str[i])) ||
               string.str[i] == '_' || MD_CharIsSpace(string.str[i]) ||
               i == string.size - 1)
            {
                if(i == string.size - 1)
                {
                    word.size += 1;
                }
                making_word = 0;
                MD_PushStringToList(&words, word);
            }
            else
            {
                word.size += 1;
                i += 1;
            }
        }
        else
        {
            if(MD_CharIsAlpha(string.str[i]))
            {
                making_word = 1;
                word.str = string.str + i;
                word.size = 1;
            }
            i += 1;
        }
    }
    
    result.size = words.total_size;
    if(words.node_count > 1)
    {
        result.size += separator.size*(words.node_count-1);
    }
    result.str = MD_PushArray(MD_u8, result.size);
    
    {
        MD_u64 write_pos = 0;
        for(MD_String8Node *node = words.first; node; node = node->next)
        {
            
            // NOTE(rjf): Write word string to result.
            {
                MD_MemoryCopy(result.str + write_pos, node->string.str, node->string.size);
                
                // NOTE(rjf): Transform string based on word style.
                switch(word_style)
                {
                    case MD_WordStyle_UpperCamelCase:
                    {
                        result.str[write_pos] = MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCamelCase:
                    {
                        result.str[write_pos] = node == words.first ? MD_CharToLower(result.str[write_pos]) : MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_UpperCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToUpper(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    default: break;
                }
                
                write_pos += node->string.size;
            }
            
            if(node->next)
            {
                MD_MemoryCopy(result.str + write_pos, separator.str, separator.size);
                write_pos += separator.size;
            }
        }
    }
    
    return result;
}

//~ Enum/Flag Strings

MD_FUNCTION_IMPL MD_String8
MD_StringFromNodeKind(MD_NodeKind kind)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeKind enum.
    static char *cstrs[MD_NodeKind_COUNT] =
    {
        "Nil",
        "File",
        "List",
        "Reference",
        "Label",
        "Tag",
        "ErrorMarker",
    };
    return MD_S8CString(cstrs[kind]);
}

MD_FUNCTION_IMPL MD_String8List
MD_StringListFromNodeFlags(MD_NodeFlags flags)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeFlags enum.
    static char *flag_cstrs[] =
    {
        "ParenLeft",
        "ParenRight",
        "BracketLeft",
        "BracketRight",
        "BraceLeft",
        "BraceRight",
        
        "BeforeSemicolon",
        "AfterSemicolon",
        
        "BeforeComma",
        "AfterComma",
        
        "StringSingleQuote",
        "StringDoubleQuote",
        "StringTick",
        "StringTriplet",
        
        "Numeric",
        "Identifier",
        "StringLiteral",
    };
    
    MD_String8List list = MD_ZERO_STRUCT;
    MD_u64 bits = sizeof(flags) * 8;
    for(MD_u64 i = 0; i < bits && i < MD_ArrayCount(flag_cstrs); i += 1)
    {
        if(flags & (1ull << i))
        {
            MD_PushStringToList(&list, MD_S8CString(flag_cstrs[i]));
        }
    }
    return list;
}

//~ Unicode Conversions

MD_GLOBAL MD_u8 md_utf8_class[32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf8(MD_u8 *str, MD_u64 max)
{
#define MD_bitmask1 0x01
#define MD_bitmask2 0x03
#define MD_bitmask3 0x07
#define MD_bitmask4 0x0F
#define MD_bitmask5 0x1F
#define MD_bitmask6 0x3F
#define MD_bitmask7 0x7F
#define MD_bitmask8 0xFF
#define MD_bitmask9  0x01FF
#define MD_bitmask10 0x03FF
    
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    MD_u8 byte = str[0];
    MD_u8 byte_class = md_utf8_class[byte >> 3];
    switch (byte_class)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= max)
            {
                MD_u8 cont_byte = str[1];
                if (md_utf8_class[cont_byte >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask5) << 6;
                    result.codepoint |=  (cont_byte & MD_bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= max)
            {
                MD_u8 cont_byte[2] = {str[1], str[2]};
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask4) << 12;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 6);
                    result.codepoint |=  (cont_byte[1] & MD_bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= max)
            {
                MD_u8 cont_byte[3] = {str[1], str[2], str[3]};
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0 &&
                    md_utf8_class[cont_byte[2] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask3) << 18;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 12);
                    result.codepoint |= ((cont_byte[1] & MD_bitmask6) <<  6);
                    result.codepoint |=  (cont_byte[2] & MD_bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    
    return(result);
}

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf16(MD_u16 *out, MD_u64 max)
{
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    result.codepoint = out[0];
    result.advance = 1;
    if (1 < max && 0xD800 <= out[0] && out[0] < 0xDC00 && 0xDC00 <= out[1] && out[1] < 0xE000)
    {
        result.codepoint = ((out[0] - 0xD800) << 10) | (out[1] - 0xDC00);
        result.advance = 2;
    }
    return(result);
}

MD_FUNCTION MD_u32
MD_Utf8FromCodepoint(MD_u8 *out, MD_u32 codepoint)
{
#define MD_bit8 0x80
    MD_u32 advance = 0;
    if (codepoint <= 0x7F)
    {
        out[0] = (MD_u8)codepoint;
        advance = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (MD_bitmask2 << 6) | ((codepoint >> 6) & MD_bitmask5);
        out[1] = MD_bit8 | (codepoint & MD_bitmask6);
        advance = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (MD_bitmask3 << 5) | ((codepoint >> 12) & MD_bitmask4);
        out[1] = MD_bit8 | ((codepoint >> 6) & MD_bitmask6);
        out[2] = MD_bit8 | ( codepoint       & MD_bitmask6);
        advance = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (MD_bitmask4 << 3) | ((codepoint >> 18) & MD_bitmask3);
        out[1] = MD_bit8 | ((codepoint >> 12) & MD_bitmask6);
        out[2] = MD_bit8 | ((codepoint >>  6) & MD_bitmask6);
        out[3] = MD_bit8 | ( codepoint        & MD_bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return(advance);
}

MD_FUNCTION MD_u32
MD_Utf16FromCodepoint(MD_u16 *out, MD_u32 codepoint)
{
    MD_u32 advance = 1;
    if (codepoint == ~((MD_u32)0))
    {
        out[0] = (MD_u16)'?';
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (MD_u16)codepoint;
    }
    else
    {
        MD_u64 v = codepoint - 0x10000;
        out[0] = 0xD800 + (v >> 10);
        out[1] = 0xDC00 + (v & MD_bitmask10);
        advance = 2;
    }
    return(advance);
}

MD_FUNCTION MD_String8
MD_S8FromS16(MD_String16 in)
{
    MD_u64 cap = in.size*3;
    MD_u8 *str = MD_PushArray(MD_u8, cap + 1);
    MD_u16 *ptr = in.str;
    MD_u16 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String16
MD_S16FromS8(MD_String8 in)
{
    MD_u64 cap = in.size*2;
    MD_u16 *str = MD_PushArray(MD_u16, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    MD_String16 result = {str, size};
    return(result);
}

MD_FUNCTION MD_String8
MD_S8FromS32(MD_String32 in)
{
    MD_u64 cap = in.size*4;
    MD_u8 *str = MD_PushArray(MD_u8, cap + 1);
    MD_u32 *ptr = in.str;
    MD_u32 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl; ptr += 1)
    {
        size += MD_Utf8FromCodepoint(str + size, *ptr);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String32
MD_S32FromS8(MD_String8 in)
{
    MD_u64 cap = in.size;
    MD_u32 *str = MD_PushArray(MD_u32, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    MD_String32 result = {str, size};
    return(result);
}

//~ Map Table Data Structure

MD_FUNCTION_IMPL MD_u64
MD_HashString(MD_String8 string)
{
    MD_u64 result = 5381;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        result = ((result << 5) + result) + string.str[i];
    }
    return result;
}

// NOTE(mal): Generic 64-bit hash function (https://nullprogram.com/blog/2018/07/31/)
//            Reversible, so no collisions. Assumes all bits of the pointer matter.
MD_FUNCTION_IMPL MD_u64 
MD_HashPointer(void *p)
{
    MD_u64 h = (MD_u64)p;
    // TODO(rjf): Do we want our own equivalent of UINT64_C?
    h = (h ^ (h >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    h = (h ^ (h >> 27)) * UINT64_C(0x94d049bb133111eb);
    h = h ^ (h >> 31);
    return h;
}

MD_FUNCTION_IMPL MD_Map
MD_MapMakeBucketCount(MD_u64 bucket_count){
    // TODO(allen): permanent arena? scratch arena? -- would really
    // make most sense with a parameter
    MD_Map result = {0};
    result.bucket_count = bucket_count;
    result.buckets = MD_PushArrayZero(MD_MapBucket, bucket_count);
    return(result);
}

MD_FUNCTION_IMPL MD_Map
MD_MapMake(void){
    MD_Map result = MD_MapMakeBucketCount(4093);
    return(result);
}

MD_FUNCTION MD_MapKey
MD_MapKeyStr(MD_String8 string){
    MD_MapKey result = {0};
    if (string.size != 0){
        result.hash = MD_HashString(string);
        result.size = string.size;
        if (string.size > 0){
            result.ptr = string.str;
        }
    }
    return(result);
}

MD_FUNCTION MD_MapKey
MD_MapKeyPtr(void *ptr){
    MD_MapKey result = {0};
    if (ptr != 0){
        result.hash = MD_HashPointer(ptr);
        result.size = 0;
        result.ptr = ptr;
    }
    return(result);
}

MD_FUNCTION_IMPL MD_MapSlot*
MD_MapLookup(MD_Map *map, MD_MapKey key){
    MD_MapSlot *result = 0;
    if (map->bucket_count > 0){
        MD_u64 index = key.hash%map->bucket_count;
        result = MD_MapScan(map->buckets[index].first, key);
    }
    return(result);
}

MD_FUNCTION_IMPL MD_MapSlot*
MD_MapScan(MD_MapSlot *first_slot, MD_MapKey key){
    MD_MapSlot *result = 0;
    if (first_slot != 0){
        MD_b32 ptr_kind = (key.size == 0);
        MD_String8 key_string = MD_S8((MD_u8*)key.ptr, key.size);
        for (MD_MapSlot *slot = first_slot;
             slot != 0;
             slot = slot->next){
            if (slot->key.hash == key.hash){
                if (ptr_kind){
                    if (slot->key.size == 0 && slot->key.ptr == key.ptr){
                        result = slot;
                        break;
                    }
                }
                else{
                    MD_String8 slot_string = MD_S8((MD_u8*)slot->key.ptr, slot->key.size);
                    if (MD_StringMatch(slot_string, key_string, 0)){
                        result = slot;
                        break;
                    }
                }
            }
        }
    }
    return(result);
}

MD_FUNCTION_IMPL MD_MapSlot*
MD_MapInsert(MD_Map *map, MD_MapKey key, void *val){
    MD_MapSlot *result = 0;
    if (map->bucket_count > 0){
        MD_u64 index = key.hash%map->bucket_count;
        // TODO(allen): again, memory? permanent arena? scratch arena?
        // should definitely match the table's memory "object"
        MD_MapSlot *slot = MD_PushArray(MD_MapSlot, 1);
        MD_MapBucket *bucket = &map->buckets[index];
        MD_QueuePush(bucket->first, bucket->last, slot);
        slot->key = key;
        slot->val = val;
        result = slot;
    }
    return(result);
}

MD_FUNCTION_IMPL MD_MapSlot*
MD_MapOverwrite(MD_Map *map, MD_MapKey key, void *val){
    MD_MapSlot *result = MD_MapLookup(map, key);
    if (result != 0){
        result->val = val;
    }
    else{
        result = MD_MapInsert(map, key, val);
    }
    return(result);
}

//~ Parsing

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_TokenBoundariesAreBalanced(MD_Token token)
{
    MD_u64 front_len = token.string.str - token.outer_string.str;
    MD_u64 back_len  = (token.outer_string.str + token.outer_string.size) - (token.string.str + token.string.size);
    MD_b32 result = (front_len == back_len);
    return result;
}

// TODO(allen): wouldn't the lexer have already figured this out? Why not just make
// it a flag or a kind on the token?
MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_CommentIsSyntacticallyCorrect(MD_Token comment_token)
{
    MD_String8 inner = comment_token.string;
    MD_String8 outer = comment_token.outer_string;
    MD_b32 incorrect = (MD_StringMatch(MD_StringPrefix(outer, 2), MD_S8Lit("/*"), 0) &&   // C-style comment
                        (inner.str != outer.str + 2 || inner.str + inner.size != outer.str + outer.size - 2)); // Internally unbalanced
    MD_b32 result = !incorrect;
    return result;
}

MD_FUNCTION MD_Token
MD_TokenFromString(MD_String8 string)
{
    MD_Token token = MD_ZERO_STRUCT;
    
    MD_u8 *one_past_last = string.str + string.size;
    MD_u8 *first = string.str;
    
    if(first < one_past_last)
    {
        MD_u8 *at = first;
        MD_u32 skip_n = 0;
        MD_u32 chop_n = 0;
        
#define MD_TokenizerScan(cond) for (; at < one_past_last && (cond); at += 1)
        
        switch (*at)
        {
            // NOTE(allen): Whitespace parsing
            case '\n':
            {
                token.kind = MD_TokenKind_Newline;
                at += 1;
            }break;
            
            case ' ': case '\r': case '\t': case '\f': case '\v':
            {
                token.kind = MD_TokenKind_Whitespace;
                at += 1;
                MD_TokenizerScan(*at == ' ' || *at == '\r' || *at == '\t' || *at == '\f' || *at == '\v');
            }break;
            
            // NOTE(allen): Comment parsing
            case '/':
            {
                if (at + 1 < one_past_last)
                {
                    if (at[1] == '/')
                    {
                        
                        // TODO(allen): This seems like an odd choice to me. What about two spaces!?
                        // What about an extra /? I'm wondering if there are other places where we make
                        // this kind of judgement call a lot, or is this the only one? Maybe the user
                        // should just always skip-chop whitespace if they want to clean this kind of
                        // thing up? They're going to have to if they ever use two spaces anyways, right?
                        
                        // NOTE(rjf): Trim off the first //, and a space after it if one is there.
                        if(at+2 < one_past_last &&
                           at[2] == ' ')
                        {
                            skip_n = 3;
                        }
                        else
                        {
                            skip_n = 2;
                        }
                        
                        at += skip_n;
                        token.kind = MD_TokenKind_Comment;
                        MD_TokenizerScan(*at != '\n' && *at != '\r');
                    }
                    else if (at[1] == '*')
                    {
                        // TODO(allen): proposal:
                        // 1. only set `kind = Comment` in the `counter == 0` case
                        // 2. otherwise set `kind = RunOnComment` (or something)
                        //    maybe also emit an error *from here* or signal to a system that
                        //    runs later that there are token errors to emit.
                        // Or: keep the `kind = Comment` but in the `counter != 0` case
                        //     set some kind of error/unclosed/run-on flag on the token.
                        
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        skip_n = 2;
                        int counter = 1;
                        for (;at < one_past_last && counter > 0; at += 1)
                        {
                            if (at + 1 < one_past_last)
                            {
                                if (at[0] == '*' && at[1] == '/')
                                {
                                    at += 1;
                                    counter -= 1;
                                }
                                else if (at[0] == '/' && at[1] == '*')
                                {
                                    at += 1;
                                    counter += 1;
                                }
                            }
                        }
                        if(counter == 0)
                        {
                            chop_n = 2;
                        }
                    }
                }
                if (token.kind == 0) goto symbol_lex;
            }break;
            
            // NOTE(allen): Strings
            case '"':
            case '\'':
            case '`':
            {
                token.kind = MD_TokenKind_StringLiteral;
                
                // TODO(allen): proposal:
                // go see the proposal in the block comment lexer, same idea here?
                
                // determine delimiter setup
                MD_u8 d = *at;
                MD_b32 is_triplet = (at + 2 < one_past_last && at[1] == d && at[2] == d);
                
                // lex triple-delimiter string
                if (is_triplet)
                {
                    skip_n = 3;
                    at += 3;
                    MD_u32 consecutive_d = 0;
                    for (;;)
                    {
                        // fail condition
                        if (at >= one_past_last){
                            break;
                        }
                        
                        if(at[0] == d)
                        {
                            consecutive_d += 1;
                            at += 1;
                            // close condition
                            if (consecutive_d == 3){
                                chop_n = 3;
                                break;
                            }
                        }
                        else
                        {
                            consecutive_d = 0;
                            
                            // escaping rule
                            if(at[0] == '\\')
                            {
                                at += 1;
                                if(at < one_past_last && (at[0] == d || at[0] == '\\'))
                                {
                                    at += 1;
                                }
                            }
                            else{
                                at += 1;
                            }
                        }
                    }
                }
                
                // lex single-delimiter string
                if (!is_triplet)
                {
                    skip_n = 1;
                    at += 1;
                    for (;at < one_past_last;)
                    {
                        // close condition
                        if (*at == d){
                            at += 1;
                            chop_n = 1;
                            break;
                        }
                        
                        // fail condition
                        if (*at == '\n'){
                            break;
                        }
                        
                        // escaping rule
                        if (at[0] == '\\'){
                            at += 1;
                            if (at < one_past_last && (at[0] == d || at[0] == '\\')){
                                at += 1;
                            }
                        }
                        else{
                            at += 1;
                        }
                    }
                }
                
                //- rjf: set relevant node flags on token
                token.node_flags |= MD_NodeFlag_StringLiteral;
                switch(d)
                {
                    case '\'': token.node_flags |= MD_NodeFlag_StringSingleQuote; break;
                    case '"':  token.node_flags |= MD_NodeFlag_StringDoubleQuote; break;
                    case '`':  token.node_flags |= MD_NodeFlag_StringTick; break;
                    default: break;
                }
                if(is_triplet)
                {
                    token.node_flags |= MD_NodeFlag_StringTriplet;
                }
                
            }break;
            
            // NOTE(allen): Identifiers, Numbers, Operators
            default:
            {
                if (MD_CharIsAlpha(*at) || *at == '_')
                {
                    token.node_flags |= MD_NodeFlag_Identifier;
                    token.kind = MD_TokenKind_Identifier;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '_');
                }
                
                else if (MD_CharIsDigit(*at) ||
                         (at + 1 < one_past_last && at[0] == '-' && MD_CharIsDigit(at[1])))
                {
                    token.node_flags |= MD_NodeFlag_Numeric;
                    token.kind = MD_TokenKind_NumericLiteral;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '.');
                }
                
                else if (MD_CharIsSymbol(*at))
                {
                    symbol_lex:
                    token.kind = MD_TokenKind_Symbol;
                    at += 1;
                }
                
                else
                {
                    token.kind = MD_TokenKind_BadCharacter;
                    at += 1;
                }
            }break;
        }
        
        token.outer_string = MD_S8Range(first, at);
        token.string = MD_StringSubstring(token.outer_string, skip_n, token.outer_string.size - chop_n);
        
#undef MD_TokenizerScan
        
    }
    
    return token;
}

MD_FUNCTION_IMPL MD_u64
MD_LexAdvanceFromSkips(MD_String8 string, MD_TokenKind skip_kinds)
{
    MD_u64 result = string.size;
    MD_u64 p = 0;
    for (;;)
    {
        MD_Token token = MD_TokenFromString(MD_StringSkip(string, p));
        if ((skip_kinds & token.kind) == 0)
        {
            result = p;
            break;
        }
        p += token.outer_string.size;
    }
    return(result);
}

MD_FUNCTION_IMPL MD_Error *
MD_MakeNodeError(MD_Node *node, MD_MessageKind kind, MD_String8 str)
{
    MD_Error *error = MD_PushArrayZero(MD_Error, 1);
    error->node = node;
    error->kind = kind;
    error->string = str;
    return error;
}

MD_FUNCTION_IMPL MD_Error *
MD_MakeTokenError(MD_String8 parse_contents, MD_Token token, MD_MessageKind kind, MD_String8 str)
{
    MD_Node *err_node = MD_MakeNode(MD_NodeKind_ErrorMarker, MD_S8Lit(""), parse_contents,
                                    token.outer_string.str - parse_contents.str);
    return MD_MakeNodeError(err_node, kind, str);
}

MD_FUNCTION_IMPL void
MD_PushErrorToList(MD_ErrorList *list, MD_Error *error)
{
    MD_QueuePush(list->first, list->last, error);
    if(error->kind > list->max_error_kind)
    {
        list->max_error_kind = error->kind;
    }
    list->node_count += 1;
}

MD_FUNCTION_IMPL void
MD_PushErrorListToList(MD_ErrorList *list, MD_ErrorList *to_push)
{
    if(list->last)
    {
        if(to_push->node_count != 0)
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
            list->node_count += to_push->node_count;
            if(to_push->max_error_kind > list->max_error_kind)
            {
                list->max_error_kind = to_push->max_error_kind;
            }
        }
    }
    else
    {
        *list = *to_push;
    }
    MD_MemoryZero(to_push, sizeof(*to_push));
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseResultZero(void)
{
    MD_ParseResult result = MD_ZERO_STRUCT;
    result.node = result.last_node = MD_NilNode();
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseNodeSet(MD_String8 string, MD_u64 offset, MD_Node *parent, MD_ParseSetRule rule)
{
    MD_ParseResult result = MD_ParseResultZero();
    MD_u64 off = offset;
    
    //- rjf: fill data from set opener
    MD_Token initial_token = MD_TokenFromString(MD_StringSkip(string, offset));
    MD_u8 set_opener = 0;
    MD_NodeFlags set_opener_flags = 0;
    MD_b32 close_with_brace = 0;
    MD_b32 close_with_paren = 0;
    MD_b32 close_with_separator = 0;
    MD_b32 parse_all = 0;
    switch(rule)
    {
        default: break;
        
        case MD_ParseSetRule_EndOnDelimiter:
        {
            MD_u64 opener_check_off = off;
            opener_check_off += MD_LexAdvanceFromSkips(MD_StringSkip(string, opener_check_off), MD_TokenGroup_Irregular);
            initial_token = MD_TokenFromString(MD_StringSkip(string, opener_check_off));
            if(initial_token.kind == MD_TokenKind_Symbol)
            {
                if(MD_StringMatch(initial_token.outer_string, MD_S8Lit("{"), 0))
                {
                    set_opener = '{';
                    set_opener_flags |= MD_NodeFlag_BraceLeft;
                    opener_check_off += initial_token.outer_string.size;
                    off = opener_check_off;
                    close_with_brace = 1;
                }
                else if(MD_StringMatch(initial_token.outer_string, MD_S8Lit("("), 0))
                {
                    set_opener = '(';
                    set_opener_flags |= MD_NodeFlag_ParenLeft;
                    opener_check_off += initial_token.outer_string.size;
                    off = opener_check_off;
                    close_with_paren = 1;
                }
                else if(MD_StringMatch(initial_token.outer_string, MD_S8Lit("["), 0))
                {
                    set_opener = '[';
                    set_opener_flags |= MD_NodeFlag_BracketLeft;
                    opener_check_off += initial_token.outer_string.size;
                    off = opener_check_off;
                    close_with_paren = 1;
                }
                else
                {
                    close_with_separator = 1;
                }
            }
            else
            {
                close_with_separator = 1;
            }
        }break;
        
        case MD_ParseSetRule_Global:
        {
            parse_all = 1;
        }break;
    }
    
    //- rjf: fill parent data from opener
    parent->flags |= set_opener_flags;
    
    //- rjf: parse children
    MD_b32 got_closer = 0;
    MD_u64 parsed_child_count = 0;
    if(set_opener != 0 || close_with_separator || parse_all)
    {
        MD_NodeFlags next_child_flags = 0;
        for(;off < string.size;)
        {
            
            //- rjf: check for separator closers
            if(close_with_separator)
            {
                MD_u64 closer_check_off = off;
                
                //- rjf: check newlines
                {
                    MD_Token potential_closer = MD_TokenFromString(MD_StringSkip(string, closer_check_off));
                    if(potential_closer.kind == MD_TokenKind_Newline)
                    {
                        closer_check_off += potential_closer.outer_string.size;
                        off = closer_check_off;
                        
                        // NOTE(rjf): always terminate with a newline if we have >0 children
                        if(parsed_child_count > 0)
                        {
                            off = closer_check_off;
                            got_closer = 1;
                            break;
                        }
                        
                        // NOTE(rjf): terminate after double newline if we have 0 children
                        MD_Token next_closer = MD_TokenFromString(MD_StringSkip(string, closer_check_off));
                        if(next_closer.kind == MD_TokenKind_Newline)
                        {
                            closer_check_off += next_closer.outer_string.size;
                            off = closer_check_off;
                            got_closer = 1;
                            break;
                        }
                    }
                }
                
                //- rjf: check separators and possible braces from higher parents
                {
                    closer_check_off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
                    MD_Token potential_closer = MD_TokenFromString(MD_StringSkip(string, closer_check_off));
                    if(potential_closer.kind == MD_TokenKind_Symbol &&
                       (MD_StringMatch(potential_closer.outer_string, MD_S8Lit(","), 0) ||
                        MD_StringMatch(potential_closer.outer_string, MD_S8Lit(";"), 0)))
                    {
                        closer_check_off += potential_closer.outer_string.size;
                        off = closer_check_off;
                        break;
                    }
                    else if(potential_closer.kind == MD_TokenKind_Symbol &&
                            (MD_StringMatch(potential_closer.string, MD_S8Lit("}"), 0) ||
                             MD_StringMatch(potential_closer.string, MD_S8Lit("]"), 0) ||
                             MD_StringMatch(potential_closer.string, MD_S8Lit(")"), 0)))
                    {
                        goto end_parse;
                    }
                }
                
            }
            
            //- rjf: check for non-separator closers
            if(!close_with_separator && !parse_all)
            {
                MD_u64 closer_check_off = off;
                closer_check_off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
                MD_Token potential_closer = MD_TokenFromString(MD_StringSkip(string, closer_check_off));
                if(potential_closer.kind == MD_TokenKind_Symbol)
                {
                    if(close_with_brace && MD_StringMatch(potential_closer.outer_string, MD_S8Lit("}"), 0))
                    {
                        closer_check_off += potential_closer.outer_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_BraceRight;
                        got_closer = 1;
                        break;
                    }
                    else if(close_with_paren && MD_StringMatch(potential_closer.outer_string, MD_S8Lit("]"), 0))
                    {
                        closer_check_off += potential_closer.outer_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_BracketRight;
                        got_closer = 1;
                        break;
                    }
                    else if(close_with_paren && MD_StringMatch(potential_closer.outer_string, MD_S8Lit(")"), 0))
                    {
                        closer_check_off += potential_closer.outer_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_ParenRight;
                        got_closer = 1;
                        break;
                    }
                }
            }
            
            //- rjf: parse next child
            MD_ParseResult child_parse = MD_ParseOneNode(string, off);
            MD_PushErrorListToList(&result.errors, &child_parse.errors);
            off += child_parse.bytes_parsed;
            
            //- rjf: hook child into parent
            if(!MD_NodeIsNil(child_parse.node))
            {
                MD_PushChild(parent, child_parse.node);
                parsed_child_count += 1;
            }
            
            //- rjf: check trailing separator
            MD_NodeFlags trailing_separator_flags = 0;
            if(!close_with_separator)
            {
                off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
                MD_Token trailing_separator = MD_TokenFromString(MD_StringSkip(string, off));
                if(MD_StringMatch(trailing_separator.string, MD_S8Lit(","), 0) &&
                   trailing_separator.kind == MD_TokenKind_Symbol)
                {
                    trailing_separator_flags |= MD_NodeFlag_BeforeComma;
                    off += trailing_separator.outer_string.size;
                }
                else if(MD_StringMatch(trailing_separator.string, MD_S8Lit(";"), 0) &&
                        trailing_separator.kind == MD_TokenKind_Symbol)
                {
                    trailing_separator_flags |= MD_NodeFlag_BeforeSemicolon;
                    off += trailing_separator.outer_string.size;
                }
            }
            
            //- rjf: fill child flags
            child_parse.node->flags |= next_child_flags | trailing_separator_flags;
            
            //- rjf: setup next_child_flags
            next_child_flags = MD_NodeFlag_AfterFromBefore(trailing_separator_flags);
        }
    }
    end_parse:;
    
    //- rjf: push missing closer error, if we have one
    if(set_opener != 0 && got_closer == 0)
    {
        MD_Error *error = MD_MakeTokenError(string, initial_token, MD_MessageKind_CatastrophicError,
                                            MD_PushStringF("Unbalanced \"%c\"", set_opener));
        MD_PushErrorToList(&result.errors, error);
    }
    
    //- rjf: push empty implicit set error,
    if(close_with_separator && parsed_child_count == 0)
    {
        MD_Error *error = MD_MakeTokenError(string, initial_token, MD_MessageKind_Error,
                                            MD_S8Lit("Empty implicitly-delimited node list"));
        MD_PushErrorToList(&result.errors, error);
    }
    
    //- rjf: fill result info
    result.node = parent->first_child;
    result.last_node = parent->last_child;
    result.bytes_parsed = off - offset;
    
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseTagList(MD_String8 string, MD_u64 offset)
{
    MD_ParseResult result = MD_ParseResultZero();
    MD_u64 off = offset;
    
    for(;off < string.size;)
    {
        //- rjf: parse @ symbol, signifying start of tag
        off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
        MD_Token next_token = MD_TokenFromString(MD_StringSkip(string, off));
        if(!MD_StringMatch(next_token.string, MD_S8Lit("@"), 0) ||
           next_token.kind != MD_TokenKind_Symbol)
        {
            break;
        }
        off += next_token.outer_string.size;
        
        //- rjf: parse string of tag node
        MD_Token name = MD_TokenFromString(MD_StringSkip(string, off));
        MD_u64 name_off = off;
        if((name.kind & MD_TokenGroup_Label) == 0)
        {
            MD_Error *error = MD_MakeTokenError(string, name, MD_MessageKind_Error,
                                                MD_PushStringF("\"%.*s\" is not a proper tag label",
                                                               MD_StringExpand(name.outer_string)));
            MD_PushErrorToList(&result.errors, error);
            break;
        }
        off += name.outer_string.size;
        
        //- rjf: build tag
        MD_Node *tag = MD_MakeNode(MD_NodeKind_Tag, name.string, name.outer_string, name_off);
        
        //- rjf: parse tag arguments
        MD_Token open_paren = MD_TokenFromString(MD_StringSkip(string, off));
        MD_ParseResult args_parse = MD_ParseResultZero();
        if(MD_StringMatch(open_paren.string, MD_S8Lit("("), 0) &&
           open_paren.kind == MD_TokenKind_Symbol)
        {
            args_parse = MD_ParseNodeSet(string, off, tag, MD_ParseSetRule_EndOnDelimiter);
            MD_PushErrorListToList(&result.errors, &args_parse.errors);
        }
        off += args_parse.bytes_parsed;
        
        //- rjf: push tag to result
        MD_NodeDblPushBack(result.node, result.last_node, tag);
    }
    
    //- rjf: fill result
    result.bytes_parsed = off - offset;
    
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseOneNode(MD_String8 string, MD_u64 offset)
{
    MD_ParseResult result = MD_ParseResultZero();
    MD_u64 off = offset;
    
    //- rjf: parse pre-comment
    MD_String8 comment_before = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;off < string.size;)
        {
            MD_Token token = MD_TokenFromString(MD_StringSkip(string, off));
            if(token.kind == MD_TokenKind_Comment)
            {
                off += token.outer_string.size;
                comment_token = token;
            }
            else if(token.kind == MD_TokenKind_Newline)
            {
                off += token.outer_string.size;
                MD_Token next_token = MD_TokenFromString(MD_StringSkip(string, off));
                if(next_token.kind == MD_TokenKind_Comment)
                {
                    // NOTE(mal): If more than one comment, use the last comment
                    comment_token = next_token;
                }
                else if(next_token.kind == MD_TokenKind_Newline)
                {
                    MD_MemoryZero(&comment_token, sizeof(comment_token));
                }
            }
            else if((token.kind & MD_TokenGroup_Whitespace) != 0)
            {
                off += token.outer_string.size;
            }
            else
            {
                break;
            }
            comment_before = comment_token.string;
            
            // TODO(allen): I find this odd. Wouldn't it have been easier to generate this
            // durring or right after the lexing phase?
            if(!_MD_CommentIsSyntacticallyCorrect(comment_token))
            {
                MD_String8 capped = MD_StringPrefix(comment_token.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
                MD_Error *error = MD_MakeTokenError(string, comment_token, MD_MessageKind_CatastrophicError,
                                                    MD_PushStringF("Unterminated comment \"%.*s\"", MD_StringExpand(capped)));
                MD_PushErrorToList(&result.errors, error);
            }
        }
    }
    
    //- rjf: parse tag list
    MD_ParseResult tags_parse = MD_ParseTagList(string, off);
    off += tags_parse.bytes_parsed;
    MD_PushErrorListToList(&result.errors, &tags_parse.errors);
    
    //- rjf: parse node
    MD_Node *parsed_node = MD_NilNode();
    MD_ParseResult children_parse = MD_ParseResultZero();
    retry:;
    {
        //- rjf: try to parse an unnamed set
        off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
        MD_Token unnamed_set_opener = MD_TokenFromString(MD_StringSkip(string, off));
        if(unnamed_set_opener.kind == MD_TokenKind_Symbol &&
           (MD_StringMatch(unnamed_set_opener.string, MD_S8Lit("("), 0) ||
            MD_StringMatch(unnamed_set_opener.string, MD_S8Lit("{"), 0) ||
            MD_StringMatch(unnamed_set_opener.string, MD_S8Lit("["), 0)))
        {
            parsed_node = MD_MakeNode(MD_NodeKind_Label, MD_S8Lit(""), MD_S8Lit(""),
                                      unnamed_set_opener.outer_string.str - string.str);
            children_parse = MD_ParseNodeSet(string, off, parsed_node, MD_ParseSetRule_EndOnDelimiter);
            off += children_parse.bytes_parsed;
            MD_PushErrorListToList(&result.errors, &children_parse.errors);
            goto end_parse;
        }
        
        //- rjf: try to parse regular node, with/without children
        off += MD_LexAdvanceFromSkips(MD_StringSkip(string, off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
        MD_Token label_name = MD_TokenFromString(MD_StringSkip(string, off));
        if((label_name.kind & MD_TokenGroup_Label) != 0)
        {
            off += label_name.outer_string.size;
            parsed_node = MD_MakeNode(MD_NodeKind_Label, label_name.string, label_name.outer_string,
                                      label_name.outer_string.str - string.str);
            parsed_node->flags |= label_name.node_flags;
            
            //- rjf: check for string literal errors
            if(label_name.kind == MD_TokenKind_StringLiteral)
            {
                if(!_MD_TokenBoundariesAreBalanced(label_name))
                {
                    MD_String8 capped = MD_StringPrefix(label_name.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
                    MD_Error *error = MD_MakeNodeError(parsed_node, MD_MessageKind_CatastrophicError,
                                                       MD_PushStringF("Unterminated text literal \"%.*s\"", MD_StringExpand(capped)));
                    MD_PushErrorToList(&result.errors, error);
                }
            }
            
            //- rjf: check for unexpected reserved symbols
            else if(label_name.kind == MD_TokenKind_Symbol && label_name.string.size == 1 && MD_CharIsReservedSymbol(label_name.string.str[0]))
            {
                MD_u8 c = label_name.string.str[0];
                if(c == '}' || c == ']' || c == ')')
                {
                    MD_Error *error = MD_MakeNodeError(parsed_node, MD_MessageKind_CatastrophicError, MD_PushStringF("Unbalanced \"%c\"", c));
                    MD_PushErrorToList(&result.errors, error);
                }
                else
                {
                    MD_Error *error = MD_MakeNodeError(parsed_node, MD_MessageKind_Error, MD_PushStringF("Unexpected reserved symbol \"%c\"", c));
                    MD_PushErrorToList(&result.errors, error);
                }
            }
            
            //- rjf: try to parse children for this node
            MD_u64 colon_check_off = off;
            colon_check_off += MD_LexAdvanceFromSkips(MD_StringSkip(string, colon_check_off), MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
            MD_Token colon = MD_TokenFromString(MD_StringSkip(string, colon_check_off));
            if(MD_StringMatch(colon.string, MD_S8Lit(":"), 0) && colon.kind == MD_TokenKind_Symbol)
            {
                colon_check_off += colon.outer_string.size;
                off = colon_check_off;
                
                //- rjf: prohibit tags here
                // TODO(allen): This poking in an error "from afar" thing seems
                // like a bad sign to me. First it took a bit of digging for me to
                // understand how this code actually detects the errors it says it
                // does. Second it's kind of unclear that this should be illegal.
                // I mean we can do these:
                //  `label: @tag child`
                //  `label: child @tag {children}`
                //  `label: @tag child`
                // I do get *why* this is an odd thing to allow, but it's weird either way.
                // Third, looks like this also is throwing out an error in the totally legal case:
                //  `label:{@tag {bar}}`
                for(MD_u64 tag_check_off = off; tag_check_off < string.size;)
                {
                    MD_Token token = MD_TokenFromString(MD_StringSkip(string, tag_check_off));
                    if(token.kind == MD_TokenKind_Symbol && MD_StringMatch(token.outer_string, MD_S8Lit("@"), 0))
                    {
                        MD_Error *error = MD_MakeTokenError(string, token, MD_MessageKind_Error,
                                                            MD_S8Lit("Tags are not allowed between a `:` and node children. Place tags before the name of the node list."));
                        MD_PushErrorToList(&result.errors, error);
                        tag_check_off += token.outer_string.size;
                        off = tag_check_off;
                    }
                    else
                    {
                        break;
                    }
                }
                
                children_parse = MD_ParseNodeSet(string, off, parsed_node, MD_ParseSetRule_EndOnDelimiter);
                off += children_parse.bytes_parsed;
                MD_PushErrorListToList(&result.errors, &children_parse.errors);
            }
            goto end_parse;
        }
        
        //- rjf: collect bad token
        MD_Token bad_token = MD_TokenFromString(MD_StringSkip(string, off));
        if(bad_token.kind == MD_TokenKind_BadCharacter)
        {
            off += bad_token.outer_string.size;
            
            // TODO(allen): tighten up with good integer <-> string helpers
            MD_String8List bytes = {0};
            for(int i_byte = 0; i_byte < bad_token.outer_string.size; ++i_byte)
            {
                MD_PushStringToList(&bytes, MD_PushStringF("0x%02X", bad_token.outer_string.str[i_byte]));
            }
            MD_String8 byte_string = MD_JoinStringList(bytes, MD_S8Lit(" "));
            
            MD_Error *error = MD_MakeTokenError(string, bad_token, MD_MessageKind_Error,
                                                MD_PushStringF("Non-ASCII character \"%.*s\"", MD_StringExpand(byte_string)));
            MD_PushErrorToList(&result.errors, error);
            goto retry;
        }
    }
    
    end_parse:;
    
    //- rjf: parse comments after nodes.
    MD_String8 comment_after = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;;)
        {
            MD_Token token = MD_TokenFromString(MD_StringSkip(string, off));
            if(token.kind == MD_TokenKind_Comment)
            {
                comment_token = token;
                off += token.outer_string.size;
                break;
            }
            
            else if(token.kind == MD_TokenKind_Newline)
            {
                break;
            }
            else if((token.kind & MD_TokenGroup_Whitespace) != 0)
            {
                off += token.outer_string.size;
            }
            else
            {
                break;
            }
        }
        comment_after = comment_token.string;
        
        // TODO(allen): I find this odd. Wouldn't it have been easier to generate this
        // durring or right after the lexing phase?
        if(!_MD_CommentIsSyntacticallyCorrect(comment_token))
        {
            MD_String8 capped = MD_StringPrefix(comment_token.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
            MD_Error *error = MD_MakeTokenError(string, comment_token, MD_MessageKind_CatastrophicError,
                                                MD_PushStringF("Unterminated comment \"%.*s\"", MD_StringExpand(capped)));
            MD_PushErrorToList(&result.errors, error);
        }
    }
    
    //- rjf: fill result
    parsed_node->comment_before = comment_before;
    parsed_node->comment_after = comment_after;
    result.node = parsed_node;
    if(!MD_NodeIsNil(result.node))
    {
        result.node->first_tag = tags_parse.node;
        result.node->last_tag = tags_parse.last_node;
    }
    result.last_node = parsed_node;
    result.bytes_parsed = off - offset;
    
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseWholeString(MD_String8 filename, MD_String8 contents)
{
    MD_Node *root = MD_MakeNode(MD_NodeKind_File, filename, contents, 0);
    MD_ParseResult result = MD_ParseNodeSet(contents, 0, root, MD_ParseSetRule_Global);
    result.node = result.last_node = root;
    for(MD_Error *error = result.errors.first; error != 0; error = error->next)
    {
        if(MD_NodeIsNil(error->node->parent))
        {
            error->node->parent = root;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseWholeFile(MD_String8 filename)
{
    MD_String8 file_contents = MD_LoadEntireFile(filename);
    MD_ParseResult parse = MD_ParseWholeString(filename, file_contents);
    if(file_contents.str == 0)
    {
        MD_Error *error = MD_MakeNodeError(parse.node, MD_MessageKind_CatastrophicError,
                                           MD_PushStringF("Could not read file \"%.*s\"", MD_StringExpand(filename)));
        MD_PushErrorToList(&parse.errors, error);
    }
    return parse;
}

//~ Location Conversions

MD_FUNCTION_IMPL MD_CodeLoc
MD_CodeLocFromFileBaseOff(MD_String8 filename, MD_u8 *base, MD_u8 *at)
{
    MD_CodeLoc loc;
    loc.filename = filename;
    loc.line = 1;
    loc.column = 1;
    if(base != 0)
    {
        for(MD_u64 i = 0; base+i < at && base[i]; i += 1)
        {
            if(base[i] == '\n')
            {
                loc.line += 1;
                loc.column = 1;
            }
            else
            {
                loc.column += 1;
            }
        }
    }
    return loc;
}

MD_FUNCTION_IMPL MD_CodeLoc
MD_CodeLocFromNode(MD_Node *node)
{
    MD_Node *root = MD_RootFromNode(node);
    MD_CodeLoc loc = MD_CodeLocFromFileBaseOff(root->string, root->whole_string.str, root->whole_string.str + node->offset);
    return loc;
}

//~ Tree/List Building

MD_FUNCTION_IMPL MD_b32
MD_NodeIsNil(MD_Node *node)
{
    return node == 0 || node == &_md_nil_node || node->kind == MD_NodeKind_Nil;
}

MD_FUNCTION_IMPL MD_Node *
MD_NilNode(void) { return &_md_nil_node; }

MD_FUNCTION_IMPL MD_Node *
MD_MakeNode(MD_NodeKind kind, MD_String8 string, MD_String8 whole_string, MD_u64 offset)
{
    MD_Node *node = MD_PushArray(MD_Node, 1);
    node->kind = kind;
    node->string = string;
    node->whole_string = whole_string;
    node->next = node->prev = node->parent =
        node->first_child = node->last_child =
        node->first_tag = node->last_tag = node->ref_target = MD_NilNode();
    node->offset = offset;
    return node;
}

MD_FUNCTION_IMPL void
MD_PushChild(MD_Node *parent, MD_Node *new_child)
{
    if (!MD_NodeIsNil(new_child))
    {
        MD_NodeDblPushBack(parent->first_child, parent->last_child, new_child);
        new_child->parent = parent;
    }
}

MD_FUNCTION_IMPL void
MD_PushTag(MD_Node *node, MD_Node *tag)
{
    if (!MD_NodeIsNil(tag))
    {
        MD_NodeDblPushBack(node->first_tag, node->last_tag, tag);
        tag->parent = node;
    }
}

MD_FUNCTION_IMPL MD_Node*
MD_MakeList(void)
{
    MD_String8 empty = {0};
    MD_Node *result = MD_MakeNode(MD_NodeKind_List, empty, empty, 0);
    return(result);
}

MD_FUNCTION_IMPL MD_Node*
MD_PushReference(MD_Node *list, MD_Node *target)
{
    MD_Node *n = MD_MakeNode(MD_NodeKind_Reference, target->string, target->whole_string, target->offset);
    n->ref_target = target;
    MD_PushChild(list, n);
    return(n);
}

//~ Introspection Helpers

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromString(MD_Node *first, MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next)
    {
        if(MD_StringMatch(string, node->string, 0))
        {
            result = node;
            break;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromIndex(MD_Node *first, MD_Node *last, int n)
{
    MD_Node *result = MD_NilNode();
    if(n >= 0)
    {
        int idx = 0;
        for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next, idx += 1)
        {
            if(idx == n)
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL int
MD_IndexFromNode(MD_Node *node)
{
    int idx = 0;
    if(node && !MD_NodeIsNil(node))
    {
        for(MD_Node *last = node->prev; !MD_NodeIsNil(last); last = last->prev, idx += 1);
    }
    return idx;
}

MD_FUNCTION MD_Node *
MD_RootFromNode(MD_Node *node)
{
    MD_Node *parent = node;
    for(MD_Node *p = parent; !MD_NodeIsNil(p); p = p->parent)
    {
        parent = p;
    }
    return parent;
}

MD_FUNCTION_IMPL MD_Node *
MD_NextNodeSibling(MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    if(last)
    {
        for(MD_Node *node = last->next; node; node = node->next)
        {
            if(MD_StringMatch(string, node->string, 0))
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromString(MD_Node *node, MD_String8 child_string)
{
    return MD_NodeFromString(node->first_child, node->last_child, child_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromString(MD_Node *node, MD_String8 tag_string)
{
    return MD_NodeFromString(node->first_tag, node->last_tag, tag_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, int n)
{
    MD_Node *tag = MD_TagFromString(node, tag_string);
    return MD_ChildFromIndex(tag, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagArgFromString(MD_Node *node, MD_String8 tag_string, MD_String8 arg_string)
{
    MD_Node *tag = MD_TagFromString(node, tag_string);
    MD_Node *arg = MD_ChildFromString(tag, arg_string);
    return arg;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeHasTag(MD_Node *node, MD_String8 tag_string)
{
    return !MD_NodeIsNil(MD_TagFromString(node, tag_string));
}

MD_FUNCTION_IMPL MD_i64
MD_ChildCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_i64
MD_TagCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(tag, node->first_tag))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_Deref(MD_Node *node)
{
    MD_Node *result = node;
    while(result->kind == MD_NodeKind_Reference)
    {
        result = result->ref_target;
    }
    return result;
}

MD_FUNCTION MD_Node *
MD_SeekNodeWithFlags(MD_Node *start, MD_NodeFlags one_past_last_flags)
{
    MD_Node *result = MD_NilNode();
    for(MD_EachNode(it, start->next))
    {
        if(it->flags & one_past_last_flags)
        {
            result = it;
            break;
        }
    }
    return result;
}

//~ Error/Warning Helpers

MD_FUNCTION void
MD_Message(FILE *out, MD_CodeLoc loc, MD_MessageKind kind, MD_String8 str)
{
    const char *kind_name = "";
    switch (kind){
        default: break;
        case MD_MessageKind_Warning: kind_name = "warning: "; break;
        case MD_MessageKind_Error: kind_name = "error: "; break;
        case MD_MessageKind_CatastrophicError: kind_name = "fatal error: "; break;
    }
    fprintf(out, "%.*s:%i:%i: %s%.*s\n",
            MD_StringExpand(loc.filename), loc.line, loc.column,
            kind_name, MD_StringExpand(str));
}

MD_FUNCTION_IMPL void
MD_MessageF(FILE *out, MD_CodeLoc loc, MD_MessageKind kind, char *fmt, ...)
{
    // TODO(allen): use scratch
    va_list args;
    va_start(args, fmt);
    MD_Message(out, loc, kind, MD_PushStringFV(fmt, args));
    va_end(args);
}

MD_FUNCTION_IMPL void
MD_NodeMessage(FILE *out, MD_Node *node, MD_MessageKind kind, MD_String8 str)
{
    MD_CodeLoc loc = MD_CodeLocFromNode(node);
    MD_Message(out, loc, kind, str);
}

MD_FUNCTION_IMPL void
MD_NodeMessageF(FILE *out, MD_Node *node, MD_MessageKind kind, char *fmt, ...)
{
    // TODO(allen): use scratch
    va_list args;
    va_start(args, fmt);
    MD_NodeMessage(out, node, kind, MD_PushStringFV(fmt, args));
    va_end(args);
}

//~ Tree Comparison/Verification

MD_FUNCTION_IMPL MD_b32
MD_NodeMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind && MD_StringMatch(a->string, b->string, flags))
    {
        result = 1;
        if(a->kind != MD_NodeKind_Tag && (flags & MD_MatchFlag_Tags))
        {
            for(MD_Node *a_tag = a->first_tag, *b_tag = b->first_tag;
                !MD_NodeIsNil(a_tag) || !MD_NodeIsNil(b_tag);
                a_tag = a_tag->next, b_tag = b_tag->next)
            {
                if(MD_NodeMatch(a_tag, b_tag, flags))
                {
                    if(flags & MD_MatchFlag_TagArguments)
                    {
                        for(MD_Node *a_tag_arg = a_tag->first_child, *b_tag_arg = b_tag->first_child;
                            !MD_NodeIsNil(a_tag_arg) || !MD_NodeIsNil(b_tag_arg);
                            a_tag_arg = a_tag_arg->next, b_tag_arg = b_tag_arg->next)
                        {
                            if(!MD_NodeDeepMatch(a_tag_arg, b_tag_arg, flags))
                            {
                                result = 0;
                                goto end;
                            }
                        }
                    }
                }
                else
                {
                    result = 0;
                    goto end;
                }
            }
        }
    }
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = MD_NodeMatch(a, b, flags);
    if(result)
    {
        for(MD_Node *a_child = a->first_child, *b_child = b->first_child;
            !MD_NodeIsNil(a_child) || !MD_NodeIsNil(b_child);
            a_child = a_child->next, b_child = b_child->next)
        {
            if(!MD_NodeDeepMatch(a_child, b_child, flags))
            {
                result = 0;
                goto end;
            }
        }
    }
    end:;
    return result;
}

//~ Generation

MD_FUNCTION_IMPL void
MD_OutputTree(FILE *file, MD_Node *node, int indent_spaces)
{
#define MD_PrintIndent() do { for(int i = 0; i < indent_spaces; i += 1) fprintf(file, " "); } while(0)
    for(MD_Node *tag = node->first_tag; !MD_NodeIsNil(tag); tag = tag->next)
    {
        MD_PrintIndent();
        fprintf(file, "@%.*s", MD_StringExpand(tag->string));
        if(!MD_NodeIsNil(tag->first_child))
        {
            fprintf(file, "(");
            for(MD_Node *child = tag->first_child; !MD_NodeIsNil(child); child = child->next)
            {
                MD_OutputTree(file, child, 0);
                fprintf(file, ", ");
            }
            fprintf(file, ")\n");
        }
        else if(!MD_NodeIsNil(tag->next))
        {
            fprintf(file, " ");
        }
    }
    if(!MD_NodeIsNil(node->first_tag))
    {
        fprintf(file, "\n");
    }
    
    if(node->whole_string.size > 0)
    {
        MD_PrintIndent();
        fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
    }
    
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->whole_string.size > 0)
        {
            fprintf(file, ":\n");
        }
        MD_PrintIndent();
        fprintf(file, "{\n");
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_OutputTree(file, child, indent_spaces+2);
        }
        MD_PrintIndent();
        fprintf(file, "}\n");
    }
    else
    {
        fprintf(file, "\n");
    }
#undef MD_PrintIndent
}

//~ Command Line Argument Helper

MD_FUNCTION MD_String8List
MD_StringListFromArgCV(int argument_count, char **arguments)
{
    MD_String8List options = MD_ZERO_STRUCT;
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_PushStringToList(&options, MD_S8CString(arguments[i]));
    }
    return options;
}

MD_FUNCTION MD_CommandLine
MD_CommandLineFromOptions(MD_String8List options)
{
    MD_CommandLine cmdln = MD_ZERO_STRUCT;
    cmdln.arguments = options;
    
    for(MD_String8Node *n = options.first, *next = 0;
        n; n = next)
    {
        next = n->next;
        
        //- rjf: figure out whether or not this is an option by checking for `-` or `--`
        // from the beginning of the string
        MD_String8 option_name = MD_ZERO_STRUCT;
        if(MD_StringMatch(MD_StringPrefix(n->string, 2), MD_S8Lit("--"), 0))
        {
            option_name = MD_StringSkip(n->string, 2);
        }
        else if(MD_StringMatch(MD_StringPrefix(n->string, 1), MD_S8Lit("-"), 0))
        {
            option_name = MD_StringSkip(n->string, 1);
        }
        //- rjf: trim off anything after a `:` or `=`, use that as the first value string
        MD_String8 first_value = MD_ZERO_STRUCT;
        MD_b32 has_many_values = 0;
        if(option_name.size != 0)
        {
            MD_u64 colon_signifier_pos = MD_FindSubstring(option_name, MD_S8Lit(":"), 0, 0);
            MD_u64 equal_signifier_pos = MD_FindSubstring(option_name, MD_S8Lit("="), 0, 0);
            MD_u64 signifier_pos = colon_signifier_pos > equal_signifier_pos ? equal_signifier_pos : colon_signifier_pos;
            if(signifier_pos < option_name.size)
            {
                first_value = MD_StringSkip(option_name, signifier_pos+1);
                option_name = MD_StringPrefix(option_name, signifier_pos);
                if(MD_StringMatch(MD_StringSuffix(first_value, 1), MD_S8Lit(","), 0))
                {
                    has_many_values = 1;
                }
            }
        }
        
        //- rjf: gather arguments
        if(option_name.size != 0)
        {
            MD_String8List option_values = MD_ZERO_STRUCT;
            
            //- rjf: push first value
            if(first_value.size != 0)
            {
                MD_PushStringToList(&option_values, first_value);
            }
            
            //- rjf: scan next string values, add them to option values until we hit a lack
            // of a ',' between values
            if(has_many_values)
            {
                for(MD_String8Node *v = next; v; v = v->next, next = v)
                {
                    MD_String8 value_str = v->string;
                    MD_b32 next_has_arguments = MD_StringMatch(MD_StringSuffix(value_str, 1), MD_S8Lit(","), 0);
                    MD_b32 in_quotes = 0;
                    MD_u64 start = 0;
                    for(MD_u64 i = 0; i <= value_str.size; i += 1)
                    {
                        if(i == value_str.size || (value_str.str[i] == ',' && in_quotes == 0))
                        {
                            if(start != i)
                            {
                                MD_PushStringToList(&option_values, MD_StringSubstring(value_str, start, i));
                            }
                            start = i+1;
                        }
                        else if(value_str.str[i] == '"')
                        {
                            in_quotes = !in_quotes;
                        }
                    }
                    if(next_has_arguments == 0)
                    {
                        break;
                    }
                }
            }
            
            //- rjf: insert the fully parsed option
            {
                MD_CommandLineOption *opt = MD_PushArray(MD_CommandLineOption, 1);
                MD_MemoryZero(opt, sizeof(*opt));
                opt->name = option_name;
                opt->values = option_values;
                if(cmdln.last_option == 0)
                {
                    cmdln.first_option = cmdln.last_option = opt;
                }
                else
                {
                    cmdln.last_option->next = opt;
                    cmdln.last_option = cmdln.last_option->next;
                }
            }
        }
        
        //- rjf: this argument is not an option, push it to regular inputs list.
        else
        {
            MD_PushStringToList(&cmdln.inputs, n->string);
        }
    }
    
    return cmdln;
}

MD_FUNCTION MD_String8List
MD_CommandLineOptionValues(MD_CommandLine cmdln, MD_String8 name)
{
    MD_String8List values = MD_ZERO_STRUCT;
    for(MD_CommandLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_StringMatch(opt->name, name, 0))
        {
            values = opt->values;
            break;
        }
    }
    return values;
}

MD_FUNCTION MD_b32
MD_CommandLineOptionPassed(MD_CommandLine cmdln, MD_String8 name)
{
    MD_b32 result = 0;
    for(MD_CommandLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_StringMatch(opt->name, name, 0))
        {
            result = 1;
            break;
        }
    }
    return result;
}

MD_FUNCTION MD_i64
MD_CommandLineOptionI64(MD_CommandLine cmdln, MD_String8 name)
{
    MD_i64 v = 0;
    MD_String8List values = MD_CommandLineOptionValues(cmdln, name);
    MD_String8 value_str = MD_JoinStringList(values, MD_S8Lit(""));
    v = MD_CStyleIntFromString(value_str);
    return v;
}

//~ File System

MD_FUNCTION_IMPL MD_String8
MD_LoadEntireFile(MD_String8 filename)
{
    MD_String8 file_contents = MD_ZERO_STRUCT;
    FILE *file = fopen((char*)MD_PushStringCopy(filename).str, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        MD_u64 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_contents.str = MD_PushArray(MD_u8, file_size+1);
        if(file_contents.str)
        {
            file_contents.size = file_size;
            fread(file_contents.str, 1, file_size, file);
        }
        fclose(file);
    }
    return file_contents;
}

MD_FUNCTION_IMPL MD_b32
MD_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info)
{
#if !defined(MD_IMPL_FileIterIncrement)
    return(0);
#else
    return(MD_IMPL_FileIterIncrement(it, path, out_info));
#endif
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
