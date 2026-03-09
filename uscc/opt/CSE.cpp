/**
 * USCC Compiler
 *
 * A client of Available Expressions Analysis to perform
 * Common Subexpression Elimination.
 */

#include "Passes.h"
#include "AvailableExpressions.h"

using namespace llvm;
using namespace std;

bool enableCSE;

namespace llvm {
void initializeCommonSubexpressionEliminationPass(PassRegistry&);
}
namespace  {
bool Search(Instruction* Inst1, Instruction* Inst2) {
    if (!Inst1 || !Inst2) return false;
    if (Inst1->getOpcode() != Inst2->getOpcode()) return false;
    if (Inst1->getNumOperands() != Inst2->getNumOperands()) return false;
    if (Inst1->getType() != Inst2->getType()) return false; // Ensure result types match

    for (unsigned i = 0; i < Inst1->getNumOperands(); ++i) {
        Value* Op1 = Inst1->getOperand(i);
        Value* Op2 = Inst2->getOperand(i);

        if (Op1 == Op2) continue; // Operands are the same Value*

        LoadInst* LI1 = dyn_cast<LoadInst>(Op1);
        LoadInst* LI2 = dyn_cast<LoadInst>(Op2);
        Constant* C1 = dyn_cast<Constant>(Op1);
        Constant* C2 = dyn_cast<Constant>(Op2);

        if (LI1 && LI2) {
            // Both are loads, compare the pointer operands
            if (LI1->getPointerOperand() != LI2->getPointerOperand()) return false;
        } else if (C1 && C2) {
            // Both are constants, compare their values
            if (C1 != C2) return false;
        } else {
            // Operands are different Value* and not comparable loads/constants
            return false;
        }
    }
    // If all operands match semantically
    return true;
}

}


namespace {
class CommonSubexpressionElimination : public FunctionPass {
public:
    static char ID;
    CommonSubexpressionElimination() : FunctionPass(ID) {
        initializeCommonSubexpressionEliminationPass(*PassRegistry::getPassRegistry());
    }
    bool runOnFunction(llvm::Function &F) override;
    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};
}

char CommonSubexpressionElimination::ID = 0;
INITIALIZE_PASS(CommonSubexpressionElimination, "cse", "Common Subexpression Elimination", false, false)

void CommonSubexpressionElimination::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
    // This pass is a client of Available Expressions Analysis.
    AU.addRequired<AvailableExpressions>();
    AU.setPreservesCFG();
}

FunctionPass *llvm::createCSEPass() {
    return new CommonSubexpressionElimination();
}

bool CommonSubexpressionElimination::runOnFunction(llvm::Function &F) {
    if (F.empty())
        return false;

    AvailableExpressions &ae = getAnalysis<AvailableExpressions>();
    bool globalChanged = false;

    // PA3: Implement

    return globalChanged;
}
