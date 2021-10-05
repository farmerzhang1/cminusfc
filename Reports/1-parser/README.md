# Lab1 实验报告
学号 ********

姓名 ***

## 实验要求
完成cminus的语法分析

(本此实验使用clang-10(gcc 9.3经测试也可以))
## 实验难点
1. %union 编译

2. 内存泄漏
## 实验设计
### 词法分析
阅读代码发现`%union`只需一个类型`syntax_tree_node*`

需要在bison中利用以下directive（可以代码段至`*.tab.h`和`*.tab.c`中），才能使生成的头文件中的token编译
```c
%code requires {
#include "syntax_tree.h"
}
```

然后，给所有终结符都分配一个token，方便生成节点。
```bison
%token <node> ID
%token <node> INT_T VOID_T FLOAT_T
%token <node> LE LT GT GE EQ NEQ // rel op
...
```
然后给语法规则定义`%type`。

在flex文件中，匹配终结符，直接返回即可。这里定义了几个helper function来处理行列数。
```c
void new_line () {
    pos_start = pos_end = 0;
    lines += 1;
}

void comments (const char* str) {
    pos_start = pos_end;
    while (*str) {
        if (*str == '\n') new_line();
        else pos_end++;
        str++;
    }
}

void routine (const char* yytext) {
    pos_start = pos_end;
    pos_end += strlen(yytext);
    pass_node (yytext);
}
```
另外，本想使用`std::map`将字符串与bison生成的enum一一对应，但是这里使用的是c语言。就此作罢。
### 语法分析
将`Basics.md`里的语法全部复制进去，加入不同的action就可以了。

典型例子如下，对于不同的语法产生式，改变字符串以及子节点数即可。
```bison
declaration_list:
declaration_list declaration {
    $$ = node ("declaration-list", 2, $1, $2);
}
| declaration {
    $$ = node ("declaration-list", 1, $1);
}

```
几处需要注意的地方：空语句也需要调用node函数（传入的参数`children_num`为0），并且最好使用`%empty`，方便查看。
```bison
%empty { $$ = node ("local-declarations", 0); }
```
### 内存检查
使用valgrind检查内存泄漏
```bash
valgrind build/parser tests/parser/easy/expr.cminus
```
flex和`parse`函数中有未释放的内存
```
LEAK SUMMARY:
    definitely lost: 0 bytes in 0 blocks
    indirectly lost: 0 bytes in 0 blocks
        possibly lost: 0 bytes in 0 blocks
    still reachable: 16,930 bytes in 4 blocks
            suppressed: 0 bytes in 0 blocks
```
需要在.y文件中的parse里加入以下两行代码
```c
syntax_tree *parse(const char *input_path) {
    ... // 省略
    fclose(yyin); // 关闭文件
    yylex_destroy(); // 释放flex buffer
    return gt;
}
```
修改过后valgrind输出如下
```
    HEAP SUMMARY:
        in use at exit: 0 bytes in 0 blocks
      total heap usage: 184 allocs, 184 frees, 117,290 bytes allocated

    All heap blocks were freed -- no leaks are possible
```
## 实验结果验证
运行测试脚本
```
tests/parser/test_syntax.sh easy verbose
tests/parser/test_syntax.sh normal verbose
tests/parser/test_syntax.sh hard verbose
```
全部通过。

### 自行测试
```c
int/* no ma_in*/ main (float f/**/) {
    int i/* // this is /*comment
    */;
    int j;
    j = i /*this is wrong, can't have rvalue to be assigned = i+j*/ = i / j + j * 2/3;
    if (i > 0)
        if (i == 100) i = 0;
        else; else;
    if (1.8 /* ?? really strange */);
    if (i = 0 /* good old bug */);
}
```
以及负数的测试
```c
int mian /* on purpose! */ (void) {
    int i;
    i = /* no minus sign - */ 1;
}
```
若取消注释，根据语法应无法通过parsing。
## 实验反馈
本次实验难度不大，利用flex与bison构建了一个类c语言的语法分析器。
