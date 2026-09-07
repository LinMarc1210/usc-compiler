/**
 * A graph coloring based register allocation
 * Pass skeleton: Jianping Zeng (zeng207@purdue.edu)
 * Reference: Chaitin-Briggs Graph Coloring Register Allocation
 * Feature: Graph Coloring Register Allocation, tested by testRegAlloc.py
 * Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
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
  std::set<LiveInterval*> markForSpill;

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
    stack.push_back(LI);
  }

  LiveInterval *dequeue() override {
    // PA6:
    if (stack.empty())
      return nullptr;
    LiveInterval *curr = stack.back();
    stack.pop_back();
    return curr;
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
  unsigned numAvailablePhysRegs(LiveInterval *vReg);
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

// Feature: Graph Coloring Register Allocation, tested by testRegAlloc.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Builds interference graph, simplifies nodes, handles spills, and colors registers
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
  while (true) {
    initGraph();
    simplifyGraph();
    if (markForSpill.empty()) {
      allocation(); // only start coloring if there is no one need spill
      break;
    }
    // if markForSpill is not empty (spill happened), then restart and rebuild IG
  }

  // Diagnostic output before rewriting
  DEBUG(dbgs() << "Post alloc VirtRegMap:\n" << *VRM << "\n");

  releaseMemory();
  return true;
}

// Build an interference graph
void RAUSCC::initGraph() {
  // PA6:
  // typedef std::map<LiveInterval *, std::set<LiveInterval *>> InterferenceGraph;

  std::set<LiveInterval *> vRegs;

  for (unsigned i = 0, e = MRI->getNumVirtRegs(); i != e; ++i) {
    // reg ID
    unsigned Reg = TargetRegisterInfo::index2VirtReg(i);
    // if it is a DEBUG register, skip
    if (MRI->reg_nodbg_empty(Reg)) {
      continue;
    }
    // get the respective LiveInterval
    LiveInterval *VirtReg = &LIS->getInterval(Reg);
    vRegs.insert(VirtReg);
    interferenceGraph[VirtReg] = std::set<LiveInterval *>();
  }
  
  // if two vregs' live ranges are overlapping, connect them by edge to avoid same register allocation
  for (auto *vreg1 : vRegs) {
    for (auto *vreg2 : vRegs) {
      if (vreg1 == vreg2) continue;
      if (vreg1->overlaps(*vreg2)) {
        // connected by edge
        interferenceGraph[vreg1].insert(vreg2);
        interferenceGraph[vreg2].insert(vreg1);
      }
    }
  }
}

void RAUSCC::simplifyGraph() {
  // PA6:

  while (!interferenceGraph.empty()) {
    LiveInterval *candidate = nullptr;    // candidate to be select (degree < K-colorable)
    const TargetRegisterClass *rc;

    // STEP 1's loop: simplification
    for (auto p : interferenceGraph) {
      LiveInterval *vertex = p.first;
      std::set<LiveInterval*> neighbors = p.second; 
      
      rc = MRI->getRegClass(vertex->reg);
      unsigned int K = numAvailablePhysRegs(vertex);
      unsigned int degree = numOverlappingPhysRegs(vertex, rc) + interferenceGraph[vertex].size();

      // victim to be removed from IG
      if (degree < K) {
        // find the smallest register number as victim
        if (!candidate || vertex->reg < candidate->reg) {
          candidate = vertex;
        }
      }
    }
    // if there is any candidate can be selected as colorable
    if (candidate) {
      unsigned int numOfNeighbors = numOverlappingPhysRegs(candidate, rc) + interferenceGraph[candidate].size();

      // remove condidate from all neighbors
      for (auto *neighbor : interferenceGraph[candidate]) {
        interferenceGraph[neighbor].erase(candidate);
      }
      interferenceGraph.erase(candidate);

      llvm::errs() << "Found neighbors = " << numOfNeighbors << " for " << *candidate << "\n";
      llvm::errs() << "Removal: " << *candidate << "\n";
      enqueue(candidate);

      continue;    // if find any candidate, we only iterate step 1 (because no need to spill)
    }


    // STEP 2's loop: victim selection for spilling
    LiveInterval *victim = nullptr;    // victim to be spilled
    float minWeight = std::numeric_limits<float>::max();

    for (auto const& p : interferenceGraph) {
      LiveInterval *v = p.first;
      float currentWeight = v->weight;  // built-in member weight = spill cost
      if (!victim || currentWeight < minWeight || (currentWeight == minWeight && v->reg < victim->reg)) {
        victim = v;
        minWeight = currentWeight;
      }
    }

    if (victim) {
      unsigned int numNeighbors = numOverlappingPhysRegs(victim, rc) + interferenceGraph[victim].size();
      
      // remove condidate from all neighbors
      for (auto *neighbor : interferenceGraph[victim]) {
        interferenceGraph[neighbor].erase(victim);
      }
      interferenceGraph.erase(victim);
      
      llvm::errs() << "Spill candidate (neighbors = " << numNeighbors << ", weight = " << minWeight << "): " << *victim << "\n";
      markForSpill.insert(victim);
      enqueue(victim);
    }


    // STEP 3's spilling code (Chaitin's algorithm)
    // allocate and see if physical register is available for virtual register
    SmallVector<unsigned, 4> splitVRegs;
    for (auto *virtReg : markForSpill) {
      splitVRegs.clear();

      // spill code
      LiveRangeEdit LRE(virtReg, splitVRegs, *MF, *LIS, VRM);
      spiller().spill(LRE);

      for (unsigned vreg : splitVRegs) {
        LiveInterval *newli = &LIS->getInterval(vreg);
        
        interferenceGraph[newli] = std::set<LiveInterval *>();
        for (auto &p : interferenceGraph) {
          LiveInterval *existingV = p.first;
          // update neighbors
          if (existingV != newli && newli->overlaps(*existingV)) {
            interferenceGraph[existingV].insert(newli);
            interferenceGraph[newli].insert(existingV);
          }
        }

        llvm::errs() << "add new interval for spilling to the IG: " << *newli << "\n";
      }
    }
    markForSpill.clear();
  }
}

void RAUSCC::allocation() {
  // PA6:
  LiveInterval *virtReg = dequeue();
  while (virtReg) {
    AllocationOrder Order(virtReg->reg, *VRM, RegClassInfo);
    while (unsigned PhysReg = Order.next()) {
      // Check for interference in PhysReg
      if (Matrix->checkInterference(*virtReg, PhysReg) == LiveRegMatrix::IK_Free) {
        llvm::errs() << "Assigning to physical register " << TRI->getName(PhysReg) << ": " << *virtReg << "\n";
        Matrix->assign(*virtReg, PhysReg);
        break;
      }
    }
    virtReg = dequeue();
  }
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

unsigned int RAUSCC::numAvailablePhysRegs(LiveInterval *vReg) {
  const TargetRegisterClass *rc = MRI->getRegClass(vReg->reg);
  return RegClassInfo.getNumAllocatableRegs(rc);
}
