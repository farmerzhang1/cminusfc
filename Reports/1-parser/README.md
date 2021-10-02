# lab1 实验报告
学号 ********

姓名 ***

## 实验要求

## 实验难点
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

内存泄漏（flex未释放的内存）
## 实验设计

## 实验结果验证

请提供部分自行设计的测试

## 实验反馈
