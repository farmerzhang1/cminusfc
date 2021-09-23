%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern int yyparse();
extern int yyrestart();
extern FILE * yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char * yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
// ! (love u zevin)
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

/* TODO: Complete this definition.
   Hint: See pass_node(), node(), and syntax_tree.h.
         Use forward declaring. */
%union {
    // char* op; // might be '<='
    // int integer;
    // double double_num;
    // char* identifier;
    syntax_tree_node* node;
}

/* TODO: Your tokens here. */

%token <node> ID
%token <node> INT_T VOID_T FLOAT_T
%token <node> LE LT GT GE EQ NEQ // rel op
%token <node> ADD MINUS // add op
%token <node> MUL DIV // mul op
%token <node> LPAREN RPAREN
%token <node> INTEGER
%token <node> FLOATPOINT

%start simple_expression

%%
/* TODO: Your rules here. */

/* Example:
program: declaration-list {$$ = node( "program", 1, $1); gt->root = $$;}
       ;
*/

/* program: declaration-list; */
relop: LE LT GT GE EQ NEQ
addop: ADD MINUS
mulop: MUL DIV
simple_expression: additive_expression relop additive_expression {}
    | additive_expression {}
additive_expression: additive_expression addop term {}
    | term {}
term: term mulop factor {}
    | factor {}
factor: LPAREN simple_expression RPAREN {}
    |   integer {}
    |   float {}
integer: INTEGER
float: FLOATPOINT


%%

/// The error reporting function.
void yyerror(const char * s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g. $$ = node("program", 1, $1);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
