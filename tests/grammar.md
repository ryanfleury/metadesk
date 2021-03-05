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

//// A bunch of common definitions
separator           : ' ' | unscoped_separator
unscoped_separator  : '\n'| ',' | ';'
alt_scope_beg       : '(' | '['
alt_scope_end       : ')' | ']'
id                  : alpha [alphanumeric] | '_' [alphanumeric]
alphanumeric        : alpha [alphanumeric] | digit [alphanumeric] | '_' [alphanumeric]
alpha               : lowercase | uppercase
lowercase           : 'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'z'|'y'|'z'
uppercase           : 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'Z'|'Y'|'Z'
digit               : '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
label               : id | integer_literal | char_literal | string_literal | symbol_label
integer_literal     : { ['-'] natural_literal }
natural_literal     : digit [natural_literal]
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

//// Arbitrarily deep tree, possibly empty
// file                : [@child set_list]
// set_list            : '{' [@child set_list] '}' [separator @sibling set_list]

//// Labeled leaves
// file            : [@child set_list]
// set_list        : set [separator @sibling set_list]
// set             : @fill 'A' | '{' [@child set_list] '}'

//// Labeled internal nodes
// file            : [@child set_list]
// set_list        : set [separator @sibling set_list]
// set             : @fill 'A' | [@fill 'A' ':'] '{' [@child set_list] '}'

//// Unscoped sets
// file                : [@child set_list]
// set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
// scoped_set          : '{' [@child set_list] '}'
// unscoped_set        : @fill 'A' [unscoped_set_tail]
// unscoped_set_tail   : ':' @child unscoped_set | ' ' @sibling unscoped_set | ':' scoped_set |' ' @sibling scoped_set

//// Tags
// file                : [@child set_list]
// set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
// scoped_set          : {[tag_list] '{' [@child set_list] '}'}
// unscoped_set        : {[tag_list] @fill 'A' [unscoped_set_tail]}
// tag_list            : '@' @tag tag ' ' [tag_list]
// tag                 : @fill 'T'['(' [@child set_list] ')']
// // unscoped_set_tail   : ':' @child unscoped_set | ' ' @sibling unscoped_set | ':' @child scoped_set | ' ' @sibling scoped_set
// unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | set_tail_hack | ' ' @sibling scoped_set}
//                       // NOTE(mal): Ideally the set_tail_hack should be captured by the simpler
//                       //            ':' @child scoped_set
//                       //            but there's a quirk in the grammar. 
//                       //            In "A:{}", A has no children but in "A:@T{}", it has one tagged children, 
//                       //            which means that *the presence of tags* introduces the child
// set_tail_hack       : ':' [@child tag_list] '{' '}' | ':' @child [tag_list] '{' @child set_list '}'

//// Alternative scope markers
// file                : [@child set_list]
// set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
// scoped_set          : {[tag_list] untagged_scoped_set}
// untagged_scoped_set : '{' [@child set_list] '}' | alt_scope_beg [@child set_list] alt_scope_end
// unscoped_set        : {[tag_list] @fill 'A' [unscoped_set_tail]}
// tag_list            : '@' @tag tag ' ' [tag_list]
// tag                 : @fill 'T'['(' [@child set_list] ')']
// unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | set_tail_hack | ' ' @sibling scoped_set}
// set_tail_hack       : {':' [@child tag_list] '{' '}' | 
//                        ':' @child [tag_list] '{' @child set_list '}' | 
//                        ':' [@child tag_list] alt_scope_beg alt_scope_end | 
//                        ':' @child [tag_list] alt_scope_beg @child set_list alt_scope_end
//                       }

//// General tags and labels
file                : [@child set_list]
set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
scoped_set          : {[tag_list] untagged_scoped_set}
untagged_scoped_set : '{' [@child set_list] '}' | alt_scope_beg [@child set_list] alt_scope_end
unscoped_set        : {[tag_list] @fill label [unscoped_set_tail]}
tag_list            : '@' @tag tag ' ' [tag_list]
tag                 : @fill id['(' [@child set_list] ')']
unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | set_tail_hack | ' ' @sibling scoped_set}
set_tail_hack       : {':' [@child tag_list] '{' '}' | 
                       ':' @child [tag_list] '{' @child set_list '}' | 
                       ':' [@child tag_list] alt_scope_beg alt_scope_end | 
                       ':' @child [tag_list] alt_scope_beg @child set_list alt_scope_end
                      }

//// Comments before 
//// TODO: This needs some work to make sure that "a /* comment_before_b */ b" can't be generated
////                                     but that "a /* comment_after_a  */ b" can
// file                : [@child set_list]
// set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
// scoped_set          : {[tag_list] [@pre_comment pre_comment] untagged_scoped_set}
// pre_comment         : '/' '/' [' '] [@fill c_code_content] '\n' | '/' '*' [@fill cpp_code_content] '*' '/'
// c_code_content      : 'c''o''m''m''e''n''t' // TODO: Arbitrary strings, including C-style comments, as long as /* */ pairs are balanced
// cpp_code_content    : 'c''o''m''m''e''n''t' // TODO: Arbitrary strings that don't start with space
// untagged_scoped_set : '{' [@child set_list] '}' | alt_scope_beg [@child set_list] alt_scope_end
// unscoped_set        : {[tag_list] @fill label [unscoped_set_tail]}
// tag_list            : '@' @tag tag ' ' [tag_list]
// tag                 : @fill id['(' [@child set_list] ')']
// unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | set_tail_hack | ' ' @sibling scoped_set}
// set_tail_hack       : {':' [@child tag_list] '{' '}' | 
//                        ':' @child [tag_list] '{' @child set_list '}' | 
//                        ':' [@child tag_list] alt_scope_beg alt_scope_end | 
//                        ':' @child [tag_list] alt_scope_beg @child set_list alt_scope_end
//                       }
