/**
 * USCC Compiler
 * Jianping Zeng (zeng207@purdue.edu)
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

    // Step #2: calculate DEF/USE set for each basic block

    // Step #3: compute post order traversal.

    // Step #4: iterate over control flow graph of the input function until the fixed point.
    unsigned cnt = 0;

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
    // TODO: Implement
    return false;
}
