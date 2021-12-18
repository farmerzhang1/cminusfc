#include <map>
#include <sstream>
#include <iomanip>

#include "Module.h"
#include "Function.h"
#include "codegen.h"
#include "reg.h"
#include "regalloc.h"
#include "instruction.h"
// 深度好文！ just kidding
// https://www.cs.sfu.ca/~ashriram/Courses/CS295/assets/notebooks/RISCV/RISCV_CALL.pdf
using namespace std::literals::string_literals;

Codegen::Codegen(Module *m) :
    m(m), ig(), sp("sp"s), s0("s0"s), ra("ra"s)
{
    RegAlloc ra(m);
    ra.run();
    // this is what a friend looks like: move your things!
    stack_mapping = std::move(ra.location);
    reg_mapping = std::move(ra.maps);
}

std::string Codegen::gen_module() {
    ss << "\t.text" << std::endl;
    ss << "\t.file\t" << std::quoted(m->get_filename()) << std::endl;
    for (auto f : m->get_functions()) gen_function(this->f_ = f);
    return ss.str();
}

void Codegen::gen_function(Function *f) {
    if (f->get_basic_blocks().empty()) return;
    allocate_stack();
    ss << "\t.globl\t" << f->get_name() << std::endl;
    ss << "\t.type\t" << f->get_name() << ", @function" << std::endl;
    ss << f->get_name() << ":" << std::endl;
    fun_prologue(f);
    for (auto bb : f->get_basic_blocks()) gen_bb(bb);
}

void Codegen::gen_bb(BasicBlock *bb) {
    for (auto instr : bb->get_instructions()) {
        switch (instr->get_instr_type()) {
        case Instruction::OpID::ret: fun_epilogue(instr->get_function()); break;
        default: throw std::runtime_error(instr->get_instr_op_name() + " not implemented!");
        }
    }
}

void Codegen::fun_prologue(Function *f) {
    comment("fun_prologue");
    ss << ig.addi(sp, sp, -stack_size) << std::endl;
    if (f->has_fcalls()) {
        ss << ig.sd(ra, stack_size - 8, sp) << std::endl;
        ss << ig.sd(s0, stack_size - 16, sp) << std::endl;
    } else {
        ss << ig.sd(s0, stack_size - 8, sp) << std::endl;
    }
    ss << ig.addi(s0, sp, stack_size) << std::endl;
}

void Codegen::fun_epilogue(Function *f) {
    comment("fun_epilogue");
    if (f->has_fcalls()) {
        ss << ig.ld(ra, stack_size - 8, sp) << std::endl;
        ss << ig.ld(s0, stack_size - 16, sp) << std::endl;
    } else {
        ss << ig.ld(s0, stack_size - 8, sp) << std::endl;
    }
    ss << ig.addi(sp, sp, stack_size) << std::endl;
    ss << ig.jr(ra) << std::endl;
}

void Codegen::allocate_stack() {
    int max {0};
    stack_size = f_->has_fcalls() ? 16 : 8;
    for (auto &[_, v] : stack_mapping) {
        max = std::max(max, v);
        v += stack_size;
    }
    stack_size += max;
}

void Codegen::comment(std::string s) {
    ss << s << "\n";
}