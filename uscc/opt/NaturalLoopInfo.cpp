//===- NaturalLoopInfo.cpp - Natural Loop Detector -----------------------------===//
//
//              uscc
//
// This file is distributed under the BSD License.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//


#include "Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"

#include <vector>
#include <set>
#include <stack>
#include<string>
#include<map>

using namespace llvm;
using namespace std;

std::map<std::pair<string, string>, std::set<string>> backEdgeLoopMap;

// Visited set to keep track of visited nodes during DFS
set<BasicBlock *> visited;
// Record the back edges
set<BasicBlock *> BackEdge;

namespace uscc{

namespace opt{


void NaturalLoops::printBackEdge(string head, string tail){


    errs() << "Back Edge: " << head << " <--- " << tail << "\n";

}

void NaturalLoops::printNaturalLoop(std::set<string> &naturalLoop){

    // Print the blocks in the natural loop
    errs() << "The size is "<<naturalLoop.size()<<", Natural Loop: ";
    for (auto BlockName: naturalLoop) {
      errs() << BlockName << " ";
    }
    errs() << "\n";


}

void NaturalLoops::mapOutput(){

    for(auto mapIter:backEdgeLoopMap){
      printBackEdge(mapIter.first.first, mapIter.first.second);
      printNaturalLoop(mapIter.second);
    }


}

bool NaturalLoops::runOnFunction(Function &F)
{
    //   PA2: Implement
    // mDT, mHeader are struct members of NaturalLoops class
    mDT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    for (auto &bb : F) {
        mHeader = &bb;
        visited.clear();
        dfsFindBackEdge(&bb, visited);
    }
    mapOutput();
    backEdgeLoopMap.clear();

    return false;
}


void NaturalLoops::dfsFindBackEdge(BasicBlock *current, set<BasicBlock *> &visited) {
    //   PA2: Implement
    // find if there is back edge point to current    
    stack<BasicBlock *> st;
    st.push(current);

    while (!st.empty()) {
        BasicBlock *curr = st.top();
        st.pop();

        if (visited.find(curr) != visited.end()) {
            continue;
        }
        visited.insert(curr);
        
        // iterate through immediate successors (1-level)
        // check if there is retreating edge
        for (succ_iterator SI = succ_begin(curr), SE = succ_end(curr); SI != SE; ++SI) {
            BasicBlock *succ = *SI;
            // retreating edge points to original target (mHeader)
            // because edge (B,A) means A is also B's successor, even if A is earlier in the CFG
            if (succ == mHeader) {  
                if (mDT->dominates(succ, curr)) {
                    findNaturalLoop(curr);
                }
            }

            // unvisited successor
            if (visited.find(succ) == visited.end()) {
                st.push(succ);
            }
        }
    }
}


void NaturalLoops::findNaturalLoop(BasicBlock *backedge)
{
    //   PA2: Implement
    vector<BasicBlock *> dominatedBlocks;
    getDominatedBlocks(mHeader, dominatedBlocks);
    std::pair<string, string> key = std::make_pair(mHeader->getName(), backedge->getName());
    for (auto &bb : dominatedBlocks) {
        // natural loop = nodes that can reach backedge without going through header + header
        if (dfsReachable(bb, backedge, mHeader, set<BasicBlock*>())) {
            // store the backedge into backEdgeLoopMap
            backEdgeLoopMap[key].insert(bb->getName());
        }
    }
    // add header and backedge to the loop
    backEdgeLoopMap[key].insert(mHeader->getName());
    backEdgeLoopMap[key].insert(backedge->getName());
}


void NaturalLoops::getDominatedBlocks(BasicBlock* block, vector<BasicBlock *>& dominatedBlocks) {
    //   PA2: Implement
    stack<BasicBlock*> st;
    set<BasicBlock*> dom_visited;
    st.push(block);
    
    while (!st.empty()) {
        BasicBlock *curr = st.top();
        st.pop();
        
        if (dom_visited.find(curr) != dom_visited.end()) {
            continue;
        }
        dom_visited.insert(curr);
        dominatedBlocks.push_back(curr);
        
        // use mDT.getNode(block) to get blocks that are immediately dominated
        // use node->getBlock() to get the basic block of this DT node
        DomTreeNode *node = mDT->getNode(curr);
        if (node) {
            for (DomTreeNode::iterator DI = node->begin(), DE = node->end(); DI != DE; ++DI) {
                BasicBlock *dominated = (*DI)->getBlock();
                if (dom_visited.find(dominated) == dom_visited.end()) {
                    st.push(dominated);
                }
            }
        }
    }
}


bool NaturalLoops::dfsReachable(BasicBlock* current, BasicBlock* target, BasicBlock* excludeBlock,set<BasicBlock*> visitedBlocks)
{
    //   PA2: Implement
    if (current == excludeBlock) return false;

    stack<BasicBlock*> st;
    st.push(current);

    while (!st.empty()) {
        BasicBlock *curr = st.top();
        st.pop();

        if (curr == target) { 
            return true;
        }
        if (visitedBlocks.find(curr) != visitedBlocks.end()) {
            continue;
        }
        visitedBlocks.insert(curr);
        
        for (succ_iterator SI = succ_begin(curr), SE = succ_end(curr); SI != SE; ++SI) {
            BasicBlock *succ = *SI;
            
            // unvisited successor (excluding excludeBlock)
            if (succ != excludeBlock && visitedBlocks.find(succ) == visitedBlocks.end()) {
                st.push(succ);
            }
        }
    }

    return false; // Target block is not reachable

}

void NaturalLoops::getAnalysisUsage(AnalysisUsage &Info) const
{
    Info.addRequired<LoopInfo>();
    Info.addRequired<DominatorTreeWrapperPass>();
    Info.setPreservesAll();
}

} //opt

} //uscc


char uscc::opt::NaturalLoops::ID = 0;



