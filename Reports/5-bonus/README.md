# 选做实验-赛道一 报告
- [选做实验-赛道一 报告](#选做实验-赛道一-报告)
  - [Abstract](#abstract)
  - [PHI 指令消除](#phi-指令消除)
  - [寄存器分配](#寄存器分配)
    - [寄存器 Reg 类](#寄存器-reg-类)
    - [Interval 构造](#interval-构造)
    - [线性扫描](#线性扫描)
  - [RISCV64 代码生成](#riscv64-代码生成)
    - [函数 prologue](#函数-prologue)
    - [函数 epilogue](#函数-epilogue)
    - [寄存器赋值](#寄存器赋值)
      - [整形常数](#整形常数)
      - [单精度浮点数](#单精度浮点数)
      - [指针](#指针)
      - [分配在寄存器上的变量](#分配在寄存器上的变量)
      - [SPILL 的变量](#spill-的变量)
    - [算术指令](#算术指令)
    - [比较指令](#比较指令)
    - [分支指令](#分支指令)
    - [int-float 类型转换](#int-float-类型转换)
      - [int to float](#int-to-float)
      - [float to int](#float-to-int)
  - [测试](#测试)
    - [环境](#环境)
    - [自动测试](#自动测试)
    - [eval_result](#eval_result)
  - [Future Works](#future-works)
  - [参考文献](#参考文献)
## Abstract
本次实验为 cminusf 语言构建了一个 RISC-V 64位后端，支持所有 cminusf 的语言特性。
## PHI 指令消除
phi elimination 采用最直观的消除法。

对于所有的 PHI 指令，在 entry 基本块中一一创建 alloca 指令，并用相对应的 load 指令替换 PHI
```cpp
auto entry = f->get_entry_block();
if (instr->is_phi()) {
    auto alloca = AllocaInst::create_alloca(instr->get_type(), entry);
}
...
for (auto it : its) { // its : vector<list<Instruction*>::iterator>
// its 存储一个基本块中所有的 phi 指令
    auto phi = dynamic_cast<PhiInst *>(*it);
    auto load = LoadInst::create_load(phi->get_type(), phi2alloca[phi]);
    load->set_parent(bb);
    phi->replace_all_use_with(load);
    instr_list.insert(it, load);
    instr_list.erase(it);
}
```
对于 PHI 指令的所有二元组 `<val, bb>`, 由于此时的中间表示为静态单赋值形式，
`val`只被赋值一次，在 val 所属基本块的末尾加入 store 指令即可。
```cpp
for (auto &[phi, alloca] : phi2alloca) {
    for (auto i = 0; i < phi->get_num_operand(); i += 2) {
        auto val = phi->get_operand(i);
        auto bb = dynamic_cast<BasicBlock *>(phi->get_operand(i + 1));
        auto bb_term = bb->get_terminator();
        bb->delete_instr(bb_term);
        auto store = StoreInst::create_store(val, alloca, bb);
        bb->add_instruction(bb_term);
    }
}
```
## 寄存器分配
寄存器分配采用线性扫描算法，针对risc-v架构做了特定修改。
### 寄存器 Reg 类
```cpp
class Reg {
private:
    bool f, d;
    int id;
}
```
id表示寄存器编号，从0到31.
由于加入了单精度浮点数的拓展，需要有一个 flag `f`表示是否为浮点数寄存器；

而 flag `d`表示该寄存器实例是否为64位（地址为64位，使用访问栈中内存使用`sd/ld`而不是`sw/lw`）

为方便检测该寄存器是否合法（不合法时 id 为 -1，且构造函数不传递参数 `id = -1`），加入一个 operator bool() 函数
```cpp
explicit operator bool() const { return id != -1; }
```
### Interval 构造
```cpp
struct interval {
    point start, end; // both inclusive
    Value *val;
};
```
interval为一段闭区间，在线性扫描过程中存储在active和intervals两个`std::set`中，分别以*increasing end point*和*increasing start point*进行排序（线性扫描算法需要）；其中，基本块的顺序为深度优先序。
```cpp
void RegAlloc::dfs(BasicBlock *s) {
    visited[s] = true;
    for (auto suc : s->get_succ_basic_blocks()) {
        if (!visited[suc]) dfs(suc);
    }
    int2bb[c] = s;
    bb2int[s] = c--;
}
...
c = f_->get_num_basic_blocks();
dfs(f_->get_entry_block());
```
### 线性扫描

线性扫描寄存器分配参考该算法经典论文，分别实现`LinearScanRegisterAllocation`, `ExpireOldIntervals`, `SpillAtInterval` 几个函数。

而为了考虑固定分配在`a0`, `a1`等在寄存器上传递的函数参数，需要在线性扫描开始前进行预分配。
```cpp
for (auto arg : f_->get_args()) {
    bool used = !arg->get_use_list().empty();
    if (!arg->get_type()->is_float_type()) {
        if (used) { // 有使用的参数才分配，节省寄存器
            available_regs.erase(
                reg_mappings[arg] = args[int_arg_counter]);
            if (arg->get_type()->is_pointer_type()) reg_mappings[arg].d = true; // 如果是指针，double word
        }
        int_arg_counter++;
    } else {
      ...
    }
}
```
并在整个扫描的流程中考虑该interval是否已分配过寄存器
```cpp
// for (auto i : intervals)
  if (pre_allocated(i)) continue;
```
## RISCV64 代码生成
构造一个 Codegen 类，传入 Module 指针，以下介绍各个后端代码生成的函数
### 函数 prologue
存储`sp`，并扫描整个函数是否有函数调用，有的话存储`ra`寄存器
### 函数 epilogue
恢复现场
### 寄存器赋值
将Value `v`赋值给寄存器`dst`，分别考虑v为 `ConstantInt`, `ConstantFP`, gep指针, 有对应寄存器的变量，以及栈上变量。
```cpp
void Codegen::assign(Reg dst, Value *v)
```
#### 整形常数
```cpp
ss << ig.addi(dst, zero, i->get_value());
```
#### 单精度浮点数
参考 gcc, 浮点数不能直接用立即数表示，需要创建一块`Local Constant`(LC)区域存储浮点数的值。
```cpp
local_floats.push_back(f->get_value());
Reg temp = get_temp();
ss << ig.lla(temp, ".LC" + std::to_string(local_floats.size() - 1));
ss << ig.flw(dst, 0, temp);
```
在汇编代码末尾处理所有的局部常量
```cpp
void Codegen::gen_local_constants() {
    for (auto i = 0; i < local_floats.size(); i++) {
        ss << ".LC" << i << ":\n";
        ss << "\t.float " << local_floats[i] << "\n";
    }
}
```
#### 指针
指针在llvm IR中分为 alloca 指令，全局变量，gep指令，gep可以包含这三种指令。

1. alloca. 参考gcc, 基地址为`s0`寄存器，偏移量在寄存器分配时计算，存储在`stack_mapping`中
2. 全局变量. 使用 `la` 指令获取该变量地址即可
```cpp
ss << ig.la(addr_temp, global_ptr->get_name());
```
3. gep. 由于`cminusf`只实现了一维数组，gep的有效索引只有一个，取出索引时`get`最后一个操作数即可。对于常量index和变量index，有不同的处理方式。
   1. index为常整数。偏移量为index乘4，加上栈中地址即可；基地址仍为`s0`。或者`la`全局变量，偏移量为index乘4
   2. index 为变量时，基地址为offset * 4+base_reg，偏移量为0
#### 分配在寄存器上的变量
由于存在函数调用等等可能会覆盖原先寄存器值的操作，需要将寄存器存储在栈中，更确切地说，是存储`caller-saved register`, 即a0到a7和t0到t6，如果没有函数调用，直接在两个寄存器之间`mv`即可，但是在函数调用后，要从内存中加载
```cpp
if (fresh[src_reg])
    ss << ig.mv(dst, src_reg);
else
    ss << ig.lw(dst, s0_offset[src_reg], s0);
```
#### SPILL 的变量
由于时间有限，只考虑了所有的变量都分配在寄存器上的情况，对于spill掉的变量全部 `throw std::runtime_error`
### 算术指令
算数指令实现较为简单，llvm IR的指令直接对应到RISC-v汇编即可。
### 比较指令
对于6种比较大小的指令，分别对应翻译即可，例如`a == b`翻译为
```cpp
ss << ig.sub(dest_reg, dest_reg, rhs_reg);
ss << ig.seqz(dest_reg, dest_reg);
```
浮点数比较同理
### 分支指令
直接使用比较的结果（故没有使用blt,bge等等效率更高的指令）
```cpp
ss << ig.bnez(reg_mapping[cond], bbmap[trueBB]);
ss << ig.j(bbmap[falseBB]);
```

### int-float 类型转换
#### int to float
```cpp
ss << ig.fcvtws(dest_reg, freg);
ss << ig.sextw(dest_reg, dest_reg);
```
#### float to int
```cpp
ss << ig.fcvtsw(dest_reg, ireg);
```
## 测试
### 环境
安装[risc-v工具链](https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2021.12.22)
下载`riscv64-elf-ubuntu-xx`解压至（例如 */opt/riscv*），或者自行编译也可;
并设置环境变量(*/opt/riscv/bin*)，以及`qemu-riscv64`. (以下两个binary用apt似乎也可以安装)
### 自动测试
在cminusfc中生成汇编文件，交叉编译
```cpp
auto output_asm = target_path + ".s";
output_stream.open(output_asm, std::ios::out);
output_stream << cg.gen_module();

auto rv64_command = "riscv64-unknown-elf-gcc " + output_asm + " " + CMAKE_LIBRARY_OUTPUT_DIRECTORY + "/io.o -o " + target_path;
```
并修改脚本
```
COMMAND = ['qemu-riscv64', TEST_PATH]
```
运行
```bash
python eval.py
```
即可

### eval_result
```
total points: 94
```
## Future Works
修改实现错误（循环实现有误），完善栈上变量的操作。使用s类寄存器减少内存访问。
## 参考文献
1. Massimiliano Poletto and Vivek Sarkar. 1999. Linear scan register allocation. <i>ACM Trans. Program. Lang. Syst.</i> 21, 5 (Sept. 1999), 895–913. DOI: https://doi.org/10.1145/330249.330250
2. RISC-V BYTES: CROSS-PLATFORM DEBUGGING WITH QEMU AND GDB, url: https://danielmangum.com/posts/risc-v-bytes-qemu-gdb/
3. “The RISC-V Instruction Set Manual, Volume I: User-Level ISA, Document Version 20191213”, Editors Andrew Waterman and Krste Asanovi´c, RISC-V Foundation, December 2019
4. riscv64-unknown-elf-gcc 生成的汇编代码
5. 2020 年全国大学生计算机系统能力大赛编译系统设计赛项目，[燃烧我的编译器](https://github.com/mlzeng/CSC2020-USTC-FlammingMyCompiler)
