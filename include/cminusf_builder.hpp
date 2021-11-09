#ifndef _CMINUSF_BUILDER_HPP_
#define _CMINUSF_BUILDER_HPP_
#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"
#include "ast.hpp"
#include <map>
#include <functional>

class Scope
{
public:
    // enter a new scope
    void enter()
    {
        inner.push_back({});
    }

    // exit a scope
    void exit()
    {
        inner.pop_back();
    }

    bool in_global()
    {
        return inner.size() == 1;
    }

    // push a name to scope
    // return true if successful
    // return false if this name already exits
    bool push(std::string name, Value *val)
    {
        auto result = inner[inner.size() - 1].insert({name, val});
        return result.second;
    }

    Value *find(std::string name)
    {
        for (auto s = inner.rbegin(); s != inner.rend(); s++)
        {
            auto iter = s->find(name);
            if (iter != s->end())
            {
                return iter->second;
            }
        }

        return nullptr;
    }
    // return global variables (functions or vars)
    Value *get_global(std::string &name) const
    {
        return inner.at(0).at(name);
    }

private:
    std::vector<std::map<std::string, Value *>> inner;
};

class CminusfBuilder : public ASTVisitor
{
public:
    CminusfBuilder()
    {
        module = std::unique_ptr<Module>(new Module("Cminus code"));
        builder = new IRBuilder(nullptr, module.get());
        auto TyVoid = Type::get_void_type(module.get());
        auto TyInt32 = Type::get_int32_type(module.get());
        auto TyFloat = Type::get_float_type(module.get());

        auto input_type = FunctionType::get(TyInt32, {});
        auto input_fun =
            Function::create(
                input_type,
                "input",
                module.get());

        std::vector<Type *> output_params;
        output_params.push_back(TyInt32);
        auto output_type = FunctionType::get(TyVoid, output_params);
        auto output_fun =
            Function::create(
                output_type,
                "output",
                module.get());

        std::vector<Type *> output_float_params;
        output_float_params.push_back(TyFloat);
        auto output_float_type = FunctionType::get(TyVoid, output_float_params);
        auto output_float_fun =
            Function::create(
                output_float_type,
                "outputFloat",
                module.get());

        auto neg_idx_except_type = FunctionType::get(TyVoid, {});
        auto neg_idx_except_fun =
            Function::create(
                neg_idx_except_type,
                "neg_idx_except",
                module.get());

        scope.enter();
        scope.push("input", input_fun);
        scope.push("output", output_fun);
        scope.push("outputFloat", output_float_fun);
        scope.push("neg_idx_except", neg_idx_except_fun);
        // static_assert(this->add_int_map.contains(AddOp::OP_PLUS)); // 打咩！ please refer to
        // https://stackoverflow.com/questions/30620356/how-can-i-ensure-that-each-case-defined-in-an-enum-class-is-treated-e-g-using

        // do some assertions here to ensure all enum members are covered
        for (const auto &enumCase : std::vector<AddOp>{AddOp::OP_PLUS, AddOp::OP_MINUS})
        {
            assert(add_int_map.contains(enumCase));
            assert(add_float_map.contains(enumCase));
        }
        for (const auto &enumCase : std::vector<MulOp>{MulOp::OP_MUL, MulOp::OP_DIV})
        {
            assert(mul_int_map.contains(enumCase));
            assert(mul_float_map.contains(enumCase));
        }
        for (const auto &enumCase : std::vector<RelOp>{RelOp::OP_EQ, RelOp::OP_NEQ, RelOp::OP_LT, RelOp::OP_LE, RelOp::OP_GT, RelOp::OP_GE})
        {
            assert(comp_int_map.contains(enumCase));
            assert(comp_float_map.contains(enumCase));
        }
    }

    std::unique_ptr<Module> getModule()
    {
        return std::move(module);
    }
    Type *type(CminusType t) const;
    Value *convert(Value *n, Type *to);
    Value *val = nullptr;
    std::map<AddOp, std::function<BinaryInst *(Value *, Value *)>> add_int_map = {
        {AddOp::OP_PLUS, [this](Value *l, Value *r)
         { return builder->create_iadd(l, r); }},
        {AddOp::OP_MINUS, [this](Value *l, Value *r)
         { return builder->create_isub(l, r); }}};
    std::map<AddOp, std::function<BinaryInst *(Value *, Value *)>> add_float_map = {
        {AddOp::OP_PLUS, [this](Value *l, Value *r)
         { return builder->create_fadd(l, r); }},
        {AddOp::OP_MINUS, [this](Value *l, Value *r)
         { return builder->create_fsub(l, r); }}};
    std::map<MulOp, std::function<BinaryInst *(Value *, Value *)>> mul_int_map = {
        {MulOp::OP_MUL, [this](Value *l, Value *r)
         { return builder->create_imul(l, r); }},
        {MulOp::OP_DIV, [this](Value *l, Value *r)
         { return builder->create_isdiv(l, r); }}};
    std::map<MulOp, std::function<BinaryInst *(Value *, Value *)>> mul_float_map = {
        {MulOp::OP_MUL, [this](Value *l, Value *r)
         { return builder->create_fmul(l, r); }},
        {MulOp::OP_DIV, [this](Value *l, Value *r)
         { return builder->create_fdiv(l, r); }}};
    std::map<RelOp, std::function<CmpInst *(Value *, Value *)>> comp_int_map = {
        {RelOp::OP_EQ, [this](Value *l, Value *r)
         { return builder->create_icmp_eq(l, r); }},
        {RelOp::OP_NEQ, [this](Value *l, Value *r)
         { return builder->create_icmp_ne(l, r); }},
        {RelOp::OP_GE, [this](Value *l, Value *r)
         { return builder->create_icmp_ge(l, r); }},
        {RelOp::OP_GT, [this](Value *l, Value *r)
         { return builder->create_icmp_gt(l, r); }},
        {RelOp::OP_LE, [this](Value *l, Value *r)
         { return builder->create_icmp_le(l, r); }},
        {RelOp::OP_LT, [this](Value *l, Value *r)
         { return builder->create_icmp_lt(l, r); }}};
    std::map<RelOp, std::function<FCmpInst *(Value *, Value *)>> comp_float_map = {
        {RelOp::OP_EQ, [this](Value *l, Value *r)
         { return builder->create_fcmp_eq(l, r); }},
        {RelOp::OP_NEQ, [this](Value *l, Value *r)
         { return builder->create_fcmp_ne(l, r); }},
        {RelOp::OP_GE, [this](Value *l, Value *r)
         { return builder->create_fcmp_ge(l, r); }},
        {RelOp::OP_GT, [this](Value *l, Value *r)
         { return builder->create_fcmp_gt(l, r); }},
        {RelOp::OP_LE, [this](Value *l, Value *r)
         { return builder->create_fcmp_le(l, r); }},
        {RelOp::OP_LT, [this](Value *l, Value *r)
         { return builder->create_fcmp_lt(l, r); }}};

private:
    virtual void visit(ASTProgram &) override final;
    virtual void visit(ASTNum &) override final;
    virtual void visit(ASTVarDeclaration &) override final;
    virtual void visit(ASTFunDeclaration &) override final;
    virtual void visit(ASTParam &) override final;
    virtual void visit(ASTCompoundStmt &) override final;
    virtual void visit(ASTExpressionStmt &) override final;
    virtual void visit(ASTSelectionStmt &) override final;
    virtual void visit(ASTIterationStmt &) override final;
    virtual void visit(ASTReturnStmt &) override final;
    virtual void visit(ASTAssignExpression &) override final;
    virtual void visit(ASTSimpleExpression &) override final;
    virtual void visit(ASTAdditiveExpression &) override final;
    virtual void visit(ASTVar &) override final;
    virtual void visit(ASTTerm &) override final;
    virtual void visit(ASTCall &) override final;

    IRBuilder *builder;
    Scope scope;
    std::unique_ptr<Module> module;
};
#endif
