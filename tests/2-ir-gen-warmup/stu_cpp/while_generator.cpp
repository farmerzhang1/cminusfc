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
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder->set_insert_point(bb);
    auto ret = builder->create_alloca(Int32Type);
    auto a = builder->create_alloca(Int32Type);
    auto i = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), a);
    builder->create_store(CONST_INT(0), i);
    auto bbwhile = BasicBlock::create(module, "while", mainFun);
    builder->create_br(bbwhile);
// while
    builder->set_insert_point(bbwhile);
    auto aload = builder->create_load(a);
    auto iload = builder->create_load(i);
    auto isuc = builder->create_iadd(iload, CONST_INT(1));
    builder->create_store(isuc, i);
    auto a_add = builder->create_iadd(aload, isuc);
    builder->create_store(a_add, a);
    auto icmp = builder->create_icmp_lt(isuc, CONST_INT(10));
    auto retbb = BasicBlock::create(module, "return", mainFun);
    builder->create_cond_br(icmp, bbwhile, retbb);
// return
    builder->set_insert_point(retbb);
    builder->create_ret(a_add);
    std::cout << module->print();
    delete module;
    return 0;


}