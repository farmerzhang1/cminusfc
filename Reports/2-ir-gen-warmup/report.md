# lab2 实验报告
学号 姓名

## 问题1: cpp 与 .ll 的对应
请说明你的 cpp 代码片段和 .ll 的每个 BasicBlock 的对应关系。
1. assign

    一一对应
2. function

    callee下的基本块对应 .ll 文件中的基本块

    main 则与 .ll 文件中的 main 对应
3. if

    entry对应main函数入口
    trueBB对应label 5
    falseBB对应label 6
    retBB对应label 7
4. while

    entry对应main函数入口
    bbwhile对应label　
## 问题2: Visitor Pattern
分析 `calc` 程序在输入为 `4 * (8 + 4 - 1) / 2` 时的行为：
1. 请画出该表达式对应的抽象语法树（使用 `calc_ast.hpp` 中的 `CalcAST*` 类型和在该类型中存储的值来表示），并给节点使用数字编号。
    ```
    >--+ CalcASTInput 1
    |  >--+ CalcASTExpression 2
    |  |  >--+ CalcASTTerm 3
    |  |  |  >--+ CalcASTTerm 4
    |  |  |  |  >--+ CalcASTTerm 5
    |  |  |  |  |  >--+ CalcASTFactor 6
    |  |  |  |  |  |  >--+ CalcASTNum 7
    |  |  |  |  |  |  |  >--* val 4 (8)
    |  |  |  |  >--+ Mulop 9
    |  |  |  |  |  >--* OP_MUL
    |  |  |  |  >--+ CalcASTFactor 10
    |  |  |  |  |  >--+ CalcASTExpression 11
    |  |  |  |  |  |  >--+ CalcASTExpression 12
    |  |  |  |  |  |  |  >--+ CalcASTExpression 13
    |  |  |  |  |  |  |  |  >--+ CalcASTTerm 14
    |  |  |  |  |  |  |  |  |  >--+ CalcASTFactor 15
    |  |  |  |  |  |  |  |  |  |  >--+ CalcASTNum 16
    |  |  |  |  |  |  |  |  |  |  |  >--* val 8 (17)
    |  |  |  |  |  |  |  >--+ Addop 18
    |  |  |  |  |  |  |  |  >--* OP_PLUS
    |  |  |  |  |  |  |  >--+ CalcASTTerm 19
    |  |  |  |  |  |  |  |  >--+ CalcASTFactor 20
    |  |  |  |  |  |  |  |  |  >--+ CalcASTNum 21
    |  |  |  |  |  |  |  |  |  |  >--* val 4 (22)
    |  |  |  |  |  |  >--+ Addop 23
    |  |  |  |  |  |  |  >--* OP_MINUS
    |  |  |  |  |  |  >--+ CalcASTTerm 24
    |  |  |  |  |  |  |  >--+ CalcASTFactor 25
    |  |  |  |  |  |  |  |  >--+ CalcASTNum 26
    |  |  |  |  |  |  |  |  |  >--* val 1 (27)
    |  |  |  >--+ Mulop 28
    |  |  |  |  >--* OP_DIVS
    |  |  |  >--+ CalcASTFactor 29
    |  |  |  |  >--+ CalcASTNum 30
    |  |  |  |  |  >--* val 2 (31)
    ```
2. 请指出示例代码在用访问者模式遍历该语法树时的遍历顺序。

    1 -> 2 -> 3 -> 4 -> ... (深度优先) -> 30 -> 31

序列请按如下格式指明（序号为问题 2.1 中的编号）：
3->2->5->1

## 问题3: getelementptr
请给出 [LightIR.md](../../Documentations/common/LightIR.md) 中提到的两种 getelementptr 用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0`
  - `%2 = getelementptr i32, i32* %1, i32 %0`

第一种用法的`i32 0`是要跨过`[10 x i32]*`这个指针的，值为0意味着从该指针上偏移0个元素。如果只提供一个偏移量，得到的类型是`[10 x i32]*`.

而第二种用法从i32指针上偏移`%0`个元素，得到一个指向i32的指针。
## 实验难点
描述在实验中遇到的问题、分析和解决方案。

## 实验反馈
吐槽?建议?
