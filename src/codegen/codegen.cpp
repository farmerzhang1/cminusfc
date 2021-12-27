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
        case Instruction::OpID::cmp:
            icmp(dynamic_cast<CmpInst *>(instr), instr->get_operand(0), instr->get_operand(1));
            break;
        case Instruction::OpID::fcmp:
            fcmp(dynamic_cast<FCmpInst *>(instr), instr->get_operand(0), instr->get_operand(1));
            break;
        case Instruction::OpID::zext:
            sext(instr, instr->get_operand(0));
            break;
        case Instruction::OpID::br:
            branch(dynamic_cast<BranchInst *>(instr));
            break;
        case Instruction::OpID::getelementptr:
            gep(dynamic_cast<GetElementPtrInst *>(instr));
            break;
        case Instruction::OpID::store:
            store(instr, instr->get_operand(0));
        default: throw std::runtime_error(instr->get_instr_op_name() + " not implemented!");
        }
    }
}

void Codegen::store(Value *dest, Value *src) {
    if (stack_mapping.contains(dest)) {
        auto src_reg = reg_mapping.contains(src) ? reg_mapping[src] : get_temp();
        ss << ig.sw(src_reg, stack_mapping[dest], s0);
    } else {
        throw std::runtime_error("");
    }
}

void Codegen::gep(GetElementPtrInst *dest) {
    // 要优美！ ()
    auto index = dest->get_operand(dest->get_num_operand() - 1);
    auto const_int = dynamic_cast<ConstantInt *>(index);
    if (const_int) return; // 如果index是常数，在store中处理(ld/sd的base寄存器为s0)
    auto ptr = dest->get_operand(0);
    if (reg_mapping.contains(ptr)) {
        // probably allocated in arguments ?
    } else if (stack_mapping.contains(ptr)) {
        // if we use pointers (e.g. int*****), this does not work
        if (reg_mapping.contains(dest)) {
            throw std::runtime_error("variable index TODO");
        } else {
            throw std::runtime_error("stack mapped dest in gep");
        }
    }
}

void Codegen::branch(BranchInst *branch) {
    if (branch->is_cond_br()) {
        auto cond = branch->get_operand(0);
        auto trueBB = branch->get_operand(1);
        auto falseBB = branch->get_operand(2);
        if (reg_mapping.contains(cond)) {
            ss << ig.bnez(reg_mapping[cond], trueBB->get_name());
            ss << ig.j(falseBB->get_name());
        } else
            throw std::runtime_error("branch cond stack");
    } else {
        auto unconditional = branch->get_operand(0);
        ss << ig.j(unconditional->get_name());
    }
}

void Codegen::sext(Value *dest, Value *src) {
    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        free_regs.erase(dest_reg);
        assign(dest_reg, src);
        ss << ig.andi(dest_reg, dest_reg, 0xff);
        ss << ig.sextw(dest_reg, dest_reg);
        fresh[dest_reg] = true;
    } else {
        throw std::runtime_error("sext stack");
    }
}

void Codegen::icmp(CmpInst *dest, Value *lhs, Value *rhs) {
    auto op = dest->get_cmp_op();
    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        free_regs.erase(dest_reg);
        assign(dest_reg, lhs);
        auto rhs_reg = reg_mapping.contains(rhs) ? reg_mapping[rhs] : get_temp();
        assign(rhs_reg, rhs);
        switch (op) {
        case CmpInst::CmpOp::EQ:
            ss << ig.sub(dest_reg, dest_reg, rhs_reg);
            ss << ig.seqz(dest_reg, dest_reg);
            break;
        case CmpInst::CmpOp::GE:
            ss << ig.slt(dest_reg, dest_reg, rhs_reg);
            ss << ig.xori(dest_reg, dest_reg, 1);
            break;
        case CmpInst::CmpOp::GT:
            ss << ig.sgt(dest_reg, dest_reg, rhs_reg);
            break;
        case CmpInst::CmpOp::LE:
            ss << ig.sgt(dest_reg, dest_reg, rhs_reg);
            ss << ig.xori(dest_reg, dest_reg, 1);
            break;
        case CmpInst::CmpOp::LT:
            ss << ig.slt(dest_reg, dest_reg, rhs_reg);
            break;
        case CmpInst::CmpOp::NE:
            ss << ig.sub(dest_reg, dest_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        }
        fresh[dest_reg] = true;
    } else
        throw std::runtime_error("stack mapping icmp");
}

void Codegen::fcmp(FCmpInst *dest, Value *lhs, Value *rhs) {
    auto op = dest->get_cmp_op();
    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        free_regs.erase(dest_reg);
        auto lhs_reg = reg_mapping.contains(lhs) ? reg_mapping[lhs] : get_temp(true);
        assign(lhs_reg, lhs);
        free_regs.erase(lhs_reg);
        auto rhs_reg = reg_mapping.contains(rhs) ? reg_mapping[rhs] : get_temp(true);
        assign(rhs_reg, rhs);
        free_regs.insert(lhs_reg);
        switch (op) {
        case FCmpInst::CmpOp::EQ:
            ss << ig.feqs(dest_reg, lhs_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        case FCmpInst::CmpOp::GE:
            ss << ig.fges(dest_reg, lhs_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        case FCmpInst::CmpOp::GT:
            ss << ig.fgts(dest_reg, lhs_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        case FCmpInst::CmpOp::LE:
            ss << ig.fles(dest_reg, lhs_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        case FCmpInst::CmpOp::LT:
            ss << ig.flts(dest_reg, lhs_reg, rhs_reg);
            ss << ig.snez(dest_reg, dest_reg);
            break;
        case FCmpInst::CmpOp::NE:
            ss << ig.feqs(dest_reg, lhs_reg, rhs_reg);
            ss << ig.seqz(dest_reg, dest_reg);
            break;
        }
        fresh[dest_reg] = true;
    } else
        throw std::runtime_error("fcmp stack");
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
        fresh[dest_reg] = true;
    } else {
        // TODO stack
        throw std::runtime_error("bin imm inst: stack");
    }
}

void Codegen::bin_inst(BinaryInst *dest, Instruction::OpID op, bool f) {
    auto lhs_val = dest->get_operand(0);
    auto rhs_val = dest->get_operand(1);
    auto lhs_const = dynamic_cast<ConstantInt *>(lhs_val);
    auto rhs_const = dynamic_cast<ConstantInt *>(rhs_val);
    // if one is constant, use immediate
    if (!f && (op == Instruction::OpID::add || op == Instruction::OpID::sub)) { // can't be float
        if (rhs_const && rhs_const->get_value() >= IMM_MIN && rhs_const->get_value() <= IMM_MAX) {
            bin_inst_imm(dest, op, lhs_val, rhs_const, f);
            return;
        } else if (lhs_const && lhs_const->get_value() >= IMM_MIN && lhs_const->get_value() <= IMM_MAX && // 减法(non-abelian!?*^&(O&))不可
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
            fresh[dest_reg] = true;
        } else if (stack_mapping.contains(dest)) {
            throw std::runtime_error("bin inst integer: stack");
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
            fresh[dest_reg] = true;
        } else if (stack_mapping.contains(dest)) {
            throw std::runtime_error("bin inst float: stack");
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
    // TODO: check StackGuard
    // (url: https://www.usenix.org/legacy/publications/library/proceedings/sec98/full_papers/cowan/cowan.pdf)
    s0_offset.clear();
    stack_size = regalloc.f_stack_size[f_];
    if (f_->has_fcalls()) {
        for (auto &[v, r] : reg_mapping)
            if (caller_saved_regs.contains(r)) {
                auto size = v->get_type()->get_size() < 4 ? 4 : v->get_type()->get_size();
                stack_size += size;
                s0_offset[r] = -stack_size;
            }
    }
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
            ss << ig.sw(a0, stack_mapping[c], sp); // TODO: change to s0
        else
            // can we add static_assert ? (i guess not)
            throw std::runtime_error("why is this call not (in register or in stack)?");
    }
}

void Codegen::assign(Reg dst, Value *v) {
    auto g = dynamic_cast<GlobalVariable *>(v);
    if (v->get_name().empty()) { // constant (i suppose)
        auto i = dynamic_cast<ConstantInt *>(v);
        auto f = dynamic_cast<ConstantFP *>(v);
        if (i && i->get_value() >= IMM_MIN && i->get_value() <= IMM_MAX)
            ss << ig.addi(dst, zero, i->get_value());
        else if (i) {
            int32_t val = i->get_value();
            int32_t upper12 = val >> 12;
            int32_t imm = val & 0b111111111111;
            if (imm >> 11) {
                upper12++;
                imm = imm - 4096;
            }
            ss << ig.lui(dst, upper12);
            ss << ig.addi(dst, dst, imm);
        } else if (f) {
            local_floats.push_back(f->get_value());
            Reg temp = get_temp();
            ss << ig.lla(temp, ".LC" + std::to_string(local_floats.size() - 1));
            ss << ig.flw(dst, 0, temp);
        }
    } else if (reg_mapping.contains(v)) {
        auto src_reg = reg_mapping[v];
        if (dst.f) {
            if (fresh[src_reg])
                ss << ig.fmvs(dst, src_reg);
            else
                ss << ig.flw(dst, s0_offset[src_reg], s0);
        } else {
            if (fresh[src_reg])
                ss << ig.mv(dst, src_reg);
            else
                ss << ig.lw(dst, s0_offset[src_reg], s0);
        }
    } else if (stack_mapping.contains(v)) {
        throw std::runtime_error("stack assign value");
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
        throw std::runtime_error("fptosi stack");
    }

    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        ss << ig.fcvtws(dest_reg, freg);
        fresh[dest_reg] = true;
        free_regs.erase(dest_reg);
    } else if (stack_mapping.contains(dest)) {
        throw std::runtime_error("fptosi stack");
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
        throw std::runtime_error("sitofp stack");
    }
    if (reg_mapping.contains(dest)) {
        auto dest_reg = reg_mapping[dest];
        ss << ig.fcvtsw(dest_reg, ireg);
        fresh[dest_reg] = true;
        free_regs.erase(dest_reg);
    } else if (stack_mapping.contains(dest)) {
        throw std::runtime_error("sitofp stack");
    } else
        throw std::runtime_error("not in stack and not in reg");
}
