//
//  Passes.h
//  uscc
//
//  Declares the opt passes supported by USCC
//
//  At the moment, there are four passes:
//     * Constant op removal
//     * Constant branch folding
//     * Removal of dead blocks from CFG
//     * Loop Invariant Code Motion (LICM)
//
//  These passes will execute if uscc is ran with -O
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------
#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Dominators.h>
#pragma clang diagnostic pop

#include <set>
#include <vector>
#include <string>

using llvm::FunctionPass;
using llvm::LoopPass;

namespace uscc
{

namespace opt
{

// Helper function for registering the opt passes
void registerOptPasses(llvm::legacy::PassManager& pm);

// Declares the Constant Propagation Pass
struct ConstantOps : public FunctionPass
{
	static char ID;
	ConstantOps() : FunctionPass(ID) {}
	
	virtual bool runOnFunction(llvm::Function& F) override;
	
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};

// Declares the Constant Branch Folding Pass
struct ConstantBranch : public FunctionPass
{
	static char ID;
	ConstantBranch() : FunctionPass(ID) {}
	
	virtual bool runOnFunction(llvm::Function& F) override;
	
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};

// Declares the Dead Block Removal Pass
struct DeadBlocks : public FunctionPass
{
	static char ID;
	DeadBlocks() : FunctionPass(ID) {}
	
	virtual bool runOnFunction(llvm::Function& F) override;
	
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};
	
// Loop invariant code motion
struct LICM : public LoopPass
{
	static char ID;
	LICM() : LoopPass(ID) {}
	
	virtual bool runOnLoop(llvm::Loop* L, llvm::LPPassManager& LPM) override;
	
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;

	// Data regarding the current loop
	llvm::Loop* mCurrLoop;

	// The dominator tree for this loop
	llvm::DominatorTree* mDomTree;

	// Loop information for this loop
	llvm::LoopInfo* mLoopInfo;

	// Denotes whether or not loop has been modified
	bool mChanged;
};

struct InstCombine : public FunctionPass {
  static char ID;
  InstCombine() : FunctionPass(ID) {}

  virtual bool runOnFunction(llvm::Function &F) override;
  virtual void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
};

// Declares the Naive Edge Profiling Pass
// Instruments ALL edges with counters
struct EdgeProfiling : public FunctionPass
{
	static char ID;
	EdgeProfiling() : FunctionPass(ID) {}

	virtual bool runOnFunction(llvm::Function& F) override;
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};

// Declares the Optimized Edge Profiling Pass (MST + extrapolation)
// Uses Knuth's optimal counter placement algorithm
struct EdgeProfilingOpt : public FunctionPass
{
	static char ID;
	EdgeProfilingOpt() : FunctionPass(ID) {}

	virtual bool runOnFunction(llvm::Function& F) override;
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};

// Helper functions for registering the edge profiling passes
void registerEdgeProfilingPass(llvm::legacy::PassManager& pm);
void registerEdgeProfilingOptPass(llvm::legacy::PassManager& pm);

// PA2: Natural Loop Detection Pass
struct NaturalLoops : public FunctionPass {
	static char ID;
	NaturalLoops() : FunctionPass(ID) {}
	virtual bool runOnFunction(llvm::Function &F) override;
	void dfsFindBackEdge(llvm::BasicBlock *current, std::set<llvm::BasicBlock *> &visited);
	void findNaturalLoop(llvm::BasicBlock *tail);
	bool dfsReachable(llvm::BasicBlock* current, llvm::BasicBlock* target,
	                  llvm::BasicBlock* excludeBlock, std::set<llvm::BasicBlock*> visitedBlocks);
	void getDominatedBlocks(llvm::BasicBlock* block, std::vector<llvm::BasicBlock *>& dominatedBlocks);
	virtual void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
	void printBackEdge(std::string head, std::string tail);
	void printNaturalLoop(std::set<std::string>& naturalLoop);
	void mapOutput();
	llvm::BasicBlock *mHeader;
	llvm::DominatorTree* mDT;
};

void registerNaturalLoopPasses(llvm::legacy::PassManager& pm);


struct CopyPropagation : public llvm::FunctionPass
{
	static char ID;
	CopyPropagation() : FunctionPass(ID) {}
 
	virtual bool runOnFunction(llvm::Function& F) override;
	virtual void getAnalysisUsage(llvm::AnalysisUsage& Info) const override;
};
 
void registerAnalysisPasses(llvm::PassRegistry &Registry);

} // opt
} // uscc

namespace llvm
{
    void initializeLivenessPass(PassRegistry &Registry);
    void initializeAvailableExpressionsPass(PassRegistry &Registry);
    void initializeSpecLICMPass(PassRegistry &Registry);
    FunctionPass* createLivenessPass();
    FunctionPass* createDCEPass();
    FunctionPass* createAEPass();
    FunctionPass* createCSEPass();
    FunctionPass* createRedundantPhiRemovalPass();
	FunctionPass* createCopyPropagationPass();
    LoopPass* createSpecLICMPass();
}
