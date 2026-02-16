//
//  EdgeProfilingOpt.cpp
//  uscc
//
//  Implements Optimized Edge Profiling pass using Knuth's optimal
//  counter placement algorithm. Uses the same logsrc/logdest matrix
//  approach as the naive pass, but only places logdest in blocks
//  that are destinations of non_maxst_edge edges (+ return blocks).
//
//  logsrc(block_id) only in predecessors of non_maxst_edge-dest blocks
//  logdest(block_id) only in non_maxst_edge-destination blocks: loads last_src,
//    computes edge index, increments matrix counter
//
//  At print time, only raw matrix values are printed. Tree edges
//  without logdest in their destination will show count 0.
//  No offline flow-conservation recovery is performed.
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
#include <llvm/Analysis/LoopInfo.h>
#include <set>
#include <map>
#include <algorithm>

using namespace llvm;

namespace uscc
{
namespace opt
{

bool EdgeProfilingOpt::runOnFunction(Function& F)
{
    // Skip external declarations
    if (F.isDeclaration())
        return false;

    LLVMContext& ctx = F.getContext();
    EdgeProfileData data;

    //   PA2: Implement

    return true;
}

void EdgeProfilingOpt::getAnalysisUsage(AnalysisUsage& Info) const
{
    Info.addRequired<LoopInfo>();
}

} // namespace opt
} // namespace uscc

char uscc::opt::EdgeProfilingOpt::ID = 0;
