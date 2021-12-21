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
    std::string addi(Reg, Reg, int);
    std::string sd(Reg, int, Reg);
    std::string sw(Reg, int, Reg);
    std::string ld(Reg, int, Reg);
    std::string lw(Reg, int, Reg);
    std::string jr(Reg);
    std::string call(std::string);
    std::string li (Reg, int);
    std::string lla(Reg, std::string);
    std::string flw(Reg, int, Reg);
    std::string fsw(Reg, int, Reg);
    std::string mv(Reg, Reg);
};
#endif
