#ifndef _REG_H_
#define _REG_H_
#include "value.h"
#include <array>
#include <string>
#include <assert.h>

using namespace std::literals::string_literals;
class Instgen;
// RISCV32 register
// TODO: RV64 ?
class Reg {
private:
    int id;
    // TODO: 弄成 static
    static const std::array<std::string, 32> regs;
    // {"x0",   "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"};
    static const std::array<std::string, 32> alts;
    // {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0",  "a1",  "a2",  "a3",  "a4",  "a5",  "a6",  "a7",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",  "s8",  "s9",  "s10", "s11", "t3",  "t4",  "t5",  "t6"};

public:
    Reg(int id = -1) :
        id(id) {}
    Reg(std::string s) {
        // TODO
        if (s == "sp"s)
            id = 2;
        else if (s == "ra"s)
            id = 1;
        else if (s == "s0"s)
            id = 8;
        else
            assert("false");
    }
    Reg(const Reg &r) :
        id(r.id) {}
    Reg &operator=(const Reg &rhs) {
        id = rhs.id;
        return *this;
    }

    const bool operator<(const Reg &rhs) const { return this->id < rhs.id; }
    const bool operator==(const Reg &rhs) const { return this->id == rhs.id; }
    const bool operator!=(const Reg &rhs) const { return this->id != rhs.id; }
    const std::string get_name() const { return alts.at(id); }
    friend Instgen;
};

#endif