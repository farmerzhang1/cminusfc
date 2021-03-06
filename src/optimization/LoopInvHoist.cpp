#include <algorithm>
#include "LoopInvHoist.hpp"
#include "LoopSearch.hpp"
#include "logging.hpp"

void LoopInvHoist::run() {
    loop_searcher.run();
    for (auto f : m_->get_functions()) {
        for (auto loop : loop_searcher.get_loops_in_func(f)) {
            bool inv = false;
            for (auto bb : *loop) {
                for (auto instr : bb->get_instructions()) {
                    inv |= inv_in_loop(instr, loop);
                }
            }
            if (inv) {
                moveout(loop);
                invariants.clear();
            }
        }
    }
}

bool LoopInvHoist::inv_in_loop(Instruction *instr, BBset_t *loop) {
    Instruction *temp;
    if (instr->is_call())   return false;
    if (instr->is_ret())    return false;
    if (instr->is_br())     return false;
    if (instr->is_phi())    return false;
    for (auto rand : instr->get_operands()) {
        if ((temp = dynamic_cast<Instruction *>(rand))) {
            if (!(loop->find(temp->get_parent()) != loop->end())
                || invariants.find(temp) != invariants.end())
                ;
            else
                return false;
        }
    }
    invariants.insert(instr);
    return true;
}

void LoopInvHoist::moveout(BBset_t *loop) {
    auto base = loop_searcher.get_loop_base(loop);
    BasicBlock *pred_pre = nullptr; // predecessor of the predicate
    std::cout << "loop base: " << base->get_name() << std::endl;
    for (auto prev : base->get_pre_basic_blocks())
        if (!(loop->find(prev) != loop->end())) pred_pre = prev;
    auto last = pred_pre->get_instructions().back();
    pred_pre->delete_instr(last); // delete the last instruction (which is branch), append all the invariants, then add the branch back
    for (auto inv : invariants) {
        auto bb = inv->get_parent();
        bb->delete_instr(inv);
        pred_pre->add_instruction(inv);
        inv->set_parent(pred_pre);
    }
    pred_pre->add_instruction(last);
}
