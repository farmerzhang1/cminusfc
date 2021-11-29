#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <iostream>

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)
{
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
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

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}

void ConstPropagation::run()
{
    ConstFolder cf(this->m_);
    std::vector<Instruction *> delete_list;
    for (auto f : this->m_->get_functions())
    {
        for (auto bb : f->get_basic_blocks())
        {
            for (auto instr : bb->get_instructions())
            {
                auto iadd = dynamic_cast<BinaryInst *>(instr);

                if (iadd)
                {
                    auto lval = cast_constantint(iadd->get_operand(0));
                    auto rval = cast_constantint(iadd->get_operand(1));
                    if (lval && rval)
                    {
                        // std::cout << iadd->print() << std::endl;
                        iadd->replace_all_use_with(cf.compute(iadd->get_instr_type(), lval, rval)); // let's see what this does
                        // bb->delete_instr(iadd);
                        delete_list.push_back(iadd);
                    }
                }
            }
            for (auto instr : delete_list)
                bb->delete_instr(instr);
        }
    }
}
