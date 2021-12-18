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
    // TODO: 研究一下别人怎么表示的
    struct point {
        int index;
        bool in; // true (in); false (out)
        bool operator<(point &rhs) {
            return index < rhs.index || index == rhs.index && in && !rhs.in;
        }
        bool operator>(point &rhs) {
            return index > rhs.index || index == rhs.index && !in && rhs.in;
        }
        bool operator<=(point &rhs) { return !(*this > rhs); }
        bool operator>=(point &rhs) { return !(*this < rhs); }
        point(int index, bool in) :
            index(index), in(in) {}
    };
    struct interval {
        point start, end; // both inclusive
        Value *val;
        interval(Value *val, point start, point end) :
            start(start), end(end), val(val) {}
        interval(Value *val, int start, bool in_s, int end, bool in_e) :
            start(start, in_s), end(end, in_e), val(val) {}
    };
    using ip = std::shared_ptr<interval>; // shared interval pointer
    Module *m_;
    Function *f_;
    // https://stackoverflow.com/questions/6286656/dependencies-in-initialization-lists
    ActiveVars av; // 是 Active Variables!
    int stack_size{0};
    std::map<BasicBlock *, int> bb2int;
    std::map<int, BasicBlock *> int2bb;
    // TODO: check registers that can be allocated
    const std::set<Reg> regs{Reg(0), Reg(1), Reg(2), Reg(3), Reg(4), Reg(5), Reg(6), Reg(7), Reg(8), Reg(9), Reg(10)}; // all regs
    std::set<Reg> available_regs;
    std::map<Value *, Reg> maps; // lightIR 的 raw 指针 (Value*, Module* etc.) 留着，改不动改不动
    std::map<Value *, int> location;
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
};
#endif