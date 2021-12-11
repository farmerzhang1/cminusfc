# 后端生成

## 0.前言

经过前四个Lab，我们的 cminus compiler 已经可以生成有效的 llvm ir （前序实验中，使用 clang 编译 llvm ir 以验证正确性），在后端生成的赛道里，我们鼓励并期待同学选择一个可验证的后端平台（x86, arm, riscv, cpu0），将生成的 llvm ir 翻译成对应后端汇编语言。

## 1.实验要求

选择一个你最熟悉的后端（x86, arm, riscv, cpu0），我们使用 [cpu0](http://ccckmit.wikidot.com/ocs:cpu0) 作为硬件的例子，来说明后端开发的流程，选择其他后端也是可以的，但需要自行调研提供一个可验证的 simulator 来证明生成汇编程序的正确性。

cpu0 是一个非常简单的 riscv 架构处理器，即使你没有接触过 cpu0，只要了解一点 riscv 架构处理器（或者 mips 处理器）就可以轻松的接受这个处理器。在 [Tutorial: Creating an LLVM Backend for the Cpu0 Architecture](https://jonathan2251.github.io/lbd/llvmstructure.html) 一文中，作者详细介绍了通过 llvm 开发 cpu0 的后端的流程，并提供了一个 cpu0 的 simulator 以验证生成汇编的正确性。

在开发后端之前，需要对后端指令集的ABI(Application Binary Interface, 应用程序二进制接口)，需要有一定的了解：寄存器使用约定，函数调用约定等，然后可以选择：

1. 借助 llvm 后端架构来开发后端  
   - 教程：[官方教程](https://llvm.org/docs/CodeGenerator.html)，[中文博客](https://zhuanlan.zhihu.com/p/149599651)
   - 优点：
     文档教程详细，中英文均有，前人经验丰富。
   - 缺点：
     由于 llvm 框架比较庞杂，需要在了解其机制的基础上精简需要开发的模块。  

2. 自行设计后端架构  
   - 要求：与中间代码生成模块解耦合，通过向后端模块传入一个 `module` 来进行翻译生成汇编代码。
   - 优点：
     可以针对 cminus 语言与后端指令的子集进行设计，更轻量级的实现在遇到问题时方便debug。
   - 缺点：
     在开发中需要不断根据需求迭代重构自己的架构，对目标平台的理解与抽象能力要求较高。  

**注**：寄存器分配模块可以看作虚拟寄存器与物理寄存器间的映射，在缺少寄存器分配模块的情况下，可以默认所有虚拟寄存器映射到栈空间上，选择仅仅使用三个寄存器的方式翻译指令，两个作为操作数寄存器，一个作为指令返回值寄存器，并且在每条指令前后插入访存指令来存储中间结果。

## 2.一些建议

1. 在开始实验前先有足够的调研以确定实验的技术路线，可以仔细参考 Tutorial: Creating an LLVM Backend for the Cpu0 Architecture([英文原版](https://jonathan2251.github.io/lbd/llvmstructure.html)，[中文博客](https://zhuanlan.zhihu.com/p/351848328))。
2. 使用 gdb 调试生成的汇编比肉眼读取更加有效。
3. 仔细思考调研 phi 指令翻译采用的方式。
4. 如果有技术路线上的问题可以及时联系助教与老师。

## 3.验收要求

需要给出一些例子编译后在 simulator 上的运行结果来证明后端的正确性，考虑到后端实验工作量可能较大，即使后端不能支持所有的语法特性，我们也会按照汇报展示的实现部分来给出合适的评价。

