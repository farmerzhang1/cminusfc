#ifndef _REG_H_
#define _REG_H_
#include <array>
#include <string>
#include <assert.h>

using namespace std::literals::string_literals;
class Instgen;
class Codegen;
class RegAlloc;

class Reg {
private:
    bool f;
    int id;
    static const std::array<std::string, 32> regs;
    static const std::array<std::string, 32> alts;
public:
    // need a constructor that takes no argument
    Reg(int id = -1, bool f = false) :
        id(id), f(f) {}
    Reg(std::string s) : f(false) {
        if (s == "sp"s)
            id = 2;
        else if (s == "ra"s)
            id = 1;
        else if (s == "s0"s)
            id = 8;
        else if (s == "a0"s)
            id = 10;
        else
            throw std::runtime_error("invalid string names for reg");
    }
    // check that regs are valid
    explicit operator bool() const { return id != -1; }

    const bool operator<(const Reg &rhs) const { return this->id < rhs.id || (this->id == rhs.id && !this->f && rhs.f); }
    const bool operator==(const Reg &rhs) const { return this->id == rhs.id && this->f == rhs.f; }
    const bool operator!=(const Reg &rhs) const { return !(*this == rhs); }
    const std::string get_name() const { return (f ? "f" : "") + alts.at(id); }
    friend Instgen;
    friend Codegen;
    friend RegAlloc;
};

extern const std::array<Reg, 8> args;
extern const std::array<Reg, 8> fargs;
extern const std::array<Reg, 12> saveds;
extern const std::array<Reg, 12> fsaveds;
extern const std::array<Reg, 7> temps;
extern const std::array<Reg, 7> ftemps;

#endif