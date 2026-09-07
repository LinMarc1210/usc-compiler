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
//  Pass skeleton and utils: Sanjay Madhav
//  Feature: Optimized Edge Profiling via Knuth MST, tested by testEdgeProfiling.py
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
#include <llvm/Analysis/LoopInfo.h>
#include <set>
#include <map>
#include <algorithm>

using namespace llvm;

namespace uscc
{
namespace opt
{

// Feature: Optimized Edge Profiling, tested by testEdgeProfiling.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Places probes only on non-tree edges of the maximum spanning tree
bool EdgeProfilingOpt::runOnFunction(Function& F)
{
    // Skip external declarations
    if (F.isDeclaration())
        return false;

    LLVMContext& ctx = F.getContext();
    EdgeProfileData data;

    //   PA2: Implement
    // assign deterministic basic block IDs, enumerate all CFG edges.
    collectEdgesAndAssignBlockIds(F, data);

    // estimate edge weights and compute MaxST
    // (a) obtain weighted edges
    LoopInfo &LI = getAnalysis<LoopInfo>();
    std::vector<WeightedEdge> weightedEdges = estimateEdgeWeights(F, LI, data.allEdges);
    // (b) sort weight edges in decreasing order
    std::sort(weightedEdges.begin(), weightedEdges.end(), 
        [](const WeightedEdge& a, const WeightedEdge& b){
            return a.weight > b.weight;
        }
    );
    // (c) Union-Find initialization
    UnionFind uf;
    for (auto &bb : F) {
        uf.makeSet(&bb);    // every bb is a disjoint set initially
    }
    // (d) Kruskal scan
    for (auto &we : weightedEdges) {
        BasicBlock* src = we.edge.src;
        BasicBlock* dest = we.edge.dest;
        if (uf.unite(src, dest)) {    // if unite succeeds, it means src and dest are in different sets (no cycle)
            data.spanningTreeEdges.insert(we.edge);
        }
    }
    // (e) derive instrument set
    for (auto &edge : data.allEdges) {
        if (data.spanningTreeEdges.count(edge) == 0) {
            data.non_maxst_edges.push_back(edge);
        }
    }

    // create global last_src and edge_counts
    createEdgeProfilingGlobals(F, data);

    // insert selective instrumentation, only in non_maxst_edges
    std::set<BasicBlock*> needsLogDest;
    for (const auto& e : data.non_maxst_edges) {
        needsLogDest.insert(e.dest);   // dest of all non-maxst edges
    }
    for (BasicBlock &bb : F) {
        if (isa<ReturnInst>(bb.getTerminator())) {   // exit block needs logdest
            needsLogDest.insert(&bb);
        }
    }
    std::set<BasicBlock*> needsLogSrc;
    for (BasicBlock &bb : F) {
        for (succ_iterator SI = succ_begin(&bb), SE = succ_end(&bb); SI != SE; ++SI) {
            BasicBlock *succ = *SI;
            if (needsLogDest.count(succ)) {   // if any of the successors needs logdest, then this bb needs logsrc
                needsLogSrc.insert(&bb);
            }
        }
    }

    IntegerType *i32Ty = IntegerType::getInt32Ty(ctx);
    IntegerType *i64Ty = IntegerType::getInt64Ty(ctx);
    ConstantInt *numBlocksConst = ConstantInt::get(i64Ty, data.numBlocks);

    for (BasicBlock &bb : F) {
        unsigned id = data.blockId[&bb];
        IRBuilder<> builder(&bb);
        // --- logdest(id) ---
        // only instrument when bb is in non-maxst edges
        if (needsLogDest.count(&bb)) {
            builder.SetInsertPoint(bb.getFirstNonPHI());
            
            Value *lastSrc = builder.CreateLoad(data.lastSrcGlobal);
            Value *lastSrc64 = builder.CreateZExt(lastSrc, i64Ty);
            Value *idConst64 = ConstantInt::get(i64Ty, id);
            
            Value *mul = builder.CreateMul(lastSrc64, numBlocksConst);
            Value *index = builder.CreateAdd(mul, idConst64);

            Value *gepIdx[] = { ConstantInt::get(i64Ty, 0), index };
            Value *counterAddr = builder.CreateGEP(data.edgeMatrix, gepIdx);
            
            Value *count = builder.CreateLoad(counterAddr);
            Value *newCount = builder.CreateAdd(count, ConstantInt::get(i64Ty, 1));
            builder.CreateStore(newCount, counterAddr);
        }

        // --- logsrc(id) ---
        // only instrument when bb is in non-maxst edges
        if (needsLogSrc.count(&bb)) {
            builder.SetInsertPoint(bb.getTerminator());
            ConstantInt *idConst32 = ConstantInt::get(i32Ty, id);
            builder.CreateStore(idConst32, data.lastSrcGlobal);
        }
    }

    // printing
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
