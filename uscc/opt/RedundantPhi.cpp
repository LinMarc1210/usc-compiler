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

// Algorithm 5
private:
  void removeRedundantPhis(std::set<PHINode*>& phiFunctions);
  void processSCC(std::set<llvm::Value *>& scc, std::set<PHINode*>& phiFunctions);  // scc type is based on getSCCs in Graph.h
  void replaceSCCByValue(std::set<Value*>& scc, Value* val, std::set<PHINode*>& currentPhiSet);
};



char RedundantPhiRemoval::ID = 0;

void RedundantPhiRemoval::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}


void RedundantPhiRemoval::removeRedundantPhis(std::set<PHINode*>& phiFunctions) {
  Graph graph;   // use actual instance

  // 1. build induced graph
  // (add node)
  for (auto &phi : phiFunctions) {
    graph.getOrAddToGraph(phi); // upcasting: PHINode* -> Value*
  }
  // (add egde)
  for (auto &phi : phiFunctions) {
    GraphNode *src = graph.getOrAddToGraph(phi);
    // only make egde in SCC iff dest is also in set P (Lemma 1)
    for (unsigned i = 0 ; i < phi->getNumIncomingValues() ; i++) {
      Value *op = phi->getIncomingValue(i);
      if (PHINode *dest = dyn_cast<PHINode>(op)) {
        if (phiFunctions.count(dest)) {
          src->addEdge(graph.getOrAddToGraph(dest));  // find dest in Graph, then add edge (src, dest)
        }
      }
    }
  }

  // 2. getSCCs (already in reverse topological order)
  std::vector<std::set<Value*>> sccs = graph.getSCCs();

  // 3. deal with every SCC
  for (auto &scc : sccs) {
    processSCC(scc, phiFunctions);
  }
}


void RedundantPhiRemoval::processSCC(std::set<llvm::Value *>& scc, std::set<PHINode*>& phiFunctions) {
  std::set<PHINode*> inner;
  std::set<Value*> outerOps;

  for (auto &v : scc) {   // Value * (but actually they are all phi node)
    PHINode* phi = dyn_cast<PHINode>(v);   // cast to phi node
    bool isInner = true;
    for (unsigned i = 0 ; i < phi->getNumIncomingValues() ; i++) {
      Value *op = phi->getIncomingValue(i);
      if (!scc.count(op)) {   // operand not in scc
        outerOps.insert(op);
        isInner = false;
      }
    }
    if (isInner) {
      inner.insert(phi);
    }
  }

  if (outerOps.size() == 1) {
    llvm::Value* uniqueVal = *outerOps.begin();   // outerOps.pop()
    replaceSCCByValue(scc, uniqueVal, phiFunctions);
  }
  else if (outerOps.size() > 1) {
    removeRedundantPhis(inner);
  }
}


void RedundantPhiRemoval::replaceSCCByValue(std::set<Value*>& scc, Value* val, std::set<PHINode*>& phiFunctions) {
	for (Value *v : scc) {
    PHINode* phi = dyn_cast<PHINode>(v);
    phi->replaceAllUsesWith(val);
    phiFunctions.erase(phi);  // avoid operation in removeRedundantPhis read the unused memory
    phi->eraseFromParent();
  }
}


bool RedundantPhiRemoval::runOnFunction(Function &F) {

  // PA4: Implement
  //
  // This is the entry point of the redudant phi removal pass.
  // Your implementation should start here.
  // Feel free to add functions to the RedundantPhiRemoval class.

  // Trivial PHINode: if all the operands, except itself, all point to a same value S, then this phi node is redundant.
  // ex. x2 = phi(x1, x1) ====> x2 can be replaced by x1
  // ex. x2 = phi(x1, x2) ====> except x2, it only points to x1. x2 can still be replaced by x1.
  
  // Recursive redundant: delete a redundant phi node can also make other nodes using it become trivial as well.

  std::set<PHINode*> allPhis;   // collect original PHINode in function

  for (auto &bb : F) {
    for (auto &inst : bb) {
      if (PHINode *phi = dyn_cast<PHINode>(&inst)) {
        allPhis.insert(phi);
      }
    }
  }

  if (allPhis.empty()) {
    return false;
  }

  removeRedundantPhis(allPhis);
  return true;
}


} // namespace opt
} // namespace uscc

FunctionPass *createRedundantPhiRemovalPass() {
  return new uscc::opt::RedundantPhiRemoval();
}

} // namespace llvm
