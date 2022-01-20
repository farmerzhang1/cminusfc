#include "reg.h"
#include "regalloc.h"
#include "Function.h"
#include <functional>

RegAlloc::RegAlloc(Module *m) :
    m_(m) {}

void RegAlloc::print_stats() {
    std::cout << f_->get_name() << std::endl
              << "bbs" << std::endl;
    for (auto [i, bb] : int2bb)
        std::cout << bb->get_name() << ": " << i << std::endl;
    std::cout << "regs" << std::endl;
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
        // print_stats();
        f_reg_map.insert({f, std::move(reg_mappings)});
        f_stack_map.insert({f, std::move(stack_mappings)});
        f_stack_size.insert({f, stack_size});
    }
}

void RegAlloc::pre_allocate_args() {
    int int_arg_counter{0}, fl_arg_counter{0};
    for (auto arg : f_->get_args()) {
        bool used = val2interval.contains(arg);
        if (!arg->get_type()->is_float_type()) {
            if (used) {
                available_regs.erase(
                    reg_mappings[arg] = args[int_arg_counter]);
                if (arg->get_type()->is_pointer_type()) reg_mappings[arg].d = true;
            }
            int_arg_counter++;
        } else /*  if (arg->get_type()->is_float_type())  */ {
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
        if (active.size() == regs.size() + fregs.size())
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

void RegAlloc::dfs(BasicBlock *s) {
    // 应该改为深度优先后序遍历的reverse （？
    // Another is depth-first ordering,
    // the reverse of the order in which nodes are
    // last visited in a preorder traversal of the flow graph
    // 参考 aho et al p 662 Depth-first search algorithm
    visited[s] = true;
    for (auto suc : s->get_succ_basic_blocks()) {
        if (!visited[suc]) dfs(suc);
    }
    int2bb[c] = s;
    bb2int[s] = c--;
}
void RegAlloc::init_func() {
    int i{0};
    reg_mappings.clear();
    stack_mappings.clear();
    stack_size = f_->has_fcalls() ? 16 : 8;
    val2interval.clear();
    active.clear();
    intervals.clear();
    available_regs = regs;
    for (auto r : fregs) available_regs.insert(r);
    for (auto a : args) available_regs.insert(a);
    for (auto fa : fargs) available_regs.insert(fa);
    int2bb.clear();
    bb2int.clear();
    visited.clear();
    c = f_->get_num_basic_blocks();
    for (auto bb : f_->get_basic_blocks()) visited[bb] = false;
    dfs(f_->get_entry_block());
    // for (auto bb : f_->get_basic_blocks()) {
    //     int2bb[i] = bb;
    //     bb2int[bb] = i++;
    // }
    int bbcounter = 0;
    int instr_counter = 0;
    for (auto arg : f_->get_args()) {
        if (arg->get_use_list().empty()) continue;
        val2interval.insert({arg, std::make_shared<interval>(arg, 0, 0, 0, 0)});
    }
    for (auto [bbcounter, bb] : int2bb) {
        // bbcounter++; // first bb is entry
        // bbcounter = bb2int.at(bb);
        instr_counter = 0;
        for (auto instr : bb->get_instructions()) {
            instr_counter++;
            if (instr->is_alloca()) { // alloca 的返回值（指针）已经在栈上了，不需要（没必要？）分配寄存器
                auto alloca = dynamic_cast<AllocaInst *>(instr);
                auto size = alloca->get_alloca_type()->get_size();
                stack_size += size;
                stack_mappings[instr] = -stack_size;
                continue;
            }
            point srcloc(bbcounter, instr_counter);
            if (!instr->get_name().empty()) {
                val2interval.insert({instr, std::make_shared<interval>(instr, srcloc, srcloc)});
            }
            for (auto rand : instr->get_operands()) {
                if (val2interval.contains(rand)) { // 直接不考虑之前没加入interval映射里的东西, might be label, constant, alloca
                    val2interval[rand]->end.bbindex = bbcounter;
                    val2interval[rand]->end.line_no = instr_counter;
                }
            }
        }
    }
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
        auto spill_size = spill->val->get_type()->get_size();
        stack_size += spill_size;
        stack_mappings[spill->val] = -spill_size;
        active.erase(spill);
        active.insert(i);
    } else {
        auto isize = i->val->get_type()->get_size();
        stack_size += isize;
        stack_mappings[i->val] = -isize;
    }
}

void RegAlloc::assign_reg(ip &i) {
    assert(!available_regs.empty());
    assert(!reg_mappings.contains(i->val));
    Reg r;
    for (auto avai : available_regs) {
        if (avai.f == i->val->get_type()->is_float_type()) {
            r = avai;
            break;
        }
    }
    reg_mappings[i->val] = r;
    available_regs.erase(r);
}

void RegAlloc::free_reg(ip &i) {
    auto r = reg_mappings[i->val];
    // maps.erase(i->val);
    available_regs.insert(r);
}
