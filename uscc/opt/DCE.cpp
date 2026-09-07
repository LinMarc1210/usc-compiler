/**
 * USCC Compiler
 * Pass skeleton and findDeadDefinitions helper: Jianping Zeng (zeng207@purdue.edu)
 * Feature: Dead Code Elimination, tested by testLiveness.py
 * Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
 *
 * A client of liveness to perform dead code elimination.
 * Note that this pass is different from the Dead Block Elimination that analyzes the reachability of each basic block
 * in a control flow graph (CFG) to determine if the basic block is dead.
*/

#include "Passes.h"
#include "Liveness.h"

using namespace llvm;
namespace
{
class DeadCodeElimination : public FunctionPass
{
public:
    static char ID;
    DeadCodeElimination() : FunctionPass(ID) {}
    virtual bool runOnFunction(llvm::Function &F) override;
    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
private:
    void findDeadDefinitions(Instruction *inst, std::set<Instruction *> &dead);
};
}
char DeadCodeElimination::ID = 0;
FunctionPass *llvm::createDCEPass()
{
    return new DeadCodeElimination();
}

void DeadCodeElimination::getAnalysisUsage(llvm::AnalysisUsage &AU) const
{
    AU.addRequired<Liveness>();
    AU.addPreserved<Liveness>();
    // This function call indicates DCE pass does not
    // (1) add/delete basic blocks;
    // (2) remove terminator instruction at the end of each basic block.
    AU.setPreservesCFG();
}

void DeadCodeElimination::findDeadDefinitions(llvm::Instruction *inst,
                                              std::set<Instruction *> &dead)
{
    for (unsigned i = 0, e = inst->getNumOperands(); i < e; ++i)
    {
        Value *val = inst->getOperand(i);
        Instruction *src;
        if ((src = dyn_cast_or_null<Instruction>(val)) != nullptr &&
            src->hasOneUse() && src->getOpcode() != Instruction::Call)
        {
            dead.insert(src);
            findDeadDefinitions(src, dead);
        }
    }
}

// Feature: Dead Code Elimination, tested by testLiveness.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Removes dead stores, dead compute instructions, and unused allocas
bool DeadCodeElimination::runOnFunction(llvm::Function &F)
{
    if (F.empty())
        return false;

    Liveness &lv = getAnalysisID<Liveness>(&Liveness::ID);

    // PA3

    // Step #1: get a set of dead instructions and remove them.
    // TODO: Implement
    bool changed = true;
    std::set<Instruction*> deadVars;
    while (changed) {
        changed = false;

        // 1. Mark step
        for (auto &bb : F) {
            for (auto &inst : bb) {
                if (lv.isDead(inst)) {
                    deadVars.insert(&inst);
                    if (StoreInst *SI = dyn_cast<StoreInst>(&inst)) {
                        Value *val = SI->getValueOperand();
                        if (Instruction *II = dyn_cast<Instruction>(&inst)) {
                            deadVars.insert(II);
                        }
                    }
                    // recursively find all dependent dead code, add to deadVars
                    findDeadDefinitions(&inst, deadVars);
                }
            }
        }

        // 2. sweep step
        if (!deadVars.empty()) {
            for (auto &inst : deadVars) {    // llvm::Instruction *
                if (!inst->use_empty()) {
                    inst->replaceAllUsesWith(
                        llvm::UndefValue::getNullValue(inst->getType())
                    );
                }
                inst->eraseFromParent();  // no need to iterate early because this is C++ set
            }
            lv.releaseMemory();
            lv.runOnFunction(F);
            changed = true;
            deadVars.clear();
        }
    }


    // Step #2: remove the Alloca instructions having no uses.
    // TODO: Implement
    for (auto &bb : F) {
        auto it = bb.begin();    // start iterator
        while (it != bb.end()) {
            Instruction &inst = *it++;    // next instruction (same woth *(it++) )
            if (inst.getOpcode() == llvm::Instruction::Alloca && inst.use_empty()) {
                inst.eraseFromParent();    // need to save next iterator first, because bb will not record ext iterator automatically.
            }
        }
    }

    return false;
}