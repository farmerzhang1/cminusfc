%option noyywrap

%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines = 1;
int pos_start = 0;
int pos_end = 0;

void pass_node(const char *text) {
     yylval.node = new_syntax_tree_node(text);
}

void new_line () {
     pos_start = pos_end = 0; lines += 1;
}

void comments (const char* str) {
     pos_start = pos_end;
     while (*str) { // 太久没写c了！！
          if (*str == '\n') new_line();
          else pos_end++;
          str++;
     }
}

void routine (const char* yytext) {
     pos_start = pos_end; pos_end += strlen(yytext); pass_node (yytext);
}
/*****************声明和选项设置  end*****************/
%}

/* to do for students */
/* two cases for you, pass_node will send flex's token to bison */

/* no map or dict in c, so we have to write one by one */
%%
\n   { new_line(); }
\+   { routine (yytext); return ADD; }
-    { routine (yytext); return MINUS; }
\*   { routine (yytext); return MUL; }
\/   { routine (yytext); return DIV; }
=    { routine (yytext); return ASSIGN; }
,    { routine (yytext); return COLON; }
;    { routine (yytext); return SEMICOLON; }
"<=" { routine (yytext); return LE; }
">=" { routine (yytext); return GE; }
"==" { routine (yytext); return EQ; }
"!=" { routine (yytext); return NEQ; }
\<   { routine (yytext); return LT; }
>    { routine (yytext); return GT; }
\(   { routine (yytext); return LPAREN; }
\)   { routine (yytext); return RPAREN; }
\[   { routine (yytext); return LBRACKET; }
\]   { routine (yytext); return RBRACKET; }
\{   { routine (yytext); return LCURLY; }
\}   { routine (yytext); return RCURLY; }
int  { routine (yytext); return INT_T; }
void { routine (yytext); return VOID_T; }
float { routine (yytext); return FLOAT_T; }
if   { routine (yytext); return IF; }
else { routine (yytext); return ELSE; }
while { routine (yytext); return WHILE; }
return { routine (yytext); return RETURN; }
[a-zA-Z]+ {
     routine (yytext);
     return ID;
}
([0-9]+\.|[0-9]*\.[0-9]+) {
     routine (yytext);
     return FLOATPOINT;
}
[0-9]+ {
     routine (yytext);
     return INTEGER;
}
\/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+\/ { comments (yytext); }
[ \t\r] { pos_start = pos_end; pos_end += strlen(yytext); }
. { pos_start = pos_end; pos_end++; printf("'.' :\t%s\n",yytext); }

%%
/****请在此补全所有flex的模式与动作  end******/
