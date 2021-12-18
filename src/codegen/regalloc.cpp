#include "reg.h"
#include "regalloc.h"
#include "Function.h"
#include <functional>

RegAlloc::RegAlloc(Module *m) :
    m_(m), av(m) /* , live_in(av.get_livein()), live_out(av.get_liveout()) */ {
    av.run(); // note the above two lives are empty, but we don't need it
}
void RegAlloc::print_stats() {
    std::cout << f_->get_name() << std::endl << "regs" << std::endl;
    for (auto [k, v] : maps) {
        std::cout << k->get_name() << ": " << v.get_name() << std::endl;
    }
    std::cout << "stack" << std::endl;
    for (auto [k, v] : location) {
        std::cout << k->get_name() << ": " << v << std::endl;
    }
}
void RegAlloc::run() {
    for (auto f : m_->get_functions()) {
        if (f->get_num_basic_blocks() == 0) continue;
        f_ = f;
        init_func();
        LinearScanRegisterAllocation();
        print_stats();
    }
}

void RegAlloc::LinearScanRegisterAllocation() {
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

void RegAlloc::init_func() {
    int i{0};
    maps.clear();
    val2interval.clear();
    active.clear();
    intervals.clear();
    available_regs = regs;
    int2bb.clear();
    bb2int.clear();
    auto &live_in = av.get_livein(f_);
    auto &live_out = av.get_liveout(f_);
    for (auto bb : f_->get_basic_blocks()) {
        int2bb[i] = bb;
        bb2int[bb] = i++;
    }
    // looks like we can use auto in func params
    auto fff = [this](bool in, auto i) -> void {
        auto bb = i.first;
        auto set = i.second;
        int bblevel = bb2int[bb];
        point srcloc(bblevel, in);
        for (auto val : set) {
            if (val2interval.contains(val)) {
                auto &t = val2interval[val];
                if (srcloc > t->end) t->end = srcloc;
                if (srcloc < t->start) t->start = srcloc;
            } else {
                val2interval[val] = std::make_shared<interval>(val, srcloc, srcloc);
            }
        }
    };
    std::for_each(live_in.begin(), live_in.end(), [fff](auto i) { fff(true, i); });
    std::for_each(live_out.begin(), live_out.end(), [fff](auto i) { fff(false, i); });
    for (auto [val, interv] : val2interval) {
        // auto [_1, b1] = active.insert(interv);
        // assert(b1);
        auto [_2, b2] = intervals.insert(interv);
        assert(b2);
    }
}

void RegAlloc::ExpireOldIntervals(ip &i) {
    std::vector<ip> delete_list;
    for (auto j : active) {
        if (j->end >= i->start) return;
        delete_list.push_back(j);
    }
    for (auto j : delete_list) {
        active.erase(j);
        free_reg(j);
    }
}

void RegAlloc::SpillAtInterval(ip &i) {
    auto &spill = *active.rbegin(); // 要待最久的变量 spill 掉
    if (i->end < spill->end) {
        maps[i->val] = maps[spill->val];
        location[spill->val] = spill->val->get_type()->get_size();
        stack_size += spill->val->get_type()->get_size();
        active.erase(spill);
        active.insert(i);
    } else {
        location[i->val] = i->val->get_type()->get_size();
        stack_size += i->val->get_type()->get_size();
    }
}

void RegAlloc::assign_reg(ip &i) {
    // TODO
    assert(!available_regs.empty());
    assert(!maps.contains(i->val));
    auto r = *available_regs.begin();
    maps[i->val] = r;
    available_regs.erase(r);
}

void RegAlloc::free_reg(ip &i) {
    auto r = maps[i->val];
    // maps.erase(i->val);
    available_regs.insert(r);
}
