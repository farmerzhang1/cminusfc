# 中间代码的高级优化

## 0.前言

经过Lab4的优化实验，相信同学们已经基本了解`pass`的概念，而在`中间代码的高级优化`赛道里，我们鼓励并期待同学进一步开发更为高级，也更为高效的pass。

下面的部分，我们将会简要地介绍几种pass。当然，在这个部分里，你也可以选择自行开发，探究其他pass，最后我们将会综合pass的实现难度，优化效果，代码完整性进行评价。

## 1.几个可供选择的pass

### SCCP Combined DCE

```c++
int fun(){
    int i = 1;
    int a = 4;
    int b = 0;
    while(i < 10){
        i = i + 1;
        if(b == 0){
            a = 1;
        }
        else{
            a = 3;
            b = 1;
        }
    }
    return a;
}
```

如上面例子所示，while里的if语句的else分支实际上是`不可达的(unreachable)`，但是一般的常量传播和死代码删除是无法识别出来，这时候如果可以基于控制流的信息，进行可达性分析，实际上可以将上面的代码优化为

```c++
int fun(){
    return 1;
}
```

具体的分析过程以及实现方式可以参考[Combining Analyzes, Combining Optimizations](https://scholarship.rice.edu/bitstream/handle/1911/96451/TR95-252.pdf?sequence=1&isAllowed=y)

### GVN & GCM

同学们已经在课堂上学习了`公共子表达式消除`的概念，实际上，在SSA形式里，我们可以做一些更为激进的优化，GVN & GCM 可以消除冗余计算，提取循环不变式，减轻寄存器压力，代数简化，常量折叠。

**GVN**：按照某个顺序访问所有表达式，如果 e2 和之前遇到的 e1 运算类型相同且参数的value numbering 相同，那么e2 和 e1 的 value numbering 相同，可以用e1替换e2

例如：

```c++
if(y > 0){
    a = b + c;
    x = a + e;
}
else{
    d = b + c;
    x = d + f;
}
```

可以变化为

```c++
if(y > 0){
    a_1 = b + c;
    x = a_1 + e;
}
else{
    x = a_1 + f;
}
```

当然，这样简单替换，产生的程序可能是不合法的，一个表达式不一定在使用前被计算，所以我们需要通过GCM使之合法。

**GCM**：先将代码移动到尽量早的地方，再移动到尽量晚的地方

```c++
a_1 = b + c;
if(y > 0){
    x = a_1 + e;
}
else{
    x = a_1 + f;
}
```

### Strength Reduction

循环是程序里面常见的一类代码，例如

```c++
int i = 0;
while( i < 10 ) {
  int j = 3 * i + 2;
  a[j] = a[j] - 2;
  i = i + 2;
}
```

在底层指令执行的时候，乘法指令大约需要3~5个时钟周期，而加法只需要1个时钟周期，若将上述代码优化成为

```c++
int i = 0;
int j = 2; // j = 3 * 0 + 2
while( i < 10 ) {
  a[j] = a[j] - 2;
  i = i + 2;
  j = j + 6; // j = j + 3 * 2
}
```

可以有效提高代码运行效率。更加具体的介绍可以参考[博客](https://www.cs.cornell.edu/courses/cs6120/2019fa/blog/strength-reduction-pass-in-llvm/)

使用类似的分析方法，可以完成一个类似的pass——Loop Unrolling，在此不再过多介绍，同学们可以自行查阅相关资料。

### Geometric Sum

这是一个比较激进的优化。

举个例子如下：

```c
int sum(int count){
  int result = 0;
  if (count > 0) {
    int j = 0;
	while (j < count){
      result = result + j*j;
      j = j + 1;
    }
  }
  return result;
}
```

实际上可以看出，最后result的值只和count有关，可以使用级数求和的方法直接算出。

实际上result的值为 

result = count - 1 + 3 * (count - 1) * (count - 2) / 2 + (count - 1) * (count - 2) * (count - 3) / 3

GCC和LLVM都在Scalar Evolution里面使用了这个优化手段(你可以尝试开启GCC O3优化来看下实际效果)。在这篇[博客](https://kristerw.blogspot.com/2019/04/how-llvm-optimizes-geometric-sums.html)里，你可以找到对于这个问题的详细探究

另外需要注意的是，如果将循环替代为级数求和，实际的代码效率不会很高，因为最后引入了代价极高的除法运算。所以如果你选择了做这个pass，你还需要额外做一个[除常数优化](https://gmplib.org/~tege/divcnst-pldi94.pdf)

## 2.注意事项

- 可以参考、阅读LLVM实现的pass的源码，当然LLVM是发展了多年的工业级别的实现，代码动辄数千行，你可以硬着头皮往下读，也可以参考**早些年的版本**
- 非常欢迎同学们自己研究一些pass，无需局限于上述提到的几种
- 答辩时至少自己准备一个例子来**实际演示**你的优化所达到的效果
- 本赛道需要**至少提交两个有效pass**(可以不是上面的pass，但难度应该不低于上述pass)，才视为有效提交
