#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "LoopSearch.hpp"
#include "PassManager.hpp"
#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"

class LoopInvHoist : public Pass {
public:
    LoopInvHoist(Module *m) : Pass(m), loop_searcher(m_, true) {}
    bool inv_in_loop(Instruction*, BBset_t*);
    void run() override;
    void moveout (BBset_t*);
private:
    std::set<Instruction*> invariants;
    LoopSearch loop_searcher;
};
