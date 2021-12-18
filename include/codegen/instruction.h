#ifndef _INSTR_H_
#define _INSTR_H_

#include <sstream>

#include "reg.h"

class Instgen {
private:
    // std::stringstream& ss;
    // i think we can add fp afterwards
    enum class Inst {
        addi,
        slti,
        andi,
        ori,
        xori,
        slli,
        srli,
        srai,
        lui,
        auipc,
        add,
        slt,
        sltu,
        and_,
        or_,
        xor_,
        sll,
        srl,
        sub,
        sra,
        jal,
        jalr,
        beq,
        bne,
        blt,
        bge,
        load,
        store,
        ret
    };

public:
    Instgen() {}
    std::string ret();
    std::string addi(Reg, Reg, int);
    std::string sd(Reg, int, Reg);
    std::string ld(Reg, int, Reg);
    std::string jr(Reg);
};
#endif
