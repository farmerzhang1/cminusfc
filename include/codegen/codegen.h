#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <map>
#include <sstream>
#include <string>

#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"
#include "reg.h"
#include "instruction.h"

using namespace std::literals::string_literals;

class Codegen {
private:
    Module *m; // top
    Function* f_; // current function
    std::stringstream ss;
    Instgen ig;
    std::map<Value *, Reg> reg_mapping;
    std::map<Value *, int> stack_mapping;
    size_t fcounter{0};
    int stack_size;
    Reg sp, s0, ra;
public:
    Codegen(Module* m);
    std::string gen_module();
    void gen_function(Function*);
    void gen_bb(BasicBlock*);
    void fun_prologue(Function*);
    void fun_epilogue(Function*);
    void comment(std::string);
    void allocate_stack();
};
#endif
