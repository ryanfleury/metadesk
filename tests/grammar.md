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
// set             : '{' [@child set] '}' | set [separator @sibling set]
// separator       : ' ' | '\n' | ',' | ';'

//// Labeled leaves
// file            : [@child set]
// set             : @fill 'A' | '{' [@child set] '}' | set [separator @sibling set]
// separator       : ' ' | '\n' | ',' | ';'

//// Labeled internal nodes
// file            : [@child set]
// set             : @fill 'A' | [@fill 'A' ':'] scoped_set | set [separator @sibling set]
// separator       : ' ' | '\n' | ',' | ';'
// scoped_set      : '{' [@child set] '}'

//// Unscoped tests (feels like there should be an easier way to express this)
// file            : general_set
// general_set     : { [@child set] | [@child uns_set] }
// set             : @fill 'A' | set [sep1 @sibling set] | scoped_set | uns_set sep2 @sibling set
// uns_set         : @fill 'A' [uns_set_tail]
// uns_set_tail    : ':' @child uns_set | ' ' @sibling uns_set | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' general_set '}'
// sep1            : ' ' | '\n' | ',' | ';'
// sep2            : '\n'| ',' | ';'

//// Tags
// file            : general_set
// general_set     : { [@child set] | [@child uns_set] }
// set             : {[tag_list] untagged_set}
// uns_set         : {[tag_list] untagged_uns_set}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill 'T'['(' general_set ')']
// untagged_set    : @fill 'A' | set [sep1 @sibling set] | scoped_set | uns_set sep2 @sibling set
// untagged_uns_set: @fill 'A' [uns_set_tail]
// uns_set_tail    : ':' @child uns_set | ' ' @sibling uns_set | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' general_set '}'
// sep1            : ' ' | '\n' | ',' | ';'
// sep2            : '\n'| ',' | ';'

//// Alternative scope markers
// file            : general_set
// general_set     : { [@child set] | [@child uns_set] }
// set             : {[tag_list] untagged_set}
// uns_set         : {[tag_list] untagged_uns_set}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill 'T'['(' general_set ')']
// untagged_set    : @fill 'A' | set [sep1 @sibling set] | scoped_set | uns_set sep2 @sibling set
// untagged_uns_set: @fill 'A' [uns_set_tail]
// uns_set_tail    : ':' @child uns_set | ' ' @sibling uns_set | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' general_set '}' | alt_scope_beg general_set alt_scope_end
// alt_scope_beg   : '(' | '['
// alt_scope_end   : ')' | ']'
// sep1            : ' ' | '\n' | ',' | ';'
// sep2            : '\n'| ',' | ';'

//// Identifiers
// file            : general_set
// general_set     : { [@child set] | [@child uns_set] }
// set             : {[tag_list] untagged_set}
// uns_set         : {[tag_list] untagged_uns_set}
// tag_list        : '@' @tag tag ' ' [tag_list]
// tag             : @fill id['(' general_set ')']
// untagged_set    : @fill 'A' | set [sep1 @sibling set] | scoped_set | uns_set sep2 @sibling set
// untagged_uns_set: @fill 'A' [uns_set_tail]
// uns_set_tail    : ':' @child uns_set | ' ' @sibling uns_set | ':'  scoped_set | ' ' @sibling scoped_set
// scoped_set      : '{' general_set '}' | alt_scope_beg general_set alt_scope_end
// alt_scope_beg   : '(' | '['
// alt_scope_end   : ')' | ']'
// sep1            : ' ' | '\n' | ',' | ';'
// sep2            : '\n'| ',' | ';'
// id              : alpha [alphanumeric] | '_' [alphanumeric]
// alphanumeric    : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]
// alpha           : lowercase | uppercase
// lowercase       : 'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'z'|'y'|'z'
// uppercase       : 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'Z'|'Y'|'Z'
// digit           : '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'

//// General labels
file            : general_set
general_set     : { [@child set] | [@child uns_set] }
set             : {[tag_list] untagged_set}
uns_set         : {[tag_list] untagged_uns_set}
tag_list        : '@' @tag tag ' ' [tag_list]
tag             : @fill id['(' general_set ')']
untagged_set    : @fill label | set [sep1 @sibling set] | scoped_set | uns_set sep2 @sibling set
untagged_uns_set: @fill label [uns_set_tail]
uns_set_tail    : ':' @child uns_set | ' ' @sibling uns_set | ':'  scoped_set | ' ' @sibling scoped_set
scoped_set      : '{' general_set '}' | alt_scope_beg general_set alt_scope_end
alt_scope_beg   : '(' | '['
alt_scope_end   : ')' | ']'
sep1            : ' ' | '\n' | ',' | ';'
sep2            : '\n'| ',' | ';'
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
