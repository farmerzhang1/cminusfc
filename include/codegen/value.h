#include <string>

// TODO: print()
// 参考 https://llvm.org/doxygen/classllvm_1_1MCOperand.html
// 或者 https://llvm.org/doxygen/classllvm_1_1MachineInstr.html
// 搞清楚啊
class Operand
{
public:
    virtual bool is_reg() const = 0;
    virtual bool is_constant() const = 0;
    virtual std::string getName() const = 0;
};
