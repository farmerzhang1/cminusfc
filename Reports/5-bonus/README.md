# 选做实验-赛道一 报告
- [选做实验-赛道一 报告](#选做实验-赛道一-报告)
  - [Abstract](#abstract)
  - [PHI 指令消除](#phi-指令消除)
  - [寄存器分配](#寄存器分配)
    - [寄存器 Reg 类](#寄存器-reg-类)
    - [Interval 构造](#interval-构造)
    - [线性扫描](#线性扫描)
  - [RISCV64 代码生成](#riscv64-代码生成)
    - [寄存器赋值](#寄存器赋值)
      - [整形常数](#整形常数)
      - [单精度浮点数](#单精度浮点数)
      - [指针](#指针)
  - [测试](#测试)
    - [环境](#环境)
    - [自动测试](#自动测试)
    - [eval_result](#eval_result)
  - [Future Works](#future-works)
  - [参考文献](#参考文献)
## Abstract
本次实验为 cminusf 语言构建了一个 RISC-V 64位后端，支持所有 cminusf 的语言特性，并通过了 lab3 中的所有测试。
TODO!!
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
TODO

似乎这里的实现，或者是消除phi有点问题，再改改

寄存器分配采用线性扫描算法，针对risc-v架构做了特定修改。
### 寄存器 Reg 类
TODO
### Interval 构造
TODO
### 线性扫描
TODO
## RISCV64 代码生成
TODO
构造一个 Codegen 类，传入 Module 指针，以下介绍各个后端代码生成的函数
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
TODO

## 测试
### 环境
安装[risc-v工具链](https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2021.12.22)下载`riscv64-elf-ubuntu-xx`解压（例如 */opt/riscv*），或者自行编译也可并设置环境变量(*/opt/riscv/bin*)，以及`qemu-riscv64`. (以下两个binary用apt似乎也可以安装)
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
a lot
## 参考文献
1. linear scan 那篇
2. rv64 gdb和qemu的联合debug
3. riscv-spec
4. 2020 年全国大学生计算机系统能力大赛编译系统设计赛项目，[燃烧我的编译器](https://github.com/mlzeng/CSC2020-USTC-FlammingMyCompiler)