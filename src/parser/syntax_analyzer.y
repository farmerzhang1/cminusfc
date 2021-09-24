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
%code requires {
    #include "syntax_tree.h"
}
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
%token <node> ASSIGN COLON SEMICOLON
%token <node> LPAREN RPAREN LBRACKET RBRACKET LCURLY RCURLY
%token <node> IF ELSE WHILE RETURN
%token <node> INTEGER
%token <node> FLOATPOINT
// 字面量、关键字、运算符与标识符
%type <node> relop addop mulop type_specifier
// 声明
%type <node> declaration_list declaration var_declaration fun_declaration local_declarations
// 语句
%type <node> compound_stmt statement_list statement expression_stmt iteration_stmt selection_stmt return_stmt
// 表达式
%type <node> simple_expression var expression additive_expression term factor integer float call
// 其他
%type <node> params param_list param args arg_list
%type <node> program
%start program

%%
/* TODO: Your rules here. */

/* Example:
program: declaration_list {$$ = node( "program", 1, $1); gt->root = $$;}
       ;
*/
// to match any '_' not inside double quotes

program: declaration_list {$$ = node( "program", 1, $1); gt->root = $$;};
declaration_list: declaration_list declaration | declaration
declaration: var_declaration | fun_declaration
var_declaration: type_specifier ID | type_specifier ID LBRACKET INTEGER RBRACKET
type_specifier: INT_T {$$ = yylval.node;} | VOID_T {$$ = yylval.node;} | FLOAT_T {$$ = yylval.node;}
fun_declaration:type_specifier ID LPAREN params RPAREN compound_stmt
params: param_list | VOID_T
param_list: param_list COLON param | param
param:type_specifier   ID | type_specifier ID LBRACKET INTEGER RBRACKET
compound_stmt:  LCURLY local_declarations statement_list RCURLY
local_declarations:local_declarations var_declaration | %empty
statement_list:statement_list statement | %empty
statement: expression_stmt | compound_stmt | selection_stmt | iteration_stmt | return_stmt
expression_stmt:expression   SEMICOLON | SEMICOLON
selection_stmt: IF LPAREN expression RPAREN statement |   IF LPAREN expression RPAREN statement ELSE statement
iteration_stmt:  WHILE LPAREN expression RPAREN statement
return_stmt:  RETURN SEMICOLON | RETURN expression SEMICOLON
expression: var ASSIGN expression | simple_expression
var: ID | ID LBRACKET expression RBRACKET
simple_expression: additive_expression relop additive_expression {     $$ = node ("simple-expression", 3, $1, $2, $3);     gt->root = $$; } | additive_expression {     $$ = node ("simple-expression", 1, $1);     gt->root = $$; }
relop: LE {$$ = yylval.node;} | LT {$$ = yylval.node;} | GT {$$ = yylval.node;} | GE {$$ = yylval.node;} | EQ {$$ = yylval.node;} | NEQ {$$ = yylval.node;}
additive_expression:
additive_expression addop term {
    $$ = node ("additive-expression", 3, $1, $2, $3);
}
| term {
    $$ = node ("additive-expression", 1, $1);
}
addop: ADD {$$ = yylval.node;} | MINUS {$$ = yylval.node;}
term:
term mulop factor {
    $$ = node ("term", 3, $1, $2, $3);
}
| factor {
    $$ = node ("term", 1, $1);
}
mulop: MUL {$$ = yylval.node;} | DIV {$$ = yylval.node;}
factor:
LPAREN simple_expression RPAREN {
    $$ = node ("factor", 3, $1, $2, $3);
}
| var
| call
| integer {
    $$ = node ("factor", 1, $1);
}
| float {
    $$ = node ("factor", 1, $1);
}
integer: INTEGER {$$ = yylval.node;}
float: FLOATPOINT {$$ = yylval.node;}
call: ID LPAREN args RPAREN
args: arg_list | %empty
arg_list: arg_list COLON expression | expression


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
