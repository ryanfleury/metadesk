/* MetaDesk grammar with semantic annotations
 *
 * Each line represents a BNF-esque production of the form:
 *      symbol : rule_1 | ... | rule_n
 * - Pipe signs indicate mutually exclusive alternatives
 * - Square quotes denote optional rules
 * - Character literals are terminal productions
 * - Tags indicate which way the productions attach to the generated tree (@child, @sibling, @leaf, @tag)
 *   and miscellaneous semantics (@delimiter)
 */

file            : [@child set_list]
set_list        : tagged_named_set [' ' @sibling set_list]
tagged_named_set: { [tag_list] set }
tag_list        : @tag tag ' ' [tag_list]
tag             : '@' identifier        // TODO(mal): tag parameters
set             : @leaf leaf | '{' [@child set_list] '}'
leaf            : identifier | integer_literal | char_literal | string_literal  // TODO(mal): Also symbol_label
identifier      : alpha [alphanumeric]  // TODO(mal): I think we should allow leading underscores
alphanumeric    : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]

integer_literal : { ['-'] natural_literal }
natural_literal : digit [natural_literal]

char_literal                    : @delimiter '\'' [char_literal_items] @delimiter '\''
char_literal_items              : char_literal_item [char_literal_items]
char_literal_item               : ascii_no_backslash_no_quotes | '"' | '\\' ascii
ascii                           : ascii_no_backslash_no_quotes | '\'' | '"' | '\\'
ascii_no_backslash_no_quotes    : digit | alpha | symbol_no_backslash_no_quotes | space
symbol_no_backslash_no_quotes   : symbol_no_backslash_no_quotes_1 | symbol_no_backslash_no_quotes_2
symbol_no_backslash_no_quotes_1 : '!'|'#'|'$'|'%'|'&'|'('|')'|'*'|'+'|','|'-'|'.'|'/'|':'|';'
symbol_no_backslash_no_quotes_2 : '<'|'='|'>'|'?'|'@'|'['|']'|'^'|'_'|'`'|'{'|'|'|'}'|'~'

string_literal                  : @delimiter '"' [string_literal_items] @delimiter '"'
string_literal_items            : string_literal_item [string_literal_items]
string_literal_item             : ascii_no_backslash_no_quotes | '\'' | '\\' ascii

digit           : '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
alpha           : lowercase | uppercase
lowercase       : 'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'z'|'y'|'z'
uppercase       : 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'Z'|'Y'|'Z'
space           : ' '

symbol_label    : '~'|'!'|'%'|'^'|'&'|'*'|'+'|'-'|'/'|'|'|'<'|'>'|'$'|'='|'.'|'?'|'_'|'\\'

/*

symbol_no_quote : symbol_label | symbol_colon | symbol_group

symbol          : symbol_label | symbol_colon | symbol_group

symbol_group    : symbol_open | symbol_close
symbol_open     : '(' | '[' | '{'
symbol_close    : ')' | ']' | '}' 

escaped_char    : '\\' '\\' | '\\' '\'' | '\\' '\"'
symbol_colon    : ':'

*/


/*
    // NOTE(mal): I think this one should work too, but MD only delimits top level constructs with newlines
    file            : [@child set_list]
    set_list        : set [',' @sibling set_list]
    set             : @leaf element | '{' [@child set_list] '}'
    element         : 'A'

    // NOTE(mal): This is the simples grammar that works
    file            : [@child set_list]
    set_list        : set ['\n' @sibling set_list]
    set             : @leaf element | '{' [@child set_list] '}'
    element         : 'A'
*/

