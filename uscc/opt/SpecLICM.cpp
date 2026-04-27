/**
 * USC Compiler
 * Jianping Zeng (zeng207@purdue.edu)
 * Speculative Loop Invariant Code Motion (SpecLICM) — profile-gated
 * speculative LICM. Hoists loads whose aliasing stores live on the
 * infrequent path (as told by !prof branch-weight metadata, read via
 * BranchProbabilityInfo), plus their dependent instruction chains.
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
    // Profile-driven frequent-path identification (Q1 extension).
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
  // Profile-driven members (Q1 extension). Populated at the start of each
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
  // PA5: Implement
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
  BasicBlock *bb = startNode->getBlock();
  if (!inCurrentLoop(bb))
    return;

  // Only deal with the blocks in the current loop. Any basic blocks inside
  // subloops should already been processed.
  if (loopInfo->getLoopFor(bb) == currLoop) {
    std::vector<Instruction *> snapshot;
    for (Instruction &Inst : *bb) snapshot.push_back(&Inst);
    for (Instruction *inst : snapshot) {
      if (inst->getParent() != bb) continue;
      if (isSafeToHoist(inst)) {
        LoadInst *li = dyn_cast<LoadInst>(inst);
        if (canHoistInst(inst)) {
          hoistInst(inst);
        } else if (enableSpecLICM && li) {
          if (!frequentPath.empty() && anyConflictOnFrequentPath(li))
            continue;
          specHoistInst(li);
        }
      }
    }
  }
  for (auto &child : startNode->getChildren())
    hoistRegion(child);
}

bool SpecLICM::isSafeToHoist(llvm::Instruction *inst) {
  return currLoop->hasLoopInvariantOperands(inst) &&
      !insertedLds.count(inst) &&
      (isSafeToSpeculativelyExecute(inst, dl) || guaranteedToExecute(inst));
}

void SpecLICM::hoistInst(Instruction *inst) {
  inst->moveBefore(preheader->getTerminator());
  changed = true;
}

bool SpecLICM::canHoistInst(Instruction *inst) {
  // Extra check for load.
  if (LoadInst *li = dyn_cast<LoadInst>(inst)) {
    // Don't hoist volatile or atomic load.
    if (!li->isUnordered())
      return false;

    // Hoist the load if it reads from constant memory.
    if (aa->pointsToConstantMemory(li->getOperand(0)))
      return true;

    // Speculatively hoist the load if it is must| may aliased with stores in the loop.
    // If so, we need to insert some run-time fix-up code at the header of the current
    // loop.
    uint64_t size = 0;
    if (li->getType()->isSized())
      aa->getTypeStoreSize(li->getType());
    return !aliasSetTracker->getAliasSetForPointer(li->getPointerOperand(),
                                                   size, nullptr).isMod();
  }

  return (inst->isBinaryOp() || inst->isCast() || inst->getOpcode() == Instruction::Select ||
      inst->getOpcode() == Instruction::GetElementPtr || inst->getOpcode() == Instruction::FCmp ||
      inst->getOpcode() == Instruction::ICmp);
}

bool SpecLICM::guaranteedToExecute(Instruction *inst) {
  // If the basic block enclosing the given instruction dominates all
  // the exit blocks of the current loop, the instruction is guaranteed
  // to be executed.
  SmallVector<BasicBlock *, 8> exitBlocks;
  currLoop->getExitBlocks(exitBlocks);
  if (exitBlocks.empty())
    return false;

  for (auto &exitBB : exitBlocks) {
    if (!domTree->dominates(inst->getParent(), exitBB))
      return false;
  }

  return true;
}

void SpecLICM::specHoistInst(llvm::LoadInst *li) {
  assert(enableSpecLICM && "must enable SpecLICM");

  // Only do it when the pointer operand of the load is defined outside the current loop.

  // Speculatively hoist the load if it is must | may aliased with stores in the loop.
  // If so, we need to insert some run-time fix-up code at the header of the current
  // loop.
  uint64_t size = 0;
  if (li->getType()->isSized())
    aa->getTypeStoreSize(li->getType());
  AliasSet &as = aliasSetTracker->getAliasSetForPointer(li->getPointerOperand(),
                                                        size, nullptr);
  if (as.isMod()) {
    LLVMContext &ctx = li->getContext();
    // Create a new header block if absent.
    BasicBlock *newHeader = BasicBlock::Create(ctx, "alias.header", header->getParent(), header);
    // Create a fix-up basic block.
    BasicBlock *fixupBB = BasicBlock::Create(ctx, "alias.fixup", header->getParent(), header);

    // Replace all uses of the header block by the new header block.
    header->replaceAllUsesWith(newHeader);
    currLoop->addBasicBlockToLoop(newHeader, loopInfo->getBase());
    currLoop->addBasicBlockToLoop(fixupBB, loopInfo->getBase());
    currLoop->moveToHeader(newHeader);

    // Fix those broken phi nodes in the old header due to inserted alias.header
    fixPhiNodes(newHeader);

    // Branch to the old header at the end of fixup basic block.
    BranchInst::Create(header, fixupBB);

    // Create a phi node to hold the comparison result in the new loop header
    Type *ty = Type::getInt1Ty(ctx);
    AllocaInst *ai = new AllocaInst(ty,
                                    ConstantInt::get(Type::getInt32Ty(ctx), 1),
                                    "alias", preheader->getTerminator());
    aliasAI.push_back(ai);

    // insert "alias.phi = 0" at the end of the preheader.
    Value *zero = Constant::getNullValue(ty);
    new StoreInst(zero, ai, preheader->getTerminator());
    // Insert "alias = 0" to the fixup block.
    new StoreInst(zero, ai, fixupBB->getTerminator());

    // Branch to the old header and fixup block depending on the comparison result.
    LoadInst *ldCond = new LoadInst(ai, "alias.ld", newHeader);
    Instruction *neg = ICmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_EQ, ldCond, zero, "neg.alias", newHeader);
    BranchInst::Create(header, fixupBB, neg, newHeader);

    SmallVector<Instruction *, 8> stack;
    DenseSet<Instruction *> visited;
    for (auto I = as.begin(), E = as.end(); I != E; ++I) {
      stack.clear();
      visited.clear();
      Instruction *aliased = dyn_cast_or_null<Instruction>(I.getPointer());
      if (aliased) {
        switch (aliased->getOpcode()) {
          case Instruction::Store: {
            StoreInst *si = dyn_cast<StoreInst>(aliased);
            // Create a comparison before the store.
            Instruction *cmp = CmpInst::Create(Instruction::ICmp,
                                               ICmpInst::ICMP_EQ,
                                               li->getPointerOperand(),
                                               si->getPointerOperand(),
                                               "alias.cmp", si);
            new StoreInst(cmp, ai, si);
            break;
          }
          case Instruction::GetElementPtr:
          case Instruction::IntToPtr:
          case Instruction::BitCast: {
            // The value produced by gep might be indirectly consumed by a store.
            // Identify the following patterna.
            //   %arrayidx1 = getelementptr inbounds i32* %a, i64 %idxprom
            //   store i32 %res.0, i32* %arrayidx1, align 4
            //
            //   %2 = inttoptr i64 %add to i32*
            //   store i32 %res.0, i32* %2, align 4
            // Then insert a comparison instruction after the store.
            stack.push_back(aliased);
            visited.insert(aliased);
            visited.insert(li);
            while (!stack.empty()) {
              Instruction *inst = stack.back();
              stack.pop_back();
              // Test if inst is a store
              if (StoreInst *si = dyn_cast<StoreInst>(inst)) {
                // Create a comparison before the store.
                Instruction *cmp = CmpInst::Create(Instruction::ICmp,
                                                   ICmpInst::ICMP_EQ,
                                                   li->getPointerOperand(),
                                                   si->getPointerOperand(),
                                                   "alias.cmp", si);
                new StoreInst(cmp, ai, si);
              }
              for (Use &u : inst->uses()) {
                Instruction *user = dyn_cast<Instruction>(u.getUser());
                if (user && !visited.count(user) && dyn_cast_or_null<StoreInst>(user)) {
                  visited.insert(user);
                  stack.push_back(user);
                }
              }
            }
            break;
          }
          default:break;
        }
      }
    }

    hoistInst(li);

    // Q2: dependent-chain hoisting. Collect loop-body instructions that
    // transitively depend on `li` and whose other operands are all
    // loop-invariant, then move them to the preheader. fillFixupBlocks
    // will clone the chain into fixupBB, rewire operands via old2New,
    // and create alloca slots for remaining loop-body uses (promoteMemToReg
    // converts those to PHIs at the end of runOnLoop).
    // Skip for inner loops: moving instructions into a preheader that
    // sits inside an outer loop invalidates the outer LoopInfo/DomTree.
    if (!currLoop->getParentLoop()) {
      std::vector<Instruction *> chain = collectDependentChain(li);
      for (Instruction *dep : chain) {
        dep->moveBefore(preheader->getTerminator());
      }
    }

    // Now change the header to the new header
    header = newHeader;

    fillFixupBlocks(li, fixupBB);
  }
}

void SpecLICM::fillFixupBlocks(LoadInst *ld, BasicBlock *fixupBB) {
  std::vector<Instruction *> stack;
  DenseSet<Instruction *> visited;
  BasicBlock &front = header->getParent()->front();
  DenseMap<Value *, Instruction *> old2New;

  LLVMContext &ctx = ld->getContext();
  Instruction *term = fixupBB->getTerminator();
  assert(term && "must have a terminator in fixup block");

  stack.clear();
  visited.clear();
  old2New.clear();
  stack.push_back(ld);
  while (!stack.empty()) {
    auto inst = stack.back();
    visited.insert(inst);
    Instruction *duplica = inst->clone();
    old2New[inst] = duplica;
    stack.pop_back();
    duplica->insertBefore(term);

    for (unsigned i = 0, e = duplica->getNumOperands(); i != e; ++i) {
      Value *old = duplica->getOperand(i);
      if (old2New.count(old))
        duplica->replaceUsesOfWith(old, old2New[old]);
    }

    std::for_each(inst->use_begin(), inst->use_end(), [&](Use &u) {
      Instruction *user = dyn_cast_or_null<Instruction>(u.getUser());
      if (!visited.count(user) && user->getParent() == preheader) {
        visited.insert(user);
        stack.push_back(user);
      }
    });

    // Create an alloca if inst is used inside the current loop.
    AllocaInst *ai = nullptr;
    for (Use &u : inst->uses()) {
      Instruction *UI = dyn_cast<Instruction>(u.getUser());
      if (UI && UI->getParent() != preheader) {
        if (!ai) {
          ai = new AllocaInst(inst->getType(),
                              ConstantInt::get(Type::getInt32Ty(ctx),
                                               inst->getType()->getScalarSizeInBits() / 8),
                              inst->getName() + ".alloca", front.getFirstInsertionPt());
          // For transforming it into SSA form.
          aliasAI.push_back(ai);
        }
        // Save the value produced by inst into the stack-allocated location at both
        // preheader and fixup block.
        new StoreInst(inst, ai, std::next(BasicBlock::iterator(inst)));
        new StoreInst(duplica, ai, term);

        // Insert a load right before the use of the inst.
        LoadInst *li = new LoadInst(ai, inst->getName() + ".ld", BasicBlock::iterator(UI));
        // Ensure not reordering this new load.
        insertedLds.insert(li);
        UI->replaceUsesOfWith(inst, li);
      }
    }
  }
}

void SpecLICM::fixPhiNodes(BasicBlock *newHeader) {
  auto insertPos = newHeader->begin();
  while (insertPos != newHeader->end() && dyn_cast<PHINode>(insertPos))
    ++insertPos;

  for (auto I = header->begin(); I != header->end() && dyn_cast<PHINode>(I);) {
    auto pos = std::next(I);
    I->removeFromParent();
    newHeader->getInstList().insert(insertPos, I);
    I = pos;
  }
}

void SpecLICM::promoteMemToReg() {
  if (!aliasAI.empty()) {
    domTree->recalculate(*fn);
    llvm::PromoteMemToReg(makeArrayRef(aliasAI), *domTree, aliasSetTracker);
  }
}

void SpecLICM::computeFrequentPath(Loop *L) {
  frequentPath.clear();
  BasicBlock *hdr = L->getHeader();
  frequentPath.insert(hdr);
  BasicBlock *cur = hdr;

  // Walk the ≥80% successor chain until we reach the backedge, leave the
  // loop, or lose majority confidence (BPI::isEdgeHot reports no hot edge).
  while (true) {
    TerminatorInst *term = cur->getTerminator();
    unsigned n = term->getNumSuccessors();
    if (n == 0) break;

    BasicBlock *next = nullptr;
    if (n == 1) {
      next = term->getSuccessor(0);
    } else {
      // Ask BPI which successor (if any) is hot (>4/5 = 80%).
      for (unsigned i = 0; i < n; ++i) {
        BasicBlock *succ = term->getSuccessor(i);
        if (bpi->isEdgeHot(cur, succ)) {
          next = succ;
          break;
        }
      }
      if (!next) break;                             // no ≥80% majority
    }

    if (next == hdr) break;                         // reached the backedge
    if (!L->contains(next)) break;                  // left the loop
    if (!frequentPath.insert(next).second) break;   // cycle guard
    cur = next;
  }
}

bool SpecLICM::anyConflictOnFrequentPath(LoadInst *ld) {
  if (frequentPath.empty()) return false;
  Value *addr = ld->getPointerOperand();
  for (BasicBlock *bb : frequentPath) {
    for (Instruction &I : *bb) {
      StoreInst *st = dyn_cast<StoreInst>(&I);
      if (!st) continue;
      Value *sa = st->getPointerOperand();
      // Trivial case: same SSA value.
      if (sa == addr) return true;
      // Syntactic GEP equality. The spec assumes no pointer aliasing, so we
      // only need to catch explicit writes to the same derived address.
      auto *g1 = dyn_cast<GetElementPtrInst>(addr);
      auto *g2 = dyn_cast<GetElementPtrInst>(sa);
      if (g1 && g2 &&
          g1->getPointerOperand() == g2->getPointerOperand() &&
          g1->getNumIndices() == g2->getNumIndices()) {
        bool equal = true;
        for (unsigned i = 0, e = g1->getNumIndices(); i < e; ++i) {
          if (g1->getOperand(i + 1) != g2->getOperand(i + 1)) {
            equal = false;
            break;
          }
        }
        if (equal) return true;
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