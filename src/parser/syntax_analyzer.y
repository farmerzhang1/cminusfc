%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern int yylex_destroy();
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

/* Example:
program: declaration_list {$$ = node( "program", 1, $1); gt->root = $$;}
       ;
*/

program: declaration_list {$$ = node("program", 1, $1); gt->root = $$;};
declaration_list:
declaration_list declaration {
    $$ = node ("declaration-list", 2, $1, $2);
}
| declaration {
    $$ = node ("declaration-list", 1, $1);
}
declaration: var_declaration { $$ = node ("declaration", 1, $1); } | fun_declaration { $$ = node ("declaration", 1, $1); }
var_declaration: type_specifier ID SEMICOLON {
    $$ = node ("var-declaration", 3, $1, $2, $3);
} | type_specifier ID LBRACKET INTEGER RBRACKET SEMICOLON {
    $$ = node ("var-declaration", 6, $1, $2, $3, $4, $5, $6);
}
type_specifier: INT_T { $$ = node ("type-specifier", 1, $1); } | VOID_T {$$ = node ("type-specifier", 1, $1);} | FLOAT_T {$$ = node ("type-specifier", 1, $1);}
fun_declaration: type_specifier ID LPAREN params RPAREN compound_stmt {
    $$ = node ("fun-declaration", 6, $1, $2, $3, $4, $5, $6);
}
params: param_list { $$ = node ("params", 1, $1); } | VOID_T { $$ = node ("params", 1, $1); }
param_list: param_list COLON param {
    $$ = node ("param-list", 3, $1, $2, $3);
} | param { $$ = node ("param-list", 1, $1); }
param: type_specifier ID {
    $$ = node ("param", 2, $1, $2);
} | type_specifier ID LBRACKET RBRACKET {
    $$ = node ("param", 4, $1, $2, $3, $4);
}
compound_stmt: LCURLY local_declarations statement_list RCURLY {
    $$ = node ("compound-stmt", 4, $1, $2, $3, $4);
}
local_declarations: local_declarations var_declaration {
    $$ = node ("local-declarations", 2, $1, $2);
} | %empty { $$ = node ("local-declarations", 0); }
statement_list: statement_list statement {
    $$ = node ("statement-list", 2, $1, $2);
} | %empty { $$ = node ("statement-list", 0); }
statement: expression_stmt { $$ = node ("statement", 1, $1); }
| compound_stmt     { $$ = node ("statement", 1, $1); }
| selection_stmt    { $$ = node ("statement", 1, $1); }
| iteration_stmt    { $$ = node ("statement", 1, $1); }
| return_stmt       { $$ = node ("statement", 1, $1); }
expression_stmt: expression SEMICOLON { $$ = node ("expression-stmt", 2, $1, $2); }
| SEMICOLON { $$ = node ("expression-stmt", 1, $1); }
selection_stmt: IF LPAREN expression RPAREN statement {
    $$ = node ("selection-stmt", 5, $1, $2, $3, $4, $5);
} | IF LPAREN expression RPAREN statement ELSE statement {
    $$ = node ("selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);
}
iteration_stmt: WHILE LPAREN expression RPAREN statement {
    $$ = node ("iteration-stmt", 5, $1, $2, $3, $4, $5);
}
return_stmt:  RETURN SEMICOLON {
    $$ = node ("return-stmt", 2, $1, $2);
} | RETURN expression SEMICOLON {
    $$ = node ("return-stmt", 3, $1, $2, $3);
}
expression: var ASSIGN expression {
    $$ = node ("expression", 3, $1, $2, $3);
} | simple_expression { $$ = node ("expression", 1, $1); }
var: ID {
    $$ = node ("var", 1, $1);
} | ID LBRACKET expression RBRACKET {
    $$ = node ("var", 4, $1, $2, $3, $4);
}
simple_expression:
additive_expression relop additive_expression { $$ = node ("simple-expression", 3, $1, $2, $3); }
| additive_expression { $$ = node ("simple-expression", 1, $1); }
relop: LE {$$ = node ("relop", 1, $1); } | LT {$$ = node ("relop", 1, $1);} | GT {$$ = node ("relop", 1, $1);} | GE {$$ = node ("relop", 1, $1);} | EQ {$$ = node ("relop", 1, $1);} | NEQ {$$ = node ("relop", 1, $1);}
additive_expression:
additive_expression addop term {
    $$ = node ("additive-expression", 3, $1, $2, $3);
}
| term {
    $$ = node ("additive-expression", 1, $1);
}
addop: ADD {$$ = node ("addop", 1, $1);} | MINUS { $$ = node ("addop", 1, $1);}
term:
term mulop factor {
    $$ = node ("term", 3, $1, $2, $3);
}
| factor {
    $$ = node ("term", 1, $1);
}
mulop: MUL {$$ = node ("mulop", 1, $1);} | DIV {$$ = node ("mulop", 1, $1);}
factor:
LPAREN expression RPAREN {
    $$ = node ("factor", 3, $1, $2, $3);
}
| var {$$ = node ("factor", 1, $1);}
| call {$$ = node ("factor", 1, $1);}
| integer {
    $$ = node ("factor", 1, $1);
}
| float {
    $$ = node ("factor", 1, $1);
}
integer: INTEGER { $$ = node ("integer", 1, $1); }
float: FLOATPOINT { $$ = node ("float", 1, $1); }
call: ID LPAREN args RPAREN {
    $$ = node ("call", 4, $1, $2, $3, $4);
}
args: arg_list {
    $$ = node ("args", 1, $1);
} | %empty { $$ = node ("args", 0); }
arg_list: arg_list COLON expression {
    $$ = node ("arg-list", 3, $1, $2, $3);
} | expression {$$ = node ("arg-list", 1, $1);}


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
    fclose(yyin);
    yylex_destroy();
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
