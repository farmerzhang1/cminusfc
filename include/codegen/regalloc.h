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

class RegAlloc {
private:
    // 研究一下别人怎么表示的
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
        point(int index, bool in) : index(index), in(in) {}
    };
    // TODO: live interval;
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
    bb2vals live_in;
    bb2vals live_out;
    ActiveVars av; // 是 Active Variables!
    int stack_offset{0};
    std::map<BasicBlock *, int> bb2int;
    std::map<int, BasicBlock *> int2bb;
    // TODO: check registers that can be allocated
    std::set<Reg> regs; // all regs
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
    std::set<ip, ActiveLess> active;      // TODO: add a comparator
    std::set<ip, IntervalLess> intervals; // TODO: add a comparator in order of increasing start point
    std::map<Value *, ip> val2interval;   // testing ref wrapper ( can't use it, please refer to (refer pun)
                                          // https://stackoverflow.com/questions/47849130/error-no-matching-function-for-call-to-stdreference-wrappermediumreferen)
public:
    RegAlloc(Module *);
    void init_func();
    void assign_reg(ip &);
    void free_reg(ip &);
    void LinearScanRegisterAllocation();
    void ExpireOldIntervals(ip &);
    void SpillAtInterval(ip &);
    void run();
};
