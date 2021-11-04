#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())
#define ENTRY "entry"
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
