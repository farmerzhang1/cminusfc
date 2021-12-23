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
    m(m), ig(), sp("sp"s), s0("s0"s), ra("ra"s), zero(0), a0("a0"s), regalloc(m) {
    regalloc.run();
    // this is what a friend looks like: move your things!
    // stack_mapping = std::move(ra.stack_mappings);
    // reg_mapping = std::move(ra.reg_mappings);
}

std::string Codegen::gen_module() {
    ss << "\t.text" << std::endl;
    ss << "\t.file\t" << std::quoted(m->get_filename()) << std::endl;
    for (auto f : m->get_functions()) gen_function(this->f_ = f);
    gen_local_constants();
    return ss.str();
}

void Codegen::gen_function(Function *f) {
    if (f->get_basic_blocks().empty()) return;
    reg_mapping = std::move(regalloc.f_reg_map[f]);
    stack_mapping = std::move(regalloc.f_stack_map[f]);
    free_regs.clear();
    for (int i = 0; i < 32; i++) fresh[Reg(i, true)] = fresh[Reg(i)] = true;
    for (auto arg : args) free_regs.insert(arg);
    for (auto arg : fargs) free_regs.insert(arg);
    for (auto temp : temps) free_regs.insert(temp);
    for (auto temp : ftemps) free_regs.insert(temp);
    for (auto arg : f->get_args()) free_regs.erase(reg_mapping[arg]);
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
        case Instruction::OpID::alloca: break; // already allocated in allocate_stack
        case Instruction::OpID::add:
        case Instruction::OpID::sub:
        case Instruction::OpID::mul:
        case Instruction::OpID::sdiv:
            bin_inst(dynamic_cast<BinaryInst *>(instr), instr->get_instr_type());
            break;
        case Instruction::OpID::fadd:
        case Instruction::OpID::fsub:
        case Instruction::OpID::fmul:
        case Instruction::OpID::fdiv:
            bin_inst(dynamic_cast<BinaryInst *>(instr), instr->get_instr_type(), true);
            break;
        case Instruction::OpID::fptosi:
            fptosi(instr, instr->get_operand(0));
            break;
        case Instruction::OpID::sitofp:
            sitofp(instr, instr->get_operand(0));
            break;
        default: throw std::runtime_error(instr->get_instr_op_name() + " not implemented!");
        }
    }
}

void Codegen::bin_inst_imm(BinaryInst *dest, Instruction::OpID op, Value *val, Constant *imm, bool f) {
    comment("immediate");
    auto constint = dynamic_cast<ConstantInt *>(imm);
    int i = constint->get_value();
    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        free_regs.erase(dest_reg);
        assign(dest_reg, val);
        switch (op) {
        case Instruction::OpID::add:
            ss << ig.addi(dest_reg, dest_reg, i);
            break;
        case Instruction::OpID::sub:
            ss << ig.addi(dest_reg, dest_reg, -i);
            break;
        default:
            break;
        }
    } else {
        // TODO stack
    }
}

void Codegen::bin_inst(BinaryInst *dest, Instruction::OpID op, bool f) {
    auto lhs_val = dest->get_operand(0);
    auto rhs_val = dest->get_operand(1);
    auto lhs_const = dynamic_cast<ConstantInt *>(lhs_val);
    auto rhs_const = dynamic_cast<ConstantInt *>(rhs_val);
    // if one is constant, use immediate
    if (!f && (op == Instruction::OpID::add || op == Instruction::OpID::sub)) { // can't be float
        if (rhs_const && rhs_const->get_value() < (1 << 11)) {
            bin_inst_imm(dest, op, lhs_val, rhs_const, f);
            return;
        } else if (lhs_const && lhs_const->get_value() < (1 << 11) && // 减法(non-abelian!?*^&(O&))不可
                   op != Instruction::OpID::sub) {
            bin_inst_imm(dest, op, rhs_val, lhs_const, f);
            return;
        }
    }

    // both lhs and rhs are variables
    auto dest_reg = reg_mapping.at(dest);
    free_regs.erase(dest_reg);
    assign(dest_reg, lhs_val);
    auto rhs_reg = reg_mapping.contains(rhs_val) ? reg_mapping[rhs_val] : get_temp(f);
    assign(rhs_reg, rhs_val);
    if (!f) {
        comment("binary instruction with variables");
        if (reg_mapping.contains(dest)) {
            switch (op) {
            case Instruction::OpID::add:
                ss << ig.add(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::sub:
                ss << ig.sub(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::mul:
                ss << ig.mul(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::sdiv:
                ss << ig.div(dest_reg, dest_reg, rhs_reg);
                break;
            default:
                break;
            }
        } else if (stack_mapping.contains(dest)) {
            // TODO
        }
    } else {
        comment("binary instruction with floats");
        if (reg_mapping.contains(dest)) {
            switch (op) {
            case Instruction::OpID::fadd:
                ss << ig.fadds(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::fsub:
                ss << ig.fsubs(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::fmul:
                ss << ig.fmuls(dest_reg, dest_reg, rhs_reg);
                break;
            case Instruction::OpID::fdiv:
                ss << ig.fdivs(dest_reg, dest_reg, rhs_reg);
                break;
            default:
                break;
            }
        } else if (stack_mapping.contains(dest)) {
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
    // 这写的是真的丑(
    for (auto &[_, v] : stack_mapping) {
        deepest = std::max(deepest, v);
        v += stack_size;
    }
    for (auto bb : f_->get_basic_blocks()) {
        for (auto instr : bb->get_instructions()) {
            if (instr->is_alloca()) {
                // TODO
                auto alloca = dynamic_cast<AllocaInst *>(instr);
                auto s = alloca->get_alloca_type()->get_size();
                auto [_, success] = alloca_offset.insert({instr, s});
                assert(success); // 无聊就加点assert~
                stack_size += s;
            }
        }
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
    ss << "# " << s << "\n";
}

void Codegen::call(CallInst *c) {
    int int_arg_counter{0}, fl_arg_counter{0};
    // prepare for arguments
    for (auto i = 1; i < c->get_num_operand(); i++) {
        auto arg = c->get_operand(i);
        if (arg->get_type()->is_integer_type()) {
            free_regs.erase(args[int_arg_counter]);
            assign(args[int_arg_counter++], arg);
        } else if (arg->get_type()->is_float_type()) {
            free_regs.erase(args[fl_arg_counter]);
            assign(fargs[fl_arg_counter++], arg);
        }
    }
    ss << ig.call(c->get_operand(0)->get_name());
    for (auto r : caller_saved_regs) fresh[r] = false;
    // process return value, if any
    if (!c->get_function_type()->get_return_type()->is_void_type()) {
        if (reg_mapping.contains(c)) {
            auto r = reg_mapping.at(c);
            ss << ig.mv(r, a0);
            ss << ig.sw(a0, s0_offset[r], s0);
            free_regs.erase(r);
            in_use.insert(r);
            fresh[r] = true;
        } else if (stack_mapping.contains(c))
            ss << ig.sw(a0, stack_mapping[c], sp); // TODO: check base register (sp, or s0?)
        else
            // can we add static_assert ? (i guess not)
            throw std::runtime_error("why is this call not (in register or in stack)?");
    }
}

void Codegen::alloca(Instruction *inst) {
    if (reg_mapping.contains(inst)) {
        // ss << ig.ld(reg_mapping[inst], alloca_offset[inst], sp);
    } else if (stack_mapping.contains(inst)) {
        // TODO
        // do nothing because it's already on stack??
    }
}

void Codegen::assign(Reg dst, Value *v) {
    auto g = dynamic_cast<GlobalVariable *>(v);
    if (v->get_name().empty()) { // constant (i suppose)
        auto i = dynamic_cast<ConstantInt *>(v);
        auto f = dynamic_cast<ConstantFP *>(v);
        if (i) // FIXME: 立即数就12(11)位！
            ss << ig.addi(dst, zero, i->get_value());
        else if (f) {
            local_floats.push_back(f->get_value());
            Reg temp = get_temp();
            ss << ig.lla(temp, ".LC" + std::to_string(local_floats.size() - 1));
            ss << ig.flw(dst, 0, temp);
        }
    } else if (reg_mapping.contains(v)) {
        // TODO
        auto src_reg = reg_mapping[v];
        if (dst.f) {
            if (fresh[src_reg])
                ss << ig.fmvs(dst, src_reg); // TODO add a fmv
            else
                ss << ig.flw(dst, s0_offset[src_reg], s0);
        } else {
            if (fresh[src_reg])
                ss << ig.mv(dst, src_reg);
            else
                ss << ig.lw(dst, s0_offset[src_reg], s0);
        }
    } else if (stack_mapping.contains(v)) {
        // TODO
    }
}

void Codegen::gen_local_constants() {
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

Reg Codegen::get_temp(bool f) {
    assert(!free_regs.empty());
    for (auto r : free_regs)
        if (r.f == f)
            return r;
    throw std::runtime_error("no free regs!");
    return Reg(-1);
}

void Codegen::fptosi(Value *dest, Value *fval) {
    auto constf = dynamic_cast<ConstantFP *>(fval);
    Reg freg;
    if (constf) {
        freg = get_temp(true);
        assign(freg, fval);
    } else if (reg_mapping.contains(fval)) {
        freg = reg_mapping[fval];
        // assign(freg, fval);
    } else if (stack_mapping.contains(fval)) {
        // TODO
    }

    if (reg_mapping.contains(dest)) {
        ss << ig.fcvtws(reg_mapping[dest], freg);
    } else if (stack_mapping.contains(dest)) {
        // TODO
    } else {
        throw std::runtime_error("not in stack and not in reg");
    }
}

void Codegen::sitofp(Value *dest, Value *ival) {
    auto consti = dynamic_cast<ConstantInt *>(ival);
    Reg ireg;
    if (consti) {
        ireg = get_temp();
        assign(ireg, ival);
    } else if (reg_mapping.contains(ival)) {
        ireg = reg_mapping[ival];
    } else if (stack_mapping.contains(ival)) {
        // TODO
    }
    if (reg_mapping.contains(dest)) {
        ss << ig.fcvtsw(reg_mapping[dest], ireg);
    } else if (stack_mapping.contains(dest)) {
        // TODO
    } else
        throw std::runtime_error("not in stack and not in reg");
}
