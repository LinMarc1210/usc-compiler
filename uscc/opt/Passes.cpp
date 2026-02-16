//
//  Passes.cpp
//  uscc
//
//  Implements helper function to register passes
//  (it's here because it must be compiled with -fno-rtti
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "Passes.h"
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/PassRegistry.h>

using namespace llvm;

namespace uscc
{
namespace opt
{

void registerOptPasses(legacy::PassManager& pm)
{
	PassRegistry& pr = *PassRegistry::getPassRegistry();
	initializeLoopInfoPass(pr);
	initializeDominatorTreeWrapperPassPass(pr);
	// pm.add(new ConstantOps());
	// pm.add(new ConstantBranch());
	// pm.add(new DeadBlocks());
	// pm.add(new LICM());
	pm.add(new DominatorTreeWrapperPass());
	pm.add(new LoopInfo());
    // pm.add(new InstCombine());
}

void registerEdgeProfilingPass(legacy::PassManager& pm)
{
	pm.add(new EdgeProfiling());
}

void registerEdgeProfilingOptPass(legacy::PassManager& pm)
{
	PassRegistry& pr = *PassRegistry::getPassRegistry();
	initializeLoopInfoPass(pr);
	initializeDominatorTreeWrapperPassPass(pr);
	pm.add(new EdgeProfilingOpt());
}

void registerNaturalLoopPasses(legacy::PassManager& pm)
{
	PassRegistry& pr = *PassRegistry::getPassRegistry();
	initializeLoopInfoPass(pr);
	initializeDominatorTreeWrapperPassPass(pr);
	pm.add(new NaturalLoops());
}

} // opt
} // uscc
