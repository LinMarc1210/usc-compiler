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

#include "Passes.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <set>
#include <queue>

using namespace llvm;

namespace uscc
{
namespace opt
{

bool InstCombine::runOnFunction(Function& F)
{
	bool changed = false;

	// We collect instructions to remove in a set, then delete them
	// after iteration to avoid invalidating iterators
	std::set<Instruction*> removeSet;

	// Iterate through all basic blocks in the function
	for (Function::iterator blockIter = F.begin(); blockIter != F.end(); ++blockIter)
	{
		// Iterate through all instructions in each basic block
		for (BasicBlock::iterator instrIter = blockIter->begin();
			 instrIter != blockIter->end(); ++instrIter)
		{
			//=============================================================
			// PART 1: Binary Operator Optimizations
			//=============================================================
			if (BinaryOperator* binOp = dyn_cast<BinaryOperator>(instrIter))
			{
				Value* lhs = binOp->getOperand(0);
				Value* rhs = binOp->getOperand(1);
				ConstantInt* lhsConst = dyn_cast<ConstantInt>(lhs);
				ConstantInt* rhsConst = dyn_cast<ConstantInt>(rhs);

				Value* replacement = nullptr;
				unsigned opcode = binOp->getOpcode();

				//=========================================================
				// 1.1: Constant Folding
				// If both operands are constants, compute the result at
				// compile time instead of runtime.
				// Example: add 3, 5 -> 8
				//=========================================================
				if (lhsConst && rhsConst)
				{
					APInt lhsVal = lhsConst->getValue();
					APInt rhsVal = rhsConst->getValue();
					APInt result;
					bool didCalc = true;

					switch (opcode)
					{
						case Instruction::Add:
							result = lhsVal + rhsVal;
							break;
						case Instruction::Sub:
							result = lhsVal - rhsVal;
							break;
						case Instruction::Mul:
							result = lhsVal * rhsVal;
							break;
						case Instruction::SDiv:
							// Avoid division by zero
							if (rhsVal != 0)
								result = lhsVal.sdiv(rhsVal);
							else
								didCalc = false;
							break;
						case Instruction::SRem:
							// Avoid division by zero
							if (rhsVal != 0)
								result = lhsVal.srem(rhsVal);
							else
								didCalc = false;
							break;
						case Instruction::And:
							result = lhsVal & rhsVal;
							break;
						case Instruction::Or:
							result = lhsVal | rhsVal;
							break;
						default:
							didCalc = false;
							break;
					}

					if (didCalc)
					{
						replacement = ConstantInt::get(binOp->getContext(), result);
					}
				}
				//=========================================================
				// 1.2: Identity and Zero Patterns (RHS is constant)
				// Simplify operations where one operand is a special value
				//=========================================================
				else if (rhsConst)
				{
					APInt rhsVal = rhsConst->getValue();

					switch (opcode)
					{
						case Instruction::Add:
						case Instruction::Sub:
						case Instruction::Or:
							// Identity: x + 0 = x, x - 0 = x, x | 0 = x
							// LLVM 3.5.0 Compatibility: APInt doesn't have isZero()
							// Use comparison: rhsVal == 0
							if (rhsVal == 0)
								replacement = lhs;
							break;

						case Instruction::Mul:
							// Zero absorbing: x * 0 = 0
							if (rhsVal == 0)
								replacement = rhsConst;
							// Identity: x * 1 = x
							// LLVM 3.5.0 Compatibility: APInt doesn't have isOne()
							// Use comparison: rhsVal == 1
							else if (rhsVal == 1)
								replacement = lhs;
							// Strength Reduction: mul x, 2^k → shl x, k
							// Multiplication by power of 2 can be replaced with left shift
							// Example: x * 8 → x << 3 (since 8 = 2^3)
							// This is faster on most architectures
							else if (rhsVal.isPowerOf2())
							{
								unsigned shiftAmt = rhsVal.logBase2();
								Instruction* newInst = BinaryOperator::Create(
									Instruction::Shl, lhs,
									ConstantInt::get(rhsConst->getType(), shiftAmt),
									"", binOp);
								replacement = newInst;
							}
							break;

						// case Instruction::UDiv:
						// 	// Strength Reduction: udiv x, 2^k → lshr x, k
						// 	// Unsigned division by power of 2 = logical right shift --> not possible in USCC
						// 	// Example: x / 8 -> x >> 3 (for unsigned x)
						// 	// Note: This only works for unsigned division!
						// 	// Signed division requires special handling due to rounding

						case Instruction::And:
							// Identity: x & -1 = x (AND with all ones returns x)
							// -1 in two's complement has all bits set to 1
							// LLVM 3.5.0 Compatibility: Use isAllOnesValue() instead of isAllOnes()
							// isAllOnesValue() returns true if all bits are 1
							// For 8-bit: 0xFF (255 or -1)
							// For 32-bit: 0xFFFFFFFF (4294967295 or -1)
							if (rhsVal.isAllOnesValue())
								replacement = lhs;
							break;

						default:
							break;
					}

					//=====================================================
					// 1.3: Reassociation with Constants
					// Fold nested operations with constants into single op
					// Also handles USC's store/load pattern by tracking
					// through load instructions to find the source operation
					//=====================================================

					// Helper: Get the source value if lhs is a load instruction
					// Searches current block and predecessor blocks for the store
					Value* innerValue = lhs;
					if (LoadInst* loadInst = dyn_cast<LoadInst>(lhs))
					{
						Value* loadPtr = loadInst->getPointerOperand();
						bool found = false;

						// First, scan backwards in the current basic block
						BasicBlock::iterator it(loadInst);
						while (it != loadInst->getParent()->begin())
						{
							--it;
							if (StoreInst* storeInst = dyn_cast<StoreInst>(it))
							{
								if (storeInst->getPointerOperand() == loadPtr)
								{
									innerValue = storeInst->getValueOperand();
									found = true;
									break;
								}
							}
						}

						// If not found in current block, search predecessor blocks
						if (!found)
						{
							std::set<BasicBlock*> visited;
							std::queue<BasicBlock*> worklist;

							// Add all predecessors of current block to worklist
							BasicBlock* currentBB = loadInst->getParent();
							for (pred_iterator PI = pred_begin(currentBB),
								 PE = pred_end(currentBB); PI != PE; ++PI)
							{
								worklist.push(*PI);
							}

							while (!worklist.empty() && !found)
							{
								BasicBlock* BB = worklist.front();
								worklist.pop();

								// Skip if already visited (handles loops)
								if (visited.count(BB))
									continue;
								visited.insert(BB);

								// Scan this block backwards (from end to begin)
								for (BasicBlock::iterator BI = BB->end(),
									 BE = BB->begin(); BI != BE; )
								{
									--BI;
									if (StoreInst* storeInst = dyn_cast<StoreInst>(BI))
									{
										if (storeInst->getPointerOperand() == loadPtr)
										{
											innerValue = storeInst->getValueOperand();
											found = true;
											break;
										}
									}
								}

								// If not found, add predecessors to worklist
								if (!found)
								{
									for (pred_iterator PI = pred_begin(BB),
										 PE = pred_end(BB); PI != PE; ++PI)
									{
										if (!visited.count(*PI))
											worklist.push(*PI);
									}
								}
							}
						}
					}

					// Reduces instruction count and enables further optimizations
					if (!replacement && opcode == Instruction::Add)
					{
						if (BinaryOperator* innerOp = dyn_cast<BinaryOperator>(innerValue))
						{
							if (innerOp->getOpcode() == Instruction::Add)
							{
								if (ConstantInt* C1 = dyn_cast<ConstantInt>(innerOp->getOperand(1)))
								{
									Value* x = innerOp->getOperand(0);
									APInt newConst = C1->getValue() + rhsVal;
									Instruction* newInst = BinaryOperator::Create(
										Instruction::Add, x,
										ConstantInt::get(rhsConst->getContext(), newConst),
										"", binOp);
									replacement = newInst;
								}
							}
						}
					}

					if (!replacement && opcode == Instruction::Mul)
					{
						if (BinaryOperator* innerOp = dyn_cast<BinaryOperator>(innerValue))
						{
							if (innerOp->getOpcode() == Instruction::Mul)
							{
								if (ConstantInt* C1 = dyn_cast<ConstantInt>(innerOp->getOperand(1)))
								{
									Value* x = innerOp->getOperand(0);
									APInt newConst = C1->getValue() * rhsVal;
									Instruction* newInst = BinaryOperator::Create(
										Instruction::Mul, x,
										ConstantInt::get(rhsConst->getContext(), newConst),
										"", binOp);
									replacement = newInst;
								}
							}
						}
					}
				}
				//=========================================================
				// 1.4: Identity Patterns (LHS is constant)
				// Handle commutative cases where constant is on the left
				//=========================================================
				else if (lhsConst)
				{
					APInt lhsVal = lhsConst->getValue();

					switch (opcode)
					{
						case Instruction::Add:
						case Instruction::Or:
							// Identity: 0 + x = x, 0 | x = x
							// These are commutative, so same as x + 0
							if (lhsVal == 0)
								replacement = rhs;
							break;

						case Instruction::Mul:
							// Zero absorbing: 0 * x = 0
							if (lhsVal == 0)
								replacement = lhsConst;
							// Identity: 1 * x = x
							else if (lhsVal == 1)
								replacement = rhs;
							break;

						case Instruction::And:
							// Identity: -1 & x = x (all ones AND x = x)
							if (lhsVal.isAllOnesValue())
								replacement = rhs;
							break;

						default:
							break;
					}
				}

				//=========================================================
				// 1.5: Same Operand Patterns (x op x)
				// Operations where both operands are the same value
				// Also handles case where both operands are loads from
				// the same memory address (common in USC-generated IR)
				//=========================================================
				bool sameOperand = (lhs == rhs);

				// Check if both operands are loads from the same address
				if (!sameOperand)
				{
					LoadInst* lhsLoad = dyn_cast<LoadInst>(lhs);
					LoadInst* rhsLoad = dyn_cast<LoadInst>(rhs);
					if (lhsLoad && rhsLoad)
					{
						// Compare the pointer operands (source addresses)
						if (lhsLoad->getPointerOperand() == rhsLoad->getPointerOperand())
						{
							sameOperand = true;
						}
					}
				}

				if (!replacement && sameOperand)
				{
					switch (opcode)
					{
						case Instruction::Sub:
							// x - x = 0: Any value minus itself is 0
							replacement = ConstantInt::get(binOp->getType(), 0);
							break;
						default:
							break;
					}
				}

				// Apply the replacement if we found one
				if (replacement)
				{
					binOp->replaceAllUsesWith(replacement);
					removeSet.insert(binOp);
					changed = true;
				}
			}
		}
	}

	//=================================================================
	// Clean up: Remove all instructions that were replaced
	// We do this after iteration to avoid invalidating iterators
	//=================================================================
	for (Instruction* i : removeSet)
	{
		i->eraseFromParent();
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
