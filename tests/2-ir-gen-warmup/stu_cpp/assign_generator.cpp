#include "IRBuilder.h"
#define CONST_INT(num) \
  ConstantInt::get(num, module)

#define CONST_FP(num) \
  ConstantFP::get(num, module)

int main(void)
{
  auto module = new Module("Cminus code"); // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  auto main_ty = FunctionType::get(Int32Type, {});
  auto main_func = Function::create(main_ty, "main", module);
  auto the_one_bb = BasicBlock::create(module, "entry", main_func);
  builder->set_insert_point(the_one_bb);
  auto retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置

  auto array10_type = ArrayType::get(Int32Type, 10);
  auto a = builder->create_alloca(array10_type);
  auto a0_gep = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});
  builder->create_store(CONST_INT(10), a0_gep);
  auto a1_gep = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});
  auto a1_load = builder->create_load(Int32Type, a0_gep);
  auto mul = builder->create_imul(a1_load, CONST_INT(2));
  builder->create_store(mul, a1_gep);
  auto ret = builder->create_load(Int32Type, a1_gep);
  builder->create_ret(ret);
  std::cout << module->print();
  delete module;
}