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

	virtual bool runOnFunction(Function &F) {
		bool Changed = false;

		// Intra-block store-to-load forwarding for array memory ops.
		// In SSA form, regular variables have no allocas — only arrays
		// remain as memory operations (via GEP + store/load).
		for (auto &BB : F) {
			// Map from (base ptr, index) -> last stored value
			std::map<std::pair<Value*, Value*>, Value*> avail;
			std::vector<Instruction*> dead;

			for (auto &I : BB) {
				if (auto *SI = dyn_cast<StoreInst>(&I)) {
					// Only handle single-index GEPs (array element access)
					auto *G = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
					if (!G || G->getNumIndices() != 1) continue;
					Value *b = G->getPointerOperand(), *i = G->getOperand(1);

					// Variable index may alias any element — invalidate all
					// entries with the same base pointer
					if (!isa<ConstantInt>(i)) {
						for (auto it = avail.begin(); it != avail.end();) {
							it = (it->first.first == b) ? avail.erase(it) : std::next(it);
						}
					}
					avail[{b, i}] = SI->getValueOperand();

				} else if (auto *LI = dyn_cast<LoadInst>(&I)) {
					auto *G = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
					if (!G || G->getNumIndices() != 1) continue;

					// If we have a stored value for this (base, index),
					// forward it and mark the load as dead
					auto it = avail.find({G->getPointerOperand(), G->getOperand(1)});
					if (it != avail.end()) {
						LI->replaceAllUsesWith(it->second);
						dead.push_back(LI);
						Changed = true;
					}

				} else if (isa<CallInst>(&I)) {
					// Calls may modify memory — invalidate everything
					avail.clear();
				}
			}

			// Erase dead loads and their unused feeding GEPs
			for (auto *D : dead) {
				auto *G = dyn_cast<GetElementPtrInst>(cast<LoadInst>(D)->getPointerOperand());
				D->eraseFromParent();
				if (G && G->use_empty()) {
					G->eraseFromParent();
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
