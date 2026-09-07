//
//  ContantBranch.cpp
//  uscc
//
//  Implements Constant Branch Folding opt pass.
//  This converts conditional branches on constants to
//  unconditional branches.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------
//  Pass skeleton: Sanjay Madhav
//  Feature: Constant Branch Folding, tested by testConstantDeadBlock.py
//  Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
//---------------------------------------------------------
#include "Passes.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#pragma clang diagnostic pop
#include <set>

using namespace llvm;

namespace uscc
{
namespace opt
{
	
// Feature: Constant Branch Folding, tested by testConstantDeadBlock.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Converts conditional branches with constant conditions into unconditional jumps
bool ConstantBranch::runOnFunction(Function& F)
{
	bool changed = false;
	
	// PA5: Implement
	std::set<Instruction*> removeSet;

	for (auto &BB : F) {
		for (auto &inst : BB) {
			if (BranchInst *br = dyn_cast<BranchInst>(&inst)) {
				if (br->isConditional()) {
					Value* cond = br->getCondition();
					if (isa<ConstantInt>(cond)) {
						removeSet.insert(&inst);    // add Instruction (br) to removeSet
					}
				}
			}
		}
	}

	for (auto *inst : removeSet) {
		changed = true;

		if (BranchInst *br = dyn_cast<BranchInst>(inst)) {
			Value* v = br->getCondition();
			ConstantInt *cond = dyn_cast<ConstantInt>(v);
			BasicBlock *parent = br->getParent();
			BasicBlock *leftSucc = br->getSuccessor(0);   // 0 for left (cond value = 1, true)
			BasicBlock *rightSucc = br->getSuccessor(1);   // 1 for right (cond value = 0, true)

			if (cond->isOne()) {   // if br's condition is true, go to left successor, notify right.
				BranchInst::Create(leftSucc, parent);
				rightSucc->removePredecessor(parent); 
			}
			else {   // if br's condition is false, go to right successor, notify left.
				BranchInst::Create(rightSucc, parent);
				leftSucc->removePredecessor(parent);
			}
		}

		inst->eraseFromParent();
	}
	
	return changed;
}

void ConstantBranch::getAnalysisUsage(AnalysisUsage& Info) const
{
	// PA5: Implement
	Info.addRequired<ConstantOps>();
}
	
} // opt
} // uscc

char uscc::opt::ConstantBranch::ID = 0;
