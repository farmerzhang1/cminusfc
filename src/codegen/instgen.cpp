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
    return "\tret\n";
}

std::string Instgen::addi(Reg dst, Reg rs1, int imm) {
    return "\taddi " + dst.get_name() + "," + rs1.get_name() + "," + std::to_string(imm) + "\n";
}

std::string Instgen::sd(Reg src, int offset, Reg base) {
    assert(!src.f);
    return "\tsd " + src.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::sw(Reg src, int offset, Reg base) {
    assert(!src.f);
    return "\tsw " + src.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::ld(Reg dst, int offset, Reg base) {
    assert(!dst.f);
    return "\tld " + dst.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::lw(Reg dst, int offset, Reg base) {
    assert(!dst.f);
    return "\tlw " + dst.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::jr(Reg ret) {
    return "\tjr " + ret.get_name() + "\n";
}

std::string Instgen::call(std::string f) {
    return "\tcall " + f + "\n";
}

std::string Instgen::lla(Reg dst, std::string label) {
    return "\tlla " + dst.get_name() + "," + label + "\n";
}

std::string Instgen::flw(Reg dst, int offset, Reg base) {
    assert(dst.f);
    return "\tflw " + dst.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::mv(Reg dst, Reg src) {
    return "\tmv " + dst.get_name() + " " + src.get_name() + "\n";
}

std::string Instgen::fsw(Reg src, int offset, Reg base) {
    assert(src.f);
    return "\tfsw " + src.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}