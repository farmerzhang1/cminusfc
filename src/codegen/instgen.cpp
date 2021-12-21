#include <string>
#include <sstream>

#include "instruction.h"
#include "reg.h"

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