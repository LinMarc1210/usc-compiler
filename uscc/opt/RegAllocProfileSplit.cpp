/**
 * RAUSCCProfileSplit — Profile-guided register allocator that extends
 * the baseline RAUSCC (Chaitin graph coloring) with the Nakaike et al.
 * (PLDI 2006) five-step pipeline: liveness, live-range splitting,
 * subrange coalescing, spill decision, spill code generation.
 *
 * Pass skeleton and rule wrappers: Course staff / Jianping Zeng
 * Feature: Profile-Guided Register Allocation, tested by testRegAlloc.py
 * Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
 *
 * Step 1 (liveness) is LLVM's LiveIntervals analysis.
 * Step 5 (spill-everywhere) is LLVM's inline spiller.
 * Steps 2, 3, 4 are implemented below.
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
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "Passes.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace llvm;

// ----------------------------------------------------------------------
// Block execution frequencies come from LLVM's
// MachineBlockFrequencyInfo (MBFI) analysis.  The driver has
// already parsed the .profile file and stamped branch-weight
// metadata onto the Module, so this pass just queries MBFI via
// `MBFI.getBlockFreq(MBB).getFrequency()`.
// ----------------------------------------------------------------------

#define DEBUG_TYPE "regalloc-profile-split"

namespace {

// A SubrangeEdge models a virtual split at a CFG join block.
// Until deploySplits() runs, no MachineInstrs are changed — the
// edge is only an entry in `insertedEdges` that Steps 2 and 3
// reason about.  After deploy, origReg has been rewritten to
// newReg in the region dominated by BB_dst, and one COPY MI per
// predecessor has been inserted at the end of that predecessor.
struct SubrangeEdge
{
	unsigned origReg;                // the live-range being (virtually) split
	MachineBasicBlock *BB_dst;       // the join block where the split happens
	unsigned newReg;                 // filled by deploySplits (0 pre-deploy)
	uint64_t maxPredFreq;            // cached for Rule 3 dispatch
};

// ======================================================================
// RAUSCCProfileSplit — the profile-split allocator class.
// ======================================================================
class RAUSCCProfileSplit : public MachineFunctionPass, public RegAllocBase {
	MachineFunction *MF;

	typedef std::map<LiveInterval *, std::set<LiveInterval *>> InterferenceGraph;
	InterferenceGraph interferenceGraph;
	std::unique_ptr<Spiller> spillerInstance;
	std::vector<LiveInterval *> stack;

	// ProfileSplit state
	std::map<const MachineBasicBlock *, uint64_t> execCount;
	uint64_t maxCount;
	std::set<SlotIndex> saturationPoints;
	std::vector<SubrangeEdge> insertedEdges;

	// Direct-access analysis handles.
	MachineLoopInfo *MLI;

public:
	static char ID;

	RAUSCCProfileSplit();

	const char *getPassName() const override {
		return "USCC Profile-Split Register Allocator";
	}

	void getAnalysisUsage(AnalysisUsage &AU) const override;
	void releaseMemory() override;

	Spiller &spiller() override { return *spillerInstance; }

	void enqueue(LiveInterval *LI) override { stack.push_back(LI); }
	LiveInterval *dequeue() override {
		if (stack.empty()) return nullptr;
		LiveInterval *LI = stack.back();
		stack.pop_back();
		return LI;
	}
	unsigned selectOrSplit(LiveInterval&,
	                       SmallVectorImpl<unsigned>&) override {
		llvm::llvm_unreachable_internal("unused in RAUSCCProfileSplit");
		return ~0u;
	}

	bool runOnMachineFunction(MachineFunction &mf) override;

private:
	// Pre-step: ingest profile and compute per-block execCount.
	void loadProfileAndBuildFreq();

	// Helper shared with RAUSCC.
	unsigned numOverlappingPhysRegs(LiveInterval *virtReg,
	                                const TargetRegisterClass *rc);

	// Step 1 equivalent: build the initial IG (same as baseline).
	void initGraph();

	// Classification helpers.
	bool active(LiveInterval *sr);
	bool significant(LiveInterval *sr);
	unsigned NumSaturationPoints(LiveInterval *sr);
	void computeSaturationPoints();

	// Step 2 — enumerate virtual split edges (no IR mutation).
	void splitAllLiveRanges();

	// Step 3 — coalescing.  Each surviving SubrangeEdge is
	// deployed to the IR in deploySplits(); coalesced edges are
	// dropped.
	void coalesceSubranges();
	bool Rule1_bothSignificantInactive(const SubrangeEdge &e);
	bool Rule2_coalesceCostBased(const SubrangeEdge &e);
	bool Rule3_frequencyRatio(const SubrangeEdge &e);

	// Convenience wrappers that apply Rule 1 / Rule 2 and emit the
	// corresponding "coalesce (RuleN):" / "keep split (RuleN):" log
	// line when the rule fires.  The coalescing dispatcher
	// (coalesceSubranges) invokes these so the student code does
	// not need to re-derive the applicability gates from the paper.
	enum class Rule2Decision {
		Coalesce,       // Rule 2 applied and decided to coalesce.
		KeepSplit,      // Rule 2 applied and decided to keep the split.
		NotApplicable   // Rule 2 does not apply — try Rule 3.
	};
	bool          tryRule1(const SubrangeEdge &e);
	Rule2Decision tryRule2(const SubrangeEdge &e);
	bool          tryRule3(const SubrangeEdge &e);

	// Deploy — commit surviving splits to the IR in one pass.
	void deploySplits();

	// Virtual-subrange classification helpers (on either side of
	// a SubrangeEdge, driven by the dominator tree).
	bool activeBefore(const SubrangeEdge &e);
	bool activeAfter(const SubrangeEdge &e);
	bool significantBefore(const SubrangeEdge &e);
	bool significantAfter(const SubrangeEdge &e);

	// Step 4 — profile-aware victim selection.
	void simplifyGraph();
	double priority(LiveInterval *sr);

	// Step 5 — assignment + late spill (reuses RAUSCC shape).
	void allocation();
};

} // anonymous namespace

char RAUSCCProfileSplit::ID = 0;

FunctionPass *llvm::createUSCCRegisterAllocatorProfileSplit() {
	return new RAUSCCProfileSplit();
}

// ----------------------------------------------------------------------
// Constructor and analysis-usage declarations.  Mirrors baseline
// RAUSCC; MachineLoopInfo is also queried directly in the pipeline.
// ----------------------------------------------------------------------
RAUSCCProfileSplit::RAUSCCProfileSplit()
	: MachineFunctionPass(ID), MF(nullptr), maxCount(1), MLI(nullptr) {
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

void RAUSCCProfileSplit::getAnalysisUsage(AnalysisUsage &AU) const {
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

void RAUSCCProfileSplit::releaseMemory() {
	spillerInstance.reset();
	interferenceGraph.clear();
	stack.clear();
	execCount.clear();
	saturationPoints.clear();
	insertedEdges.clear();
	maxCount = 1;
}

// Feature: Profile-Guided Register Allocation, tested by testRegAlloc.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Executes 5-step pipeline: splitting, coalescing, and profile-based spill decisions
bool RAUSCCProfileSplit::runOnMachineFunction(MachineFunction &mf) {
	errs() << "********** USCC PROFILE-SPLIT REGISTER ALLOCATION **********\n";
	errs() << "********** Function: " << mf.getName() << '\n';
	MF = &mf;
	MLI = &getAnalysis<MachineLoopInfo>();

	// ----------------------------------------------------------------
	// Early phase — splitting and coalescing run before
	// RegAllocBase::init.  Step 2's deploySplits() creates new
	// vregs, and those must exist before init snapshots the vreg
	// count into VirtRegMap / LiveRegMatrix storage.  So that
	// Steps 2-3 can classify live intervals before init, we
	// populate MRI, TRI, LIS, and RegClassInfo by hand here.
	// ----------------------------------------------------------------
	this->MRI = &mf.getRegInfo();
	this->TRI = mf.getTarget().getRegisterInfo();
	this->LIS = &getAnalysis<LiveIntervals>();
	RegClassInfo.runOnMachineFunction(mf);

	loadProfileAndBuildFreq();   // pre-step: profile → execCount
	computeSaturationPoints();   // classify vregs before splitting
	splitAllLiveRanges();        // Step 2: enumerate virtual edges
	coalesceSubranges();         // Step 3: apply Rules 1/2/3
	deploySplits();              // commit surviving splits to IR

	// ----------------------------------------------------------------
	// Main phase — standard RegAllocBase flow on the post-deploy
	// vreg set.  Steps 4 and 5 mirror baseline RAUSCC.
	// ----------------------------------------------------------------
	RegAllocBase::init(getAnalysis<VirtRegMap>(),
	                   getAnalysis<LiveIntervals>(),
	                   getAnalysis<LiveRegMatrix>());
	// VRM and LiveRegMatrix sized their internal maps during the
	// prerequisite analyses, which ran before deploy created the
	// new vregs.  Extend them so the downstream rewriter sees
	// every vreg index.
	VRM->grow();
	calculateSpillWeightsAndHints(*LIS, *MF,
	                              getAnalysis<MachineLoopInfo>(),
	                              getAnalysis<MachineBlockFrequencyInfo>());
	spillerInstance.reset(createInlineSpiller(*this, *MF, *VRM));

	initGraph();                 // Step 1 (final IG on post-deploy vregs)
	computeSaturationPoints();   // recompute for Step 4 priority
	simplifyGraph();             // Step 4: victim selection by priority
	allocation();                // Step 5: color + late spill

	DEBUG(dbgs() << "Post alloc VirtRegMap:\n" << *VRM << "\n");
	releaseMemory();
	return true;
}

// ======================================================================
// Fill `execCount[MBB]` from MachineBlockFrequencyInfo.  Called
// once at the start of runOnMachineFunction; read by Steps 2-4.
// ======================================================================
void RAUSCCProfileSplit::loadProfileAndBuildFreq() {
	execCount.clear();
	maxCount = 1;
	MachineBlockFrequencyInfo &MBFI =
	    getAnalysis<MachineBlockFrequencyInfo>();
	for (MachineBasicBlock &B : *MF) {
		uint64_t f = MBFI.getBlockFreq(&B).getFrequency();
		execCount[&B] = f;
		if (f > maxCount) maxCount = f;
	}
}

// ======================================================================
// Helper shared with baseline RAUSCC.
// ======================================================================
unsigned RAUSCCProfileSplit::numOverlappingPhysRegs(LiveInterval *virtReg,
                                              const TargetRegisterClass *rc) {
	unsigned res = 0;
	for (auto physReg : RegClassInfo.getOrder(rc)) {
		if (Matrix->checkInterference(*virtReg, physReg))
			++res;
	}
	return res;
}

// ======================================================================
// Build the initial interference graph.  Same structure as
// baseline RAUSCC.
// ======================================================================
void RAUSCCProfileSplit::initGraph() {
	std::vector<LiveInterval *> liveIntervals;
	interferenceGraph.clear();
	for (unsigned i = 0; i < MRI->getNumVirtRegs(); ++i) {
		auto reg = TargetRegisterInfo::index2VirtReg(i);
		if (MRI->reg_nodbg_empty(reg)) continue;
		LiveInterval *v = &LIS->getInterval(reg);
		liveIntervals.push_back(v);
		interferenceGraph[v] = std::set<LiveInterval *>();
	}
	for (std::size_t i = 0; i < liveIntervals.size(); ++i) {
		for (std::size_t j = i + 1; j < liveIntervals.size(); ++j) {
			if (liveIntervals[i]->overlaps(*liveIntervals[j])) {
				interferenceGraph[liveIntervals[i]].insert(liveIntervals[j]);
				interferenceGraph[liveIntervals[j]].insert(liveIntervals[i]);
			}
		}
	}
}

// ======================================================================
// Classification helpers.
// ======================================================================
bool RAUSCCProfileSplit::active(LiveInterval *sr) {
	for (auto I = MRI->reg_nodbg_begin(sr->reg),
	          E = MRI->reg_nodbg_end(); I != E; ++I) {
		return true;
	}
	return false;
}

void RAUSCCProfileSplit::computeSaturationPoints() {
	saturationPoints.clear();
	// A slot is saturated iff the number of live virtual registers
	// at that slot exceeds the number of allocatable physical
	// registers.  K is approximated as the minimum allocatable-reg
	// count across every register class in use — a target-uniform
	// lower bound that's simple to compute.
	unsigned K = UINT_MAX;
	for (unsigned i = 0; i < MRI->getNumVirtRegs(); ++i) {
		unsigned reg = TargetRegisterInfo::index2VirtReg(i);
		if (MRI->reg_nodbg_empty(reg)) continue;
		const TargetRegisterClass *rc = MRI->getRegClass(reg);
		unsigned availPhysRegs = RegClassInfo.getNumAllocatableRegs(rc);
		if (availPhysRegs < K) K = availPhysRegs;
	}
	if (K == UINT_MAX) K = 0;

	// Collect all vreg intervals.
	std::vector<LiveInterval *> vregs;
	for (unsigned i = 0; i < MRI->getNumVirtRegs(); ++i) {
		unsigned reg = TargetRegisterInfo::index2VirtReg(i);
		if (MRI->reg_nodbg_empty(reg)) continue;
		vregs.push_back(&LIS->getInterval(reg));
	}

	// Walk instructions; at each slot, count how many intervals are live.
	for (MachineBasicBlock &B : *MF) {
		for (MachineInstr &MI : B) {
			if (MI.isDebugValue()) continue;
			if (MI.isInsideBundle()) continue;
			SlotIndex si = LIS->getInstructionIndex(&MI);
			unsigned liveAtHere = 0;
			for (LiveInterval *v : vregs) {
				if (v->liveAt(si)) ++liveAtHere;
			}
			if (liveAtHere > K) saturationPoints.insert(si);
		}
	}
}

bool RAUSCCProfileSplit::significant(LiveInterval *sr) {
	for (SlotIndex sp : saturationPoints) {
		if (sr->liveAt(sp)) return true;
	}
	return false;
}

unsigned RAUSCCProfileSplit::NumSaturationPoints(LiveInterval *sr) {
	unsigned cnt = 0;
	for (SlotIndex sp : saturationPoints) {
		if (sr->liveAt(sp)) ++cnt;
	}
	return cnt;
}

// ======================================================================
// Step 2 — enumerate candidate live-range splits.
//
// Two enumerations feed `insertedEdges`:
//
//   (a) Join-side: for every CFG join block B (pred_size >= 2) and
//       every virtual register live-in to B, append one
//       SubrangeEdge{origReg=reg, BB_dst=&B, newReg=0,
//       maxPredFreq=max over B's preds of execCount[P]}.
//
//   (b) Fork-side: for every fork block B (succ_size >= 2) and every
//       successor S of B that has a single predecessor (i.e., S is
//       NOT itself a join — the join-side loop above already handles
//       those), append one SubrangeEdge{origReg=reg, BB_dst=S,
//       newReg=0, maxPredFreq=execCount[B]} for each vreg live-in
//       to S.  The pred_size()>=2 filter on successors prevents
//       double-enumeration of the same physical edge.
//
// Step 3 filters this list; surviving edges are committed to the IR
// by deploySplits().  This function must NOT modify any
// MachineInstrs.
// ======================================================================
void RAUSCCProfileSplit::splitAllLiveRanges() {
	insertedEdges.clear();
	// PA6: Implement
	// PURPOSE: enumerate candidate split edges and save them in insertedEdges for the later coalescing stage
	
	// Collect all vreg intervals.
	std::vector<LiveInterval *> vregs;
	for (unsigned i = 0; i < MRI->getNumVirtRegs(); ++i) {    // MRI is from class MachineRegisterInfo
		unsigned reg = TargetRegisterInfo::index2VirtReg(i);
		if (MRI->reg_nodbg_empty(reg)) continue;    // skip debug register 
		vregs.push_back(&LIS->getInterval(reg));
	}

	// Separate join and fork block into 2 loops to match test case order

	// 1. join block
	for (MachineBasicBlock &MBB : *MF) {
		if (MBB.pred_size() >= 2) {
			// calculate max predecessor frequency
			uint64_t maxFreq = 0;
			for (auto *pred : MBB.predecessors()) {
				maxFreq = std::max(maxFreq, execCount[pred]);
			}
			
			// create one SubrangeEdge for every non-debug virtual register whose live interval is live-in to B
			for (auto *LI : vregs) {    // LiveInterval *
				if (LIS->isLiveInToMBB(*LI, &MBB)) {
					SubrangeEdge e = SubrangeEdge{
						.origReg=LI->reg, 
						.BB_dst=&MBB, 
						.newReg=0, // 0 for the not-yet-created new register
						.maxPredFreq=maxFreq
					};
					insertedEdges.push_back(e);
				}
			}
		}
	}


	// 2. fork block
	for (MachineBasicBlock &MBB : *MF) {
		if (MBB.succ_size() >= 2) {
			for (auto *succ : MBB.successors()) {
				if (succ->pred_size() < 2) {    // prevents double enumeration
					for (auto *LI : vregs) {
						if (LIS->isLiveInToMBB(*LI, succ)) {
							SubrangeEdge e = SubrangeEdge{
								.origReg=LI->reg, 
								.BB_dst=succ, 
								.newReg=0, // 0 for the not-yet-created new register
								.maxPredFreq=execCount[&MBB]  // MBB's frequency is simply fork point's predFreq
							};
							insertedEdges.push_back(e);
						}
					}
				}
			}
		}
	}
}

// ======================================================================
// Virtual-subrange classifiers.  For each SubrangeEdge e, the two
// conceptual subranges of the pre-deploy state are:
//   BEFORE : the portion of origReg live up to BB_dst's entry
//            (blocks NOT dominated by BB_dst)
//   AFTER  : the portion live from BB_dst onward
//            (blocks dominated by BB_dst)
// These classifications are computed on the fly from the single
// un-split origReg; no MachineInstrs have been modified yet.
// ======================================================================
bool RAUSCCProfileSplit::activeBefore(const SubrangeEdge &e) {
	MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
	for (auto I = MRI->reg_nodbg_begin(e.origReg),
	          E = MRI->reg_nodbg_end(); I != E; ++I) {
		MachineBasicBlock *B = I->getParent()->getParent();
		if (!MDT->dominates(e.BB_dst, B))
			return true;
	}
	return false;
}

bool RAUSCCProfileSplit::activeAfter(const SubrangeEdge &e) {
	MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
	for (auto I = MRI->reg_nodbg_begin(e.origReg),
	          E = MRI->reg_nodbg_end(); I != E; ++I) {
		MachineBasicBlock *B = I->getParent()->getParent();
		if (MDT->dominates(e.BB_dst, B))
			return true;
	}
	return false;
}

bool RAUSCCProfileSplit::significantBefore(const SubrangeEdge &e) {
	MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
	LiveInterval &li = LIS->getInterval(e.origReg);
	for (SlotIndex sp : saturationPoints) {
		if (!li.liveAt(sp)) continue;
		MachineInstr *MI = LIS->getInstructionFromIndex(sp);
		if (!MI) continue;
		if (!MDT->dominates(e.BB_dst, MI->getParent()))
			return true;
	}
	return false;
}

bool RAUSCCProfileSplit::significantAfter(const SubrangeEdge &e) {
	MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
	LiveInterval &li = LIS->getInterval(e.origReg);
	for (SlotIndex sp : saturationPoints) {
		if (!li.liveAt(sp)) continue;
		MachineInstr *MI = LIS->getInstructionFromIndex(sp);
		if (!MI) continue;
		if (MDT->dominates(e.BB_dst, MI->getParent()))
			return true;
	}
	return false;
}

// ======================================================================
// Step 3 — coalescing driver.  For each candidate SubrangeEdge,
// apply the three rules in priority order and keep only those
// edges that the rules say should *not* be coalesced.  No IR
// changes here — the decision is committed by deploySplits().
// ======================================================================
// Convenience wrapper around Rule 1.  Returns true iff the edge was
// coalesced (and the log line was printed).  Shipped so the student's
// dispatcher can handle Rule 1 as a single method call.
bool RAUSCCProfileSplit::tryRule1(const SubrangeEdge &e) {
	if (!Rule1_bothSignificantInactive(e)) return false;
	errs() << "coalesce (Rule1): %vreg"
	       << TargetRegisterInfo::virtReg2Index(e.origReg)
	       << " at join BB#" << e.BB_dst->getNumber() << "\n";
	return true;
}

// Convenience wrapper around Rule 2.  Encapsulates the paper's
// applicability gate (one side insignificant-active AND the other
// side significant) and emits the appropriate log line when
// applicable.  Shipped so the student's dispatcher can handle
// Rule 2 as a single method call.
RAUSCCProfileSplit::Rule2Decision
RAUSCCProfileSplit::tryRule2(const SubrangeEdge &e) {
	bool beforeIsInsigActive = !significantBefore(e) && activeBefore(e);
	bool afterIsInsigActive  = !significantAfter(e)  && activeAfter(e);
	bool otherIsSignificant  = significantBefore(e) || significantAfter(e);
	bool applies = otherIsSignificant &&
	               (beforeIsInsigActive ^ afterIsInsigActive);
	if (!applies) return Rule2Decision::NotApplicable;

	unsigned origIdx = TargetRegisterInfo::virtReg2Index(e.origReg);
	if (Rule2_coalesceCostBased(e)) {
		errs() << "coalesce (Rule2): %vreg" << origIdx
		       << " at join BB#" << e.BB_dst->getNumber() << "\n";
		return Rule2Decision::Coalesce;
	}
	errs() << "keep split (Rule2): %vreg" << origIdx
	       << " at join BB#" << e.BB_dst->getNumber() << "\n";
	return Rule2Decision::KeepSplit;
}

// Convenience wrapper around Rule 3.  Rule 3 is the catch-all:
// it always applies, and returns either Coalesce or KeepSplit.
// Returns true iff the edge was coalesced.  Shipped so the
// student's dispatcher can handle Rule 3 as a single method call.
bool RAUSCCProfileSplit::tryRule3(const SubrangeEdge &e) {
	unsigned origIdx = TargetRegisterInfo::virtReg2Index(e.origReg);
	if (Rule3_frequencyRatio(e)) {
		errs() << "coalesce (Rule3): %vreg" << origIdx
		       << " at join BB#" << e.BB_dst->getNumber() << "\n";
		return true;
	}
	errs() << "keep split (Rule3): %vreg" << origIdx
	       << " at join BB#" << e.BB_dst->getNumber() << "\n";
	return false;
}

// Step 3 dispatcher.  Walks insertedEdges and invokes the shipped
// tryRule1 / tryRule2 / tryRule3 helpers.  Coalesced edges are
// dropped; kept edges flow to deploySplits().
void RAUSCCProfileSplit::coalesceSubranges() {
	if (insertedEdges.empty()) return;

	std::vector<SubrangeEdge> surviving;
	surviving.reserve(insertedEdges.size());

	for (const SubrangeEdge &e : insertedEdges) {
		if (tryRule1(e)) continue;

		switch (tryRule2(e)) {
		case Rule2Decision::Coalesce:
			continue;
		case Rule2Decision::KeepSplit:
			surviving.push_back(e);
			continue;
		case Rule2Decision::NotApplicable:
			break;   // fall through to Rule 3
		}

		if (tryRule3(e)) continue;
		surviving.push_back(e);
	}

	insertedEdges.swap(surviving);
}

// Rule 1 — coalesce when both virtual sides are
// significant-inactive (cross a saturation point, no real def/use).
bool RAUSCCProfileSplit::Rule1_bothSignificantInactive(const SubrangeEdge &e) {
	return significantBefore(e) && !activeBefore(e) &&
	       significantAfter(e)  && !activeAfter(e);
}

// Rule 2 — cost-based coalesce.  Protects an insignificant-active
// virtual subrange from being coalesced into a spill-heavy region
// unless the combined cost is acceptable.  See Nakaike et al.,
// PLDI 2006, §3 Step 3 for the underlying cost-ledger reasoning.
bool RAUSCCProfileSplit::Rule2_coalesceCostBased(const SubrangeEdge &e) {
	MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
	double costBefore = 0, costAfter = 0;
	for (auto I = MRI->reg_nodbg_begin(e.origReg),
	          E = MRI->reg_nodbg_end(); I != E; ++I) {
		MachineBasicBlock *B = I->getParent()->getParent();
		auto it = execCount.find(B);
		double f = (it == execCount.end()) ? 1.0 : static_cast<double>(it->second);
		if (MDT->dominates(e.BB_dst, B)) costAfter += f;
		else                             costBefore += f;
	}
	double combined = costBefore + costAfter;
	return combined <= costBefore * 1.5 && combined <= costAfter * 1.5;
}

// Rule 3 — frequency-ratio catch-all.  Coalesce iff the two
// endpoint execution frequencies are within a 2:1 ratio.
bool RAUSCCProfileSplit::Rule3_frequencyRatio(const SubrangeEdge &e) {
	uint64_t fd = execCount.count(e.BB_dst) ? execCount[e.BB_dst] : 0;
	uint64_t fs = e.maxPredFreq;
	uint64_t hi = std::max(fs, fd);
	uint64_t lo = std::min(fs, fd);
	if (hi == 0) return true;
	uint64_t gap = hi - lo;
	uint64_t threshold = hi / 2;
	return gap <= threshold;
}

// ======================================================================
// Commit surviving SubrangeEdges to the MachineFunction.  Runs
// AFTER Step 3, BEFORE Step 4.  Inserts all COPYs first, then
// rebuilds LIS and the IG in a single pass.
// ======================================================================
void RAUSCCProfileSplit::deploySplits() {
	// PA6: Implement
	// PURPOSE: commits those surviving split decisions to the machine function.

	// rebuild LiveIntervals for the affected virtual registers 
	std::set<unsigned> affectedRegs;
	
	for (auto &e : insertedEdges) {    // Surviving SubrangeEdge
		if (MRI->reg_nodbg_empty(e.origReg)) {     //  skip if register e.origReg is debug-empty
			continue;
		}

		const TargetRegisterClass *rc = MRI->getRegClass(e.origReg);
		unsigned newReg = MRI->createVirtualRegister(rc);    // build new vreg in same class (rc)
		e.newReg = newReg;   // newReg is the new vreg's number

		// record both the original and new registers as affected live ranges 
		// so that their liveness information can be rebuilt later in one bulk pass
		affectedRegs.insert(e.origReg);
		affectedRegs.insert(e.newReg);

		for (MachineBasicBlock *pred : (e.BB_dst)->predecessors()) {
			// insert a machine-level copy instruction that transfers the value from e.origReg to e.newReg 
			// before the predecessor’s terminator

			/* MachineInstrBuilder llvm::BuildMI(MachineBasicBlock &BB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, const MCInstrDesc &MCID,
                                  bool IsIndirect, Register Reg,
                                  const MDNode *Variable, const MDNode *Expr)
			*/
			const TargetInstrInfo &TII = *MF->getTarget().getInstrInfo();
			MachineInstr *MI = BuildMI(
				*pred, 
				pred->getFirstTerminator(), 
				DebugLoc(),
				TII.get(TargetOpcode::COPY), 
				newReg
			).addReg(e.origReg);

			// register instruction with LiveIntervals
			LIS->InsertMachineInstrInMaps(MI);
		}


		// rewrite register operands in machine instructions whose parent basic blocks are dominated by e.BB dst
		MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
		for (auto &MBB : *MF) {
			if (MDT->dominates(e.BB_dst, &MBB)) {
				for (auto &MI : MBB) {
					for (auto &op : MI.operands()) {
						if (op.isReg() && op.getReg() == e.origReg) {
							op.setReg(e.newReg);
						}
					}
				}
			}
		}

		// debug message for each surviving split
		llvm::errs() << "deploy: split %vreg" 
					 << TargetRegisterInfo::virtReg2Index(e.origReg) 
					 << " at BB#" 
					 << e.BB_dst->getNumber() 
					 << " -> %vreg" 
					 << TargetRegisterInfo::virtReg2Index(e.newReg)
					 << "\n";
	}


	// rebuild LiveIntervals for the affected virtual registers 
	for (auto &reg : affectedRegs) {
		if (LIS->hasInterval(reg)) {
			LIS->removeInterval(reg);
		}
		if (!MRI->reg_empty(reg)) {
			LIS->createAndComputeVirtRegInterval(reg);
		}
	}
}

// ======================================================================
// Step 4 — Profile-aware spill priority.
//   priority(sr) = sum of execCount[B] over blocks B containing a
//                  def or use of sr, divided by the number of
//                  saturation points sr spans.
// Lower priority = preferred spill victim.
// ======================================================================
double RAUSCCProfileSplit::priority(LiveInterval *sr) {
	// Sum of execCount over blocks containing defs/uses of sr.
	uint64_t sumFreq = 0;
	std::set<const MachineBasicBlock *> seen;
	for (auto I = MRI->reg_nodbg_begin(sr->reg),
	          E = MRI->reg_nodbg_end(); I != E; ++I) {
		const MachineBasicBlock *B = I->getParent()->getParent();
		if (seen.insert(B).second) {
			auto it = execCount.find(B);
			sumFreq += (it == execCount.end()) ? 0 : it->second;
		}
	}
	unsigned sats = NumSaturationPoints(sr);
	if (sats == 0) sats = 1;
	return static_cast<double>(sumFreq) / static_cast<double>(sats);
}

void RAUSCCProfileSplit::simplifyGraph() {
	// Rebuild the IG in case Step 2/3 changed the vreg set.
	initGraph();

	bool changed;
	std::set<LiveInterval *> onStack;
	SmallVector<unsigned, 4> splitVRegs;
	while (interferenceGraph.size() > 0) {
		// Simplification loop: trivially-colorable nodes go straight
		// to the coloring stack (same as baseline RAUSCC).
		do {
			changed = false;
			auto lowest = interferenceGraph.end();
			unsigned lowestReg = __UINT32_MAX__;
			for (auto iter = interferenceGraph.begin();
			     iter != interferenceGraph.end(); ++iter) {
				const TargetRegisterClass *rc = MRI->getRegClass(iter->first->reg);
				unsigned availPhysRegs = RegClassInfo.getNumAllocatableRegs(rc);
				unsigned numNeighbors =
					iter->second.size() + numOverlappingPhysRegs(iter->first, rc);
				if (numNeighbors < availPhysRegs && iter->first->reg < lowestReg) {
					lowest = iter;
					lowestReg = lowest->first->reg;
				}
			}
			if (lowest != interferenceGraph.end()) {
				LiveInterval *v = lowest->first;
				const TargetRegisterClass *rc = MRI->getRegClass(v->reg);
				unsigned numNeighbors =
					lowest->second.size() + numOverlappingPhysRegs(v, rc);
				errs() << "Found neighbors = " << numNeighbors << " for " << *v << "\n";
				enqueue(v);
				for (LiveInterval *li : lowest->second)
					interferenceGraph[li].erase(v);
				interferenceGraph.erase(lowest);
				errs() << "Removal: " << *v << "\n";
				changed = true;
			}
		} while (changed);

		// Victim selection loop — use profile-aware priority when
		// available.  Lowest priority = best spill victim.
		bool go = true;
		onStack.clear();
		while (go && !interferenceGraph.empty()) {
			double minPriority = 1e30;
			auto min = interferenceGraph.end();
			for (auto iter = interferenceGraph.begin();
			     iter != interferenceGraph.end(); ++iter) {
				double p = priority(iter->first);
				if (p < minPriority ||
				    (p == minPriority && (min == interferenceGraph.end() ||
				                          iter->first->reg < min->first->reg))) {
					minPriority = p;
					min = iter;
				}
			}
			if (min != interferenceGraph.end()) {
				LiveInterval *li = min->first;
				const TargetRegisterClass *rc = MRI->getRegClass(li->reg);
				unsigned numNeighbors =
					min->second.size() + numOverlappingPhysRegs(li, rc);
				errs() << "Spill candidate (neighbors = " << numNeighbors
				       << ", priority = " << minPriority
				       << "): " << *li << "\n";
				onStack.insert(min->first);
				for (LiveInterval *li2 : min->second) {
					auto &conflicts = interferenceGraph[li2];
					conflicts.erase(min->first);
					const TargetRegisterClass *rc2 = MRI->getRegClass(li2->reg);
					unsigned availPhysRegs =
						RegClassInfo.getNumAllocatableRegs(rc2);
					if ((conflicts.size() + numOverlappingPhysRegs(li2, rc2))
					    < availPhysRegs)
						go = false;
				}
				interferenceGraph.erase(min);
			}
		}

		// Step 5 kickoff: spill the selected victims, add new split
		// intervals to the IG, repeat.
		for (LiveInterval *virtReg : onStack) {
			splitVRegs.clear();
			if (virtReg->isSpillable()) {
				LiveRangeEdit LRE(virtReg, splitVRegs, *MF, *LIS, VRM);
				spiller().spill(LRE);
			}
			for (auto reg : splitVRegs) {
				LiveInterval *splitVirtReg = &LIS->getInterval(reg);
				assert(!VRM->hasPhys(splitVirtReg->reg) && "Register already assigned");
				if (MRI->reg_nodbg_empty(reg)) {
					LIS->removeInterval(splitVirtReg->reg);
					continue;
				}
				errs() << "add new interval for spilling to the IG: "
				       << *splitVirtReg << "\n";
				assert(TargetRegisterInfo::isVirtualRegister(splitVirtReg->reg) &&
				       "expect split value in virtual register");
				auto newEntry = std::make_pair(splitVirtReg,
				                               std::set<LiveInterval *>());
				for (auto &entry : interferenceGraph) {
					if (entry.first->overlaps(*splitVirtReg)) {
						entry.second.insert(splitVirtReg);
						newEntry.second.insert(entry.first);
					}
				}
				interferenceGraph.insert(newEntry);
			}
		}
		onStack.clear();
	}
}

// ======================================================================
// Step 5 — Assignment (same shape as RAUSCC).
// ======================================================================
void RAUSCCProfileSplit::allocation() {
	while (LiveInterval *virtReg = dequeue()) {
		Matrix->invalidateVirtRegs();
		SmallVector<unsigned, 8> PhysRegSpillCands;
		AllocationOrder Order(virtReg->reg, *VRM, RegClassInfo);
		unsigned availablePhysReg;
		while ((availablePhysReg = Order.next()) != 0) {
			if (Matrix->checkInterference(*virtReg, availablePhysReg) ==
			    LiveRegMatrix::IK_Free) {
				errs() << "Assigning to physical register "
				       << TRI->getName(availablePhysReg) << ": " << *virtReg << "\n";
				break;
			}
		}
		if (!availablePhysReg) {
			std::string str;
			raw_string_ostream os(str);
			os << "ran out of registers during register allocation on "
			   << *virtReg << "\n";
			report_fatal_error(os.str());
		}
		Matrix->assign(*virtReg, availablePhysReg);
	}
}
