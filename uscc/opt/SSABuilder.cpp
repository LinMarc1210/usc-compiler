//
//  SSABuilder.cpp
//  uscc
//
//  Implements SSABuilder class
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------
//  Interface skeleton: Sanjay Madhav
//  Reference: Braun et al. 2013 "Simple and Efficient Construction of SSA Form"
//  Feature: SSA Construction, tested by testSSA.py
//  Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
//---------------------------------------------------------

#include "SSABuilder.h"
#include "../parse/Symbols.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#pragma clang diagnostic pop

#include <list>

using namespace uscc::opt;
using namespace uscc::parse;
using namespace llvm;

// Feature: SSA Construction, tested by testSSA.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Resets SSA variable mappings for each function
void SSABuilder::reset()
{
	// PA4: Implement
	for (auto &p : mVarDefs) {
		delete p.second;
	}
	for (auto &p : mIncompletePhis) {
		delete p.second;
	}

	mVarDefs.clear();
	mIncompletePhis.clear();
	mSealedBlocks.clear();
}

// For a specific variable in a specific basic block, write its value
void SSABuilder::writeVariable(Identifier* var, BasicBlock* block, Value* value)
{
	// PA4: Implement
    if (mVarDefs.count(block) && mVarDefs[block]) {
        (*mVarDefs[block])[var] = value;
    }
}

// Read the value assigned to the variable in the requested basic block
// Will recursively search predecessor blocks if it was not written in this block
Value* SSABuilder::readVariable(Identifier* var, BasicBlock* block)
{
	// PA4: Implement
	if (mVarDefs.count(block) && mVarDefs[block] && mVarDefs[block]->count(var)) {
		return (*mVarDefs[block])[var];
	}

	return readVariableRecursive(var, block);
}

// This is called to add a new block to the maps
void SSABuilder::addBlock(BasicBlock* block, bool isSealed /* = false */)
{
	// PA4: Implement
	mVarDefs[block] = new SubMap();
	mIncompletePhis[block] = new SubPHI();
	if (isSealed) {
		sealBlock(block);
	}
}

// This is called when a block is "sealed" which means it will not have any
// further predecessors added. It will complete any PHI nodes (if necessary)
void SSABuilder::sealBlock(llvm::BasicBlock* block)
{
	// PA4: Implement
	for (auto &p : *mIncompletePhis[block]) {    // SubPHI*
		addPhiOperands(p.first, p.second);
	}
	mSealedBlocks.insert(block);
}

// Recursively search predecessor blocks for a variable
Value* SSABuilder::readVariableRecursive(Identifier* var, BasicBlock* block)
{
	Value* retVal = nullptr;

	// PA4: Implement

	llvm::Type *Ty = var->llvmType();
	llvm::Instruction* insertPoint = block->getFirstNonPHI();

	// incomplete CFG
	if (!mSealedBlocks.count(block)) {
		// 1: Type of var, 2: preserved basic blocks before, 3: name, 4: insertion point of phi
		llvm::PHINode* phi = nullptr;
		// 1) when getFirstNonPHI returns end iterator
		if (insertPoint == block->end()) {
			// give block iterator as the argument to create PHINode
			// (NO non-PHI instructions --> append the new PHI node at the end of basic block)
			phi = llvm::PHINode::Create(Ty, 0, "Phi", block);
		// 2) when getFirstNonPHI NOT returns end iterator
		} else {
			// give instruction iterator instead
			phi = llvm::PHINode::Create(Ty, 0, "Phi", insertPoint);
		}
		retVal = phi;   // upcasting
		(*mIncompletePhis[block])[var] = phi;
	}
	// return the only single predecessor. If more than one predecessor, return nullptr
	else if (llvm::BasicBlock* pred = block->getSinglePredecessor()) {
		// Optimize the common case of one predecessor: No phi needed
		retVal = readVariable(var, pred);
	}
	// Break potential cycles with operandless phi
	else {
		llvm::PHINode* phi = nullptr;
		if (insertPoint == block->end()) {
			phi = llvm::PHINode::Create(Ty, 0, "Phi", block);
		} else {
			phi = llvm::PHINode::Create(Ty, 0, "Phi", insertPoint);
		}
		retVal = phi;
		writeVariable(var, block, phi);
		retVal = addPhiOperands(var, phi);
	}
	writeVariable(var, block, retVal);

	return retVal;
}

// Adds phi operands based on predecessors of the containing block
Value* SSABuilder::addPhiOperands(Identifier* var, PHINode* phi)
{
	// PA4: Implement

	// Determine operands from predecessors
	llvm::BasicBlock* block = phi->getParent();   // get parent level
	// for pred in phi.block.pred
	for (auto PI = pred_begin(block) ; PI != pred_end(block) ; PI++) {
		// 1: value, 2: source BB
		phi->addIncoming(readVariable(var, *PI), *PI);
	}

	return tryRemoveTrivialPhi(phi);
}

// Removes trivial phi nodes
Value* SSABuilder::tryRemoveTrivialPhi(llvm::PHINode* phi)
{
	Value* same = nullptr;

	// PA4: Implement
	// traverse all the operands for PHI node
	unsigned operands = phi->getNumIncomingValues();
	for (unsigned i = 0 ; i < operands ; i++) {
		Value* op = phi->getIncomingValue(i);
		if (op == same || op == phi) {
			continue;  // Unique value or self−reference
		}
		if (same != nullptr) {
			return phi;  // The phi merges at least two values: not trivial
		}
		same = op;
	}
	if (same == nullptr) {
		same = UndefValue::get(phi->getType());  // The phi is unreachable or in the start block
	}


	// save all the users first, then remove
	// Remember all users except the phi itself
	std::vector<llvm::PHINode*> phiUsers;
	for (auto UI = phi->user_begin() ; UI != phi->user_end() ; UI++) {
		llvm::User *user = *UI;
		if (user != phi) {
			// only collected when user is a PHINode
			if (llvm::PHINode* usePhi = llvm::dyn_cast<llvm::PHINode>(user)) {
                phiUsers.push_back(usePhi);
            }
		}
	}
	// Reroute all uses of phi to same and remove phi
	// must update the variable definition map from "phi" to use "same"
	phi->replaceAllUsesWith(same);
	BasicBlock *block = phi->getParent();
	if (mVarDefs.count(block)) {
		for (auto &p : *mVarDefs[block]) {  // SubMap*
			if (p.second == phi) {
				p.second = same;
			}
		}
	}
	// erase the replaced phi node
	phi->eraseFromParent();


	// Try to recursively remove all phi users, which might have become trivial
	for (auto use : phiUsers) {
		// already done the casting before
		tryRemoveTrivialPhi(use);
	}
	return same;
}
