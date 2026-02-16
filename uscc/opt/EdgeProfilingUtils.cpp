//
//  EdgeProfilingUtils.cpp
//  uscc
//
//  Shared utility functions for edge profiling passes.
//  See EdgeProfilingUtils.h for documentation.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "EdgeProfilingUtils.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Analysis/LoopInfo.h>
#include <sstream>
#include <stack>
#include <algorithm>

using namespace llvm;

namespace uscc
{
namespace opt
{

// =========================================================
// collectEdgesAndAssignBlockIds
// =========================================================
void collectEdgesAndAssignBlockIds(Function& F, EdgeProfileData& data)
{
    data.numBlocks = 0;

    // Assign sequential IDs to each basic block
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        BasicBlock* block = &*BB;
        data.blockId[block] = data.numBlocks++;
    }

    // Collect all edges (src -> dest) from the CFG
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        BasicBlock* block = &*BB;
        TerminatorInst* term = block->getTerminator();
        for (unsigned i = 0; i < term->getNumSuccessors(); ++i)
        {
            BasicBlock* succ = term->getSuccessor(i);
            data.allEdges.push_back(Edge(block, succ));
        }
    }
}

// =========================================================
// UnionFind
// =========================================================
void UnionFind::makeSet(BasicBlock* bb)
{
    parent[bb] = bb;
    rnk[bb] = 0;
}

BasicBlock* UnionFind::find(BasicBlock* bb)
{
    if (parent[bb] != bb)
        parent[bb] = find(parent[bb]);
    return parent[bb];
}

bool UnionFind::unite(BasicBlock* a, BasicBlock* b)
{
    a = find(a);
    b = find(b);
    if (a == b) return false;
    if (rnk[a] < rnk[b]) std::swap(a, b);
    parent[b] = a;
    if (rnk[a] == rnk[b]) rnk[a]++;
    return true;
}

// =========================================================
// estimateEdgeWeights
// =========================================================
std::vector<WeightedEdge> estimateEdgeWeights(
    Function& F,
    LoopInfo& LI,
    const std::vector<Edge>& allEdges)
{
    const unsigned long long BASE_WEIGHT = 1ULL << 20;
    const unsigned long long LOOP_MULT = 10;

    // Compute reverse post-order (RPO) for topological traversal
    std::vector<BasicBlock*> rpo;
    {
        std::vector<BasicBlock*> postOrder;
        std::set<BasicBlock*> visited;
        std::stack<std::pair<BasicBlock*, unsigned>> dfsStack;
        dfsStack.push({&F.getEntryBlock(), 0});
        visited.insert(&F.getEntryBlock());

        while (!dfsStack.empty())
        {
            auto& top = dfsStack.top();
            BasicBlock* bb = top.first;
            unsigned idx = top.second;
            TerminatorInst* term = bb->getTerminator();

            if (idx < term->getNumSuccessors())
            {
                top.second++;
                BasicBlock* succ = term->getSuccessor(idx);
                if (!visited.count(succ))
                {
                    visited.insert(succ);
                    dfsStack.push({succ, 0});
                }
            }
            else
            {
                postOrder.push_back(bb);
                dfsStack.pop();
            }
        }
        rpo.assign(postOrder.rbegin(), postOrder.rend());
    }

    // Identify back edges: (src -> dest) where dest is a loop header
    // and src is contained in that loop
    auto isBackEdge = [&](BasicBlock* src, BasicBlock* dest) -> bool {
        Loop* L = LI.getLoopFor(dest);
        if (!L) return false;
        if (L->getHeader() != dest) return false;
        return L->contains(src);
    };

    // Estimate edge weights
    std::map<Edge, unsigned long long> edgeWeight;
    std::map<BasicBlock*, unsigned long long> blockWeight;

    for (BasicBlock* bb : rpo)
    {
        // Compute incoming weight (sum of non-back-edge predecessors)
        unsigned long long incomingWeight = 0;
        if (bb == &F.getEntryBlock())
        {
            incomingWeight = BASE_WEIGHT;
        }
        else
        {
            for (auto it = pred_begin(bb), end = pred_end(bb); it != end; ++it)
            {
                BasicBlock* pred = *it;
                Edge e(pred, bb);
                if (!isBackEdge(pred, bb))
                {
                    auto wit = edgeWeight.find(e);
                    if (wit != edgeWeight.end())
                        incomingWeight += wit->second;
                }
            }
            if (incomingWeight == 0)
                incomingWeight = 1;
        }

        // Loop headers get multiplied weight
        Loop* headerLoop = LI.getLoopFor(bb);
        if (headerLoop && headerLoop->getHeader() == bb)
        {
            blockWeight[bb] = incomingWeight * LOOP_MULT;
        }
        else
        {
            blockWeight[bb] = incomingWeight;
        }

        // Distribute weight to outgoing edges
        TerminatorInst* term = bb->getTerminator();
        unsigned numSucc = term->getNumSuccessors();
        if (numSucc == 0) continue;

        // Separate loop-exit edges from other edges
        std::vector<Edge> exitEdges;
        std::vector<Edge> otherEdges;
        for (unsigned i = 0; i < numSucc; ++i)
        {
            BasicBlock* succ = term->getSuccessor(i);
            Edge e(bb, succ);
            Loop* bbLoop = LI.getLoopFor(bb);
            if (bbLoop && !bbLoop->contains(succ))
            {
                exitEdges.push_back(e);
            }
            else
            {
                otherEdges.push_back(e);
            }
        }

        unsigned long long exitWeightSum = 0;
        if (!exitEdges.empty())
        {
            unsigned long long perExit = incomingWeight / exitEdges.size();
            if (perExit == 0) perExit = 1;
            for (const Edge& e : exitEdges)
            {
                edgeWeight[e] = perExit;
                exitWeightSum += perExit;
            }
        }

        if (!otherEdges.empty())
        {
            unsigned long long remaining = blockWeight[bb];
            if (remaining > exitWeightSum)
                remaining -= exitWeightSum;
            else
                remaining = 1;
            unsigned long long perOther = remaining / otherEdges.size();
            if (perOther == 0) perOther = 1;
            for (const Edge& e : otherEdges)
            {
                edgeWeight[e] = perOther;
            }
        }
    }

    // Set back edge weights
    for (const Edge& e : allEdges)
    {
        if (isBackEdge(e.src, e.dest))
        {
            // back edge weight = block_weight - non_back_incoming
            unsigned long long nonBackIncoming = 0;
            for (auto it = pred_begin(e.dest), end = pred_end(e.dest);
                 it != end; ++it)
            {
                BasicBlock* pred = *it;
                if (!isBackEdge(pred, e.dest))
                {
                    Edge predEdge(pred, e.dest);
                    auto wit = edgeWeight.find(predEdge);
                    if (wit != edgeWeight.end())
                        nonBackIncoming += wit->second;
                }
            }
            unsigned long long bw = blockWeight[e.dest];
            if (bw > nonBackIncoming)
                edgeWeight[e] = bw - nonBackIncoming;
            else
                edgeWeight[e] = 1;
        }
    }

    // Build weighted edge vector in same order as allEdges
    std::vector<WeightedEdge> weightedEdges;
    for (const Edge& e : allEdges)
    {
        auto wit = edgeWeight.find(e);
        unsigned long long w = (wit != edgeWeight.end()) ? wit->second : 1;
        weightedEdges.push_back(WeightedEdge(e, w));
    }

    return weightedEdges;
}

// =========================================================
// createEdgeProfilingGlobals
// =========================================================
void createEdgeProfilingGlobals(Function& F, EdgeProfileData& data)
{
    Module* M = F.getParent();
    LLVMContext& ctx = F.getContext();

    // @__last_src_{func} = internal global i32 <numBlocks>
    // Initialized to numBlocks (sentinel value = no valid predecessor)
    std::string lastSrcName = "__last_src_" +  F.getName().str();
    data.lastSrcGlobal = new GlobalVariable(
        *M,
        IntegerType::getInt32Ty(ctx),
        false,  // not constant
        GlobalValue::InternalLinkage,
        ConstantInt::get(IntegerType::getInt32Ty(ctx), data.numBlocks),
        lastSrcName);

    // @__edge_matrix_{func} = internal global [(N+1)*N x i64] zeroinitializer
    // Extra row (N+1) accommodates the sentinel source (id = numBlocks)
    data.matrixSize = (data.numBlocks + 1) * data.numBlocks;
    ArrayType* matrixType = ArrayType::get(
        IntegerType::getInt64Ty(ctx), data.matrixSize);

    std::string matrixName = "__edge_matrix_" + F.getName().str();
    data.edgeMatrix = new GlobalVariable(
        *M,
        matrixType,
        false,  // not constant
        GlobalValue::InternalLinkage,
        ConstantAggregateZero::get(matrixType),
        matrixName);
}

// =========================================================
// insertEdgeProfilePrinting
// =========================================================
void insertEdgeProfilePrinting(Function& F, EdgeProfileData& data,
                               const std::string& mode)
{
    Module* M = F.getParent();
    LLVMContext& ctx = F.getContext();

    // --- Get or create printf ---
    Function* printfFunc = M->getFunction("printf");
    if (!printfFunc)
    {
        std::vector<Type*> printfArgs;
        printfArgs.push_back(PointerType::get(IntegerType::getInt8Ty(ctx), 0));
        FunctionType* printfType = FunctionType::get(
            IntegerType::getInt32Ty(ctx), printfArgs, true);
        printfFunc = Function::Create(
            printfType, Function::ExternalLinkage, "printf", M);
    }

    // --- Helper: create a global string constant ---
    auto createGlobalString = [&](const std::string& str,
                                  const std::string& name) -> Value* {
        Constant* strConstant = ConstantDataArray::getString(ctx, str, true);
        GlobalVariable* strVar = new GlobalVariable(
            *M,
            strConstant->getType(),
            true,  // constant
            GlobalValue::PrivateLinkage,
            strConstant,
            name);

        std::vector<Value*> indices;
        indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(ctx), 0));
        indices.push_back(ConstantInt::get(IntegerType::getInt64Ty(ctx), 0));
        return ConstantExpr::getGetElementPtr(strVar, indices);
    };

    std::string suffix = mode + "_" + F.getName().str();

    // --- Create format string constants ---

    // Banner
    std::string bannerText;
    if (mode == "naive")
        bannerText = "=== NAIVE EDGE PROFILING ===\n";
    else
        bannerText = "=== OPTIMIZED EDGE PROFILING (MST) ===\n";
    Value* bannerFmt = createGlobalString(bannerText,
        ".edge_banner_" + suffix);

    // Function name header
    Value* headerFmt = createGlobalString(
        "EDGE_PROFILE: " + F.getName().str() + "\n",
        ".edge_header_" + suffix);

    // Spanning tree and instrumented lines (opt mode only)
    Value* spanTreeFmt = nullptr;
    Value* instrFmt = nullptr;

    if (mode == "opt")
    {
        // Sort spanning tree edges by (srcBlockId, destBlockId)
        std::vector<Edge> sortedTree(
            data.spanningTreeEdges.begin(),
            data.spanningTreeEdges.end());
        std::sort(sortedTree.begin(), sortedTree.end(),
            [&](const Edge& a, const Edge& b) {
                unsigned aSrc = data.blockId[a.src];
                unsigned aDst = data.blockId[a.dest];
                unsigned bSrc = data.blockId[b.src];
                unsigned bDst = data.blockId[b.dest];
                return (aSrc != bSrc) ? (aSrc < bSrc) : (aDst < bDst);
            });

        std::stringstream spanTreeSS;
        spanTreeSS << "SPANNING_TREE: ";
        bool first = true;
        for (const Edge& e : sortedTree)
        {
            if (!first) spanTreeSS << ", ";
            spanTreeSS << e.src->getName().str()
                       << " -> " << e.dest->getName().str();
            first = false;
        }
        spanTreeSS << "\n";
        spanTreeFmt = createGlobalString(spanTreeSS.str(),
            ".edge_spantree_" + suffix);

        // Sort non_maxst_edges by (srcBlockId, destBlockId)
        std::vector<Edge> sortednon_maxst_edges(data.non_maxst_edges);
        std::sort(sortednon_maxst_edges.begin(), sortednon_maxst_edges.end(),
            [&](const Edge& a, const Edge& b) {
                unsigned aSrc = data.blockId[a.src];
                unsigned aDst = data.blockId[a.dest];
                unsigned bSrc = data.blockId[b.src];
                unsigned bDst = data.blockId[b.dest];
                return (aSrc != bSrc) ? (aSrc < bSrc) : (aDst < bDst);
            });

        std::stringstream instrSS;
        instrSS << "INSTRUMENTED: ";
        first = true;
        for (const Edge& e : sortednon_maxst_edges)
        {
            if (!first) instrSS << ", ";
            instrSS << e.src->getName().str()
                    << " -> " << e.dest->getName().str();
            first = false;
        }
        instrSS << "\n";
        instrFmt = createGlobalString(instrSS.str(),
            ".edge_instr_" + suffix);
    }

    // Edges header and format
    Value* edgesHeaderFmt = createGlobalString("EDGES:\n",
        ".edge_edges_header_" + suffix);
    Value* edgeFmt = createGlobalString("%s -> %s : %lld\n",
        ".edge_fmt_" + suffix);
    Value* endFmt = createGlobalString("END_PROFILE\n",
        ".edge_end_" + suffix);

    // --- Sort allEdges by (srcBlockId, destBlockId) for deterministic output ---
    std::vector<Edge> sortedEdges(data.allEdges);
    std::sort(sortedEdges.begin(), sortedEdges.end(),
        [&](const Edge& a, const Edge& b) {
            unsigned aSrc = data.blockId[a.src];
            unsigned aDst = data.blockId[a.dest];
            unsigned bSrc = data.blockId[b.src];
            unsigned bDst = data.blockId[b.dest];
            return (aSrc != bSrc) ? (aSrc < bSrc) : (aDst < bDst);
        });

    // --- Create global strings for block names ---
    std::map<std::string, Value*> nameStrings;
    for (const Edge& e : sortedEdges)
    {
        std::string srcName = e.src->getName().str();
        std::string destName = e.dest->getName().str();
        if (!nameStrings.count(srcName))
        {
            nameStrings[srcName] = createGlobalString(srcName,
                ".edge_name_" + suffix + "_" + srcName);
        }
        if (!nameStrings.count(destName))
        {
            nameStrings[destName] = createGlobalString(destName,
                ".edge_name_" + suffix + "_" + destName);
        }
    }

    // --- Find all return instructions ---
    std::vector<ReturnInst*> returns;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        if (ReturnInst* ret = dyn_cast<ReturnInst>(BB->getTerminator()))
        {
            returns.push_back(ret);
        }
    }

    // --- Insert profiling output before each return ---
    for (ReturnInst* ret : returns)
    {
        IRBuilder<> builder(ret);

        // Banner
        builder.CreateCall(printfFunc, bannerFmt);

        // Function name
        builder.CreateCall(printfFunc, headerFmt);

        // Opt-only: spanning tree and instrumented edges
        if (mode == "opt")
        {
            builder.CreateCall(printfFunc, spanTreeFmt);
            builder.CreateCall(printfFunc, instrFmt);
        }

        // Edges header
        builder.CreateCall(printfFunc, edgesHeaderFmt);

        // Print each edge with its count from the matrix
        for (const Edge& e : sortedEdges)
        {
            std::string srcName = e.src->getName().str();
            std::string destName = e.dest->getName().str();
            unsigned srcId = data.blockId[e.src];
            unsigned destId = data.blockId[e.dest];
            unsigned matIdx = srcId * data.numBlocks + destId;

            std::vector<Value*> gepIdx;
            gepIdx.push_back(ConstantInt::get(IntegerType::getInt64Ty(ctx), 0));
            gepIdx.push_back(ConstantInt::get(IntegerType::getInt64Ty(ctx), matIdx));
            Value* ptr = builder.CreateGEP(data.edgeMatrix, gepIdx);
            Value* count = builder.CreateLoad(ptr);

            std::vector<Value*> args;
            args.push_back(edgeFmt);
            args.push_back(nameStrings[srcName]);
            args.push_back(nameStrings[destName]);
            args.push_back(count);
            builder.CreateCall(printfFunc, args);
        }

        // End marker
        builder.CreateCall(printfFunc, endFmt);
    }
}

} // namespace opt
} // namespace uscc
