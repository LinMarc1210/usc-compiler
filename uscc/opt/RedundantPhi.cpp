#include "Graph.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"

namespace llvm {
namespace uscc {
namespace opt {

class RedundantPhiRemoval : public FunctionPass {
public:
  static char ID;
  RedundantPhiRemoval() : FunctionPass(ID) {}
  virtual bool runOnFunction(llvm::Function &F) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};



char RedundantPhiRemoval::ID = 0;

void RedundantPhiRemoval::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}


bool RedundantPhiRemoval::runOnFunction(Function &F) {

  // PA4: Implement
  //
  // This is the entry point of the redudant phi removal pass.
  // Your implementation should start here.
  // Feel free to add functions to the RedundantPhiRemoval class.


  return true;
}


} // namespace opt
} // namespace uscc

FunctionPass *createRedundantPhiRemovalPass() {
  return new uscc::opt::RedundantPhiRemoval();
}

} // namespace llvm
