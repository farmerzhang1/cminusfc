#include "cminusf_builder.hpp"
#include "utils.hpp"
#include <algorithm>

// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node)
{
    for (const auto &decl : node.declarations)
        decl->accept(*this);
}

void CminusfBuilder::visit(ASTNum &node)
{
    switch (node.type)
    {
    case CminusType::TYPE_FLOAT:
        val = CONST_FP(node.f_val);
        break;
    case CminusType::TYPE_INT:
        val = ConstantInt::get(node.i_val, module.get());
        break;
    default: /* [[unlikely]] (wanted to use this qwq) */
        break;
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    Value *v; // do we still need it?
    Type *t;
    t = type(node.type); // utility
    if (node.num)        // array type // int a[10];
        t = module->get_array_type(t, node.num->i_val);

    if (scope.in_global())
    {
        Constant *init = ConstantZero::get(t, module.get());
        // if (node.num)
        // {
        //     std::vector<Constant *> vec(node.num->i_val, init);
        //     init = ConstantArray::get(static_cast<ArrayType *>(t), vec);
        // }
        val = GlobalVariable::create(node.id, module.get(), t, false, init);
    }
    else
        val = builder->create_alloca(t);

    if (!scope.push(node.id, val))
    {
        // why not just shadow it?
        // well, c doesn't support it...
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    std::vector<Type *> params;
    /*
    how to use std::transform & std::for_each
    参数：
    first iterator(iterator基本上就是指针): 通常是 vec.begin()
    last iterator: 通常是vec.end() (.end()指向的是最后一个元素再往后加一)
    result: 存储对第一个容器迭代的结果
    func: lambda 表达式 (匿名函数) [capture list](parameters){body} capture就是在body中出现但是不在参数列表里的变量
    params.push_back( return-thing )
    */
    // so this basically traverse NODE.PARAMS and store each parameter's type in PARAMS
    std::transform(node.params.begin(), node.params.end(), std::back_inserter(params), [this](const auto &param)
                   {
                       auto contained = type(param->type);
                       return param->isarray ? this->module->get_pointer_type(contained) : contained;
                   });
    auto func_type = FunctionType::get(type(node.type), params);
    auto f = Function::create(func_type, node.id, module.get());
    scope.push(node.id, f);
    size_t i{0};
    auto bb = BasicBlock::create(module.get(), "entry", f);
    builder->set_insert_point(bb);
    for (auto &arg : f->get_args())
    {
        arg->set_name(node.params[i]->id);
        node.params[i]->accept(*this);
        val = builder->create_store(arg, val);
        // if (node.params[i]->isarray)
        // {
        //     Value *ptr = scope.find(node.params[i]->id); // FIXME

        //     val = builder->create_load(ptr); // error from after load
        // }
        i++;
    }
    bb_counter = 0;
    return_type = node.type;
    node.compound_stmt->accept(*this);
}

// how can we use this??
void CminusfBuilder::visit(ASTParam &node)
{
    Type *t = type(node.type);
    if (node.isarray)
    {
        val = builder->create_alloca(Type::get_pointer_type(t));
    }
    else
    {
        val = builder->create_alloca(t);
    }
    scope.push(node.id, val);
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    scope.enter();
    for (auto &decl_ptr : node.local_declarations)
        decl_ptr->accept(*this);
    for (auto &stmt_ptr : node.statement_list)
        stmt_ptr->accept(*this);
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    if (node.expression)
        node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    BasicBlock *falseBB = nullptr;
    node.expression->accept(*this);
    if (val->get_type()->get_size() > 1)
    {
        if (val->get_type()->is_integer_type())
            val = comp_int_map[RelOp::OP_NEQ](val, CONST_INT(0));
        else
            val = comp_float_map[RelOp::OP_NEQ](val, CONST_FP(0));
    }
    auto trueBB = BasicBlock::create(module.get(), "true" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
    if (node.else_statement)
        falseBB = BasicBlock::create(module.get(), "false" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
    auto out = BasicBlock::create(module.get(), "out" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
    builder->create_cond_br(val, trueBB, falseBB ? falseBB : out);
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    builder->create_br(out);
    if (node.else_statement)
    {
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        builder->create_br(out);
    }
    builder->set_insert_point(out);
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    auto predicate = BasicBlock::create(module.get(), "predicate" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
    auto body = BasicBlock::create(module.get(), "body" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
    auto out = BasicBlock::create(module.get(), "out" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());

    builder->create_br(predicate);

    builder->set_insert_point(predicate);
    node.expression->accept(*this);
    if (val->get_type()->get_size() > 1)
    {
        if (val->get_type()->is_integer_type())
            val = comp_int_map[RelOp::OP_NEQ](val, CONST_INT(0));
        else
            val = comp_float_map[RelOp::OP_NEQ](val, CONST_FP(0));
    }
    builder->create_cond_br(val, body, out);
    builder->set_insert_point(body);
    node.statement->accept(*this);
    builder->create_br(predicate);
    builder->set_insert_point(out);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression)
    {
        CminusType t = return_type;
        node.expression->accept(*this);
        builder->create_ret(convert(val,type(t)));
    }
    else
        builder->create_void_ret();
}

void CminusfBuilder::visit(ASTVar &node)
{
    Value *ptr = scope.find(node.id); // ptr = alloca([10 x i32])
    if (node.expression)
    {
        node.expression->accept(*this);
        Value *offset = convert(val, module->get_int32_type());
        Value *nonnegative = comp_int_map[RelOp::OP_GE](offset, CONST_INT(0));
        auto t = BasicBlock::create(module.get(), "true" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
        auto f = BasicBlock::create(module.get(), "false" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
        builder->create_cond_br(nonnegative, t, f);
        builder->set_insert_point(f);
        builder->create_call(scope.get_global("neg_idx_except"), {});
        builder->create_br(t);
        builder->set_insert_point(t);
        if (ptr->get_type()->get_pointer_element_type()->is_array_type())
        {
            ptr = builder->create_gep(ptr, {CONST_INT(0), offset});
        }
        else if (ptr->get_type()->get_pointer_element_type()->is_pointer_type())
        {
            ptr = builder->create_load(ptr);
            ptr = builder->create_gep(ptr, {offset});
        }
    }
    val = this->address_only ? ptr : builder->create_load(ptr);
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    node.expression->accept(*this);
    Value *rhs = val;
    address_only = true;
    node.var->accept(*this);
    builder->create_store(convert(rhs, val->get_type()->get_pointer_element_type()), val);
    val = rhs;
    address_only = false;
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
    node.additive_expression_l->accept(*this);
    auto lhs = val;
    if (node.additive_expression_r)
    {
        node.additive_expression_r->accept(*this);
        auto rhs = val;
        if (both_int(lhs, rhs))
        {
            lhs = convert(lhs, module->get_int32_type()); // convert to i32 if it's i1
            rhs = convert(rhs, module->get_int32_type());
            val = comp_int_map[node.op](lhs, rhs);
        }
        else // at least one is float
        {
            lhs = convert(lhs, module->get_float_type()); // convert to float, return it directly if is float already
            rhs = convert(rhs, module->get_float_type());
            // 可不可以用一个 map, 从 operator 映射到 函数
            val = comp_float_map[node.op](lhs, rhs);
        }
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    Value *lhs = nullptr;
    if (node.additive_expression)
    {
        node.additive_expression->accept(*this);
        lhs = val;
    }
    node.term->accept(*this);
    Value *rhs = val;
    if (node.additive_expression)
    {
        if (both_int(lhs, rhs))
        {
            lhs = convert(lhs, module->get_int32_type());
            rhs = convert(rhs, module->get_int32_type());
            val = add_int_map[node.op](lhs, rhs);
        }
        else
        {
            lhs = convert(lhs, module->get_float_type());
            rhs = convert(rhs, module->get_float_type());
            val = add_float_map[node.op](lhs, rhs);
        }
    }
}

void CminusfBuilder::visit(ASTTerm &node)
{
    Value *lhs = nullptr;
    if (node.term)
    {
        node.term->accept(*this);
        lhs = val;
    }
    node.factor->accept(*this);
    Value *rhs = val;
    if (node.term)
    {
        if (both_int(lhs, rhs))
        {
            lhs = convert(lhs, module->get_int32_type());
            rhs = convert(rhs, module->get_int32_type());
            val = mul_int_map[node.op](lhs, rhs);
        }
        else
        {
            lhs = convert(lhs, module->get_float_type());
            rhs = convert(rhs, module->get_float_type());
            val = mul_float_map[node.op](lhs, rhs);
        }
    }
}

void CminusfBuilder::visit(ASTCall &node)
{
    auto func = static_cast<Function *>(scope.get_global(node.id));
    // func->get_function_type()->get_param_type(0);
    std::vector<Value *> args;
    size_t i = 0;
    std::transform(node.args.begin(), node.args.end(), std::back_inserter(args), [this, &i, func](auto &e)
                   {
                       // so storing means we need its address, not value
                       address_only = func->get_function_type()->get_param_type(i)->is_pointer_type();
                       e->accept(*this);
                       address_only = false;
                       return convert(val, func->get_function_type()->get_param_type(i++));
                   });
    val = builder->create_call(func, std::move(args));
}
