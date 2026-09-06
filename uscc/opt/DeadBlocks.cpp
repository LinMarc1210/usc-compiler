//
//  DeadBlocks.cpp
//  uscc
//
//  Implements Dead Block Removal optimization pass.
//  This removes blocks from the CFG which are unreachable.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------
#include "Passes.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>
#include <llvm/ADT/DepthFirstIterator.h>
#pragma clang diagnostic pop
#include <set>

using namespace llvm;

namespace uscc
{
namespace opt
{
	
// // 	==== version for succ_begin and succ_end ====
// void entryDFS(BasicBlock *entry, std::set<BasicBlock*> &visitedSet) {
	
// 	visitedSet.insert(entry);

// 	if (succ_begin(entry) == succ_end(entry)) {    // no successors
// 		return;
// 	}

// 	for (auto it = succ_begin(entry) ; it != succ_end(entry) ; it++) {
// 		if (!visitedSet.count(*it)) {    // if not yet visited
// 			entryDFS(*it, visitedSet);
// 		}	
// 	}
// }

bool DeadBlocks::runOnFunction(Function& F)
{
	bool changed = false;
	
	// PA5: Implement
	std::set<BasicBlock*> visitedSet;
	std::set<BasicBlock*> unreachableSet;

	// first perform a DFS from the entry block
	// used to check reachability
	// entryDFS(&F.front(), visitedSet);

	// use df_ext_begin, df_ext_end instead
	// doing DFS and add to visitedSet automatically
	for (auto it = df_ext_begin(&F.front(), visitedSet) ; it != df_ext_end(&F.front(), visitedSet) ; it++) {
		// do nothing
		// because DepthFirstIterator automatically filled it to the set.
	}

	for (auto &BB : F) {
		if (!visitedSet.count(&BB)) {
			unreachableSet.insert(&BB);
		}
	}
	for (auto *BB : unreachableSet) {
		for (auto it = succ_begin(BB) ; it != succ_end(BB) ; it++) {
			(*it)->removePredecessor(BB);
		}
		BB->eraseFromParent();
	}

	return changed;
}
	
void DeadBlocks::getAnalysisUsage(AnalysisUsage& Info) const
{
	// PA5: Implement
	// DeadBlocks pass should execute after ConstantBranch
	Info.addRequired<ConstantBranch>();
}

} // opt
} // uscc

char uscc::opt::DeadBlocks::ID = 0;
