#include <boost/preprocessor.hpp>
// https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string
#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
    case elem : return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
    enum name {                                                               \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
    };                                                                        \
                                                                              \
    inline const char* ToString(name v)                                       \
    {                                                                         \
        switch (v)                                                            \
        {                                                                     \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
        }                                                                     \
    }

class Instruction {
    // i think we can add fp afterwards
    enum class Inst {
        ADDI, SLTI, ANDI, ORI, XORI,
        SLLI, SRLI, SRAI,
        LUI, AUIPC,
        ADD, SLT, SLTU, AND, OR, XOR, SLL, SRL, SUB, SRA,
        JAL, JALR,
        BEQ, BNE, BLT, BGE,
        LOAD, STORE
    };
};