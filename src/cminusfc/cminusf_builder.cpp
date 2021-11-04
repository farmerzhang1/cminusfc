#include "cminusf_builder.hpp"
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
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    Value *v;
    Type *t;
    t = type(node.type);
    if (node.num) // array type
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
    std::transform(node.params.begin(), node.params.end(), std::back_inserter(params), [this](const auto &param)
                   { return this->type(param->type); });
    auto func_type = FunctionType::get(type(node.type), params);
    auto f = Function::create(func_type, node.id, module.get());
    size_t i{0};
    std::for_each(f->arg_begin(), f->arg_end(), [i, node](Argument *arg)
                  { arg->set_name(node.params[i]->id); });
    scope.enter();
    auto bb = BasicBlock::create(module.get(), "entry", f);
    builder->set_insert_point(bb);
    node.compound_stmt->accept(*this);
}

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
}

void CminusfBuilder::visit(ASTVar &node)
{
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
}

void CminusfBuilder::visit(ASTTerm &node)
{
}

void CminusfBuilder::visit(ASTCall &node)
{
}
