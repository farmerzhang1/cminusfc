#include "reg.h"
#include "Value.h"
#include <map>
#include <vector>
#include <set>

class RegAlloc {
private:
    // TODO: live interval;
    struct interval {
        int start, end;
        Value* val;
    };
    // TODO: check registers that can be allocated
    const std::vector<Reg> regs;
    std::map<Value *, Reg *> maps;
    // active is the list, sorted in order of increasing end point,
    // of live intervals overlapping the current point and placed in registers.
    std::set<interval> active;    // TODO: add a comparator
    std::set<interval> intervals; // TODO: add a comparator in order of increasing start point
public:
    void assign_reg(interval);
    void free_reg(interval);
    void LinearScanRegisterAllocation();
    void ExpireOldIntervals(interval i);
    void SpillAtInterval(interval i);
    // spill ← last interval in active
    // if endpoint[spill] > endpoint[i] then
    //     register[i] ← register[spill]
    //     location[spill] ← new stack location
    //     remove spill from active
    //     add i to active, sorted by increasing end point
    // else
    //     location[i] ← new stack location
};