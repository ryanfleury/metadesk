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
symbol_label                    : '~'|'!'|'%'|'^'|'&'|'*'|'+'|'-'|'/'|'|'|'<'|'>'|'$'|'='|'.'|'?'|'_'

/* What follows is a range of annotated grammars that can be used to generate
 * tests of increasing complexity and completeness to check against MetaDesk.
 * To run them, uncomment them one by one and run the build/grammar test.
 */

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
// scoped_set          : {[tag_list] untagged_scoped_set}
// untagged_scoped_set : '{' [@child set_list] '}'
// unscoped_set        : {[tag_list] @fill 'A' [unscoped_set_tail]}
// tag_list            : '@' @tag tag ' ' [tag_list]
// tag                 : @fill 'T'['(' [@child set_list] ')']
// unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | ':' untagged_scoped_set | ' ' @sibling scoped_set}

//// Alternative scope markers
// file                : [@child set_list]
// set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
// scoped_set          : {[tag_list] untagged_scoped_set}
// untagged_scoped_set : '{' [@child set_list] '}' | alt_scope_beg [@child set_list] alt_scope_end
// unscoped_set        : {[tag_list] @fill 'A' [unscoped_set_tail]}
// tag_list            : '@' @tag tag ' ' [tag_list]
// tag                 : @fill 'T'['(' [@child set_list] ')']
// unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | ':' untagged_scoped_set | ' ' @sibling scoped_set}

//// General tags and labels
file                : [@child set_list]
set_list            : scoped_set [separator @sibling set_list] | unscoped_set [unscoped_separator @sibling set_list]
scoped_set          : {[tag_list] untagged_scoped_set}
untagged_scoped_set : '{' [@child set_list] '}' | alt_scope_beg [@child set_list] alt_scope_end
unscoped_set        : {[tag_list] @fill label [unscoped_set_tail]}
tag_list            : '@' @tag tag ' ' [tag_list]
tag                 : @fill id['(' [@child set_list] ')']
unscoped_set_tail   : {':' @child unscoped_set | ' ' @sibling unscoped_set | ':' untagged_scoped_set | ' ' @sibling scoped_set}

/* Comments
 * Comments around nodes are accessible to the user. Here's how they behave:
 * - The text inside a comment immediatly following a node is stored as the 
 *   comment_after member of that node. No newlines can happen between a 
 *   node and its after_comment.
 * - The text inside a comment preceding a node is stored as the 
 *   comment_before of that node _unless_ it is already the comment_after 
 *   of another node. One newline between the comment and the node is 
 *   obviously necessary in the case of C++-style comments and it's also
 *   allowed for C-style comments.
 * - If the first character inside a C++-style comment is a space, it's
 *   omitted from the stored string
 *
 * The semantically annotated Backus-Naur form that we're using is not a 
 * good fit to describe the grammar of comments.
 * To prevent the comment in "a /* comment */ b" from being interpreted as 
 * a comment_before of "b", instead of what it is (a comment_after of "a"),
 * we would have to complicate the grammar by introducing several extra 
 * productions with this specific purpose in mind.
 * The sensitivity of whitespace in the attachment of comments to nodes is
 * also cumbersome to express in BNF.
 */
