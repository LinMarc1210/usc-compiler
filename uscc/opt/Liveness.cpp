/**
 * USCC Compiler
 * Pass skeleton: Jianping Zeng (zeng207@purdue.edu)
 * Feature: Backward Liveness Analysis, tested by testLiveness.py
 * Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
 *
 * An iterative backward liveness analysis.
 * This pass intends to compute a set of live-out/in variables for each LLVM basic block
 * and maintain a set of LLVM instructions that are dead---not used by any following others.
*/

#include "Liveness.h"

using namespace std;
using namespace llvm;

bool enableLiveness;

char Liveness::ID = 0;
INITIALIZE_PASS(Liveness, "liveness", "Liveness Analysis", true, true)

FunctionPass *llvm::createLivenessPass()
{
    return new Liveness();
}

namespace llvm
{
std::set<StringRef> operator+(const std::set<StringRef> & lhs, const std::set<StringRef> & rhs)
{
    std::set<StringRef> ret = lhs;
    for (auto & i : rhs)
        ret.insert(i);
    return ret;
}

void operator+=(std::set<StringRef> & lhs, const std::set<StringRef> & rhs)
{
    for (auto & i : rhs)
        lhs.insert(i);
}

std::set<StringRef> operator-(const std::set<StringRef> & lhs, const std::set<StringRef> & rhs)
{
    std::set<StringRef> ret = lhs;
    for (auto & i : rhs)
        ret.erase(i);
    return ret;
}

void operator-=(std::set<StringRef> & lhs, const std::set<StringRef> & rhs)
{
    for (auto & i : rhs)
        lhs.erase(i);
}
}

void computePostOrder(BasicBlock *entry, set<BasicBlock *> &visited, deque<BasicBlock *> &order)
{
    visited.insert(entry);
    auto succItr = llvm::succ_begin(entry), end = llvm::succ_end(entry);
    for (; succItr != end; ++succItr)
        if (!visited.count(*succItr))
        computePostOrder(*succItr, visited, order);
    order.push_back(entry);
}

// Feature: Liveness Analysis, tested by testLiveness.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Iterates backward dataflow equations to find live variables at each basic block
bool Liveness::runOnFunction(Function &F)
{
    if (F.empty())
        return false;
    BasicBlock &frontBB = F.front();
    BasicBlock &endBB = F.back();
    assert(!frontBB.empty() && !endBB.empty() && "the front/end basic block must not be empty!");
    // The OUT set of the last block is empty.
    bb2Out[&endBB] = std::set<StringRef>();

    // PA3: Implement
    // Step #1: identify program variables.
    for (auto &bb : F) {
        for (auto &inst : bb) {
            if (inst.getOpcode() == llvm::Instruction::Alloca) {
                if (inst.hasName()) {
                    namedVars.insert(inst.getName());
                }
            }
        }
    }

    // Step #2: calculate DEF/USE set for each basic block
    std::map<BasicBlock *, std::set<StringRef>> bb2Use, bb2Def;
    for (auto &bb : F) {
        set<StringRef> USE = {};
        set<StringRef> DEF = {};
        for (auto it = bb.rbegin() ; it != bb.rend() ; it++) {
            // LoadInst
            if (LoadInst* LI = dyn_cast_or_null<LoadInst>(&*it)) {
                if (namedVars.count(LI->getPointerOperand()->getName())) {  // found
                    USE.insert(LI->getPointerOperand()->getName());
                    DEF.erase(LI->getPointerOperand()->getName());
                }
            }
            // StoreInst
            else if (StoreInst* SI = dyn_cast_or_null<StoreInst>(&*it)) {
                if (namedVars.count(SI->getPointerOperand()->getName())) {  // found
                    USE.erase(SI->getPointerOperand()->getName());
                    DEF.insert(SI->getPointerOperand()->getName());
                }
            }
        }
        
        // save to bb2Use, bb2Def
        bb2Use[&bb] = USE;
        bb2Def[&bb] = DEF;
    }

    // Step #3: compute post order traversal.
    set<BasicBlock *> visited;
    deque<BasicBlock *> postorder;
    computePostOrder(&(F.front()), visited, postorder);

    // Step #4: iterate over control flow graph of the input function until the fixed point.
    unsigned cnt = 0;
    bool changed = true;
    for (auto &bb : F) {
        bb2In[&bb] = {};
    }
    while (changed) {
        changed = false;
        for (auto &bb : postorder) {    // BasicBlock *
            set<StringRef> old_IN = bb2In[bb];
            // for all direct successor basic block Y
            // OUT[X] = Union(IN[Y])
            for (auto it = succ_begin(bb), end = succ_end(bb) ; it != end ; it++) {
                bb2Out[bb] += bb2In[*it];
            }
            // IN[X] = USE[X] + (OUT[X] - DEF[X])
            bb2In[bb] = bb2Use[bb] + (bb2Out[bb] - bb2Def[bb]);

            // fixed_point check
            if (old_IN != bb2In[bb]) {
                changed = true;
            }
        }
        // iteration counter
        cnt++;
    }

    // Step #5: output IN/OUT set for each basic block.
    if (enableLiveness)
    {
        llvm::outs() << "********** Live-in/Live-out information **********\n";
        llvm::outs() << "********** Function: " << F.getName().str() << ", analysis iterates " << cnt << " times\n";
        for (auto &bb : F)
        {
            llvm::outs() << bb.getName() << ":\n";
            llvm::outs() << "  IN:";
            for (auto &var : bb2In[&bb])
                llvm::outs() << " " << var.substr(0, var.size() - 5);
            llvm::outs() << "\n";
            llvm::outs() << "  OUT:";
            for (auto &var : bb2Out[&bb])
                llvm::outs() << " " << var.substr(0, var.size() - 5);
            llvm::outs() << "\n";
        }
    }
    // Liveness does not change the input function at all.
    return false;
}

bool Liveness::isDead(llvm::Instruction &inst)
{
    BasicBlock *bb = inst.getParent();
    if (!bb)
        return true;
    if (!bb2Out.count(bb))
        return true;

    // PA3: Implement
    StoreInst *is_SI = dyn_cast<StoreInst>(&inst);
    if (!is_SI) 
        return false;    // non Store inst
    StringRef varName = is_SI->getPointerOperand()->getName();
    if (!namedVars.count(varName)) 
        return false;    // other store

    
    // TODO: Implement
    set<StringRef> LiveWithinBB = bb2Out[bb];
    for (auto it = bb->rbegin() ; it != bb->rend() ; it++) {
        Instruction *curr = &*it;    // current instruction
        if (curr == &inst) {         // target instruction
            if (!LiveWithinBB.count(varName))    // not found, so it is dead, return true
                return true;
            else return false;
        }
        // Update LiveWithinBB by LoadInst, StoreInst
        if (LoadInst* LI = dyn_cast_or_null<LoadInst>(&*it)) {
            LiveWithinBB.insert(LI->getPointerOperand()->getName());
        }
        else if (StoreInst* SI = dyn_cast_or_null<StoreInst>(&*it)) {
            LiveWithinBB.erase(SI->getPointerOperand()->getName());
        }
    }
    
    return false;
}