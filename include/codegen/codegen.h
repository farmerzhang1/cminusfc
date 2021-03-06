#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <map>
#include <sstream>
#include <string>
#include <set>
#include <array>

#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"
#include "reg.h"
#include "instruction.h"
#include "regalloc.h"
#include "phi_elim.h"

using namespace std::literals::string_literals;

class Codegen {
private:
    const size_t XLEN = 64;
    const size_t IMM_LEN = 12;
    const int IMM_MAX = (1 << 11) - 1;  // 0b011111111111;
    const int IMM_MIN = -(1 << 11);     // 0b100000000000;
    Module *m;    // top
    Function *f_; // current function
    PhiElim pe;
    RegAlloc regalloc;
    std::stringstream ss;
    Instgen ig;
    std::map<Value *, Reg> reg_mapping;
    std::map<Value *, int> stack_mapping;
    std::map<BasicBlock *, std::string> bbmap;
    std::map<Reg, int> s0_offset; // offset(s0), offset is negative (cr. riscv64-linux-gnu-gcc -S)
    std::map<Instruction*, int> alloca_offset;
    // TODO: so much?
    const std::set<Reg> caller_saved_regs {Reg(1), Reg(5), Reg(6), Reg(7), Reg(10), Reg(11), Reg(12), Reg(13), Reg(14), Reg(15), Reg(16), Reg(17), Reg(28), Reg(29), Reg(30), Reg(31),
        // TODO: add full f caller saved regs
        Reg(1, true), Reg(5, true), Reg(6, true), Reg(7, true),
        Reg(10, true), Reg(11, true), Reg(12, true), Reg(13, true), Reg(14, true), Reg(15, true), Reg(16, true), Reg(17, true), Reg(28, true), Reg(29, true), Reg(30, true), Reg(31, true)};
    const std::set<Reg> callee_saved_regs {Reg(2)};
    std::set<Reg> free_regs; // 对于 free registers，生成riscv64代码的过程中才会逐渐减少 ()
    std::map<Reg, bool> fresh;
    size_t fcounter{0}, bbcounter{0};
    std::vector<float> local_floats;
    int stack_size;
    const Reg sp, s0, ra, zero, a0, fa0;

public:
    Reg get_temp(bool f = false, bool d = false);
    Codegen(Module *m);
    std::string gen_module();
    void gen_function(Function *);
    void gen_bb(BasicBlock *);
    void fun_prologue(Function *);
    void fun_epilogue(Function *, ReturnInst*);
    void comment(std::string);
    void allocate_stack();
    void call(CallInst *);
    void assign(Reg, Value *);
    void gen_local_constants();
    void push_caller_saved_regs();
    void bin_inst(BinaryInst*, Instruction::OpID, bool f = false);
    void bin_inst_imm(BinaryInst*, Instruction::OpID, Value*, Constant*, bool f = false);
    void fptosi(Value*, Value*);
    void sitofp(Value*, Value*);
    void icmp(CmpInst*, Value*, Value*);
    void fcmp(FCmpInst*, Value*, Value*);
    void sext(Value*, Value*);
    void branch(BranchInst*);
    void gep(GetElementPtrInst*);
    void store(Value*, Value*);
    void load(LoadInst*, Value*);
    void save(Reg);
};
#endif
