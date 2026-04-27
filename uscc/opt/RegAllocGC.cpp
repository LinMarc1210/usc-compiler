/**
 * A graph coloring based register allocation
 * Jianping Zeng (zeng207@purdue.edu)
*/
#include "llvm/CodeGen/Passes.h"
#include "../../llvm/lib/CodeGen/AllocationOrder.h"
#include "../../llvm/lib/CodeGen/LiveDebugVariables.h"
#include "../../llvm/lib/CodeGen/RegAllocBase.h"
#include "../../llvm/lib/CodeGen/Spiller.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/CalcSpillWeights.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/LiveRangeEdit.h"
#include "llvm/CodeGen/LiveRegMatrix.h"
#include "llvm/CodeGen/LiveStackAnalysis.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/Support/Debug.h"
#include "Passes.h"
#include <queue>
#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "regalloc"

namespace {
// PA6: Add any global members needed

/// RAUSCC allocator pass
class RAUSCC : public MachineFunctionPass, public RegAllocBase {
  // context
  MachineFunction *MF;

  // PA6: Add any member variables needed
  typedef std::map<LiveInterval *, std::set<LiveInterval *>> InterferenceGraph;
  InterferenceGraph interferenceGraph;

  // state
  std::unique_ptr<Spiller> spillerInstance;
  std::vector<LiveInterval *> stack;

public:
  static char ID;

  RAUSCC();

  /// Return the pass name.
  const char *getPassName() const override {
    return "USCC Register Allocator";
  }

  /// RAUSCC analysis usage.
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void releaseMemory() override;

  Spiller &spiller() override { return *spillerInstance; }

  void enqueue(LiveInterval *LI) override {
    // PA6:
  }

  LiveInterval *dequeue() override {
    // PA6:
    return nullptr;
  }

  unsigned selectOrSplit(LiveInterval &VirtReg,
                         SmallVectorImpl<unsigned> &SplitVRegs) override {
    llvm::llvm_unreachable_internal("This function is not used in RAUSCC!");
    return ~0u;
  }

  /// Perform register allocation.
  bool runOnMachineFunction(MachineFunction &mf) override;

private:
  void initGraph();
  void simplifyGraph();
  void allocation();
  // Get the number of the physical registers overlapping with the given virtual register
  unsigned numOverlappingPhysRegs(LiveInterval *virtReg,
                                  const TargetRegisterClass *rc);
};
} // end anonymous namespace

char RAUSCC::ID = 0;

FunctionPass *llvm::createUSCCRegisterAllocator() {
  return new RAUSCC();
}

RAUSCC::RAUSCC() : MachineFunctionPass(ID) {
  initializeLiveDebugVariablesPass(*PassRegistry::getPassRegistry());
  initializeLiveIntervalsPass(*PassRegistry::getPassRegistry());
  initializeSlotIndexesPass(*PassRegistry::getPassRegistry());
  initializeRegisterCoalescerPass(*PassRegistry::getPassRegistry());
  initializeMachineSchedulerPass(*PassRegistry::getPassRegistry());
  initializeLiveStacksPass(*PassRegistry::getPassRegistry());
  initializeMachineDominatorTreePass(*PassRegistry::getPassRegistry());
  initializeMachineLoopInfoPass(*PassRegistry::getPassRegistry());
  initializeVirtRegMapPass(*PassRegistry::getPassRegistry());
  initializeLiveRegMatrixPass(*PassRegistry::getPassRegistry());
}

void RAUSCC::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<AliasAnalysis>();
  AU.addPreserved<AliasAnalysis>();
  AU.addRequired<LiveIntervals>();
  AU.addPreserved<LiveIntervals>();
  AU.addPreserved<SlotIndexes>();
  AU.addRequired<LiveDebugVariables>();
  AU.addPreserved<LiveDebugVariables>();
  AU.addRequired<LiveStacks>();
  AU.addPreserved<LiveStacks>();
  AU.addRequired<MachineBlockFrequencyInfo>();
  AU.addPreserved<MachineBlockFrequencyInfo>();
  AU.addRequiredID(MachineDominatorsID);
  AU.addPreservedID(MachineDominatorsID);
  AU.addRequired<MachineLoopInfo>();
  AU.addPreserved<MachineLoopInfo>();
  AU.addRequired<VirtRegMap>();
  AU.addPreserved<VirtRegMap>();
  AU.addRequired<LiveRegMatrix>();
  AU.addPreserved<LiveRegMatrix>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void RAUSCC::releaseMemory() {
  spillerInstance.reset();
  interferenceGraph.clear();
  stack.clear();
  // PA6: Delete any member data stored for each function
}

bool RAUSCC::runOnMachineFunction(MachineFunction &mf) {
  errs() << "********** USCC REGISTER ALLOCATION **********\n";
  std::string funcName(mf.getName());
  errs() << "********** Function: " << funcName << '\n';
  MF = &mf;
  RegAllocBase::init(getAnalysis<VirtRegMap>(),
                     getAnalysis<LiveIntervals>(),
                     getAnalysis<LiveRegMatrix>());

  calculateSpillWeightsAndHints(*LIS, *MF,
                                getAnalysis<MachineLoopInfo>(),
                                getAnalysis<MachineBlockFrequencyInfo>());

  spillerInstance.reset(createInlineSpiller(*this, *MF, *VRM));

  // PA6: main entry

  // Diagnostic output before rewriting
  DEBUG(dbgs() << "Post alloc VirtRegMap:\n" << *VRM << "\n");

  releaseMemory();
  return true;
}

// Build an interference graph
void RAUSCC::initGraph() {
  // PA6:
}

void RAUSCC::simplifyGraph() {
  // PA6:
}

void RAUSCC::allocation() {
  // PA6:
}

unsigned int RAUSCC::numOverlappingPhysRegs(llvm::LiveInterval *virtReg,
                                            const TargetRegisterClass *rc) {
  unsigned res = 0;
  for (auto physReg : RegClassInfo.getOrder(rc)) {
    if (Matrix->checkInterference(*virtReg, physReg))
      ++res;
  }
  return res;
}
