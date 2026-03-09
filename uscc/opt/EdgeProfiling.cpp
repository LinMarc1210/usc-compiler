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

    // =========================================================
    // Step 1: Collect all edges and assign block IDs (provided)
    // =========================================================
    collectEdgesAndAssignBlockIds(F, data);

    if (data.allEdges.empty())
        return false;

    // =========================================================
    // Step 2: Create globals for logsrc/logdest (provided)
    // =========================================================
    createEdgeProfilingGlobals(F, data);

    // =========================================================
    // Step 3: Insert logsrc/logdest in EVERY basic block
    //
    // For the naive approach, every block gets both:
    //   - logdest at entry (after PHI nodes): load last_src,
    //     compute matrix index, increment counter
    //   - logsrc before terminator: store this block's ID
    //
    // TODO: Student implements this section
    // =========================================================
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        BasicBlock* block = &*BB;
        unsigned destId = data.blockId[block];

        // --- logdest: insert after PHI nodes ---
        Instruction* insertPt = block->getFirstNonPHI();
        IRBuilder<> destBuilder(insertPt);

        // Load last_src
        Value* srcIdVal = destBuilder.CreateLoad(
            data.lastSrcGlobal, "last.src");

        // Compute index = src_id * numBlocks + dest_id
        Value* srcIdExt = destBuilder.CreateZExt(
            srcIdVal, IntegerType::getInt64Ty(ctx), "src.ext");
        Value* index = destBuilder.CreateAdd(
            destBuilder.CreateMul(
                srcIdExt,
                ConstantInt::get(IntegerType::getInt64Ty(ctx), data.numBlocks),
                "mul.idx"),
            ConstantInt::get(IntegerType::getInt64Ty(ctx), destId),
            "edge.idx");

        // GEP into edge matrix
        std::vector<Value*> gepIndices;
        gepIndices.push_back(ConstantInt::get(IntegerType::getInt64Ty(ctx), 0));
        gepIndices.push_back(index);
        Value* counterPtr = destBuilder.CreateGEP(
            data.edgeMatrix, gepIndices, "counter.ptr");

        // Increment counter
        Value* count = destBuilder.CreateLoad(counterPtr, "count");
        Value* incCount = destBuilder.CreateAdd(
            count,
            ConstantInt::get(IntegerType::getInt64Ty(ctx), 1),
            "count.inc");
        destBuilder.CreateStore(incCount, counterPtr);

        // --- logsrc: insert before terminator ---
        TerminatorInst* term = block->getTerminator();
        IRBuilder<> srcBuilder(term);
        srcBuilder.CreateStore(
            ConstantInt::get(IntegerType::getInt32Ty(ctx), destId),
            data.lastSrcGlobal);
    }

    // =========================================================
    // Step 4: Insert profiling output (provided)
    // =========================================================
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
