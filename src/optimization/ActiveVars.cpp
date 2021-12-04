#include <functional>

#include "ActiveVars.hpp"
void ActiveVars::run() {
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : m_->get_functions()) {
        func_ = func;
        func_->set_instr_name(); // ?
        calc_def_and_use();
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            auto &bbs = func_->get_basic_blocks();
            bool changed;
            live_in.clear();
            live_out.clear();
            do {
                changed = false;
                // 假装是在深度优先逆序遍历
                for (auto itr = bbs.rbegin(); itr != bbs.rend(); ++itr) {
                    auto &in = live_in[*itr];
                    auto &out = live_out[*itr];
                    auto size_before = out.size();
                    // live_out[*itr].clear();
                    for (auto suc : (*itr)->get_succ_basic_blocks()) {
                        for (auto var : live_in[suc]) {
                            bool var_from_phi = false;
                            for (auto [_, vars] : phiuses[suc]) var_from_phi |= vars.contains(var);
                            if (!var_from_phi || var_from_phi && phiuses[suc][*itr].contains(var)) {
                                out.insert(var);
                            }
                        }
                        // out.merge(live_in[suc]);
                    }
                    changed |= out.size() != size_before;
                    in = out;
                    for (auto def : def[*itr]) in.erase(def);
                    for (auto use : use[*itr]) in.insert(use);
                }
            } while (changed);
            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return;
}

std::string ActiveVars::print() {
    std::string active_vars;
    active_vars += "{\n";
    active_vars += "\"function\": \"";
    active_vars += func_->get_name();
    active_vars += "\",\n";

    active_vars += "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second) {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    },\n";

    active_vars += "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second) {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}

void ActiveVars::calc_def_and_use() {
    for (auto bb : func_->get_basic_blocks()) {
        auto &uses = use[bb];
        auto &defs = def[bb];
        for (auto instr : bb->get_instructions()) {
            // how to distinguish between constants and variables? (get_name.empty)
            // FIXME: handle phi (here?)
            for (auto rand : instr->get_operands()) {
                if (!defs.contains(rand) && !rand->get_name().empty() && !rand->get_type()->is_label_type())
                    uses.insert(rand);
            }
            if (instr->is_phi()) {
                auto &curbb = phiuses[bb];
                for (int i = 0; i < instr->get_num_operand(); i += 2) {
                    auto otherbb = dynamic_cast<BasicBlock*>(instr->get_operand(i+1)); // must succeed, don't check for nullptr
                    curbb[otherbb].insert(instr->get_operand(i));
                }
            }
            if (instr->is_call()) uses.erase(instr->get_operand(0)); // delete first operand (pointer to function)
            if (!uses.contains(instr) && !instr->get_type()->is_void_type())
                defs.insert(instr);
        }
        std::cout << "use list for " << bb->get_name() << std::endl;
        for (auto u : uses) std::cout << u->get_name() << std::endl;
        std::cout << "and def list" << std::endl;
        for (auto d : defs) std::cout << d->print() << std::endl;
    }
}
