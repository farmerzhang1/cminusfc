#ifndef ACTIVEVARS_HPP
#define ACTIVEVARS_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <map>
#include <queue>
#include <fstream>

using bb2vals = std::map<BasicBlock *, std::set<Value *>>;

class ActiveVars : public Pass {
public:
    ActiveVars(Module *m) :
        Pass(m) {}
    void run();
    void calc_def_and_use();
    std::string print();
    const bb2vals &get_livein() const { return live_in; }
    const bb2vals &get_liveout() const { return live_out; }
    const bb2vals &get_livein(Function* f) const { return f2avin.at(f); }
    const bb2vals &get_liveout(Function* f) const { return f2avout.at(f); }
private:
    Function *func_;
    bb2vals live_in, live_out, def, use;
    std::map<BasicBlock *, std::map<BasicBlock *, std::set<Value *>>> phiuses; // don't know which identifier to use
    std::map<Function*, bb2vals> f2avin, f2avout;
};

#endif
