# Lab4 实验报告
- [Lab4 实验报告](#lab4-实验报告)
  - [实验要求](#实验要求)
    - [常量传播与死代码删除](#常量传播与死代码删除)
    - [循环不变式外提](#循环不变式外提)
    - [活跃变量分析](#活跃变量分析)
  - [实验难点](#实验难点)
    - [常量传播与死代码删除](#常量传播与死代码删除-1)
    - [循环不变式外提](#循环不变式外提-1)
    - [活跃变量分析](#活跃变量分析-1)
  - [实验设计](#实验设计)
    - [常量传播](#常量传播)
      - [实现思路](#实现思路)
      - [相应代码](#相应代码)
      - [IR对比](#ir对比)
    - [循环不变式外提](#循环不变式外提-2)
      - [实现思路](#实现思路-1)
      - [相应代码](#相应代码-1)
      - [IR对比](#ir对比-1)
    - [活跃变量分析](#活跃变量分析-2)
      - [实现思路](#实现思路-2)
      - [相应代码](#相应代码-2)
    - [实验总结](#实验总结)
    - [实验反馈 （可选 不会评分）](#实验反馈-可选-不会评分)
    - [组间交流 （可选）](#组间交流-可选)

## 实验要求
实现三个 Pass.
### 常量传播与死代码删除

如果一个变量的值可以在编译优化阶段直接计算出，那么就直接将该变量替换为常量（即计算出的结果值）。补充以下几点需要注意的地方：

a. 只需要考虑过程内的常量传播，可以不用考虑数组，全局变量只需要考虑块内的常量传播。

b. 整形浮点型都需要考虑。

c. 对于`a = 1 / 0`的情形，可以不考虑，即可以做处理也可以不处理。

### 循环不变式外提
要能够实现将与循环无关的表达式提取到循环的外面。不用考虑数组与全局变量。

a. 判断语句与循环无关，且外提没有副作用

b. 循环的条件块（就是在 LoopSearch 中找到的 Base 块）最多只有两个前驱，判断不变式应该外提到哪一个前驱。

### 活跃变量分析

能够实现分析 bb 块的入口和出口的活跃变量。

在`ActiveVars.hpp`中定义了两个成员`live_in`, `live_out`：

1. 将`pair<bb, IN[bb]>`插入`live_in`的 map 结构中。

2. 将`pair<bb, OUT[bb]>`插入`live_out` 的 map 结构中。

3. 调用`ActiveVars`类中的`print()`方法输出 bb 活跃变量情况到 json 文件。

数据流方程：

$`OUT[B] =\cup_{s是B的后继}IN[S]`$

的定义说明了S入口处活跃的变量在它所有前驱的出口处都是活跃的。由于`phi`指令的特殊性，只有控制流从`%bb1`传过来phi才产生`%op1`的活跃性，从`%bb2`传过来phi才产生`%op2`的活跃性。因此对此数据流方程需要做一些调整：

$`OUT[B] =\cup_{s是B的后继}IN[S]\cup_{s是B的后继} phi\_uses[S,B]`$。

其中`IN[S]`是S中剔除`phi`指令后分析出的入口变量结果。`phi_uses[S,B]`表示S中的`phi`指令参数中`label`为B的对应变量。

## 实验难点

### 常量传播与死代码删除 

1.需要考虑整数与浮点数之间的运算。

2.对于比较运算而言，需要实现新的函数进行操作。

3.删除可能是因为因为常量传播引起的无用分支。

### 循环不变式外提 

1.判断该语句是否为需要外提的循环不变式。

2.不变式应该提取到哪一个循环。

3.循环不变式外提后是否会出现新的循环不变式。


### 活跃变量分析

1.因为phi指令的特殊性导致数据流方程的修改。

2.如何正确的输出到json文件中。

3.根据use和def计算每个块的块头活跃变量集合和块尾活跃变量集合。


## 实验设计

### 常量传播

#### 实现思路

分别实现整数型与浮点数的四则运算与比较计算的函数，在主函数中，由函数体逐层遍历至每个指令，判断当前指令执行的具体操作，执行具体的函数，若存在相对应的常量浮点数或者常量整数，则需遍历其所有使用的地方，用常量代替，并且用delete_list存储需要删除的部分。

#### 相应代码

函数部分中整数型加法的实现
```cpp
case Instruction::add:
    return ConstantInt::get(c_value1 + c_value2, module_);
    break;
```
函数部分处理整数浮点混合运算的实现

```cpp
ConstantInt *ConstFolder::f_to_i(ConstantFP *value) {
int ans = (int)value->get_value();
return ConstantInt::get(ans, module_);
}
```
判断是否为浮点数常量的代码实现
```cpp
auto f_lhs = cast_constantfp(instr_type->get_operand(0));
```
用常量代替，并且用delete_list存储需要删除的部分的实现
```cpp
instr_type->replace_all_use_with(cf.compute_f(instr_type->get_instr_type(), f_lhs, f_rhs));
        delete_list.push_back(instr_type);
```
遍历常量所有使用的地方的实现
```cpp
void Value::replace_all_use_with(Value *new_val) {
for (auto use : use_list_) {
    auto val = dynamic_cast<User *>(use.val_);
    assert(val && "new_val is not a user");
    val->set_operand(use.arg_no_, new_val);
    }
}
```
#### IR对比
```c
int main(void) {
    int i;
    i = 1 + 1 + 1;
    return i;
}
```
未优化
```
define i32 @main() {
label_entry:
    %op2 = add i32 1, 1
    %op3 = add i32 %op2, 1
    ret i32 %op3
}
```
优化
```
define i32 @main() {
label_entry:
    ret i32 3
}
```
可见1+1+1变成了3
### 循环不变式外提

#### 实现思路

主函数由函数体逐层遍历到每一个循环以及循环中的每一个基本块，在基本块中遍历每一条指令，判断是否出现了需要外提的循环不变式，出现之后，需要及时外提。而判断与外提这两个功能由两个函数分别实现。

#### 相应代码

主函数中逐层遍历的过程实现
```cpp
    for (auto f : m_->get_functions()) {
        for (auto loop : loop_searcher.get_loops_in_func(f)) {
            bool inv = false;
            for (auto bb : *loop) {
                for (auto instr : bb->get_instructions()) {
                    inv |= inv_in_loop(instr, loop);
                }
            }
            if (inv) {
                moveout(loop);
                invariants.clear();
            }
        }
    }
```
判断是否出现了需要外提的循环不变式函数中，出现ret，call，br等指令时，显然没有循环不变式
```cpp
    if (instr->is_call())   return false;
    if (instr->is_ret())    return false;
    if (instr->is_br())     return false;
```
判断是否出现了需要外提的循环不变式，如果出现，则需要将函数返回true，并且将需要外提的循环不变式存入invariants
```cpp
    for (auto rand : instr->get_operands()) {
        if ((temp = dynamic_cast<Instruction *>(rand))) {
            if (!loop->contains(temp->get_parent())
                || invariants.contains(temp))
                ;
            else
                return false;
        }
    }
    invariants.insert(instr);
    return true;
```
对于每一个循环，遍历它的invariants，将目标循环不变式外提。
```cpp
    for (auto inv : invariants) {
        auto bb = inv->get_parent();
        bb->delete_instr(inv);
        pred_pre->add_instruction(inv);
        inv->set_parent(pred_pre);
    }
```
#### IR对比
```c
int i;
int j;
j = 10;
while (j > 0)
{i = 1 + 1 + 1; j = j - 1;}
return i;
```

```
define i32 @main() {
label_body1:          ; preds = %label_predicate0
  %op5 = add i32 1, 1
  %op6 = add i32 %op5, 1
  %op8 = sub i32 %op12, 1
  br label %label_predicate0
}
```
优化后，与循環无关的代码被移出循环体
```
define i32 @main() {
label_entry:
  %op5 = add i32 1, 1
  %op6 = add i32 %op5, 1
  br label %label_predicate0
...
label_body1:          ; preds = %label_predicate0
  %op8 = sub i32 %op12, 1
  br label %label_predicate0
}
```
该实现有两个问题
1. 若循环不执行，则不该移出
2. 移出的 invariant 应修改与之相关的 phi 语句
### 活跃变量分析

#### 实现思路

使用map嵌套结构`map<bb, map<bb, set<val>>>`处理phi语句，例如 pre1->suc 和 pre2->suc 这样三个bb可以有一个phi语句
```
_ = phi [%1, pre1] [%2, pre2]
```
在 pre1 的 out 里，%2 不是活跃变量； pre2 同理。
#### 相应代码
对于phi的特殊处理
```cpp
if (instr->is_phi()) {
    auto &curbb = phiuses[bb];
    for (int i = 0; i < instr->get_num_operand(); i += 2) {
        auto otherbb = dynamic_cast<BasicBlock*>(instr->get_operand(i+1));
        curbb[otherbb].insert(instr->get_operand(i));
    }
}
```
利用use和def以及后继块的in，来计算当前基本块的in和out的实现
```cpp
for (auto suc : (*itr)->get_succ_basic_blocks()) {
    for (auto var : live_in[suc]) {
        bool var_from_phi = false;
        for (auto [_, vars] : phiuses[suc]) var_from_phi |= (vars.find(var) != vars.end());// vars.contains(var);
        if (!var_from_phi || var_from_phi && (phiuses[suc][*itr].find(var) != phiuses[suc][*itr].end())) {
            out.insert(var);
        }
    }
}
changed |= out.size() != size_before; // do { ... } while (changed);
in = out;
for (auto def : def[*itr]) in.erase(def);
for (auto use : use[*itr]) in.insert(use);
```
虽然用嵌套的映射存储phi语句，但空间开销较大，可以优化。
### 实验总结

加深了对于C++语言中一些容器的使用理解，逐渐深入的了解到控制流图的实际跳转方式，通过常量传播，循环不变式外提，活跃变量分析等方法进行优化，熟悉了数据流分析的基本方法和处理逻辑。

### 实验反馈 （可选 不会评分）
使用 c++20 标准，容器元素使用智能指针/引用而不是 raw pointer

### 组间交流 （可选）
无
