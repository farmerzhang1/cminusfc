# lab1 实验报告
学号 ********

姓名 ***

## 实验要求

## 实验难点
1. %union编译

    需要在bison中利用以下directive（复制该段代码至`*.tab.h`和`*.tab.c`中），才能使生成的头文件中的token编译
    ```bison
    %code requires {
    #include "syntax_tree.h"
    }
    ```
    ```c
    union YYSTYPE
    {
        syntax_tree_node* node;
    };
    ```

2. 内存泄漏
    使用valgrind检查内存泄漏
    ```bash
    valgrind build/parser tests/parser/easy/expr.cminus
    ```
    flex(不是助教提供的代码)有未释放的内存
    ```
    LEAK SUMMARY:
        definitely lost: 0 bytes in 0 blocks
        indirectly lost: 0 bytes in 0 blocks
          possibly lost: 0 bytes in 0 blocks
        still reachable: 16,930 bytes in 4 blocks
             suppressed: 0 bytes in 0 blocks
    ```
## 实验设计
### 词法分析
阅读代码发现`%union`只需一个类型`syntax_tree_node*`

首先，给所有终结符都分配一个token，方便生成节点。
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

几处需要注意的地方：空语句也需要调用node函数（传入的参数为0），并且最好使用`%empty`，方便查看。
```bison
%empty { $$ = node ("local-declarations", 0); }
```
## 实验结果验证

请提供部分自行设计的测试

## 实验反馈
挺简单的
