#ifndef _REG_ALLOC_H_
#define _REG_ALLOC_H_
#include "reg.h"
#include "Value.h"
#include "Module.h"
#include "Function.h"
#include "ActiveVars.hpp"

#include <map>
#include <vector>
#include <set>
#include <memory>
#include <functional>

class Codegen;

class RegAlloc {
private:
    struct point {
        int bbindex;
        int line_no;
        // bool in; // true (in); false (out)
        bool operator<(point &rhs) {
            return bbindex < rhs.bbindex || bbindex == rhs.bbindex && line_no < rhs.line_no;
        }
        bool operator>(point &rhs) {
            return bbindex > rhs.bbindex || bbindex == rhs.bbindex && line_no > rhs.line_no;
        }
        bool operator<=(point &rhs) { return !(*this > rhs); }
        bool operator>=(point &rhs) { return !(*this < rhs); }
        point(int index, int line_no) :
            bbindex(index), line_no(line_no) {}
    };
    struct interval {
        point start, end; // both inclusive
        Value *val;
        interval(Value *val, point start, point end) :
            start(start), end(end), val(val) {}
        interval(Value *val, int bbstart, int line_no_s, int bbend, int line_no_e) :
            start(bbstart, line_no_s), end(bbend, line_no_s), val(val) {}
    };
    using ip = std::shared_ptr<interval>; // shared interval pointer
    Module *m_;
    Function *f_;
    int stack_size{0};
    std::map<BasicBlock *, int> bb2int;
    std::map<int, BasicBlock *> int2bb;
    // TODO: check registers that can be allocated
    const std::set<Reg> regs{Reg(5), Reg(6), Reg(7), Reg(28), Reg(29), Reg(30), Reg(31)};
    const std::set<Reg> fregs {Reg(5, true), Reg(6, true), Reg(7, true), Reg(28, true), Reg(29, true), Reg(30, true), Reg(31, true)};
    std::set<Reg> available_regs;
    std::map<Value *, Reg> reg_mappings; // lightIR 的 raw 指针 (Value*, Module* etc.) 留着，改不动改不动
    std::map<Value *, int> stack_mappings;
    std::map<Function*, std::map<Value *, Reg>> f_reg_map;
    std::map<Function*, std::map<Value *, int>> f_stack_map;
    std::map<Function*, int> f_stack_size;

    struct ActiveLess {
        const bool operator()(const ip &l, const ip &r) const {
            if (l->end < r->end) return true;
            if (r->end < l->end) return false;
            return std::less{}(l->val, r->val);
        }
    };
    struct IntervalLess {
        const bool operator()(const ip &l, const ip &r) const {
            if (l->start < r->start) return true;
            if (r->start < l->start) return false;
            return std::less{}(l->val, r->val);
        }
    };
    // active is the list, sorted in order of increasing end point,
    // of live intervals overlapping the current point and placed in registers.
    std::set<ip, ActiveLess> active;
    std::set<ip, IntervalLess> intervals;
    std::map<Value *, ip> val2interval;

public:
    friend Codegen;
    RegAlloc(Module *);
    void init_func();
    void assign_reg(ip &);
    void free_reg(ip &);
    void LinearScanRegisterAllocation();
    void ExpireOldIntervals(ip &);
    void SpillAtInterval(ip &);
    void print_stats();
    void run();
    bool pre_allocated(ip &);
    void pre_allocate_args();
};
#endif
