//
//  EdgeProfilingUtils.h
//  uscc
//
//  Shared data structures and utilities for edge profiling passes.
//
//  Both the Naive and Optimized edge profiling passes use the same
//  Edge struct and EdgeProfileData to store profiling information.
//  The shared print function (insertEdgeProfilePrinting) ensures
//  consistent, deterministic output regardless of implementation.
//
//  Students MUST populate the EdgeProfileData fields as documented
//  below. The provided utility functions handle edge collection,
//  global variable creation, and output printing.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Analysis/LoopInfo.h>
#include <set>
#include <map>
#include <vector>
#include <string>

namespace uscc
{
namespace opt
{

// =========================================================
// Edge: represents a directed edge in the CFG
// =========================================================
struct Edge
{
    llvm::BasicBlock* src;
    llvm::BasicBlock* dest;

    Edge(llvm::BasicBlock* s, llvm::BasicBlock* d) : src(s), dest(d) {}

    bool operator<(const Edge& other) const
    {
        if (src != other.src) return src < other.src;
        return dest < other.dest;
    }

    bool operator==(const Edge& other) const
    {
        return src == other.src && dest == other.dest;
    }
};

struct WeightedEdge
{
    Edge edge;
    unsigned long long weight;
    WeightedEdge(const Edge& e, unsigned long long w) : edge(e), weight(w) {}
};

// =========================================================
// UnionFind: disjoint set data structure for BasicBlock pointers
//
// Used by Kruskal's algorithm to detect cycles when building
// the maximum spanning tree. Supports path compression and
// union by rank for near O(1) amortized operations.
//
// Usage:
//   UnionFind uf;
//   uf.makeSet(bb);          // initialize each block as its own set
//   BasicBlock* r = uf.find(bb);  // find representative (with path compression)
//   bool merged = uf.unite(a, b); // merge sets; returns true if they were different
// =========================================================
class UnionFind
{
    std::map<llvm::BasicBlock*, llvm::BasicBlock*> parent;
    std::map<llvm::BasicBlock*, unsigned> rnk;
public:
    void makeSet(llvm::BasicBlock* bb);
    llvm::BasicBlock* find(llvm::BasicBlock* bb);
    bool unite(llvm::BasicBlock* a, llvm::BasicBlock* b);
};

// =========================================================
// EdgeProfileData: core data structure for edge profiling
//
// Both naive and optimized passes must populate this struct.
// The shared print function reads from these fields to produce
// deterministic output.
// =========================================================
struct EdgeProfileData
{
    // --- Populated by collectEdgesAndAssignBlockIds() ---
    std::vector<Edge> allEdges;                     // All CFG edges
    std::map<llvm::BasicBlock*, unsigned> blockId;  // Block -> sequential ID
    unsigned numBlocks;                              // Total number of blocks

    // --- Populated by createEdgeProfilingGlobals() ---
    llvm::GlobalVariable* lastSrcGlobal;  // @__last_src_{func}
    llvm::GlobalVariable* edgeMatrix;     // @__edge_matrix_{func}
    unsigned matrixSize;                  // (numBlocks + 1) * numBlocks

    // --- Populated by student (optimized pass only) ---
    // For naive pass, leave these empty.
    std::set<Edge> spanningTreeEdges;     // Spanning tree edges (from DFS)
    std::vector<Edge> non_maxst_edges;             // Non-tree edges (instrumented)

    EdgeProfileData()
        : numBlocks(0)
        , lastSrcGlobal(nullptr)
        , edgeMatrix(nullptr)
        , matrixSize(0) {}
};

// =========================================================
// Shared utility functions
// =========================================================

// Collect all CFG edges and assign sequential block IDs.
// Iterates basic blocks in function order, successors in instruction order.
//
// Populates: data.allEdges, data.blockId, data.numBlocks
void collectEdgesAndAssignBlockIds(llvm::Function& F, EdgeProfileData& data);

// Estimate static edge weights using the Ball-Larus naive estimator.
// Uses LoopInfo to identify loop headers and back edges, then propagates
// weights in reverse post-order. Loop headers get a 10x multiplier.
// Loop exit edges get lower weight; back edges get residual weight.
//
// Returns a vector of WeightedEdge in the same order as allEdges.
// Higher weight = estimated higher execution frequency.
//
// Students: call this to get edge weights, then use them to build
// a maximum spanning tree (Kruskal's algorithm).
std::vector<WeightedEdge> estimateEdgeWeights(
    llvm::Function& F,
    llvm::LoopInfo& LI,
    const std::vector<Edge>& allEdges);

// Create global variables for edge profiling.
// mode: "naive" or "opt" (used in global variable naming)
//
// Creates:
//   @__last_src_{mode}_{func}   = internal global i32 <numBlocks>  (sentinel)
//   @__edge_matrix_{mode}_{func} = internal global [(N+1)*N x i64] zeroinitializer
//
// Populates: data.lastSrcGlobal, data.edgeMatrix, data.matrixSize
void createEdgeProfilingGlobals(llvm::Function& F, EdgeProfileData& data);

// Insert printf-based profiling output before every return instruction.
// mode: "naive" or "opt"
//
// For "naive" mode, prints:
//   === NAIVE EDGE PROFILING ===
//   EDGE_PROFILE: <func>
//   EDGES:
//   <src> -> <dest> : <count>   (for each edge, sorted by blockId)
//   END_PROFILE
//
// For "opt" mode, prints:
//   === OPTIMIZED EDGE PROFILING (MST) ===
//   EDGE_PROFILE: <func>
//   SPANNING_TREE: <edges sorted by blockId>
//   INSTRUMENTED: <edges sorted by blockId>
//   EDGES:
//   <src> -> <dest> : <count>   (for each edge, sorted by blockId)
//   END_PROFILE
//
// All edge lists are sorted by (srcBlockId, destBlockId) for
// deterministic output regardless of implementation details.
void insertEdgeProfilePrinting(llvm::Function& F, EdgeProfileData& data,
                               const std::string& mode);

} // namespace opt
} // namespace uscc
