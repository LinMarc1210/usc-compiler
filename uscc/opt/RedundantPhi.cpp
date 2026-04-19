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

private:
  void addPhiNodesToGraph(PHINode *PN, const std::set<Value *> &InnerPNSet,
                          llvm::uscc::Graph &G);
  bool removeRedundantPhis(std::set<Value *>);
  bool processSCC(std::set<Value *>);
  void replaceSCCByValue(std::set<Value *>, Value *V);
};

void RedundantPhiRemoval::addPhiNodesToGraph(
    llvm::PHINode *PN, const std::set<Value *> &InnerPNSet,
    llvm::uscc::Graph &G) {
  auto pNode = G.getOrAddToGraph(PN);
  for (unsigned i = 0; i < PN->getNumIncomingValues(); ++i) {
    auto I = PN->getIncomingValue(i);
    if (isa<PHINode>(I) && InnerPNSet.count(I)) {
      PHINode *childPN = cast<PHINode>(I);
      auto cNode = G.getOrAddToGraph(childPN);
      pNode->addEdge(cNode);
    }
  }
}

bool RedundantPhiRemoval::processSCC(std::set<Value *> SCC) {
  bool changed = false;

  if (true) {
    std::set<Value *> inner;
    std::set<Value *> outerOps;
    for (auto N : SCC) {
      bool isInner = true;
      auto PN = cast<PHINode>(N);
      for (unsigned i = 0; i < PN->getNumIncomingValues(); ++i) {
        auto I = PN->getIncomingValue(i);
        if (SCC.count(I) == 0) {
          outerOps.insert(I);
          isInner = false;
        }
      }
      if (isInner)
        inner.insert(PN);
    }

    if (outerOps.size() == 1) {
      replaceSCCByValue(SCC, *outerOps.begin());
      outerOps.clear();
      changed = true;
    } else if (outerOps.size() > 1 && !inner.empty()) {
      changed |= removeRedundantPhis(inner);
    }
  }
  return changed;
}

char RedundantPhiRemoval::ID = 0;

void RedundantPhiRemoval::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool RedundantPhiRemoval::removeRedundantPhis(std::set<Value *> Vs) {
  llvm::uscc::Graph G;
  for (auto V : Vs) {
    addPhiNodesToGraph(cast<PHINode>(V), Vs, G);
  }

  std::vector<std::set<Value *>> SCCs = G.getSCCs();

  bool changed = false;

  for (auto & scc : SCCs) {
    changed |= processSCC(scc);
  }
  return changed;
}

void RedundantPhiRemoval::replaceSCCByValue(std::set<Value *> PNs,
                                                Value *newVal) {
  for (auto V : PNs) {
    auto PN = cast<PHINode>(V);
    PN->replaceAllUsesWith(newVal);
    PN->eraseFromParent();
  }
}

bool RedundantPhiRemoval::runOnFunction(Function &F) {
  // PA4: Implement
  std::set<Value *> PhiNodes;

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (isa<PHINode>(&I))
        PhiNodes.insert(&I);
    }
  }

  return removeRedundantPhis(PhiNodes);
}

} // namespace opt
} // namespace uscc

FunctionPass *createRedundantPhiRemovalPass() {
  return new uscc::opt::RedundantPhiRemoval();
}

} // namespace llvm
