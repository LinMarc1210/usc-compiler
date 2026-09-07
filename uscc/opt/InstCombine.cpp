//
//  InstCombine.cpp
//  uscc
//
//  Implements Instruction Combining optimization pass.
//  This performs peephole optimizations on binary operations,
//  comparisons, and select instructions.
//
//  Compatible with LLVM 3.5.0
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------
//  Pass skeleton: Sanjay Madhav
//  Feature: Instruction Combine and Constant Folding, tested by testInstcombine.py
//  Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
//---------------------------------------------------------

#include "Passes.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <set>
#include <queue>
#include <vector>

using namespace llvm;

namespace uscc
{
namespace opt
{

StoreInst *reAssociate(LoadInst *LI)
{
	Value *loadPtr = LI->getPointerOperand();
	BasicBlock *startBB = LI->getParent();    // getParent(): get the basic block that contains the instruction
	
	// (1) first, search in the same basic block
	BasicBlock::iterator it(LI);
    while (it != startBB->begin()) {
        --it;
        if (StoreInst *SI = dyn_cast<StoreInst>(&*it)) {
            if (SI->getPointerOperand() == loadPtr) {
                return SI;
            }
        }
    }

	// (2) if StoreInst is on different basic block, find all predecessors of startBB
	std::set<BasicBlock*> visited;
	std::queue<BasicBlock*> worklist;
	for (pred_iterator PI = pred_begin(startBB), E = pred_end(startBB); PI != E; ++PI) {
		worklist.push(*PI);
	}
	while (!worklist.empty()) {
		BasicBlock *BB = worklist.front();
		worklist.pop();
		if (visited.count(BB)) {
			continue;
		}
		visited.insert(BB);

		// first find closest StoreInst in current basic block
		for (BasicBlock::reverse_iterator RI = BB->rbegin(), RE = BB->rend(); RI != RE; ++RI) {
            if (StoreInst *SI = dyn_cast<StoreInst>(&*RI)) {
                if (SI->getPointerOperand() == loadPtr) {
                    return SI;
                }
            }
        }

		// if can't find, visit it's predecessors
		for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
			BasicBlock *predBB = *PI;
			if (!visited.count(predBB)) {
				worklist.push(predBB);
			}
		}
	}
	
	return nullptr;
}

// Feature: Instruction Combine and Constant Folding, tested by testInstcombine.py
// Implemented by Marc Lin <lin2315@purdue.edu, marc1210899@gmail.com>
// Performs constant folding, algebraic simplifications, and expression reassociation
bool InstCombine::runOnFunction(Function& F)
{
	bool changed = false;
	std::vector<Instruction*> toErase;

    // PA1: Implement Part 1 ~ 5 of instcombine optimization
	for (BasicBlock& BB : F) {  // Iterate basic block in function
		for (Instruction& I : BB) {  // Iterate instruction in the basic block
			BinaryOperator *BI = dyn_cast<BinaryOperator>(&I);
			if (BI) {   // if not BinaryOpt, return nullptr to BI
				ConstantInt *lhs = dyn_cast<ConstantInt>(BI->getOperand(0));  // if not ConstantInt, return nullptr to lhs
				ConstantInt *rhs = dyn_cast<ConstantInt>(BI->getOperand(1));  // if not ConstantInt, return nullptr to rhs

				// Part 1: Local Constant folding
				if (lhs && rhs) {
					APInt result = lhs->getValue();
					bool didCalc = true;
					switch (BI->getOpcode()) 
					{
						case Instruction::Add:
							result += rhs->getValue();
							break;
						case Instruction::Sub:
							result -= rhs->getValue();
							break;
						case Instruction::Mul:
							result *= rhs->getValue();
							break;
						case Instruction::SDiv:
							if (rhs->getValue() == 0) {
								didCalc = false;
							} else {
								result = result.sdiv(rhs->getValue());
							}
							break;
						case Instruction::SRem:
							if (rhs->getValue() == 0) {
								didCalc = false;
							} else {
								result = result.srem(rhs->getValue());
							}
							break;
						case Instruction::And:
							result &= rhs->getValue();
							break;
						case Instruction::Or:
							result |= rhs->getValue();
							break;
						default:
							didCalc = false;
							break;
					}
					
					if (didCalc) {
						// Replace the instruction with the constant result
						BI->replaceAllUsesWith(ConstantInt::get(BI->getType(), result));
						// Do not erase the instruction when iterating over the basic block
						toErase.push_back(&I);
						changed = true;
					}
				}
				// Part 2: Algebraic Simplication Constant (rhs)
				else if (rhs) {
					// Part 3: Re-association of Nested Expressions to Fold Constants
					if (BI->getOpcode() == Instruction::Add || BI->getOpcode() == Instruction::Mul) {
						Value* opLhs = BI->getOperand(0);
						BinaryOperator* innerBI = nullptr;

						// (1) if associate by register
						if (BinaryOperator* tempBI = dyn_cast<BinaryOperator>(opLhs)) {
							innerBI = tempBI;
						}

						// (2) if associate by memory (stack)
						else if (LoadInst* LI = dyn_cast<LoadInst>(opLhs)) {
							if (StoreInst* SI = reAssociate(LI)) {
								innerBI = dyn_cast<BinaryOperator>(SI->getValueOperand());
							}
						}

						// optimize by constant folding
						if (innerBI && innerBI->getOpcode() == BI->getOpcode()) {
							if (ConstantInt *C1 = dyn_cast<ConstantInt>(innerBI->getOperand(1))) {
								APInt combinedVal;
								if (BI->getOpcode() == Instruction::Add)
									combinedVal = C1->getValue() + rhs->getValue();
								else
									combinedVal = C1->getValue() * rhs->getValue();

								ConstantInt *C3 = ConstantInt::get(BI->getContext(), combinedVal);
								BinaryOperator *newBI = BinaryOperator::Create(
									BI->getOpcode(), 
									innerBI->getOperand(0), 
									C3, 
									"", 
									BI
								);
								BI->replaceAllUsesWith(newBI);
								toErase.push_back(BI);

								changed = true;
								continue;
							}
						}
					}

					Value* replacement = nullptr;    // operand lhs or constant
					Instruction::BinaryOps newOp = Instruction::BinaryOpsEnd;   // shl inst
					bool didCalc = true;
					switch (BI->getOpcode()) 
					{
						case Instruction::Add:
						case Instruction::Sub:
						case Instruction::Or:
							if (rhs->getValue() == 0) {
								replacement = BI->getOperand(0);
							} else {
								didCalc = false;
							}
							break;
						case Instruction::Mul:
							if (rhs->getValue() == 0) {
								replacement = ConstantInt::get(BI->getType(), 0);
							} else if (rhs->getValue() == 1) {
								replacement = BI->getOperand(0);
							} else if (rhs->getValue().isPowerOf2()) {
								newOp = Instruction::Shl;
							} else {
								didCalc = false;
							}
							break;
						case Instruction::And:
							if (rhs->getValue().isAllOnesValue()) {
								replacement = BI->getOperand(0);
							} else {
								didCalc = false;
							}
							break;
						default:
							didCalc = false;
							break;
					}
					if (didCalc) {
						if (replacement) {
							BI->replaceAllUsesWith(replacement);
						}
						else if (newOp != Instruction::BinaryOpsEnd) {
							APInt shiftAmt(BI->getType()->getIntegerBitWidth(), rhs->getValue().logBase2());
							// Create a new shift instruction
							BI->replaceAllUsesWith(BinaryOperator::Create(
								newOp,
								BI->getOperand(0),
								ConstantInt::get(BI->getType(), shiftAmt),
								"",
								BI
							));
						}
						toErase.push_back(&I);
						changed = true;
					}
				}

				// Part 4: Algebraic Simplication Constant (lhs)
				else if (lhs) {
					Value* replacement = nullptr;    // operand rhs or constant
					Instruction::BinaryOps newOp = Instruction::BinaryOpsEnd;   // shl inst
					bool didCalc = true;
					switch (BI->getOpcode()) 
					{
						case Instruction::Add:
						case Instruction::Sub:
						case Instruction::Or:
							if (lhs->getValue() == 0) {
								replacement = BI->getOperand(1);
							} else {
								didCalc = false;
							}
							break;
						case Instruction::Mul:
							if (lhs->getValue() == 0) {
								replacement = ConstantInt::get(BI->getType(), 0);
							} else if (lhs->getValue() == 1) {
								replacement = BI->getOperand(1);
							} else if (lhs->getValue().isPowerOf2()) {
								newOp = Instruction::Shl;
							} else {
								didCalc = false;
							}
							break;
						case Instruction::And:
							if (lhs->getValue().isAllOnesValue()) {
								replacement = BI->getOperand(1);
							} else {
								didCalc = false;
							}
							break;
						default:
							didCalc = false;
							break;
					}
					if (didCalc) {
						if (replacement) {
							BI->replaceAllUsesWith(replacement);
						}
						else if (newOp != Instruction::BinaryOpsEnd) {
							APInt shiftAmt(BI->getType()->getIntegerBitWidth(), lhs->getValue().logBase2());
							// Create a new shift instruction
							BI->replaceAllUsesWith(BinaryOperator::Create(
								newOp,
								BI->getOperand(1),
								ConstantInt::get(BI->getType(), shiftAmt),
								"",
								BI
							));
						}
						toErase.push_back(&I);
						changed = true;
					}
				}

				// Part 5: Same Operand Simplification
				else {
					LoadInst* lhsValue = dyn_cast<LoadInst>(BI->getOperand(0));
					LoadInst* rhsValue = dyn_cast<LoadInst>(BI->getOperand(1));
					bool didCalc = true;
					APInt result = APInt(BI->getType()->getIntegerBitWidth(), 0);
					switch (BI->getOpcode()) 
					{
						case Instruction::Sub:
							if (!(lhsValue && rhsValue && lhsValue->getPointerOperand() == rhsValue->getPointerOperand())) {
								didCalc = false;
							}
							break;
						default:
							didCalc = false;
							break;
					}
					if (didCalc) {
						BI->replaceAllUsesWith(ConstantInt::get(BI->getType(), result));
						toErase.push_back(&I);
						changed = true;
					}
				}
			}
		}
	}

	// Safe deletion after iteration
	for (Instruction* inst : toErase) {
		inst->eraseFromParent();
	}
	return changed;
}

void InstCombine::getAnalysisUsage(AnalysisUsage& Info) const
{
	// This pass does not require any analysis results
}

} // namespace opt
} // namespace uscc

char uscc::opt::InstCombine::ID = 0;
