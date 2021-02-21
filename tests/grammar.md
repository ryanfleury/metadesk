// Arbitrarily deep tree, possibly empty
// file            : [@child set]
// set             : '{' [@child set] '}' | set [' ' @sibling set]

// Labeled leaves
// file            : [@child set]
// set             : @fill 'A' | '{' [@child set] '}' | set [' ' @sibling set]

// Labels also on internal nodes    ('iset' is an implicitly separated set)
// file                            : [@child set]
// set                             : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] curly_set  | set [' ' @sibling set]
// iset /* implicit separator */  : @fill 'A' [':' @child iset]      |  @fill 'A'[':'  curly_set] | @fill 'A' [' ' @sibling iset] | @fill 'A' [' ' @sibling curly_set]
// curly_set                       : '{' [@child set] '}'

// Labels also on internal nodes (v2) ('iset' is an implicitly separated set)
// file                            : [@child set]
// set                             : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] curly_set | set [' ' @sibling set]
// iset /* implicit separator */   : @fill 'A' [iset_tail]
// iset_tail                       : ':' @child iset | ' ' @sibling iset | ':'  curly_set | ' ' @sibling curly_set
// curly_set                       : '{' [@child set] '}'

// Tags
// file          : [@child set]
// set           : {[tag_list] untagged_set}
// iset          : {[tag_list] untagged_iset}
// tag_list      : '@' @tag tag ' ' [tag_list]
// tag           : 'T'
// untagged_set  : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] curly_set | set [' ' @sibling set]
// untagged_iset : @fill 'A' [iset_tail]
// iset_tail     : ':' @child iset | ' ' @sibling iset | ':'  curly_set | ' ' @sibling curly_set
// curly_set     : '{' [@child set] '}'

// Tag parameters
file          : [@child set]
set           : {[tag_list] untagged_set}
iset          : {[tag_list] untagged_iset}
tag_list      : '@' @tag tag ' ' [tag_list]
tag           : @fill 'T'['(' [@child set] ')']
untagged_set  : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] curly_set | set [' ' @sibling set]
untagged_iset : @fill 'A' [iset_tail]
iset_tail     : ':' @child iset | ' ' @sibling iset | ':'  curly_set | ' ' @sibling curly_set
curly_set     : '{' [@child set] '}'


// top-level and set separators
// file            : [@child file_set_list]
// file_set_list   : set [file_set_separator @sibling file_set_list]
// file_set_separator   : ' ' | '\n'
// set             : '{' [@child set] '}' | set [separator @sibling set]
// separator       : ' ' | ','



/////////////////////////////////////////// REFERENCE ///////////////////////////////////////////////////////////

/*

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

// TODO(mal): Unify file_set_separator and set_separator by allowing ',' and ';' everywhere ?
set_list        : { [tag_list] set [set_separator @sibling set_list] }
set_separator   : ' ' | '\n'        // TODO(mal):  | ',' | ';'
tag_list        : '@' @tag tag ' ' [tag_list]

/*
    tag             : identifier [@markup '(' [@child set_list] @markup ')']
    set             : @fill leaf | @fill identifier ':' @child @fill leaf | [@fill identifier ':'] '{' [@child set_list] '}' | [@fill identifier ':'] set_open [@child set_list] set_close
    set             : @fill leaf | @fill identifier ':' @child space_separated_set_list | [@fill identifier ':'] set_open [@child set_list] set_close
*/

tag             : identifier 
set             : @fill leaf [':' @child space_separated_set_list '\n'] | @fill leaf [':' '{' [@child set_list] '}'] | space_separated_set_list '\n' | '{' [@child set_list] '}' 

space_separated_set_list : { [tag_list] set [' ' @sibling space_separated_set_list] }


// set_open        : '[' | '('
// set_close       : ']' | ')'
set_open        : '['           // TODO(mal): Should also accept '('
set_close       : ']'           // TODO(mal): Should also accept ')'
leaf            : identifier | integer_literal | char_literal | string_literal  // TODO(mal): Also symbol_label
identifier      : alpha [alphanumeric]
name_identifier : alpha [alphanumeric]
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
*/

