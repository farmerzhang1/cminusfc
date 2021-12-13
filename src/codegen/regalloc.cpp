#include "regalloc.h"

void RegAlloc::LinearScanRegisterAllocation() {
    active.clear();
    for (auto i : intervals) {
        ExpireOldIntervals(i);
        if (active.size() == regs.size())
            SpillAtInterval(i);
        else {
            assign_reg(i);
            active.insert(i);
        }
    }
}

void RegAlloc::ExpireOldIntervals(interval i) {
    std::vector<interval> delete_list;
    for (auto j : active) {
        if (j.end >= i.start) return;
        delete_list.push_back(j);
    }
    for (auto j : delete_list) {
        active.erase(j);
        free_reg(j);
    }
}

void RegAlloc::SpillAtInterval(interval i) {
    auto &spill = *active.rbegin(); // 要待最久的 spill 掉
    if (spill.end > i.end) {
        maps[i.val] = maps[spill.val];
        // location[spill.val] = new_stack;
    } else {
    }
}