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

int main(void)
{
  auto module = new Module("Cminus code"); // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);
  // callee
  auto callee_type = FunctionType::get(Int32Type, {Int32Type});
  auto callee = Function::create(callee_type, "callee", module);
  auto bb_callee = BasicBlock::create(module, "entry", callee);
  builder->set_insert_point(bb_callee);
  auto a = *callee->arg_begin();
  auto r = builder->create_imul(CONST_INT(2), a);
  builder->create_ret(r);
  // main
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb_main = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb_main);
  auto ret = builder->create_call(callee, {CONST_INT(110)});
  builder->create_ret(ret);
  std::cout << module->print();
  delete module;
  return 0;
}