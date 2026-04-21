/**
 * USC Compiler
 * Jianping Zeng (zeng207@purdue.edu)
 * Speculative Loop Invariant Code Motion (SpecLICM).
*/

#include "Passes.h"
#include "llvm/IR/DataLayout.h"
#include <llvm/IR/Constants.h>
#include <llvm/Analysis/BranchProbabilityInfo.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/AliasSetTracker.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <queue>
#include <vector>
#include <stack>

using namespace llvm;

bool enableSpecLICM;
namespace {
// Speculative Loop invariant code motion
class SpecLICM : public LoopPass {
public:
  static char ID;
  SpecLICM() : LoopPass(ID) {
    initializeSpecLICMPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequiredID(LoopSimplifyID);
    AU.addPreservedID(LoopSimplifyID);
    AU.addRequiredID(LCSSAID);
    AU.addPreservedID(LCSSAID);

    // Use the built-in Dominator tree and loop info passes
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfo>();
    // Use alias analysis to hoist loads
    AU.addRequired<AliasAnalysis>();
    AU.addPreserved<AliasAnalysis>();
    // Profile-driven frequent-path identification.
    AU.addRequired<BranchProbabilityInfo>();
  }

private:
  // Data regarding the current loop
  Loop *currLoop;
  // The dominator tree for this loop
  DominatorTree *domTree;
  // Loop information for this loop
  LoopInfo *loopInfo;
  // Denotes whether or not loop has been modified
  bool changed;
  AliasSetTracker *aliasSetTracker;
  // Predheader of the current loop
  BasicBlock *preheader;
  BasicBlock *header;
  Function *fn;
  AliasAnalysis *aa;
  const DataLayout *dl;
  DenseMap<Loop *, AliasSetTracker *> loop2AliasSet;
  std::vector<AllocaInst *> aliasAI;
  DenseSet<Instruction*> insertedLds;
  // Profile-driven members. Populated at the start of each
  // runOnLoop invocation from !prof metadata via BranchProbabilityInfo.
  BranchProbabilityInfo *bpi;
  std::set<BasicBlock*> frequentPath;
private:
  void hoistRegion(DomTreeNode *startNode);
  bool isSafeToHoist(Instruction *inst);
  void hoistInst(Instruction *inst);
  bool inCurrentLoop(BasicBlock *bb) {
    return currLoop->contains(bb);
  }
  // Return true if it is safe to hoist this instruction to the preheader.
  bool canHoistInst(Instruction *inst);

  // Speculative hoist loads up to the preheader.
  void specHoistInst(LoadInst *li);

  // Fill up fixup basic block for each speculatively hoisted load.
  void fillFixupBlocks(LoadInst *ld, BasicBlock *fixupBB);

  bool guaranteedToExecute(Instruction *inst);
  void fixPhiNodes(BasicBlock *newHeader);
  void promoteMemToReg();

  // Walk the ≥80% successor chain from the loop header, populating
  // `frequentPath` with the set of blocks on the frequent path. Uses
  // BranchProbabilityInfo::isEdgeHot (threshold built in at >4/5).
  void computeFrequentPath(Loop *L);

  // Return true if any store on the frequent path may-aliases the given
  // load's pointer operand. Syntactic check only (spec assumes no pointer
  // aliasing — we only worry about explicit writes to the same address).
  bool anyConflictOnFrequentPath(LoadInst *ld);

  // Collect the BFS-closure of loop-body users of `li` whose other operands
  // are all loop-invariant (or other chain members). Returns the chain in
  // program (topological) order, ready to be moveBefore'd to the preheader.
  std::vector<Instruction*> collectDependentChain(LoadInst *li);
};
}

char SpecLICM::ID = 0;
INITIALIZE_PASS_BEGIN(SpecLICM, "speclicm", "Speculative Loop Invariant Code Motion", false, false)
  INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
  INITIALIZE_PASS_DEPENDENCY(LCSSA)
  INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
  INITIALIZE_PASS_DEPENDENCY(LoopInfo)
  INITIALIZE_PASS_DEPENDENCY(BranchProbabilityInfo)
  INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_END(SpecLICM, "speclicm", "Speculative Loop Invariant Code Motion", false, false)

LoopPass *llvm::createSpecLICMPass() {
  return new SpecLICM();
}

bool SpecLICM::runOnLoop(Loop *L, LPPassManager &LPM) {
  // PA5: Implement if necessary
  changed = false;
  // Save the current loop
  currLoop = L;
  // Grab the loop info
  loopInfo = &getAnalysis<LoopInfo>();
  // Grab the dominator tree
  domTree = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  aa = &getAnalysis<AliasAnalysis>();
  // Grab the branch probability info. If !prof metadata was stamped on the
  // Module by Emitter::doSpecLICM, BPI will reflect it; otherwise it falls
  // back to static heuristics (which won't usually identify an inner-body
  // frequent path, making the profile filter a no-op — exactly the
  // zero-regression fallback we want for plain `-fplicm`).
  bpi = &getAnalysis<BranchProbabilityInfo>();
  computeFrequentPath(L);

  // Get the preheader block to move instructions into...
  preheader = L->getLoopPreheader();
  header = currLoop->getHeader();
  assert(preheader && header && "loop must have preheader and header!");
  fn = header->getParent();

  dl = header->getDataLayout();
  // Collect alias information of subloops.
  aliasSetTracker = new AliasSetTracker(*aa);
  for (Loop::iterator LoopItr = L->begin(), LoopItrE = L->end();
       LoopItr != LoopItrE; ++LoopItr) {
    Loop *subLoop = *LoopItr;
    AliasSetTracker *subLoopAST = loop2AliasSet[subLoop];
    assert(subLoopAST && "must already have alias tracking set for the subloop!");
    aliasSetTracker->add(*subLoopAST);
    delete subLoopAST;
    loop2AliasSet.erase(subLoop);
  }

  // Loop over all basic blocks of the current loop and add them to the alias tracking set.
  // Note that we skip the subloops.
  for (auto I = L->block_begin(), E = L->block_end(); I != E; ++I) {
    if (loopInfo->getLoopFor(*I) == currLoop)
      aliasSetTracker->add(**I);
  }

  // Call hoistRegion function with proper argument
  hoistRegion(domTree->getNode(L->getHeader()));

  // Leverage LLVM's PromoteMemToReg to promote stack-allocated variables to be in SSA form.
  promoteMemToReg();

  // Clear up the information for the next iteration.
  currLoop = nullptr;
  preheader = nullptr;
  aliasAI.clear();
  insertedLds.clear();

  if (L->getParentLoop())
    loop2AliasSet[L] = aliasSetTracker;
  else
    delete aliasSetTracker;
  return changed;
}

void SpecLICM::hoistRegion(DomTreeNode *startNode) {
  // PA5: Implement
  BasicBlock *bb = startNode->getBlock();

  if (inCurrentLoop(bb)){

    std::vector<Instruction*> InstList;
    for (Instruction &inst : *bb) {    // copy inst in bb
      InstList.push_back(&inst);
    }

    for (auto *currentInstr : InstList) {
      if (std::find(InstList.begin(), InstList.end(), currentInstr) == InstList.end()) {
        continue; // if moved out of bb, just skip
      }
      if (isSafeToHoist(currentInstr)) {
        if (canHoistInst(currentInstr)) {
          hoistInst(currentInstr);
        }
        else if (enableSpecLICM && isa<LoadInst>(currentInstr)) {
          LoadInst *LI = dyn_cast<LoadInst>(currentInstr);
          if (!frequentPath.empty() && anyConflictOnFrequentPath(LI)) {
            continue;
          }
          specHoistInst(LI);
        }
      }
    }
  }

  std::vector<DomTreeNode*> children = startNode->getChildren();
  for (auto *child : children) {
    hoistRegion(child);
  }

}

bool SpecLICM::isSafeToHoist(llvm::Instruction *inst) {
  // PA5: Implement

  // not have loop invariant operand ==> not safe
  if (!currLoop->hasLoopInvariantOperands(inst)) {
    return false;
  }

  // if it's recorded ==> not safe
  if (insertedLds.count(inst)) {  // LLVM DenseSet can use count.
    return false;
  }

  // neither can be safely speculatively executed nor guaranteed to be executed ==> not safe
  if (!isSafeToSpeculativelyExecute(inst, dl) && !guaranteedToExecute(inst)) {
    return false;
  }

  return true;
}

void SpecLICM::hoistInst(Instruction *inst) {
  // PA5: Implement
  // move to right before preheader BB's terminator
  inst->moveBefore(preheader->getTerminator());
  changed = true;
}

bool SpecLICM::canHoistInst(Instruction *inst) {
  // PA5: Implement
  if (LoadInst *LI = dyn_cast<LoadInst>(inst)) {
    if (!LI->isUnordered()) {
      return false;
    }

    Value* op = LI->getPointerOperand();
    uint64_t loadSize = dl->getTypeStoreSize(LI->getType());
    // not point to constant memory
    // exists conflict
    if (!aa->pointsToConstantMemory(op) && 
        aliasSetTracker->getAliasSetForPointer(op, loadSize, nullptr).isMod()) {
      return false;
    }

    return true;
  }
  else if (isa<BinaryOperator>(inst) ||
           isa<CastInst>(inst) ||
           isa<SelectInst>(inst) ||
           isa<GetElementPtrInst>(inst) ||
           isa<CmpInst>(inst)) {
            return true;
           }

  return false;
}

bool SpecLICM::guaranteedToExecute(Instruction *inst) {
  // PA5: Implement
  SmallVector<BasicBlock*, 8> exitBB;
  currLoop->getExitBlocks(exitBB);
  for (auto *bb : exitBB) {
    // if inst does not dominate exit bb
    if (!domTree->dominates(inst->getParent(), bb)) {
      return false;
    }
  }
  return true;
}

void SpecLICM::specHoistInst(llvm::LoadInst *li) {
  assert(enableSpecLICM && "must enable SpecLICM");
  // PA5: Implement

  auto &ctx = li->getContext();
  Function *F = li->getParent()->getParent();
  Value* op = li->getPointerOperand();
  uint64_t loadSize = dl->getTypeStoreSize(li->getType());

  // (1) create new control flow and header
  // if conflict exists (memory addr is modified during loop)
  // add alias.header & alias.fixup to monitor
  if (aliasSetTracker->getAliasSetForPointer(op, loadSize, nullptr).isMod()) {
    BasicBlock *newHeader = BasicBlock::Create(ctx, "alias.header", F, header);  // inserted before header of loop
    BasicBlock *fixupBB = BasicBlock::Create(ctx, "alias.fixup", F, header);
    header->replaceAllUsesWith(newHeader);

    currLoop->addBasicBlockToLoop(newHeader, loopInfo->getBase());
    currLoop->addBasicBlockToLoop(fixupBB, loopInfo->getBase());
    currLoop->moveToHeader(newHeader);


    // (2) fix PhiNodes and CFG
    fixPhiNodes(newHeader);
    BranchInst::Create(header, fixupBB);


    // (3) Stack allocation for variable "alias"
    AllocaInst *alias = new AllocaInst(
      Type::getInt1Ty(ctx),
      ConstantInt::get(Type::getInt32Ty(ctx), 1), 
      "alias", 
      preheader->getTerminator()
    );
    aliasAI.push_back(alias);    // for later use in PromoteMemToReg 


    // (4) Use StoreInst, LoadInst to operate memory in stack
    Value *constZero = Constant::getNullValue(Type::getInt1Ty(ctx));
    new StoreInst(constZero, alias, preheader->getTerminator());
    new StoreInst(constZero, alias, fixupBB->getTerminator());

    LoadInst *aliasVal = new LoadInst(alias, "alias.ld", newHeader);
    // insertpoint, operator, op1, op2
    ICmpInst *cmp = new ICmpInst(*newHeader, CmpInst::ICMP_EQ, aliasVal, constZero);
    BranchInst::Create(header, fixupBB, cmp, newHeader);  // if-true, if-false, cmp, branchpoint


    // (5) insert checking code after each store that may conflict with the input load
    AliasSet &AS = aliasSetTracker->getAliasSetForPointer(op, loadSize, nullptr);
    std::vector<Instruction*> targetSI;
    std::stack<Instruction*> worklist;
    // identify Instructions in AliasSet
    for (auto I = AS.begin(), E = AS.end(); I != E; ++I) {
      Instruction *aliased = dyn_cast_or_null<Instruction>(I.getPointer());
      if (Instruction *inst = dyn_cast<Instruction>(aliased)) {
        worklist.push(inst);
      }
    }
    // stack-based recursive traversal (DFS-like)
    while (!worklist.empty()) {
      Instruction *curr = worklist.top();
      worklist.pop();

      if (isa<StoreInst>(curr)) {
        targetSI.push_back(curr);
      }
      else if (isa<GetElementPtrInst>(curr) || isa<IntToPtrInst>(curr) || isa<BitCastInst>(curr)) {
        // track def-use chain
        for (User *U : curr->users()) {
          if (Instruction *UI = dyn_cast<Instruction>(U)) {
            worklist.push(UI);
          }
        }
      }
    }
    // icmp BEFORE the store, store-to-alias AFTER the store
    for (auto *inst : targetSI) {
      StoreInst *SI = dyn_cast<StoreInst>(inst);
      
      Value *cmpConflict = new ICmpInst(SI, ICmpInst::ICMP_EQ, op, SI->getPointerOperand(), "alias.cmp");
      Instruction *I = SI->getNextNode();
      if (inst)
        new StoreInst(cmpConflict, alias, I);   // store to alias
      else
        new StoreInst(cmpConflict, alias, SI->getParent()->getTerminator());
    }


    // (6) execute actual hoisting of code
    hoistInst(li);
    if (currLoop->getParentLoop() == nullptr) {  // i.e.  current loop is not nested inside another loop
      std::vector<Instruction*> depChain;
      depChain = collectDependentChain(li);
      for (Instruction *depInst : depChain) {
        depInst->moveBefore(preheader->getTerminator());
      }
    }
    this->header = newHeader;
    fillFixupBlocks(li, fixupBB);
  }
}

void SpecLICM::fillFixupBlocks(LoadInst *ld, BasicBlock *fixupBB) {
  // PA5: Implement
  std::stack<Instruction*> st;
  st.push(ld);

  while (!st.empty()) {
    Instruction *inst = st.top();
    st.pop();

    Instruction *dup = inst->clone();
    dup->setName(inst->getName() + ".fixed");
    dup->insertBefore(fixupBB->getTerminator());
    for (unsigned i = 0 ; i < dup->getNumOperands() ; i++) {
      // Update dup’ operands from the original instructions
      // to the replicas in fixupBB
      // Here, dup might use the values produced
      // by the ld and its dependents (users)
      Value* op = dup->getOperand(i);
      dup->replaceAllUsesWith(op);
    }

    for (User *U : inst->users()) {
      Instruction *user = dyn_cast<Instruction>(U);
      // if user is in preheader
      if (user->getParent() == preheader) {
        st.push(user);
      }
    }

    AllocaInst *ai = nullptr;

    for (User *U : inst->users()) {
      Instruction *user = dyn_cast<Instruction>(U);
      if (user->getParent() != preheader && user->getParent() != fixupBB) {
        if (!ai) {
          ai = new AllocaInst(
            inst->getType(), 
            ConstantInt::get(Type::getInt32Ty(inst->getContext()), 1),
            inst->getName() + ".addr", 
            preheader->getTerminator()
          );
          aliasAI.push_back(ai);

          // new StoreInst(inst, ai, "after inst")
          new StoreInst(inst, ai, inst->getNextNode());
          // new StoreInst(dup, ai, "terminator of fixupBB")
          new StoreInst(dup, ai, fixupBB->getTerminator());
        }

        // li = new LoadInst(ai, "before user")
        LoadInst *li = new LoadInst(ai, inst->getName() + ".ld", user);
        insertedLds.insert(li);
        user->replaceUsesOfWith(inst, li);  // Update user's dependency from inst to the new LoadInst li
      }
    }
    
  }
}

void SpecLICM::fixPhiNodes(BasicBlock *newHeader) {
  // PA5: Implement
  BasicBlock::iterator insertPos = newHeader->getFirstNonPHI();

  for (BasicBlock::iterator it = header->begin() ; it != header->end() ; ) {
    Instruction *i = &*it;
    ++it;    // before changing, iterate next position first
    if (PHINode *pn = dyn_cast<PHINode>(i)) {
      pn->removeFromParent();
      newHeader->getInstList().insert(insertPos, pn);   // insertPos, Instruction*
    }
    else {
      // PHInode must be in top of BB
      break;
    }
  }
}

void SpecLICM::promoteMemToReg() {
  // PA5: Implement
  if (!aliasAI.empty()) {
    Function *F = aliasAI.front()->getParent()->getParent();
    domTree->recalculate(*F);   //  reconstruct the dominator tree of the input function fn
    llvm::PromoteMemToReg(aliasAI, *domTree);
  }
}

void SpecLICM::computeFrequentPath(Loop *L) {
  // PA5: Implement
  frequentPath.clear();
  frequentPath.insert(header);  // header is member function of SpecLICM
  BasicBlock *cur = header;

  while (true) {
    BasicBlock *next = nullptr;
    TerminatorInst *ter = cur->getTerminator();
    unsigned numSuccs = ter->getNumSuccessors();

    if (numSuccs == 1) {
      next = ter->getSuccessor(0);
    }
    else if (numSuccs > 1) {
      for (unsigned i = 0 ; i < numSuccs ; i++) {
        BasicBlock *succ = ter->getSuccessor(i);
        if (bpi->isEdgeHot(cur, succ)) {
          next = succ;
          break;
        }
      }
    }

    if (!next) {
      break;
    }

    // 1. Reaches the backedge: next == header
    // 2. Leaves the current loop: !currLoop->contains(next)
    // 3. Creates a cycle: already in frequentPath
    bool isCycle = false;
    for (auto *BB : frequentPath) {
      if (BB == next) {
        isCycle = true;
        break;
      }
    }
    if (next == header || !currLoop->contains(next) || isCycle) {
      break;
    }

    // Otherwise, insert next into frequentPath, update cur to next
    frequentPath.insert(next);
    cur = next;
  }
}

bool SpecLICM::anyConflictOnFrequentPath(LoadInst *ld) {
  // PA5: Implement
  if (frequentPath.empty()) {
    return false;
  }

  Value* opLoad = ld->getPointerOperand();

  for (auto *bb : frequentPath) {
    for (auto &inst : *bb) {
      if (!isa<StoreInst>(&inst)) {
        continue;
      }
      StoreInst *SI = dyn_cast<StoreInst>(&inst);
      Value *opStore = SI->getPointerOperand();
      if (opLoad == opStore) {
        return true;
      }

      GetElementPtrInst *gepStore = dyn_cast<GetElementPtrInst>(opStore);
      GetElementPtrInst *gepLoad = dyn_cast<GetElementPtrInst>(opLoad);
      if (gepStore && gepLoad) {
        if (gepStore->isIdenticalTo(gepLoad)) {
          return true;
        }
      }
    }
  }
  
  return false;
}

std::vector<Instruction *> SpecLICM::collectDependentChain(LoadInst *li) {
  // BFS over loop-body users of `li`. An instruction joins the chain iff every
  // operand is either (a) the load itself, (b) loop-invariant, or (c) already
  // in the chain. We skip unsupported shapes (loads, stores, phis, terminators)
  // and anything that isn't safe to speculatively execute.
  //
  // Additional safety: if an instruction's value feeds a store whose pointer
  // may-alias the original load's address (a read-modify-write pattern like
  // `X = X + 1`), we must NOT hoist it. Hoisting would cache the add result
  // in the preheader, but the loop body's store would then write that stale
  // cached value instead of `current(X) + 1`. Rejecting these cases keeps the
  // existing SpecLICM read-modify-write semantics intact.
  std::set<Instruction *> inChain;
  std::queue<Instruction *> worklist;

  Value *loadAddr = li->getPointerOperand();
  auto pointerMayAlias = [&](Value *pa, Value *pb) {
    if (pa == pb) return true;
    auto *g1 = dyn_cast<GetElementPtrInst>(pa);
    auto *g2 = dyn_cast<GetElementPtrInst>(pb);
    if (!g1 || !g2) return false;
    if (g1->getPointerOperand() != g2->getPointerOperand()) return false;
    if (g1->getNumIndices() != g2->getNumIndices()) return false;
    for (unsigned i = 0, e = g1->getNumIndices(); i < e; ++i)
      if (g1->getOperand(i + 1) != g2->getOperand(i + 1)) return false;
    return true;
  };
  auto feedsConflictingStore = [&](Instruction *from) {
    for (User *u : from->users()) {
      if (auto *st = dyn_cast<StoreInst>(u)) {
        if (from == st->getValueOperand() &&
            currLoop->contains(st->getParent()) &&
            pointerMayAlias(st->getPointerOperand(), loadAddr)) {
          return true;
        }
      }
    }
    return false;
  };

  auto pushUsersInLoop = [&](Instruction *from) {
    for (User *u : from->users()) {
      if (auto *user = dyn_cast<Instruction>(u)) {
        if (currLoop->contains(user->getParent())) {
          worklist.push(user);
        }
      }
    }
  };
  pushUsersInLoop(li);

  while (!worklist.empty()) {
    Instruction *I = worklist.front();
    worklist.pop();
    if (inChain.count(I)) continue;
    if (isa<LoadInst>(I) || isa<StoreInst>(I) ||
        isa<PHINode>(I) || isa<TerminatorInst>(I)) continue;
    if (!isSafeToSpeculativelyExecute(I, dl)) continue;

    // Reject read-modify-write patterns: if I's value is stored back to an
    // address that may-aliases the original load, hoisting I would cache a
    // stale value in the preheader.
    if (feedsConflictingStore(I)) continue;

    bool ok = true;
    for (Use &u : I->operands()) {
      Value *v = u.get();
      if (v == li) continue;
      if (currLoop->isLoopInvariant(v)) continue;
      if (auto *vi = dyn_cast<Instruction>(v)) {
        if (inChain.count(vi)) continue;
      }
      ok = false;
      break;
    }
    if (!ok) continue;

    inChain.insert(I);
    // Extend the chain transitively: this instruction's users may now also
    // satisfy the "almost invariant" predicate.
    pushUsersInLoop(I);
  }

  // Emit in program (topological) order by walking loop blocks + instructions
  // in their natural sequence, picking up chain members as we go.
  std::vector<Instruction *> ordered;
  for (auto bi = currLoop->block_begin(), be = currLoop->block_end(); bi != be; ++bi) {
    BasicBlock *bb = *bi;
    for (Instruction &I : *bb) {
      if (inChain.count(&I)) ordered.push_back(&I);
    }
  }
  return ordered;
}
