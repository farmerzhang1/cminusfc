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
    if (node.num)        // array type
        t = module->get_array_type(t, node.num->i_val);

    if (scope.in_global())
        val = GlobalVariable::create(node.id, module.get(), type(node.type), false, nullptr);
    else
        val = builder->create_alloca(t);

    if (!scope.push(node.id, v))
    {
        // why not just shadow it?
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
    */
    // so this basically traverse NODE.PARAMS and store each parameter's type in PARAMS
    std::transform(node.params.begin(), node.params.end(), std::back_inserter(params), [this](const auto &param)
                   { return this->type(param->type); });
    auto func_type = FunctionType::get(type(node.type), params);
    auto f = Function::create(func_type, node.id, module.get());
    size_t i{0};
    // 声明函数时没有参数名，这里设置一下
    // std::for_each(f->arg_begin(), f->arg_end(), [i, node](Argument *arg)
    //               { arg->set_name(node.params[i]->id); });
    for (auto &arg : f->get_args())
        arg->set_name(node.params[i++]->id);

    scope.enter();
    auto bb = BasicBlock::create(module.get(), "entry", f);
    builder->set_insert_point(bb);
    node.compound_stmt->accept(*this);
}

// how can we use this??
void CminusfBuilder::visit(ASTParam &node)
{
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    for (auto &decl_ptr : node.local_declarations)
        decl_ptr->accept(*this);
    for (auto &stmt_ptr : node.statement_list)
        stmt_ptr->accept(*this);
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    if (node.expression)
        node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression)
    {
        node.expression->accept(*this);
        builder->create_ret(val);
    }
    else
        builder->create_void_ret();
    scope.exit(); // where should we use terminator?
}

void CminusfBuilder::visit(ASTVar &node)
{
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
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
            // 两个差不多的 switch 太丑了！！！ done!
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
                       e->accept(*this);
                       return convert(val, func->get_function_type()->get_param_type(i++));
                   });
    val = builder->create_call(func, std::move(args));
}
