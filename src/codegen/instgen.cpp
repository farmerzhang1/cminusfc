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
std::string Instgen::add(Reg dst, Reg rs1, Reg rs2) {
    return "\tadd " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
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
    assert(dst.f && !base.f);
    return "\tflw " + dst.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::mv(Reg dst, Reg src) {
    assert(!dst.f && !src.f);
    return "\tmv " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::fsw(Reg src, int offset, Reg base) {
    assert(src.f && !base.f);
    return "\tfsw " + src.get_name() + "," + std::to_string(offset) + "(" + base.get_name() + ")\n";
}

std::string Instgen::sub(Reg src, Reg rs1, Reg rs2) {
    assert(!src.f && !rs1.f && !rs2.f);
    return "\tsub " + src.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::subi(Reg src, Reg rs1, int imm) {
    assert(!src.f && !rs1.f);
    return "\tsubi " + src.get_name() + "," + rs1.get_name() + "," + std::to_string(imm) + "\n";
}

std::string Instgen::fcvtsw(Reg dst, Reg src) {
    assert(dst.f && !src.f);
    return "\tfcvt.s.w " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::fcvtws(Reg dst, Reg src) {
    assert(!dst.f && src.f);
    return "\tfcvt.w.s " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::fadds(Reg dst, Reg rs1, Reg rs2) {
    assert(dst.f && rs1.f && rs2.f);
    return "\tfadd.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fmvs(Reg dst, Reg src) {
    assert(dst.f && src.f);
    return "\tfmv.s " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::mul(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && !rs1.f && !rs2.f);
    return "\tmul " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fmuls(Reg dst, Reg rs1, Reg rs2) {
    assert(dst.f && rs1.f && rs2.f);
    return "\tfmul.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::div(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && !rs1.f && !rs2.f);
    return "\tdiv " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fdivs(Reg dst, Reg rs1, Reg rs2) {
    assert(dst.f && rs1.f && rs2.f);
    return "\tfdiv.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fsubs(Reg dst, Reg rs1, Reg rs2) {
    assert(dst.f && rs1.f && rs2.f);
    return "\tfsub.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::lui(Reg dst, int imm) {
    assert(!dst.f);
    return "\tlui " + dst.get_name() + "," + std::to_string(imm) + "\n";
}

std::string Instgen::seqz(Reg dst, Reg src) {
    assert(!dst.f && !src.f);
    return "\tseqz " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::snez(Reg dst, Reg src) {
    assert(!dst.f && !src.f);
    return "\tsnez " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::slt(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && !rs1.f && !rs2.f);
    return "\tslt " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::xori(Reg dst, Reg src, int imm) {
    assert(!dst.f && !src.f);
    return "\txori " + dst.get_name() + "," + src.get_name() + "," + std::to_string(imm) + "\n";
}

std::string Instgen::sgt(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && !rs1.f && !rs2.f);
    return "\tsgt " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::sextw(Reg dst, Reg src) {
    assert(!dst.f && !src.f);
    return "\tsext.w " + dst.get_name() + "," + src.get_name() + "\n";
}

std::string Instgen::andi(Reg dst, Reg src, int imm) {
    assert(!dst.f && !src.f);
    return "\tandi " + dst.get_name() + "," + src.get_name() + "," + std::to_string(imm) + "\n";
}

std::string Instgen::feqs(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && rs1.f && rs2.f);
    return "\tfeq.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::flts(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && rs1.f && rs2.f);
    return "\tflt.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fgts(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && rs1.f && rs2.f);
    return "\tfgt.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fges(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && rs1.f && rs2.f);
    return "\tfge.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::fles(Reg dst, Reg rs1, Reg rs2) {
    assert(!dst.f && rs1.f && rs2.f);
    return "\tfle.s " + dst.get_name() + "," + rs1.get_name() + "," + rs2.get_name() + "\n";
}

std::string Instgen::bnez(Reg cond, std::string label) {
    assert (!cond.f);
    return "\tbnez " + cond.get_name() + ", ." + label + "\n";
}

std::string Instgen::j(std::string label) {
    return "\tj ." + label + "\n";
}
