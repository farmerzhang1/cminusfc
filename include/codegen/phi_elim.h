#ifndef _PHI_ELIM_H_
#define _PHI_ELIM_H_
#include <map>

#include "Module.h"
#include "Instruction.h"
class PhiElim {
private:
    Module *m;
    std::map<PhiInst*, AllocaInst*> phi2alloca;
public:
    PhiElim(Module *m);
    void run();
};
#endif
