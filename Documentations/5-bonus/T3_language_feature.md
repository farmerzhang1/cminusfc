# 语言特性的扩展

## 0.前言

为编程语言增加新的语法特性，是进一步发展该语言的关键所在。而本学期的[`Cminus-f`](../1-parser/Basics.md)比较简单小巧，因此本赛道将通过为其增加一些新的特性来完善其功能。

下面的部分，我们将会简要地介绍几种可选的特性。当然，在这个部分里，你也可以自行选择并探索其他高级特性，最后我们将会综合语言特性的实现难度、实现效果和代码完整性进行评价。

## 1.几个可供选择的语言特性

### 指针
对于C语言的指针，大家都比较熟悉。下面给出1个简单的样例方便大家参考。

``` c
void main(void){
  int x;
  int *y;
  int **z;
  y = &x;
  z = &y;
  x = 1;
  **z = 2;
  return;
}
```

### 多维数组
由于数组的实现相对简单，因此建议和指针搭配实现。下面给出2个简单的参考样例。
``` c
void main(void){
  int x[2][2];
  int *y[2];
  y[0] = x[0];
  y[1] = x[1];
  x[0][0] = 1;
  *(x[0]+1) = 2;
  y[1][0] = 3;
  *(y[1]+1) = 4;
  return;
}
```

``` c
void main(void){
  int x[2][3];
  int (*y)[3];
  y = x;
  y[0][1] = 1;
  y[1][2] = 2;
  return;
}
```
### 结构体
结构体是高级语言非常重要的特性之一。

基本结构体样例如下：
  ``` c
  // 定义一个结构体 `struct foo`， 成员是 `a` 和 `b`
  struct foo {
      int a;
      float b;
  };
  // 定义一个结构体实例 s 并进行初始化
  struct foo s = { 1, 2. };
  // 访问 a 成员
  s.a;

  // 定义一个匿名结构体的实例 `c`， 成员是 `a` 和 `b`，同时进行了初始化
  struct {
      int a;
      float b;
  } c = { 1, 2. };

  // 定义一个结构体 `struct foo`， 成员是 `a` 和 `b`，并定义一个该结构体的实例 `c`
  struct foo {
      int a;
      float b;
  } c;
  ```

嵌套结构体样例如下：
  ``` c
  struct foo {
      int a;
      struct bar {
         float b;
         float c;
      } d;
      struct {
          int e;
          int f;
      } g;
  };
  struct foo h = {1, {2., 3.}, {4, 5}};
  struct bar i = {2, 3};
  ```
### 初始化
一个友好且合理的初始化，在开发过程中也是很有帮助的。鉴于初始化功能较为简单，建议搭配结构体特性一起实现。

下面是可供参考的数组的初始化样例。

  ``` c
  // 初始化为前三个值分别是 1，2 和 3，剩下的初始化为0
  int a[10] = {1, 2, 3};
  // 结构体数组初始化
  struct foo {
      int a;
  };
  // 结构体数组初始化，未写明的结构体初始化为 {0}
  struct foo s[10] = {{0},{1}}
  ```
### 参考链接与其他选项
#### 参考链接
指针相关: [LLVM IR - Pointer Type](https://releases.llvm.org/10.0.0/docs/LangRef.html#pointer-type)，[LLVM IR - GEP Instruction](https://releases.llvm.org/10.0.0/docs/LangRef.html#getelementptr-instruction)。
#### 其他选项
最后，大家也可以参考[`GNU C`的扩展](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html#C-Extensions)，来选择自己感兴趣又富有挑战性的特性进行开发。

## 2.注意事项

- 本赛道必需完成下面特性中的一个才视为有效提交并参与评分。这些特性是：
  - 结构体
  - 指针
  - 不低于指针实现难度的其他特性（由老师和助教们进行判定）
- 本赛道同赛道4，需要好好思考如何体现出工作量（或创新性），否则评级不会很高。
- 本赛道的扩展不必限定在C语言的子集中。例如关于指针这一特性，[`Golang`](https://gobyexample.com/pointers)给出了另一种设计方式，大家可以进行参考。
