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
}


void NaturalLoops::dfsFindBackEdge(BasicBlock *current, set<BasicBlock *> &visited) {
    //   PA2: Implement
}


void NaturalLoops::findNaturalLoop(BasicBlock *backedge)
{
    //   PA2: Implement
}


void NaturalLoops::getDominatedBlocks(BasicBlock* block, vector<BasicBlock *>& dominatedBlocks) {
    //   PA2: Implement
}


bool NaturalLoops::dfsReachable(BasicBlock* current, BasicBlock* target, BasicBlock* excludeBlock,set<BasicBlock*> visitedBlocks)
{
    //   PA2: Implement
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



