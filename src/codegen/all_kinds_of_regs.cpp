#include <array>
#include <set>

#include "reg.h"

const std::array<std::string, 32> Reg::regs {
    "x0",   "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

const std::array<std::string, 32> Reg::alts {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0",  "a1",  "a2",  "a3",  "a4",  "a5",  "a6",  "a7",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",  "s8",  "s9",  "s10", "s11", "t3",  "t4",  "t5",  "t6"
};

const std::array<Reg, 8> args{
    Reg(10), Reg(11), Reg(12), Reg(13), Reg(14), Reg(15), Reg(16), Reg(17)
};

const std::array<Reg, 8> fargs{
    Reg(10, true), Reg(11, true), Reg(12, true), Reg(13, true), Reg(14, true), Reg(15, true), Reg(16, true), Reg(17, true)
};

const std::array<Reg, 12> saveds {
    Reg(8), Reg(9), Reg(18), Reg(19), Reg(20), Reg(21), Reg(22), Reg(23), Reg(24), Reg(25), Reg(26), Reg(27)
};

const std::array<Reg, 12> fsaveds {
    Reg(8, true), Reg(9, true), Reg(18, true), Reg(19, true), Reg(20, true), Reg(21, true), Reg(22, true), Reg(23, true), Reg(24, true), Reg(25, true), Reg(26, true), Reg(27, true)
};

const std::array<Reg, 7> temps {
    Reg(5), Reg(6), Reg(7), Reg(28), Reg(29), Reg(30), Reg(31)
};

const std::array<Reg, 7> ftemps {
    Reg(5, true), Reg(6, true), Reg(7, true), Reg(28, true), Reg(29, true), Reg(30, true), Reg(31, true)
};
