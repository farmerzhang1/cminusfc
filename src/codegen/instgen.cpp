#include <string>
#include <sstream>

#include "instruction.h"
#include "reg.h"

const std::array<std::string, 32> Reg::regs {
    "x0",   "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

const std::array<std::string, 32> Reg::alts {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0",  "a1",  "a2",  "a3",  "a4",  "a5",  "a6",  "a7",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",  "s8",  "s9",  "s10", "s11", "t3",  "t4",  "t5",  "t6"
};

std::string Instgen::ret() {
    return "\tret";
}

std::string Instgen::addi(Reg dst, Reg rs1, int imm) {
    return "\taddi " + dst.get_name() + "," + rs1.get_name() + "," + std::to_string(imm);
}

std::string Instgen::sd(Reg src, int offset, Reg base) {
    return "\tsd " + src.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")";
}

std::string Instgen::ld(Reg dst, int offset, Reg base) {
    return "\tld " + dst.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")";
}

std::string Instgen::jr(Reg ret) {
    return "\tjr " + ret.get_name();
}