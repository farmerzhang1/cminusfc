#include "cminusf_builder.hpp"
#include "utils.hpp"

Type *CminusfBuilder::type(CminusType t) const
{
    switch (t)
    {
    case CminusType::TYPE_FLOAT:
        return module->get_float_type();
        break;
    case CminusType::TYPE_INT:
        return module->get_int32_type();
        break;
    case CminusType::TYPE_VOID:
        return module->get_void_type();
        break;
    }
}

// both types are i32 or i1, which means when generating IR, should call create_i...
bool both_int(Value *lhs, Value *rhs)
{
    return lhs->get_type()->is_integer_type() && rhs->get_type()->is_integer_type();
}

Value *CminusfBuilder::convert(Value *n, Type *to)
{
    if (n->get_type()->get_type_id() == to->get_type_id())
    {
        if (n->get_type()->get_size() < to->get_size()) return builder->create_zext(n, to);
        return n;
    }
    if (n->get_type()->is_integer_type() && to->is_float_type())
    {
        return builder->create_sitofp(n, to);
    }
    if (n->get_type()->is_float_type() && to->is_integer_type())
    {
        return builder->create_fptosi(n, to);
    }
    if(to->is_pointer_type())
    {
        return n;
    }
    assert(!"what type are you converting to?");
    return nullptr;
}
