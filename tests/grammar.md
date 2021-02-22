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

//// Arbitrarily deep tree, possibly empty
// file            : [@child set]
// set             : '{' [@child set] '}' | set [' ' @sibling set]

//// Labeled leaves
// file            : [@child set]
// set             : @fill 'A' | '{' [@child set] '}' | set [' ' @sibling set]

//// Labels also on internal nodes ('iset' is an implicitly separated set)
// file            : [@child set]
// set             : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] scoped_set | set [' ' @sibling set]
// iset            : @fill 'A' [iset_tail]
// iset_tail       : ':' @child iset | ' ' @sibling iset | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' [@child set] '}'

//// Tags
// file            : [@child set]
// set             : {[tag_list] untagged_set}
// iset            : {[tag_list] untagged_iset}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill 'T'['(' [@child set] ')']
// untagged_set    : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] scoped_set | set [' ' @sibling set]
// untagged_iset   : @fill 'A' [iset_tail]
// iset_tail       : ':' @child iset | ' ' @sibling iset | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' [@child set] '}'

//// Alternative scope markers
// file            : [@child set]
// set             : {[tag_list] untagged_set}
// iset            : {[tag_list] untagged_iset}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill 'T'['(' [@child set] ')']
// untagged_set    : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] scoped_set | set [' ' @sibling set]
// untagged_iset   : @fill 'A' [iset_tail]
// iset_tail       : ':' @child iset | ' ' @sibling iset | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' [@child set] '}' | scope_beg [@child set] scope_end
// scope_beg       : '(' | '['
// scope_end       : ')' | ']'

//// Identifiers
// file            : [@child set]
// set             : {[tag_list] untagged_set}
// iset            : {[tag_list] untagged_iset}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill id['(' [@child set] ')']
// untagged_set    : @fill 'A' [':' @child iset '\n'] | [@fill 'A' ':'] scoped_set | set [' ' @sibling set]
// untagged_iset   : @fill 'A' [iset_tail]
// iset_tail       : ':' @child iset | ' ' @sibling iset | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' [@child set] '}' | scope_beg [@child set] scope_end
// scope_beg       : '(' | '['
// scope_end       : ')' | ']'
// id              : alpha [alphanumeric]
// alphanumeric    : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]
// alpha           : lowercase | uppercase
// lowercase       : 'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'z'|'y'|'z'
// uppercase       : 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'Z'|'Y'|'Z'
// digit           : '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'

//// General labels
file            : [@child set]
set             : {[tag_list] untagged_set}
iset            : {[tag_list] untagged_iset}
tag_list        : '@' @tag tag ' ' [tag_list]
tag             : @fill id['(' [@child set] ')']
untagged_set    : @fill label [':' @child iset '\n'] | [@fill label ':'] scoped_set | set [' ' @sibling set]
untagged_iset   : @fill label [iset_tail]
iset_tail       : ':' @child iset | ' ' @sibling iset | ':'  scoped_set | ' ' @sibling scoped_set
scoped_set      : '{' [@child set] '}' | scope_beg [@child set] scope_end
scope_beg       : '(' | '['
scope_end       : ')' | ']'
id              : alpha [alphanumeric] | '_' [alphanumeric]
alphanumeric    : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]
alpha           : lowercase | uppercase
lowercase       : 'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'z'|'y'|'z'
uppercase       : 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'Z'|'Y'|'Z'
digit           : '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
label           : id | integer_literal | char_literal | string_literal | symbol_label
integer_literal : { ['-'] natural_literal }
natural_literal : digit [natural_literal]
char_literal                    : @markup '\'' [char_literal_items] @markup '\''
char_literal_items              : char_literal_item [char_literal_items]
char_literal_item               : ascii_no_backslash_no_quotes | '"' | '\\' ascii
ascii                           : ascii_no_backslash_no_quotes | '\'' | '"' | '`' | '\\'
ascii_no_backslash_no_quotes    : digit | alpha | symbol_no_backslash_no_quotes | ' '
symbol_no_backslash_no_quotes   : symbol_no_backslash_no_quotes_1 | symbol_no_backslash_no_quotes_2
symbol_no_backslash_no_quotes_1 : '!'|'#'|'$'|'%'|'&'|'('|')'|'*'|'+'|','|'-'|'.'|'/'|':'
symbol_no_backslash_no_quotes_2 : ';'|'<'|'='|'>'|'?'|'@'|'['|']'|'^'|'_'|'{'|'|'|'}'|'~'
string_literal                  : @markup '"' [string_literal_items] @markup '"' | @markup '`' [string_literal_items] @markup '`'
string_literal_items            : string_literal_item [string_literal_items]
string_literal_item             : ascii_no_backslash_no_quotes | '\'' | '\\' ascii
symbol_label                    : '~'|'!'|'%'|'^'|'&'|'*'|'+'|'-'|'/'|'|'|'<'|'>'|'$'|'='|'.'|'?'|'$'
