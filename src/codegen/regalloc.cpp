#include "reg.h"
#include "regalloc.h"
#include "Function.h"
#include <functional>

RegAlloc::RegAlloc(Module *m) :
    m_(m) /* , live_in(av.get_livein()), live_out(av.get_liveout()) */
// av.run(); // note the above two lives are empty, but we don't need it
{}

void RegAlloc::print_stats() {
    std::cout << f_->get_name() << std::endl
              << "regs" << std::endl;
    for (auto [k, v] : reg_mappings) {
        std::cout << k->get_name() << ": " << v.get_name() << std::endl;
    }
    std::cout << "stack" << std::endl;
    for (auto [k, v] : stack_mappings) {
        std::cout << k->get_name() << ": " << v << std::endl;
    }
}

void RegAlloc::run() {
    for (auto f : m_->get_functions()) {
        if (f->get_num_basic_blocks() == 0) continue;
        f_ = f;
        init_func();
        pre_allocate_args();
        LinearScanRegisterAllocation();
        print_stats();
        f_reg_map.insert({f, std::move(reg_mappings)});
        f_stack_map.insert({f, std::move(stack_mappings)});
    }
}

void RegAlloc::pre_allocate_args() {
    int int_arg_counter{0}, fl_arg_counter{0};
    for (auto arg : f_->get_args()) {
        bool used = val2interval.contains(arg);
        if (arg->get_type()->is_integer_type()) {
            if (used)
                available_regs.erase(
                    reg_mappings[arg] = args[int_arg_counter]);
            int_arg_counter++;
        } else if (arg->get_type()->is_float_type()) {
            if (used)
                available_regs.erase(
                    reg_mappings[arg] = fargs[fl_arg_counter]);
            fl_arg_counter++;
        }
    }
}

void RegAlloc::LinearScanRegisterAllocation() {
    for (auto i : intervals) {
        ExpireOldIntervals(i);
        if (pre_allocated(i)) continue;
        if (active.size() == regs.size())
            SpillAtInterval(i);
        else {
            assign_reg(i);
            active.insert(i);
        }
    }
}

bool RegAlloc::pre_allocated(ip &interval) {
    for (auto arg : f_->get_args())
        if (interval->val == arg) return true;

    return false;
}

void RegAlloc::init_func() {
    int i{0};
    reg_mappings.clear();
    stack_mappings.clear();
    val2interval.clear();
    active.clear();
    intervals.clear();
    available_regs = regs;
    for (auto a : args) available_regs.insert(a);
    for (auto fa : fargs) available_regs.insert(fa);
    int2bb.clear();
    bb2int.clear();
    // auto &live_in = av.get_livein(f_);
    // auto &live_out = av.get_liveout(f_);
    for (auto bb : f_->get_basic_blocks()) {
        int2bb[i] = bb;
        bb2int[bb] = i++;
    }
    // looks like we can use auto in func params
    // auto fff = [this](auto i) -> void {
    //     auto bb = i.first;
    //     auto set = i.second;
    //     int bblevel = bb2int[bb];
    //     point srcloc(bblevel, in);
    //     for (auto val : set) {
    //         if (val2interval.contains(val)) {
    //             auto &t = val2interval[val];
    //             if (srcloc > t->end) t->end = srcloc;
    //             if (srcloc < t->start) t->start = srcloc;
    //         } else {
    //             val2interval[val] = std::make_shared<interval>(val, srcloc, srcloc);
    //         }
    //     }
    // };
    int bbcounter = 0;
    int instr_counter = 0;
    for (auto arg : f_->get_args()) {
        if (arg->get_use_list().empty()) continue;
        val2interval.insert({arg, std::make_shared<interval>(arg, 0, 0, 0, 0)});
    }
    for (auto bb : f_->get_basic_blocks()) {
        bbcounter++; // first bb is entry
        instr_counter = 0;
        for (auto instr : bb->get_instructions()) {
            instr_counter++;
            point srcloc(bbcounter, instr_counter);
            if (!instr->get_name().empty()) {
                val2interval.insert({instr, std::make_shared<interval>(instr, srcloc, srcloc)});
            }
            for (auto rand : instr->get_operands()) {
                if (val2interval.contains(rand)) { // 直接不考虑之前没加入interval映射里的东西, might be label, constant...
                    val2interval[rand]->end.bbindex = bbcounter;
                    val2interval[rand]->end.line_no = instr_counter;
                }
            }
        }
    }
    // std::for_each(live_in.begin(), live_in.end(), [fff](auto i) { fff(i); });
    // std::for_each(live_out.begin(), live_out.end(), [fff](auto i) { fff(i); });
    for (auto [val, interv] : val2interval) {
        auto [_2, success] = intervals.insert(interv);
        assert(success);
    }
}

void RegAlloc::ExpireOldIntervals(ip &i) {
    std::vector<ip> delete_list;
    for (auto j : active) {
        if (j->end >= i->start) break;
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
        reg_mappings[i->val] = reg_mappings[spill->val];
        stack_mappings[spill->val] = spill->val->get_type()->get_size();
        stack_size += spill->val->get_type()->get_size();
        active.erase(spill);
        active.insert(i);
    } else {
        stack_mappings[i->val] = i->val->get_type()->get_size();
        stack_size += i->val->get_type()->get_size();
    }
}

void RegAlloc::assign_reg(ip &i) {
    assert(!available_regs.empty());
    assert(!reg_mappings.contains(i->val));
    auto r = *available_regs.begin();
    reg_mappings[i->val] = r;
    available_regs.erase(r);
}

void RegAlloc::free_reg(ip &i) {
    auto r = reg_mappings[i->val];
    // maps.erase(i->val);
    available_regs.insert(r);
}
