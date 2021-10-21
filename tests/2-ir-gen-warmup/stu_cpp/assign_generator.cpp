#include "IRBuilder.h"
#include <memory>

#define CONST_INT(num) \
  std::shared_ptr<Value>(ConstantInt::get(num, module))

#define CONST_FP(num) \
  std::shared_ptr<Value>(ConstantFP::get(num, module))

int main(void)
{
  auto module = new Module("Cminus code"); // module name是什么无关紧要
  auto builder = std::unique_ptr<IRBuilder>(new IRBuilder(nullptr, module));
  Type *Int32Type = Type::get_int32_type(module);

  auto main_ty = std::unique_ptr<FunctionType>(FunctionType::get(Int32Type, {}));
  auto main_func = std::shared_ptr<Function> (Function::create(main_ty.get(), "main", module));
  auto the_one_bb = std::shared_ptr<BasicBlock> (BasicBlock::create(module, "entry", main_func.get()));
  builder->set_insert_point(the_one_bb.get());
  auto retAlloca = std::shared_ptr<Value> (builder->create_alloca(Int32Type)); // 在内存中分配返回值的位置

  auto array10_type = ArrayType::get(Int32Type, 10);
  auto a = std::shared_ptr<Value>(builder->create_alloca(array10_type));
  auto temp1 = CONST_INT(0);
  auto temp2 = CONST_INT(0);
  auto temp3 = CONST_INT(0);
  auto temp4 = CONST_INT(1);
  auto temp5 = CONST_INT(2);
  auto temp6 = CONST_INT(10);
  auto a0_gep = std::shared_ptr<Value>(builder->create_gep(a.get(), {temp1.get(), temp2.get()}));
  auto temp_inst = std::shared_ptr<Value>(builder->create_store(temp6.get(), a0_gep.get()));
  auto a1_gep = std::shared_ptr<Value>(builder->create_gep(a.get(), {temp3.get(), temp4.get()}));
  auto a1_load = std::shared_ptr<Value>(builder->create_load(Int32Type, a0_gep.get()));
  auto mul = std::shared_ptr<Value>(builder->create_imul(a1_load.get(), temp5.get()));
  auto temp_inst1 = std::shared_ptr<Value>(builder->create_store(mul.get(), a1_gep.get()));
  auto ret = std::shared_ptr<Value>(builder->create_load(Int32Type, a1_gep.get()));
  auto return_inst = std::shared_ptr<Value>(builder->create_ret(ret.get()));
  std::cout << module->print();
  delete module;
}
