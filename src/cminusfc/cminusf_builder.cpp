#include "cminusf_builder.hpp"
#include <algorithm>

// TA: 呃呃鹅鹅鹅

// me: 真对不起大家
Value *val = nullptr;
bool address_only = false;        // dealing with pointers
bool return_in_branch = false;    // 用在 selection statement 判断分支语句中是否有返回，如果有则不跳转到 out 中
bool in_branch = false;           // 当前 builder 处于分支语句中，主要用在 return statement 判断该 store 还是 load
bool pre_returns = false;         // 在之前的语句中有 return, 创建一个基本块用来返回
bool enter_in_fun_decl = false;   // need to push function parameters and return value into the scope
const std::string return_val{""}; // set to empty string to avoid conflicts, load it when need to return (at the end of a function)
CminusType return_type;
size_t bb_counter{0};

#define both_int(lhs, rhs) (lhs->get_type()->is_integer_type() && rhs->get_type()->is_integer_type())

#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_INT(num) \
    ConstantInt::get(num, module.get())

#define add_int_map(op, l, r)                 \
    do                                        \
    {                                         \
        switch ((op))                         \
        {                                     \
        case AddOp::OP_PLUS:                  \
        {                                     \
            val = builder->create_iadd(l, r); \
            break;                            \
        }                                     \
        case AddOp::OP_MINUS:                 \
        {                                     \
            val = builder->create_isub(l, r); \
            break;                            \
        };                                    \
        }                                     \
    } while (false);
// std::map<AddOp, std::function<BinaryInst *(Value *, Value *)>> add_int_map = {
//     {AddOp::OP_PLUS, [this](Value *l, Value *r)
//      { return builder->create_iadd(l, r); }},
//     {AddOp::OP_MINUS, [this](Value *l, Value *r)
//      { return builder->create_isub(l, r); }}};
#define add_float_map(op, l, r)               \
    do                                        \
    {                                         \
        switch ((op))                         \
        {                                     \
        case AddOp::OP_PLUS:                  \
        {                                     \
            val = builder->create_fadd(l, r); \
            break;                            \
        }                                     \
        case AddOp::OP_MINUS:                 \
        {                                     \
            val = builder->create_fsub(l, r); \
            break;                            \
        };                                    \
        }                                     \
    } while (false);
// std::map<AddOp, std::function<BinaryInst *(Value *, Value *)>> add_float_map = {
//     {AddOp::OP_PLUS, [this](Value *l, Value *r)
//      { return builder->create_fadd(l, r); }},
//     {AddOp::OP_MINUS, [this](Value *l, Value *r)
//      { return builder->create_fsub(l, r); }}};
#define mul_int_map(op, l, r)                  \
    do                                         \
    {                                          \
        switch ((op))                          \
        {                                      \
        case MulOp::OP_MUL:                    \
        {                                      \
            val = builder->create_imul(l, r);  \
            break;                             \
        }                                      \
        case MulOp::OP_DIV:                    \
        {                                      \
            val = builder->create_isdiv(l, r); \
            break;                             \
        };                                     \
        }                                      \
    } while (false);
// std::map<MulOp, std::function<BinaryInst *(Value *, Value *)>> mul_int_map = {
//     {MulOp::OP_MUL, [this](Value *l, Value *r)
//      { return builder->create_imul(l, r); }},
//     {MulOp::OP_DIV, [this](Value *l, Value *r)
//      { return builder->create_isdiv(l, r); }}};
#define mul_float_map(op, l, r)               \
    do                                        \
    {                                         \
        switch ((op))                         \
        {                                     \
        case MulOp::OP_MUL:                   \
        {                                     \
            val = builder->create_fmul(l, r); \
            break;                            \
        }                                     \
        case MulOp::OP_DIV:                   \
        {                                     \
            val = builder->create_fdiv(l, r); \
            break;                            \
        }                                     \
        }                                     \
    } while (false);
// std::map<MulOp, std::function<BinaryInst *(Value *, Value *)>> mul_float_map = {
//     {MulOp::OP_MUL, [this](Value *l, Value *r)
//      { return builder->create_fmul(l, r); }},
//     {MulOp::OP_DIV, [this](Value *l, Value *r)
//      { return builder->create_fdiv(l, r); }}};
#define comp_int_map(op, l, r)                   \
    do                                           \
    {                                            \
        switch ((op))                            \
        {                                        \
        case RelOp::OP_EQ:                       \
        {                                        \
            val = builder->create_icmp_eq(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_NEQ:                      \
        {                                        \
            val = builder->create_icmp_ne(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_GE:                       \
        {                                        \
            val = builder->create_icmp_ge(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_GT:                       \
        {                                        \
            val = builder->create_icmp_gt(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_LE:                       \
        {                                        \
            val = builder->create_icmp_le(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_LT:                       \
        {                                        \
            val = builder->create_icmp_lt(l, r); \
            break;                               \
        }                                        \
        }                                        \
    } while (false);
// std::map<RelOp, std::function<CmpInst *(Value *, Value *)>> comp_int_map = {
//     {RelOp::OP_EQ, [this](Value *l, Value *r)
//      { return builder->create_icmp_eq(l, r); }},
//     {RelOp::OP_NEQ, [this](Value *l, Value *r)
//      { return builder->create_icmp_ne(l, r); }},
//     {RelOp::OP_GE, [this](Value *l, Value *r)
//      { return builder->create_icmp_ge(l, r); }},
//     {RelOp::OP_GT, [this](Value *l, Value *r)
//      { return builder->create_icmp_gt(l, r); }},
//     {RelOp::OP_LE, [this](Value *l, Value *r)
//      { return builder->create_icmp_le(l, r); }},
//     {RelOp::OP_LT, [this](Value *l, Value *r)
//      { return builder->create_icmp_lt(l, r); }}};
#define comp_float_map(op, l, r)                 \
    do                                           \
    {                                            \
        switch ((op))                            \
        {                                        \
        case RelOp::OP_EQ:                       \
        {                                        \
            val = builder->create_fcmp_eq(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_NEQ:                      \
        {                                        \
            val = builder->create_fcmp_ne(l, r); \
            break;                               \
        };                                       \
        case RelOp::OP_GE:                       \
        {                                        \
            val = builder->create_fcmp_ge(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_GT:                       \
        {                                        \
            val = builder->create_fcmp_gt(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_LE:                       \
        {                                        \
            val = builder->create_fcmp_le(l, r); \
            break;                               \
        }                                        \
        case RelOp::OP_LT:                       \
        {                                        \
            val = builder->create_fcmp_lt(l, r); \
            break;                               \
        }                                        \
        }                                        \
    } while (false);
// std::map<RelOp, std::function<FCmpInst *(Value *, Value *)>> comp_float_map = {
//     {RelOp::OP_EQ, [this](Value *l, Value *r)
//      { return builder->create_fcmp_eq(l, r); }},
//     {RelOp::OP_NEQ, [this](Value *l, Value *r)
//      { return builder->create_fcmp_ne(l, r); }},
//     {RelOp::OP_GE, [this](Value *l, Value *r)
//      { return builder->create_fcmp_ge(l, r); }},
//     {RelOp::OP_GT, [this](Value *l, Value *r)
//      { return builder->create_fcmp_gt(l, r); }},
//     {RelOp::OP_LE, [this](Value *l, Value *r)
//      { return builder->create_fcmp_le(l, r); }},
//     {RelOp::OP_LT, [this](Value *l, Value *r)
//      { return builder->create_fcmp_lt(l, r); }}};
#define type(t) (t == CminusType::TYPE_FLOAT ? (module->get_float_type()) : (t == CminusType::TYPE_INT ? (module->get_int32_type()) : module->get_void_type()))

#define convert(n, to) ((n->get_type()->get_type_id() == to->get_type_id() && (n->get_type()->is_integer_type() || n->get_type()->is_float_type())) ? ((n->get_type()->get_size() < to->get_size()) ? (builder->create_zext(n, to)) : (n))                                                                                                                                                                                                                                                     \
                                                                                                                                                    : ((n->get_type()->is_integer_type() && to->is_float_type()) ? builder->create_sitofp(n, to) : ((n->get_type()->is_float_type() && to->is_integer_type()) ? builder->create_fptosi(n, to) : ((n->get_type()->get_pointer_element_type()->is_array_type() && to->is_pointer_type())) ? ((Value*)builder->create_gep(n, {CONST_INT(0), CONST_INT(0)})) \
                                                                                                                                                                                                                                                                                                                                            : (n->get_type()->get_pointer_element_type()->is_pointer_type() && to->is_pointer_type())   ? builder->create_load(n)                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                        : nullptr)))

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
    default: /* [[unlikely]] */
        break;
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    Type *t;
    t = type(node.type);
    if (node.num)
        t = module->get_array_type(t, node.num->i_val);

    if (scope.in_global())
    {
        Constant *init = ConstantZero::get(t, module.get());
        val = GlobalVariable::create(node.id, module.get(), t, false, init);
    }
    else
        val = builder->create_alloca(t);
    assert(scope.push(node.id, val));
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    std::vector<Type *> params;
    std::transform(node.params.begin(), node.params.end(), std::back_inserter(params), [this](const auto &param)
                   {
                       auto contained = type(param->type);
                       return param->isarray ? module->get_pointer_type(contained) : contained;
                   });
    auto func_type = FunctionType::get(type(node.type), params);
    auto f = Function::create(func_type, node.id, module.get());
    assert(scope.push(node.id, f));
    size_t i{0};
    auto bb = BasicBlock::create(module.get(), "entry", f);
    builder->set_insert_point(bb);
    scope.enter();
    enter_in_fun_decl = true;
    for (auto &arg : f->get_args())
    {
        arg->set_name(node.params[i]->id);
        node.params[i]->accept(*this);
        val = builder->create_store(arg, val);
        i++;
    }
    bb_counter = 0;
    return_type = node.type;
    if (node.type != CminusType::TYPE_VOID)
    {
        val = builder->create_alloca(type(return_type));
        scope.push(return_val, val);
    }
    node.compound_stmt->accept(*this);
    if (!builder->get_insert_block()->get_terminator())
    {
        auto ret = std::make_unique<ASTReturnStmt>();
        ret->accept(*this);
    }
}

void CminusfBuilder::visit(ASTParam &node)
{
    Type *t = type(node.type);
    if (node.isarray)
        t = Type::get_pointer_type(t);
    val = builder->create_alloca(t);
    assert(scope.push(node.id, val));
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    if (!enter_in_fun_decl)
        scope.enter();
    enter_in_fun_decl = false;

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
    // TODO: factor out common code
    bool if_return_in_branch{false}, else_return_in_branch{false};
    BasicBlock *falseBB = nullptr;
    node.expression->accept(*this);
    if (val->get_type()->get_size() > 1)
    {
        if (val->get_type()->is_integer_type())
        {
            comp_int_map(RelOp::OP_NEQ, val, CONST_INT(0));
        }
        else
        {
            comp_float_map(RelOp::OP_NEQ, val, CONST_FP(0));
        }
    }
    in_branch = true;
    auto current_func = builder->get_insert_block()->get_parent();
    auto trueBB = BasicBlock::create(module.get(),
                                     "true" + std::to_string(bb_counter++),
                                     current_func);
    if (node.else_statement)
        falseBB = BasicBlock::create(module.get(),
                                     "false" + std::to_string(bb_counter++),
                                     current_func);
    auto out = BasicBlock::create(module.get(),
                                  "out" + std::to_string(bb_counter++),
                                  current_func);
    builder->create_cond_br(val, trueBB, falseBB ? falseBB : out);
    in_branch = true;

    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    if (!return_in_branch)
        builder->create_br(out);
    if_return_in_branch = return_in_branch;
    return_in_branch = false;
    if (node.else_statement)
    {
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if (!return_in_branch)
            builder->create_br(out);
        else_return_in_branch = return_in_branch;
        return_in_branch = false;
    }
    in_branch = false;
    builder->set_insert_point(out);
    if (if_return_in_branch && else_return_in_branch)
    {
        if (return_type != CminusType::TYPE_VOID)
        {
            val = builder->create_load(scope.find(return_val));
            builder->create_ret(val);
        }
        else
            builder->create_void_ret();

        builder->set_insert_point(trueBB);
        builder->create_br(out);
        builder->set_insert_point(falseBB);
        builder->create_br(out);

        builder->set_insert_point(out); // TODO: don't generate any more instructions
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    auto current_func = builder->get_insert_block()->get_parent();

    auto predicate = BasicBlock::create(module.get(),
                                        "predicate" + std::to_string(bb_counter++),
                                        current_func);
    auto body = BasicBlock::create(module.get(),
                                   "body" + std::to_string(bb_counter++),
                                   current_func);
    auto out = BasicBlock::create(module.get(),
                                  "out" + std::to_string(bb_counter++),
                                  current_func);

    builder->create_br(predicate);

    builder->set_insert_point(predicate);
    node.expression->accept(*this);
    if (val->get_type()->get_size() > 1)
    {
        if (val->get_type()->is_integer_type())
        {
            comp_int_map(RelOp::OP_NEQ, val, CONST_INT(0));
        }
        else
        {
            comp_float_map(RelOp::OP_NEQ, val, CONST_FP(0));
        }
    }
    builder->create_cond_br(val, body, out);
    builder->set_insert_point(body);
    node.statement->accept(*this);
    builder->create_br(predicate);
    builder->set_insert_point(out);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    // TODO: factor out common code
    if (node.expression)
    {
        CminusType t = return_type;
        node.expression->accept(*this);
        if (in_branch)
        {
            Value *ptr = scope.find(return_val);
            Type *aaa = type(t);
            builder->create_store(convert(val, aaa), ptr);
            return_in_branch = true;
            pre_returns = true;
        }
        else
        {
            Value *ptr = scope.find(return_val);
            builder->create_store(convert(val, type(t)), ptr);
            if (pre_returns)
            {
                auto returnBB = BasicBlock::create(module.get(), "return", builder->get_insert_block()->get_parent());
                builder->create_br(returnBB);
                for (auto &bb : builder->get_insert_block()->get_parent()->get_basic_blocks()) // 这也绕太多圈了
                {
                    if (bb == returnBB)
                        continue;
                    if (!bb->get_terminator())
                    {
                        builder->set_insert_point(bb);
                        builder->create_br(returnBB);
                    }
                }
                builder->set_insert_point(returnBB);
            }
            val = builder->create_load(ptr);
            builder->create_ret(val);
        }
    }
    else
    {
        if (in_branch)
        {
            return_in_branch = true;
            pre_returns = true;
        }
        else
        {
            if (pre_returns)
            {
                auto returnBB = BasicBlock::create(module.get(), "return", builder->get_insert_block()->get_parent());
                builder->create_br(returnBB);
                for (auto &bb : builder->get_insert_block()->get_parent()->get_basic_blocks()) // 这也绕太多圈了
                {
                    if (bb == returnBB)
                        continue;
                    if (!bb->get_terminator())
                    {
                        builder->set_insert_point(bb);
                        builder->create_br(returnBB);
                    }
                }
                builder->set_insert_point(returnBB);
            }

            builder->create_void_ret();
        }
    }
}

void CminusfBuilder::visit(ASTVar &node)
{
    Value *ptr = scope.find(node.id); // consider ptr = alloca([10 x i32])
    if (node.expression)
    {
        bool temp = address_only;
        address_only = false;
        node.expression->accept(*this);
        address_only = temp;
        Value *offset = convert(val, module->get_int32_type());
        comp_int_map(RelOp::OP_GE, offset, CONST_INT(0));
        auto t = BasicBlock::create(module.get(), "true" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
        auto f = BasicBlock::create(module.get(), "false" + std::to_string(bb_counter++), builder->get_insert_block()->get_parent());
        builder->create_cond_br(val, t, f);
        builder->set_insert_point(f);
        builder->create_call(scope.find("neg_idx_except"), {});
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
    val = address_only ? ptr : builder->create_load(ptr);
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
            comp_int_map(node.op, lhs, rhs);
        }
        else // at least one is float
        {
            lhs = convert(lhs, module->get_float_type()); // convert to float, return it directly if is float already
            rhs = convert(rhs, module->get_float_type());
            comp_float_map(node.op, lhs, rhs);
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
            add_int_map(node.op, lhs, rhs);
        }
        else
        {
            lhs = convert(lhs, module->get_float_type());
            rhs = convert(rhs, module->get_float_type());
            add_float_map(node.op, lhs, rhs);
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
            mul_int_map(node.op, lhs, rhs);
        }
        else
        {
            lhs = convert(lhs, module->get_float_type());
            rhs = convert(rhs, module->get_float_type());
            mul_float_map(node.op, lhs, rhs);
        }
    }
}

void CminusfBuilder::visit(ASTCall &node)
{
    auto func = static_cast<Function *>(scope.find(node.id));
    std::vector<Value *> args;
    size_t i = 0;
    std::transform(node.args.begin(), node.args.end(), std::back_inserter(args), [this, &i, func](auto &e)
                   {
                       address_only = func->get_function_type()->get_param_type(i)->is_pointer_type();
                       e->accept(*this);
                       address_only = false;
                       auto ret = convert(val, func->get_function_type()->get_param_type(i));
                       i++;
                       return ret;
                   });
    val = builder->create_call(func, std::move(args));
}
