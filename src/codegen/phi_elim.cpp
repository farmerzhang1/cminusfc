#include <map>
#include <iostream>
#include <fstream>
#include <string>

#include "phi_elim.h"
#include "Module.h"
#include "Function.h"

using namespace std::literals::string_literals;

PhiElim::PhiElim(Module *m) :
    m(m) {}

void PhiElim::run() {
    std::vector<std::list<Instruction *>::iterator> its;
    for (auto f : m->get_functions()) {
        if (f->get_basic_blocks().empty()) continue;
        auto entry = f->get_entry_block();
        auto entry_terminator = entry->get_terminator();
        entry->delete_instr(entry_terminator);
        phi2alloca.clear();
        for (auto bb : f->get_basic_blocks()) {
            its.clear();
            auto &instr_list = bb->get_instructions();
            for (auto it = instr_list.begin(); it != instr_list.end(); it++) {
                auto instr = *it;
                if (instr->is_phi()) {
                    its.push_back(it);
                    // std::cout << instr->print() << std::endl;
                    auto alloca = AllocaInst::create_alloca(instr->get_type(), entry);
                    phi2alloca.insert({dynamic_cast<PhiInst *>(instr), alloca});
                }
            }
            for (auto it : its) {
                auto phi = dynamic_cast<PhiInst *>(*it);
                auto load = LoadInst::create_load(phi->get_type(), phi2alloca[phi]);
                load->set_parent(bb);
                phi->replace_all_use_with(load);
                instr_list.insert(it, load);
                instr_list.erase(it);
            }
        }
        entry->add_instruction(entry_terminator);
        for (auto &[phi, alloca] : phi2alloca) {
            for (auto i = 0; i < phi->get_num_operand(); i += 2) {
                auto val = phi->get_operand(i);
                auto bb = dynamic_cast<BasicBlock *>(phi->get_operand(i + 1));
                auto bb_term = bb->get_terminator();
                bb->delete_instr(bb_term);
                auto store = StoreInst::create_store(val, alloca, bb);
                bb->add_instruction(bb_term);
            }
        }
    }
    auto output_file = "phi_elimination.ll"s;
    std::ofstream output_stream;
    output_stream.open(output_file, std::ios::out);
    output_stream << m->print();
}
