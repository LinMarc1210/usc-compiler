//
//  EdgeProfiling.cpp
//  uscc
//
//  Implements Naive Edge Profiling pass that instruments ALL edges
//  using logsrc/logdest approach (no edge splitting).
//
//  Instead of creating intermediate basic blocks, we use paired
//  logsrc/logdest calls per basic block:
//    - logdest (at block entry, after PHI nodes): reads @__last_src,
//      computes edge index (src_id * numBlocks + dest_id), increments counter
//    - logsrc (before terminator): stores this block's ID into @__last_src
//
//  This leaves the CFG completely unchanged.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "Passes.h"
#include "EdgeProfilingUtils.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

using namespace llvm;

namespace uscc
{
namespace opt
{

bool EdgeProfiling::runOnFunction(Function& F)
{
    // Skip external declarations
    if (F.isDeclaration())
        return false;

    LLVMContext& ctx = F.getContext();
    EdgeProfileData data;

    //   PA2: Implement

    return true;
}

void EdgeProfiling::getAnalysisUsage(AnalysisUsage& Info) const
{
    // This pass does not require any analysis results
}

} // namespace opt
} // namespace uscc

char uscc::opt::EdgeProfiling::ID = 0;
