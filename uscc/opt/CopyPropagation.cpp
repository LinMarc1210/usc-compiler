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
