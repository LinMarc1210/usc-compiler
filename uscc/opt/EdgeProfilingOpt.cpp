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

    // =========================================================
    // Step 1: Collect all edges and assign block IDs (provided)
    // =========================================================
    collectEdgesAndAssignBlockIds(F, data);

    if (data.allEdges.empty())
        return false;

    // =========================================================
    // Step 2: Build maximum spanning tree
    //
    // Goal: populate data.spanningTreeEdges with exactly
    //       (numBlocks - 1) edges that form a spanning tree.
    //
    // Use a MAXIMUM spanning tree so that high-frequency edges
    // (loops, hot paths) go into the tree and don't need counters,
    // while low-frequency edges become instrumented non_maxst_edges.
    //
    // Sub-steps:
    //   2a. Get estimated edge weights (provided)
    //   2b. Implement Kruskal's algorithm for maximum spanning tree
    //       - You'll need a Union-Find (disjoint set) data structure
    //       - Sort edges by weight DESCENDING
    //       - Greedily add edges that don't form cycles
    //
    // TODO: Student implements this section
    // =========================================================

    // --- Sub-step 2a: Get edge weights (provided) ---
    LoopInfo& LI = getAnalysis<LoopInfo>();
    std::vector<WeightedEdge> weightedEdges = estimateEdgeWeights(F, LI, data.allEdges);

    // --- Sub-step 2b: Kruskal's maximum spanning tree ---

    // Sort by weight descending (maximum spanning tree)
    std::sort(weightedEdges.begin(), weightedEdges.end(),
        [](const WeightedEdge& a, const WeightedEdge& b) {
            return a.weight > b.weight;
        });

    // Initialize Union-Find with all basic blocks
    UnionFind uf;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        uf.makeSet(&*BB);
    }

    // Greedily add highest-weight edges that don't form cycles
    unsigned treeEdgeCount = 0;
    unsigned targetTreeEdges = data.numBlocks - 1;
    for (const WeightedEdge& we : weightedEdges)
    {
        if (treeEdgeCount >= targetTreeEdges)
            break;
        if (uf.unite(we.edge.src, we.edge.dest))
        {
            data.spanningTreeEdges.insert(we.edge);
            treeEdgeCount++;
        }
    }

    // =========================================================
    // Step 3: Identify non_maxst_edges (non-spanning-tree edges)
    //
    // Must populate data.non_maxst_edges with all edges NOT in the
    // spanning tree. These are the edges that need instrumentation.
    //
    // TODO: Student implements this section
    // =========================================================
    for (const Edge& e : data.allEdges)
    {
        if (!data.spanningTreeEdges.count(e))
        {
            data.non_maxst_edges.push_back(e);
        }
    }

    // =========================================================
    // Step 4: Determine which blocks need instrumentation
    //
    // non_maxst_edgeDestBlocks: blocks that are the destination of at
    // least one non_maxst_edge edge. Return blocks are also included
    // so edges into them are captured.
    //
    // logsrcBlocks: blocks with at least one successor in
    // non_maxst_edgeDestBlocks. Only these blocks need logsrc.
    //
    // TODO: Student implements this section
    // =========================================================
    std::set<BasicBlock*> non_maxst_edgeDestBlocks;
    for (const Edge& c : data.non_maxst_edges)
    {
        non_maxst_edgeDestBlocks.insert(c.dest);
    }
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        if (isa<ReturnInst>(BB->getTerminator()))
        {
            non_maxst_edgeDestBlocks.insert(&*BB);
        }
    }

    std::set<BasicBlock*> logsrcBlocks;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        BasicBlock* block = &*BB;
        TerminatorInst* term = block->getTerminator();
        for (unsigned i = 0; i < term->getNumSuccessors(); ++i)
        {
            if (non_maxst_edgeDestBlocks.count(term->getSuccessor(i)))
            {
                logsrcBlocks.insert(block);
                break;
            }
        }
    }

    // =========================================================
    // Step 5: Create globals (provided)
    // =========================================================
    createEdgeProfilingGlobals(F, data);

    // =========================================================
    // Step 6: Insert selective logsrc/logdest instrumentation
    //
    // Unlike the naive pass, only:
    //   - logdest in non_maxst_edge-destination blocks
    //   - logsrc in predecessors of non_maxst_edge-destination blocks
    //
    // TODO: Student implements this section
    // =========================================================
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        BasicBlock* block = &*BB;
        unsigned destId = data.blockId[block];

        // --- logdest: only in non_maxst_edge-destination blocks ---
        if (non_maxst_edgeDestBlocks.count(block))
        {
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
        }

        // --- logsrc: only in predecessors of non_maxst_edge-dest blocks ---
        if (logsrcBlocks.count(block))
        {
            TerminatorInst* term = block->getTerminator();
            IRBuilder<> srcBuilder(term);
            srcBuilder.CreateStore(
                ConstantInt::get(IntegerType::getInt32Ty(ctx), destId),
                data.lastSrcGlobal);
        }
    }

    // =========================================================
    // Step 7: Insert profiling output (provided)
    // =========================================================
    insertEdgeProfilePrinting(F, data, "opt");

    return true;
}

void EdgeProfilingOpt::getAnalysisUsage(AnalysisUsage& Info) const
{
    Info.addRequired<LoopInfo>();
}

} // namespace opt
} // namespace uscc

char uscc::opt::EdgeProfilingOpt::ID = 0;
