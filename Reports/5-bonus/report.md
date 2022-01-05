# Lab5 实验报告
参加赛道一

线性扫描的live interval不是活跃变量，而是所有的(虚拟)寄存器（！！555~~~）

alloca数组不分配寄存器，分配栈空间

gdb debug risc-v, c.r. https://danielmangum.com/posts/risc-v-bytes-qemu-gdb/
（太痛苦了吧）

一个实现中有错误的地方是，比如，在complex2.cminus中，sort函数签名为 `(float*, int, float)->void`，寄存器分配时会分别给三个参数分配`(a0, a1, fa0)`（架构依赖的寄存器分配）
在
```c
while(i < high - 1)
```
中，high因为ssa form(而且我认为这时的 `fa0` 是 `fresh` 的)，会直接使用 `fsub fa0`，但是在
```c
k = minloc(a, i, high)
```
中， `fa0` 被赋给 `sitofp i`, 原本的 `high` 值被抹掉
