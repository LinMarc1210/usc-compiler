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
    //PA2 Implement

    //Iterate over all basic blocks in the function

    mDT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    // Iterate over all basic blocks in the function

    for (auto &BB : F) {
      if (visited.find(&BB) == visited.end()) {
        visited.insert(&BB);
        mHeader = &BB;
        // Start DFS from the current basic block
        dfsFindBackEdge(&BB, visited);
        BackEdge.clear();
      }
    }
    mapOutput();
    backEdgeLoopMap.clear();
    return false;


}


void NaturalLoops::dfsFindBackEdge(BasicBlock *current, set<BasicBlock *> &visited) {

      //PA2 Implement

      //Find whether there is a back edge point to the 'header'

      // Mark the current block as visited
      visited.insert(current);

      // Iterate over the successors of the current block
      for (auto succIt = succ_begin(current), succEnd = succ_end(current); succIt != succEnd; succIt++) {
        BasicBlock *succ = *succIt;
        // Check if the successor is in the current loop
        for (auto iter = succ_begin(succ), succEnd = succ_end(succ);iter != succEnd; iter++)
        {
            BasicBlock *next = *iter;
             if (next == mHeader && mDT->dominates(mHeader, succ) && BackEdge.find(succ) == BackEdge.end()) {
              // Back edge found
                //errs() << "Back Edge: " << header->getName() << " <--- " << succ->getName() << "\n";
                //printBackEdge(mHeader, succ);
                BackEdge.insert(succ);
                findNaturalLoop(succ);

             }
             break;

        }
        // Add unvisited successors to the stack
        if (visited.find(succ) == visited.end()) {
          dfsFindBackEdge(succ, visited);
          visited.erase(succ);
        }

      }
}


void NaturalLoops::findNaturalLoop(BasicBlock *backedge)
{
      //PA2 Implement


      //Find the corresponding natural loop for the back edge ('backedge block' -> 'header')
    set<string> loopBlocks;
    set<BasicBlock *> visitedSet;
    vector<BasicBlock *> DomSet;

    loopBlocks.insert(mHeader->getName());

    getDominatedBlocks(mHeader, DomSet);

    for (BasicBlock* current : DomSet){
      if(dfsReachable(current, backedge, mHeader, visitedSet))
        loopBlocks.insert(current->getName());

    }

    // Print the blocks in the natural loop
    /*errs() << "Natural Loop: ";
    for (BasicBlock *BB : loopBlocks) {
      errs() << BB->getName() << " ";
    }
    errs() << "\n";*/
    pair <string, string> edgePair;
    edgePair.first = mHeader->getName();
    edgePair.second = backedge->getName();
    backEdgeLoopMap[edgePair] = loopBlocks;
    //printNaturalLoop(loopBlocks);

}


void NaturalLoops::getDominatedBlocks(BasicBlock* block, vector<BasicBlock *>& dominatedBlocks) {
    // PA2 Implement

    // Use DominatorTree to find all blocks dominated by 'block'
    DomTreeNode* node = mDT->getNode(block);
    if (node) {
      for (auto child : *node) {
          dominatedBlocks.push_back(child->getBlock());
          getDominatedBlocks(child->getBlock(), dominatedBlocks);
      }
    }
}


bool NaturalLoops::dfsReachable(BasicBlock* current, BasicBlock* target, BasicBlock* excludeBlock,set<BasicBlock*> visitedBlocks)
{


  //PA2 Implement

  //Find whether 'current block' can reach 'target block' without visiting 'excludeBlock'.
        if (current == excludeBlock) {
            return false; // Skip visiting the excluded block
        }

        visitedBlocks.insert(current);

        if (current == target) {
            return true; // Target block is reachable
        }

        for (auto succ = succ_begin(current), end = succ_end(current); succ != end; ++succ) {
            BasicBlock* succBlock = *succ;
            if (!visitedBlocks.count(succBlock)) {
                if (dfsReachable(succBlock, target, excludeBlock, visitedBlocks)) {
                    return true; // Target block is reachable
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



