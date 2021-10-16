#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#define CONST_INT(num) \
  ConstantInt::get(num, module)

#define CONST_FP(num) \
  ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main(void) {
    auto module = new Module("Cminus code"); // module name是什么无关紧要
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *FloatType = Type::get_float_type(module);
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder->set_insert_point(bb);
    auto ret = builder->create_alloca(Int32Type);
    auto a = builder->create_alloca(FloatType);
    builder->create_store(CONST_FP(5.555), a);
    auto float_load = builder->create_load(FloatType, a);
    auto fcmp = builder->create_fcmp_gt(float_load, CONST_FP(1.0));
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);   // true分支
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun); // false分支
    auto retBB = BasicBlock::create(
        module, "", mainFun); // return分支,提前create,以便true分支可以br
    builder->create_cond_br(fcmp, trueBB, falseBB);

// true:
    builder->set_insert_point(trueBB);
    builder->create_store(CONST_INT(233), ret);
    builder->create_br(retBB);

// false:
    builder->set_insert_point(falseBB);
    builder->create_store(CONST_INT(0), ret);
    builder->create_br(retBB);

// return
    builder->set_insert_point(retBB);
    auto retttt = builder->create_load(ret);
    builder->create_ret(retttt);
    std::cout << module->print();
    delete module;
    return 0;

}