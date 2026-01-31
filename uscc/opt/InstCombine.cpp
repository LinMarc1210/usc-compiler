//
//  InstCombine.cpp
//  uscc
//
//  Implements Instruction Combining optimization pass.
//  This performs peephole optimizations on binary operations,
//  comparisons, and select instructions.
//
//  Compatible with LLVM 3.5.0
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "Passes.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <set>
#include <queue>

using namespace llvm;

namespace uscc
{
namespace opt
{

bool InstCombine::runOnFunction(Function& F)
{
	bool changed = false;

    // PA1: Implement Part 1 ~ 5 of instcombine optimization

	return changed;
}

void InstCombine::getAnalysisUsage(AnalysisUsage& Info) const
{
	// This pass does not require any analysis results
}

} // namespace opt
} // namespace uscc

char uscc::opt::InstCombine::ID = 0;
