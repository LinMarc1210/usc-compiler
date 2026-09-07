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
//  Pass skeleton: Sanjay Madhav
//  Feature: Naive Edge Profiling, tested by testEdgeProfiling.py
//  Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
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

// Feature: Naive Edge Profiling, tested by testEdgeProfiling.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Instruments every CFG edge with paired logsrc and logdest probes
bool EdgeProfiling::runOnFunction(Function& F)
{
    // Skip external declarations
    if (F.isDeclaration())
        return false;

    LLVMContext& ctx = F.getContext();
    EdgeProfileData data;

    //   PA2: Implement

    // assign deterministic basic block IDs, enumerate all CFG edges.
    collectEdgesAndAssignBlockIds(F, data);
    // create global last_src and edge_counts
    createEdgeProfilingGlobals(F, data);

    // instrument each basic block with logsrc/logdest calls
    IntegerType *i32Ty = IntegerType::getInt32Ty(ctx);
    IntegerType *i64Ty = IntegerType::getInt64Ty(ctx);
    ConstantInt* numBlocksConst = ConstantInt::get(IntegerType::getInt64Ty(ctx), data.numBlocks);
    for (auto &bb : F) {
        unsigned id = data.blockId[&bb];
        IRBuilder<> builder(&bb);
        // logdest step 1: insertion point
        builder.SetInsertPoint(bb.getFirstNonPHI());

        Value *lastSrc = builder.CreateLoad(data.lastSrcGlobal);   // member variable
        Value *lastSrc64 = builder.CreateZExt(lastSrc, i64Ty);   // zero to extend

        // logdest step 2: index = last src * numBlocks + id
        Value *idConst64 = ConstantInt::get(i64Ty, id);
        Value *mul = builder.CreateMul(lastSrc64, numBlocksConst);
        Value *index = builder.CreateAdd(mul, idConst64);

        // logdest step 3: obtain a pointer to the corresponding counter entry in edge counts by GEP
        Value *gepIdx[] = {ConstantInt::get(i64Ty, 0), index};
        Value *counterPtr = builder.CreateGEP(data.edgeMatrix, gepIdx);

        // logdest step 4: create IR to load the counter value, add 1, and store the incremented value back.
        Value *count = builder.CreateLoad(counterPtr);
        Value *newCount = builder.CreateAdd(count, ConstantInt::get(i64Ty, 1));
        builder.CreateStore(newCount, counterPtr);

        // logsrc step 1: 
        builder.SetInsertPoint(bb.getTerminator());
        ConstantInt *idConst32 = ConstantInt::get(i32Ty, id);   // not null if type is consistent
        builder.CreateStore(idConst32, data.lastSrcGlobal);
    }

    // printing
    insertEdgeProfilePrinting(F, data, "naive");
    return true;
}

void EdgeProfiling::getAnalysisUsage(AnalysisUsage& Info) const
{
    // This pass does not require any analysis results
}

} // namespace opt
} // namespace uscc

char uscc::opt::EdgeProfiling::ID = 0;
