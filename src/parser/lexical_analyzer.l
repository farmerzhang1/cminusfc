%option noyywrap

%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.tab.h"

int lines;
int pos_start;
int pos_end;

void pass_node(char *text) {
     yylval.node = new_syntax_tree_node(text);
}

/*****************声明和选项设置  end*****************/
%}

%%
/* to do for students */
/* two cases for you, pass_node will send flex's token to bison */

/* no map or dict in c, so we have to write one by one */
\+   { pos_start = pos_end; pos_end += 1; pass_node (yytext); return ADD; }
-    { pos_start = pos_end; pos_end += 1; pass_node (yytext); return MINUS; }
\*   { pos_start = pos_end; pos_end += 1; pass_node (yytext); return MUL; }
/    { pos_start = pos_end; pos_end += 1; pass_node (yytext); return DIV; }
"<=" { pos_start = pos_end; pos_end += 2; pass_node (yytext); return LE; }
">=" { pos_start = pos_end; pos_end += 2; pass_node (yytext); return GE; }
"==" { pos_start = pos_end; pos_end += 2; pass_node (yytext); return EQ; }
"!=" { pos_start = pos_end; pos_end += 2; pass_node (yytext); return NEQ; }
<    { pos_start = pos_end; pos_end += 1; pass_node (yytext); return LT; }
>    { pos_start = pos_end; pos_end += 1; pass_node (yytext); return GT; }
\(   { pos_start = pos_end; pos_end += 1; pass_node (yytext); return LPAREN; }
\)   { pos_start = pos_end; pos_end += 1; pass_node (yytext); return RPAREN; }
int  { pos_start = pos_end; pos_end += 3; pass_node (yytext); return INT_T; }
void { pos_start = pos_end; pos_end += 4; pass_node (yytext); return VOID_T; }
float { pos_start = pos_end; pos_end += 5; pass_node (yytext); return FLOAT_T; }
[a-z_][a-z_0-9]* {
     pos_start = pos_end;
     pos_end += strlen(yytext);
     pass_node (yytext);
     return ID;
     }
[1-9]\d+|\d  { pos_start = pos_end; pos_end += strlen(yytext); pass_node (yytext); return INTEGER}
([1-9]\d+|\d)\.[0-9]+ { pos_start = pos_end; pos_end += strlen(yytext); pass_node (yytext); return FLOATPOINT; }

. { pos_start = pos_end; pos_end++; }

/****请在此补全所有flex的模式与动作  end******/
%%
