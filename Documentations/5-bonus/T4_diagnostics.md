# 诊断信息增强

## 0.前言

在经过四个实验之后，你应该得到了一个基本能用的编译器。然而，这个编译器怎么看怎么像是个「玩具」：遇到错误输入会直接罢工，根本不给出更详细的诊断信息。

本赛道则是一个完善已有的 Cminusf 编译器的机会：如何从错误中恢复，继续解析？如何在各个阶段给出丰富翔实的诊断信息？下面给出一些可供参考的想法。（并不全面，请自己补充更多情况！）

## 1. 可供参考的诊断信息增强

### 词法和语法

目前语法错误仅给出 "syntax error" 报告，请扩展这个报错，使之说明报错发生的具体位置、代码上下文等等。例如：

```c
file.cminus:1:void main2()
file.cminus:1:     ~~~~^
error: identifier can only contain letters
file.cminus:1:void main2()
file.cminus:1:          ^~
error: the param list must not be empty
```

此外，解析器目前在碰到一个语法错误时就会直接罢工。你应该让它尽可能继续解析，一次性揭露尽可能多的错误。

参考 Bison 手册：[Error Recovery](https://www.gnu.org/software/bison/manual/html_node/Error-Recovery.html)。

### 语义检查

在做代码生成时，会遇到一些特殊的错误情况，如：

1. 变量、函数未定义
2. 变量未初始化
3. 忘记 `return`

编译器应该可以针对如上错误产生相应的报错。此外，还有一些情况值得警告。如：

1. 除0
2. 数组越界访问
3. 数字字面量太大
4. 定义了的变量未使用/未赋值过
5. 赋值从来没被使用过

```
file.cminus:10:    while (x = true) {
file.cminus:10:           ^
warning: possibly unintended use of assignment in while, consider using () if it's what you mean
file.cminus:11:        y = x;
file.cminus:11:            ^
warning: x can be uninitialized at this point
file.cminus:10:    arr[10] = 1;
file.cminus:10:        ^~
warning: access element at 10 but arr only contains 10 elements
note: array arr defined as int arr[10], but the index is 10
```

### 其他功能

1. 编译器通常应支持 `-Werror`，将警告变成错误。
2. 编译器通常应支持 `-Wxxxx` 和 `-Wno-xxxx`，将某一类警告开启/关闭。
3. 实现更复杂的分析，产生更精确的语义检查。例如，你可以实现一个更精确的数组越界检查：

  ```c
  int arr[10] = ...; // 数组常量
  // 用户输入 x
  // return arr[x]; // 产生警告! 因为 x 可能不在 [0,10) 之内
  if (x >= 0 && x < 10) {
      return arr[x];  // 没有警告
  } else {
      return default_value;
  }
  ```
  
  同理，可以实现一个更精确的除0检查。

## 2. 注意事项

- 上面提供的仅是一部分可能性，更多情况可以参考 [Clang 的警告选项](https://clang.llvm.org/docs/DiagnosticsReference.html)。如果额外实现了其他语言扩展，如指针，也可以针对这些功能产生相应的诊断信息。
- 该赛道没有较大的技术难点，所以如果选择本赛道，你真的应该好好考虑一下如何体现出工作量（或创新性），否则评级不会很高。
- 本赛道必须至少完成错误恢复 **和** 5 种语法报错 **和** 变量未定义警告 **和** (不是或) 另一种语义警告方才视作有效提交。
