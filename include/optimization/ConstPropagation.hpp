#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>

// tips: 用来判断value是否为ConstantFP/ConstantInt
ConstantFP* cast_constantfp(Value *value);
ConstantInt* cast_constantint(Value *value);


// tips: ConstFloder类

class ConstFolder
{
public:
    ConstFolder(Module *m) : module_(m) {}
    ConstantInt *compute_i(
        Instruction::OpID op,
        ConstantInt *value1,
        ConstantInt *value2);
    ConstantFP *compute_f(
        Instruction::OpID op,
        ConstantFP *value1,
        ConstantFP *value2);
    ConstantInt *f_to_i(ConstantFP *value);
    ConstantFP *i_to_f(ConstantInt *value);
    // ...
private:
    Module *module_;
};

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m) {}
    void run();
};

#endif
