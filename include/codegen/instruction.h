#ifndef _INSTR_H_
#define _INSTR_H_

#include <sstream>

#include "reg.h"

class Instgen {
private:
    // std::stringstream& ss;
    // i think we can add fp afterwards
public:
    Instgen() {}
    std::string ret();
    std::string sd(Reg, int, Reg);
    std::string sw(Reg, int, Reg);
    std::string ld(Reg, int, Reg);
    std::string lw(Reg, int, Reg);
    std::string flw(Reg, int, Reg);
    std::string fsw(Reg, int, Reg);
    std::string jr(Reg);
    std::string call(std::string);
    std::string li (Reg, int);
    std::string lla(Reg, std::string);
    std::string la(Reg, std::string);
    std::string mv(Reg, Reg);
    std::string fmvs(Reg, Reg);
    std::string add(Reg, Reg, Reg);
    std::string fadds(Reg, Reg, Reg);
    std::string addi(Reg, Reg, int);
    std::string lui(Reg, int);
    std::string sub(Reg, Reg, Reg);
    std::string subi(Reg, Reg, int);
    std::string fsubs(Reg, Reg, Reg);
    std::string mul(Reg, Reg, Reg);
    std::string fmuls(Reg, Reg, Reg);
    std::string div(Reg, Reg, Reg);
    std::string fdivs(Reg, Reg, Reg);
    std::string fcvtsw(Reg, Reg);
    std::string fcvtws(Reg, Reg);
    std::string seqz(Reg, Reg);
    std::string snez(Reg, Reg);
    std::string slt(Reg, Reg, Reg);
    std::string sgt(Reg, Reg, Reg);
    std::string xori(Reg, Reg, int);
    std::string sextw(Reg, Reg);
    std::string andi(Reg, Reg, int);
    std::string feqs(Reg, Reg, Reg);
    std::string flts(Reg, Reg, Reg);
    std::string fgts(Reg, Reg, Reg);
    std::string fges(Reg, Reg, Reg);
    std::string fles(Reg, Reg, Reg);
    std::string j(std::string);
    std::string bnez(Reg, std::string);
    std::string slli(Reg, Reg, int);
};
#endif
