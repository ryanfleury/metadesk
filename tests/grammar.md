/* MetaDesk grammar with semantic annotations
 *
 * Each line represents a BNF-esque production:
 *      symbol : rule_1 | ... | rule_n
 * - Pipe signs indicate mutually exclusive alternatives
 * - Square quotes denote optional rules
 * - Character literals are terminal productions
 * - Tags indicate which way the productions attach to the generated tree (@child, @sibling, @tag)
 *   and miscellaneous semantics (@fill, @markup)
 */

file                 : [@child file_set_list]
file_set_list        : { [tag_list] set [file_set_separator @sibling file_set_list] }
file_set_separator   : ' ' | '\n'

// TODO(mal): Unify file_set_separator and set_separator by allowing ',' and ';' everywhere
set_list        : { [tag_list] set [set_separator @sibling file_set_list] }
set_separator   : ' ' | '\n' | ',' | ';'
                                        // TODO(mal): Accept other open/close tokens
tag_list        : '@' @tag tag ' ' [tag_list]
tag             : identifier [@markup '(' [@child set_list] @markup ')']
set             : @fill leaf | @fill identifier ':' @child @fill leaf | [@fill identifier ':'] set_open [@child set_list] set_close
set_open        : '{' | '[' | '('
set_close       : '}' | ']' | ')'
leaf            : identifier | integer_literal | char_literal | string_literal  // TODO(mal): Also symbol_label
identifier      : alpha [alphanumeric] | '_' [alphanumeric]
alphanumeric    : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]

integer_literal : { ['-'] natural_literal }
natural_literal : digit [natural_literal]

char_literal                    : @markup '\'' [char_literal_items] @markup '\''
char_literal_items              : char_literal_item [char_literal_items]
char_literal_item               : ascii_no_backslash_no_quotes | '"' | '\\' ascii
ascii                           : ascii_no_backslash_no_quotes | '\'' | '"' | '\\'
ascii_no_backslash_no_quotes    : digit | alpha | symbol_no_backslash_no_quotes | space
symbol_no_backslash_no_quotes   : symbol_no_backslash_no_quotes_1 | symbol_no_backslash_no_quotes_2
symbol_no_backslash_no_quotes_1 : '!'|'#'|'$'|'%'|'&'|'('|')'|'*'|'+'|','|'-'|'.'|'/'|':'|';'
symbol_no_backslash_no_quotes_2 : '<'|'='|'>'|'?'|'@'|'['|']'|'^'|'_'|'`'|'{'|'|'|'}'|'~'

string_literal                  : @markup '"' [string_literal_items] @markup '"'
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
    // NOTE(mal): This is the simplest subset of the grammar that works
    file            : [@child set_list]
    set_list        : set ['\n' @sibling set_list]
    set             : @fill element | '{' [@child set_list] '}'
    element         : 'A'
*/

