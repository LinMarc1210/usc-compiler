//
// Pass skeleton: Course staff
// Feature: Copy Propagation, tested by testCopyProp.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
//

#include "llvm/Pass.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <map>
#include <vector>

using namespace llvm;

namespace uscc
{
namespace opt
{
	class CopyPropagation : public FunctionPass {
	public:
		static char ID;
		CopyPropagation() : FunctionPass(ID) {}

	// Feature: Copy Propagation, tested by testCopyProp.py
	// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
	// Intra-block store-to-load forwarding for array elements
	virtual bool runOnFunction(Function &F) {
		bool Changed = false;

		// PA4: Implement intra-block store-to-load forwarding
		//
		// In SSA form, regular variables have no allocas — only arrays
		// remain as memory operations (via GEP + store/load).
		//
		// For each basic block, track stores to array elements (via GEP)
		// and forward stored values to subsequent loads from the same address.
		// Handle aliasing conservatively: a store through a variable index
		// invalidates all forwarding entries for that array base.
		// Clear the map on function calls (may modify memory).

		for (auto &bb : F) {
			// maps (base,index) to the last stored LLVM value
			// { {base, index}, value }
			std::map<std::pair<Value*, Value*>, Value*> avail;
			std::vector<LoadInst*> dead;   // redundant load instructions to erase later

			// GetElementPtr (GEP): array index calculation is done by GEP
			// only do ptr offset, don't do store/load
			// base: base ptr address
			// index: offset from base

			for (auto &inst : bb) {
				// if Inst is a store to a single-index GetElementPtr
				// StoreInst ex. ====> store i32 %val, i32* %ptr
				// GEP ex. ====> %ptr = getelementptr <type>, <base>, <index 0>, <index 1>, ...
				if (auto *SI = dyn_cast<StoreInst>(&inst)) {
					if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand())) {

						Value *base = GEP->getPointerOperand();
						Value *index = GEP->getOperand(GEP->getNumOperands() - 1);   // index operand is the last operand
						Value *val = SI->getValueOperand();

						if (!isa<ConstantInt>(index)) {
							for (auto it = avail.begin() ; it != avail.end() ; ) {
								if ((it->first).first == base) {
									it = avail.erase(it);    // return the following iterator
								}
								else {
									it++;  // when not removing, move to next iterator
								}
							}
						}
						
						avail[{base, index}] = val;
					}
				}
				else if (auto *LI = dyn_cast<LoadInst>(&inst)) {
					if (auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand())) {

						Value *base = GEP->getPointerOperand();
						Value *index = GEP->getOperand(GEP->getNumOperands() - 1);   // index operand is the last operand

						if (avail.count({base, index})) {
							Value *replacement = avail[{base, index}];
							LI->replaceAllUsesWith(replacement);
							dead.push_back(LI);

							Changed = true;
						}
					}
				}
				else if (isa<CallInst>(inst)) {
					avail.clear();    // the call may modify memory
				}
			}

			for (auto *D : dead) {    // LoadInst*
				Value *G = D->getPointerOperand();
				D->eraseFromParent();
				if (auto *GEP = dyn_cast<GetElementPtrInst>(G)) {
					if (GEP->use_empty()) {
						GEP->eraseFromParent();
					}
				}
			}
		}

		return Changed;
	}
};

char CopyPropagation::ID = 0;
static RegisterPass<CopyPropagation> X("-copyprop",
                                             "Simple Copy Propagation via Stores");

}
}

namespace llvm
{
FunctionPass *createCopyPropagationPass()
{
    return new uscc::opt::CopyPropagation();
}
}
