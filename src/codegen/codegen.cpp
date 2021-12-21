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
    m(m), ig(), sp("sp"s), s0("s0"s), ra("ra"s), zero(0), regalloc(m) {
    regalloc.run();
    // this is what a friend looks like: move your things!
    // stack_mapping = std::move(ra.stack_mappings);
    // reg_mapping = std::move(ra.reg_mappings);
}

std::string Codegen::gen_module() {
    ss << "\t.text" << std::endl;
    ss << "\t.file\t" << std::quoted(m->get_filename()) << std::endl;
    for (auto f : m->get_functions()) gen_function(this->f_ = f);
    generate_local_constants();
    return ss.str();
}

void Codegen::gen_function(Function *f) {
    if (f->get_basic_blocks().empty()) return;
    reg_mapping = std::move(regalloc.f_reg_map[f]);
    stack_mapping = std::move(regalloc.f_stack_map[f]);
    allocate_stack();
    ss << "\t.globl\t" << f->get_name() << std::endl;
    ss << "\t.type\t" << f->get_name() << ", @function" << std::endl;
    ss << f->get_name() << ":" << std::endl;
    fun_prologue(f);
    for (auto bb : f->get_basic_blocks()) gen_bb(bb);
}

void Codegen::gen_bb(BasicBlock *bb) {
    for (auto instr : bb->get_instructions()) {
        comment(instr->print());
        switch (instr->get_instr_type()) {
        case Instruction::OpID::ret: fun_epilogue(instr->get_function()); break;
        case Instruction::OpID::call: call(static_cast<CallInst *>(instr)); break;
        default: throw std::runtime_error(instr->get_instr_op_name() + " not implemented!");
        }
    }
}

void Codegen::fun_prologue(Function *f) {
    comment("fun_prologue");
    ss << ig.addi(sp, sp, -stack_size);
    if (f->has_fcalls()) {
        ss << ig.sd(ra, stack_size - 8, sp);
        ss << ig.sd(s0, stack_size - 16, sp);
    } else {
        ss << ig.sd(s0, stack_size - 8, sp);
    }
    ss << ig.addi(s0, sp, stack_size);
    if (f->has_fcalls())
        push_caller_saved_regs();
}

void Codegen::fun_epilogue(Function *f) {
    comment("fun_epilogue");
    if (f->has_fcalls()) {
        ss << ig.ld(ra, stack_size - 8, sp);
        ss << ig.ld(s0, stack_size - 16, sp);
    } else {
        ss << ig.ld(s0, stack_size - 8, sp);
    }
    ss << ig.addi(sp, sp, stack_size);
    ss << ig.jr(ra);
}

void Codegen::allocate_stack() {
    int deepest{0};
    s0_offset.clear();
    stack_size = f_->has_fcalls() ? 16 : 8;
    // TODO: calculate stack size here, not in register allocation phase
    for (auto &[_, v] : stack_mapping) {
        deepest = std::max(deepest, v);
        v += stack_size;
    }
    if (f_->has_fcalls()) {
        int current_offset = -20;
        for (auto &[v, r] : reg_mapping)
            if (caller_saved_regs.contains(r)) {
                s0_offset[r] = current_offset;
                current_offset -= v->get_type()->get_size();
                stack_size += v->get_type()->get_size();
            }
    }
    stack_size += deepest;
}

void Codegen::comment(std::string s) {
    ss << "#" << s << "\n";
}

void Codegen::call(CallInst *c) {
    int int_arg_counter{0}, fl_arg_counter{0};
    for (auto i = 1; i < c->get_num_operand(); i++) {
        auto arg = c->get_operand(i);
        if (arg->get_type()->is_integer_type()) {
            assign(args[int_arg_counter++], arg);
        } else if (arg->get_type()->is_float_type()) {
            assign(fargs[fl_arg_counter++], arg);
        }
    }
    ss << ig.call(c->get_operand(0)->get_name());
}

void Codegen::assign(Reg r, Value *v) {
    auto g = dynamic_cast<GlobalVariable *>(v);
    if (v->get_name().empty()) { // constant (i suppose)
        auto i = dynamic_cast<ConstantInt *>(v);
        auto f = dynamic_cast<ConstantFP *>(v);
        if (i)
            ss << ig.addi(r, zero, i->get_value());
        else if (f) {
            local_floats.push_back(f->get_value());
            Reg temp = get_temp();
            ss << ig.lla(temp, ".LC" + std::to_string(local_floats.size() - 1));
            ss << ig.flw(r, 0, temp);
        }
    } else {
        // TODO
        // throw(std::runtime_error("register allocation is not done!"));
        assert(reg_mapping.contains(v));
        if (r.f)
            ss << ig.flw(r, s0_offset[reg_mapping[v]], s0);
        else
            ss << ig.lw(r, s0_offset[reg_mapping[v]], s0);
    }
}

void Codegen::generate_local_constants() {
    for (auto i = 0; i < local_floats.size(); i++) {
        ss << ".LC" << i << ":\n";
        ss << "\t.float " << local_floats[i] << "\n";
    }
}

void Codegen::push_caller_saved_regs() {
    comment("saving caller-saved registers");
    for (auto &[caller_reg, offset] : s0_offset)
        if (caller_reg.f)
            ss << ig.fsw(caller_reg, offset, s0);
        else
            ss << ig.sw(caller_reg, offset, s0);
}

Reg Codegen::get_temp() {
    // FIXME: 加一个 free regs 的set, or?
    for (auto r : args) {
        bool found = true;
        for (auto [k, v] : reg_mapping)
            if (v == r) found = false;
        if (found) return r;
    }
    return Reg(-1);
}
