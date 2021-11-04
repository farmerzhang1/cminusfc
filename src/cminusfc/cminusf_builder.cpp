#include "cminusf_builder.hpp"
#include <algorithm>
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())

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
    switch(node.type) {
        case CminusType::TYPE_FLOAT: val = CONST_FP(node.f_val); break;
        case CminusType::TYPE_INT  : val = ConstantInt::get(node.i_val, module.get()); break;
        default: /* [[unlikely]] (wanted to use this qwq) */ break;
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    Value *v;
    Type *t;
    t = type(node.type); // utility
    if (node.num)        // array type
        t = module->get_array_type(t, node.num->i_val);

    if (scope.in_global())
        v = GlobalVariable::create(node.id, module.get(), type(node.type), false, nullptr);
    else
        v = builder->create_alloca(t);

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
    std::for_each(f->arg_begin(), f->arg_end(), [i, node](Argument *arg)
                  { arg->set_name(node.params[i]->id); });
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
    std::for_each(node.local_declarations.begin(), node.local_declarations.end(), [this](auto &decl_ptr)
                  { decl_ptr->accept(*this); });
    std::for_each(node.statement_list.begin(), node.statement_list.end(), [this](auto &stmt_ptr)
                  { stmt_ptr->accept(*this); });
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    if (node.expression) node.expression->accept(*this);
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
    if (node.additive_expression_r) {
        node.additive_expression_r->accept(*this);
        auto rhs = val;
        // builder->create // TODO: how to distinguish its type?
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    if (node.additive_expression) {
        // TODO
    }
    node.term->accept(*this);
}

void CminusfBuilder::visit(ASTTerm &node)
{
    if (node.term) {
        // TODO
    } else {
        node.factor->accept(*this);
    }
}

void CminusfBuilder::visit(ASTCall &node)
{
    auto func = scope.get_global(node.id);
    std::vector<Value *> args;
    std::transform(node.args.begin(), node.args.end(), std::back_inserter(args), [this](auto& e)
                   {
                       e->accept(*this);
                       return val;
                   });
    builder->create_call(func, std::move(args));
}
