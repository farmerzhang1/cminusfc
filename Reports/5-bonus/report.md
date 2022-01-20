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
### linear scan
不知道为什么把线性扫描的bb顺序改成按int2bb就好了很多神秘bug，明明加入intervals的排序是intervalLess啊（？？？）

还有complex4.cminus, 这个的问题是
```c
while (i >= 0) {
    temp = get(equ, i, var, varone);
    j = i + 1;
    while (j < var) {
        if (1 - isZero(get(equ, i, j, varone)))
            temp = temp - get(equ, i, j, varone) * vars[j];
        j = j + 1;
    }
    vars[i] = temp / get(equ, i, i, varone);
    i = i - 1;
}
```

`varone`在倒二行就结束了，然后我的寄存器分配把分给varone的寄存器分给了下一行的`i-1`，但是循环还在呢