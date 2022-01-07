# 选做实验-赛道一 报告
- [选做实验-赛道一 报告](#选做实验-赛道一-报告)
  - [Abstract](#abstract)
  - [PHI 指令消除](#phi-指令消除)
  - [寄存器分配](#寄存器分配)
    - [Interval 构造](#interval-构造)
    - [线性扫描](#线性扫描)
  - [RISCV64 代码生成](#riscv64-代码生成)
  - [参考文献](#参考文献)
## Abstract
本次实验为 cminusf 语言构建了一个 RISC-V 64位后端，支持所有 cminusf 的语言特性，并通过了 lab3 中的所有测试。

## PHI 指令消除
phi elimination 采用最直观的消除法，并无优化。

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
## 寄存器分配
TODO

寄存器分配采用线性扫描算法，针对risc-v架构做了特定修改。
### Interval 构造
TODO
### 线性扫描

## RISCV64 代码生成
TODO
## 参考文献
1. linear scan 那篇
2. rv64 gdb和qemu的联合debug
3. riscv-spec
