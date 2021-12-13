#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <iostream>

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute_i(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2) {
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);
        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantFP *ConstFolder::compute_f(
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2) {
    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op) {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);
        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get(c_value1 / c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::f_to_i(ConstantFP *value) {
    int ans = (int)value->get_value();
    return ConstantInt::get(ans, module_);
}

ConstantFP *ConstFolder::i_to_f(ConstantInt *value) {
    float ans = (float)value->get_value();
    return ConstantFP::get(ans, module_);
}

ConstantInt *ConstFolder::compare(
    CmpInst::CmpOp op,
    ConstantInt *value1,
    ConstantInt *value2) {
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
    case CmpInst::EQ:
        return ConstantInt::get((c_value1 == c_value2), module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get((c_value1 != c_value2), module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get((c_value1 > c_value2), module_);
        break;
    case CmpInst::GE:
        return ConstantInt::get((c_value1 >= c_value2), module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get((c_value1 < c_value2), module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get((c_value1 <= c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::compare(
    FCmpInst::CmpOp op,
    ConstantFP *value1,
    ConstantFP *value2) {
    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op) {
    case FCmpInst::EQ:
        return ConstantInt::get((c_value1 == c_value2), module_);
        break;
    case FCmpInst::NE:
        return ConstantInt::get((c_value1 != c_value2), module_);
        break;
    case FCmpInst::GT:
        return ConstantInt::get((c_value1 > c_value2), module_);
        break;
    case FCmpInst::GE:
        return ConstantInt::get((c_value1 >= c_value2), module_);
        break;
    case FCmpInst::LT:
        return ConstantInt::get((c_value1 < c_value2), module_);
        break;
    case FCmpInst::LE:
        return ConstantInt::get((c_value1 <= c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value) {
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr) {
        return constant_fp_ptr;
    } else {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value) {
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr) {
        return constant_int_ptr;
    } else {
        return nullptr;
    }
}

void ConstPropagation::run() {
    ConstFolder cf(this->m_);
    std::vector<Instruction *> delete_list;
    for (auto f : this->m_->get_functions()) {
        for (auto bb : f->get_basic_blocks()) {
            for (auto instr : bb->get_instructions()) {
                if (instr->get_instr_type() == Instruction::OpID::add || instr->get_instr_type() == Instruction::OpID::sub || instr->get_instr_type() == Instruction::OpID::mul || instr->get_instr_type() == Instruction::OpID::sdiv) {
                    auto instr_type = dynamic_cast<BinaryInst *>(instr);
                    if (instr_type) {
                        auto i_lhs = cast_constantint(instr_type->get_operand(0));
                        auto i_rhs = cast_constantint(instr_type->get_operand(1));
                        if (i_lhs && i_rhs) {
                            // std::cout << iadd->print() << std::endl;
                            instr_type->replace_all_use_with(cf.compute_i(instr_type->get_instr_type(), i_lhs, i_rhs)); // let's see what this does
                            // bb->delete_instr(iadd);
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::fadd || instr->get_instr_type() == Instruction::OpID::fsub || instr->get_instr_type() == Instruction::OpID::fmul || instr->get_instr_type() == Instruction::OpID::fdiv) {
                    auto instr_type = dynamic_cast<BinaryInst *>(instr);
                    if (instr_type) {
                        auto f_lhs = cast_constantfp(instr_type->get_operand(0));
                        auto f_rhs = cast_constantfp(instr_type->get_operand(1));
                        if (f_lhs && f_rhs) {
                            instr_type->replace_all_use_with(cf.compute_f(instr_type->get_instr_type(), f_lhs, f_rhs));
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::fptosi) {
                    auto instr_type = dynamic_cast<FpToSiInst *>(instr);
                    if (instr_type) {
                        auto fhs = cast_constantfp(instr_type->get_operand(0));
                        if (fhs) {
                            instr_type->replace_all_use_with(cf.f_to_i(fhs));
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::sitofp) {
                    auto instr_type = dynamic_cast<SiToFpInst *>(instr);
                    if (instr_type) {
                        auto ihs = cast_constantint(instr_type->get_operand(0));
                        if (ihs) {
                            instr_type->replace_all_use_with(cf.i_to_f(ihs));
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::cmp) {
                    auto instr_type = dynamic_cast<CmpInst *>(instr);
                    if (instr_type) {
                        auto i_lhs = cast_constantint(instr_type->get_operand(0));
                        auto i_rhs = cast_constantint(instr_type->get_operand(1));
                        if (i_lhs && i_rhs) {
                            instr_type->replace_all_use_with(cf.compare(instr_type->get_cmp_op(), i_lhs, i_rhs));
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::fcmp) {
                    auto instr_type = dynamic_cast<FCmpInst *>(instr);
                    if (instr_type) {
                        auto f_lhs = cast_constantfp(instr_type->get_operand(0));
                        auto f_rhs = cast_constantfp(instr_type->get_operand(1));
                        if (f_lhs && f_rhs) {
                            instr_type->replace_all_use_with(cf.compare(instr_type->get_cmp_op(), f_lhs, f_rhs));
                            delete_list.push_back(instr_type);
                        }
                    }
                } else if (instr->get_instr_type() == Instruction::OpID::call) {
                    auto instr_type = dynamic_cast<CallInst *>(instr);
                } else if (instr->get_instr_type() == Instruction::OpID::store) {
                    auto instr_type = dynamic_cast<StoreInst *>(instr);
                    cf.map_[dynamic_cast<GlobalVariable *>(instr_type->get_lval())] = instr_type->get_rval();
                } else if (instr->get_instr_type() == Instruction::OpID::load) {
                    auto instr_type = dynamic_cast<LoadInst *>(instr);
                    auto iter = cf.map_.find(dynamic_cast<GlobalVariable *>(instr_type->get_lval()));
                    if (iter != cf.map_.end()) {
                        instr_type->replace_all_use_with(iter->second);
                        delete_list.push_back(instr_type);
                    }
                }
                // if (instr->get_use_list().empty())
                // delete_list.push_back(instr);
            }
            for (auto instr : delete_list)
                bb->delete_instr(instr);
            cf.map_.clear();
        }
    }
}
