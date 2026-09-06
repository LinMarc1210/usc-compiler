//
//  ASTNodes.cpp
//  uscc
//
//  Implements the emitIR function for every AST node.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "ASTNodes.h"
#include "Emitter.h"

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
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/CFG.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <set>
#include <deque>
#pragma clang diagnostic pop

#include <vector>

using namespace uscc::parse;
using namespace llvm;

#define AST_EMIT(a) llvm::Value* a::emitIR(CodeContext& ctx) noexcept

// Program/Functions
AST_EMIT(ASTProgram)
{
	ctx.mModule = new Module("main", ctx.mGlobal);

	// Write the global string table
	ctx.mStrings.emitIR(ctx);

	// Emit declaration for stdlib "printf", if we need it
	if (ctx.mPrintfIdent != nullptr)
	{
		std::vector<llvm::Type*> printfArgs;
		printfArgs.push_back(llvm::Type::getInt8PtrTy(ctx.mGlobal));

		FunctionType* printfType = FunctionType::get(llvm::Type::getInt32Ty(ctx.mGlobal),
													 printfArgs, true);

		Function* func = Function::Create(printfType, GlobalValue::LinkageTypes::ExternalLinkage,
										  "printf", ctx.mModule);
		func->setCallingConv(CallingConv::C);

		// Map the printf ident to this function
		ctx.mPrintfIdent->setAddress(func);
	}

	// Emit code for all the functions
	for (auto f : mFuncs)
	{
		f->emitIR(ctx);
        // Once the peeling happens, AST reconstruction is applied (reconstruct function)
        f->reconstruct(ctx);
        ctx.ASTConstMap.clear();
	}
	// A program actually doesn't have a value to return, since everything
	// is stored in Module
	return nullptr;
}

AST_EMIT(ASTFunction)
{
	FunctionType* funcType = nullptr;

	// First get the return type (there's only three choices)
	llvm::Type* retType = nullptr;
	if (mReturnType == Type::Int)
	{
		retType = llvm::Type::getInt32Ty(ctx.mGlobal);
	}
	else if (mReturnType == Type::Char)
	{
		retType = llvm::Type::getInt8Ty(ctx.mGlobal);
	}
	else
	{
		retType = llvm::Type::getVoidTy(ctx.mGlobal);
	}

	if (mArgs.size() == 0)
	{
		funcType = FunctionType::get(retType, false);
	}
	else
	{

		std::vector<llvm::Type*> args;
		for (auto arg : mArgs)
		{
			args.push_back(arg->getIdent().llvmType());
		}

		funcType = FunctionType::get(retType, args, false);
	}

	// Create the function, and make it the current one
	ctx.mFunc = Function::Create(funcType,
								 GlobalValue::LinkageTypes::ExternalLinkage,
								 mIdent.getName(), ctx.mModule);

	// Now that we have a new function, reset our SSA builder
	ctx.mSSA.reset();

	// Map the ident to this function
	mIdent.setAddress(ctx.mFunc);

	// Create the entry basic block
	ctx.mBlock = BasicBlock::Create(ctx.mGlobal, "entry", ctx.mFunc);
	// Add and seal this block
	ctx.mSSA.addBlock(ctx.mBlock, true);

	// If we have arguments, we need to set the name/value of them
	if (mArgs.size() > 0)
	{
		Function::arg_iterator iter = ctx.mFunc->arg_begin();
		Function::arg_iterator end = ctx.mFunc->arg_end();
		int i = 0;
		while (iter != end)
		{
			Identifier& argIdent = mArgs[i]->getIdent();
			iter->setName(argIdent.getName());

			// PA4: Remove the setAddress call
			// (Technically, iter actually has the value of the
			// arg, not its address...but we will use the address
			// member for this value)
			// argIdent.setAddress(iter);
			
			// PA4: Write to this identifier
			argIdent.writeTo(ctx, iter);

			++i;
			++iter;
		}
	}

	ctx.mFunc->setCallingConv(CallingConv::C);

	// Add all the declarations for variables created in this function
	mScopeTable.emitIR(ctx);

	// Now emit the body
	mBody->emitIR(ctx);

	return ctx.mFunc;
}

AST_EMIT(ASTArgDecl)
{
	// This node actually doesn't have anything to emit
	return nullptr;
}

AST_EMIT(ASTArraySub)
{
	// Evaluate the sub expression to get the desired index
	Value* arrayIdx = mExpr->emitIR(ctx);

	// This address should already be saved
	Value* addr = mIdent.readFrom(ctx);

	// GEP from the array address
	IRBuilder<true, NoFolder> build(ctx.mBlock);
	return build.CreateInBoundsGEP(addr, arrayIdx);
}

// Expressions

AST_EMIT(ASTBadExpr)
{
	// This node will never be emitted
	return nullptr;
}

AST_EMIT(ASTLogicalAnd)
{
	// This is extremely similar to logical or

	// Create the block for the RHS
	BasicBlock* rhsBlock = BasicBlock::Create(ctx.mGlobal, "and.rhs", ctx.mFunc);
	// Add the rhs block to SSA (not sealed)
	ctx.mSSA.addBlock(rhsBlock);

	// In both "true" and "false" condition, we'll jump to and.end
	// This is because we'll insert a phi node that assume false
	// if the and.end jump was from the lhs block
	BasicBlock* endBlock = BasicBlock::Create(ctx.mGlobal, "and.end", ctx.mFunc);
	// Also not sealed
	ctx.mSSA.addBlock(endBlock);

	// Now generate the LHS
	Value* lhsVal = mLHS->emitIR(ctx);

	BasicBlock* lhsBlock = ctx.mBlock;

	// Add the branch to the end of the LHS
	{
		IRBuilder<true, NoFolder> build(ctx.mBlock);
		// We can assume it WILL be an i32 here
		// since it'd have been zero-extended otherwise
		lhsVal = build.CreateICmpNE(lhsVal, ctx.mZero, "tobool");
		build.CreateCondBr(lhsVal, rhsBlock, endBlock);
	}

	// rhsBlock should now be sealed
	ctx.mSSA.sealBlock(rhsBlock);

	// Code should now be generated in the RHS block
	ctx.mBlock = rhsBlock;
	Value* rhsVal = mRHS->emitIR(ctx);

	// This is the final RHS block (for the phi node)
	rhsBlock = ctx.mBlock;

	// Add the branch and the end of the RHS
	{
		IRBuilder<true, NoFolder> build(ctx.mBlock);
		rhsVal = build.CreateICmpNE(rhsVal, ctx.mZero, "tobool");

		// We do an unconditional branch because the phi mode will handle
		// the correct value
		build.CreateBr(endBlock);
	}

	// endBlock should now be sealed
	ctx.mSSA.sealBlock(endBlock);

	ctx.mBlock = endBlock;

	IRBuilder<true, NoFolder> build(ctx.mBlock);

	// Figure out the value to zext
	Value* zextVal = nullptr;

	// If rhs is not also false, we need to make a phi
	if (rhsVal != ConstantInt::getFalse(ctx.mGlobal))
	{
		PHINode* phi = build.CreatePHI(llvm::Type::getInt1Ty(ctx.mGlobal), 2);
		// If we came from the lhs, it had to be false
		phi->addIncoming(ConstantInt::getFalse(ctx.mGlobal), lhsBlock);
		phi->addIncoming(rhsVal, rhsBlock);
		zextVal = phi;
	}
	else
	{
		zextVal = ConstantInt::getFalse(ctx.mGlobal);
	}

	return build.CreateZExt(zextVal, llvm::Type::getInt32Ty(ctx.mGlobal));
}

AST_EMIT(ASTLogicalOr)
{
	// Create the block for the RHS
	BasicBlock* rhsBlock = BasicBlock::Create(ctx.mGlobal, "lor.rhs", ctx.mFunc);
	// Add the rhs block to SSA (not sealed)
	ctx.mSSA.addBlock(rhsBlock);

	// In both "true" and "false" condition, we'll jump to lor.end
	// This is because we'll insert a phi node that assume true
	// if the lor.end jump was from the lhs block
	BasicBlock* endBlock = BasicBlock::Create(ctx.mGlobal, "lor.end", ctx.mFunc);
	// Also not sealed
	ctx.mSSA.addBlock(endBlock);

	// Now generate the LHS
	Value* lhsVal = mLHS->emitIR(ctx);

	BasicBlock* lhsBlock = ctx.mBlock;

	// Add the branch to the end of the LHS
	{
		IRBuilder<true, NoFolder> build(ctx.mBlock);
		// We can assume it WILL be an i32 here
		// since it'd have been zero-extended otherwise
		lhsVal = build.CreateICmpNE(lhsVal, ctx.mZero, "tobool");
		build.CreateCondBr(lhsVal, endBlock, rhsBlock);
	}

	// rhsBlock should now be sealed
	ctx.mSSA.sealBlock(rhsBlock);

	// Code should now be generated in the RHS block
	ctx.mBlock = rhsBlock;
	Value* rhsVal = mRHS->emitIR(ctx);

	// This is the final RHS block (for the phi node)
	rhsBlock = ctx.mBlock;

	// Add the branch and the end of the RHS
	{
		IRBuilder<true, NoFolder> build(ctx.mBlock);
		rhsVal = build.CreateICmpNE(rhsVal, ctx.mZero, "tobool");

		// We do an unconditional branch because the phi mode will handle
		// the correct value
		build.CreateBr(endBlock);
	}

	// endBlock should now be sealed
	ctx.mSSA.sealBlock(endBlock);

	ctx.mBlock = endBlock;

	IRBuilder<true, NoFolder> build(ctx.mBlock);

	// Figure out the value to zext
	Value* zextVal = nullptr;

	// If rhs is not also true, we need to make a phi
	if (rhsVal != ConstantInt::getTrue(ctx.mGlobal))
	{
		PHINode* phi = build.CreatePHI(llvm::Type::getInt1Ty(ctx.mGlobal), 2);
		// If we came from the lhs, it had to be false
		phi->addIncoming(ConstantInt::getTrue(ctx.mGlobal), lhsBlock);
		phi->addIncoming(rhsVal, rhsBlock);
		zextVal = phi;
	}
	else
	{
		zextVal = ConstantInt::getTrue(ctx.mGlobal);
	}

	return build.CreateZExt(zextVal, llvm::Type::getInt32Ty(ctx.mGlobal));
}

AST_EMIT(ASTBinaryCmpOp)
{
	Value* retVal = nullptr;

    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    Value * rhs = mRHS->emitIR(ctx);
    Value * lhs = mLHS->emitIR(ctx);
    switch (mOp)
    {
    case scan::Token::EqualTo:
        retVal = builder.CreateICmpEQ(lhs, rhs, "eq");
        break;
    case scan::Token::NotEqual:
        retVal = builder.CreateICmpNE(lhs, rhs, "ne");
        break;
    case scan::Token::GreaterThan:
        retVal = builder.CreateICmpSGT(lhs, rhs, "gt");
        break;
    case scan::Token::LessThan:
        retVal = builder.CreateICmpSLT(lhs, rhs, "lt");
        break;

    default:
        break;
    }

	return retVal;
}

AST_EMIT(ASTBinaryMathOp)
{
	Value* retVal = nullptr;

    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    Value * rhs = mRHS->emitIR(ctx);
    Value * lhs = mLHS->emitIR(ctx);
    switch (mOp)
    {
    case scan::Token::Plus:
        retVal = builder.CreateAdd(lhs, rhs, "add");
        break;
    case scan::Token::Minus:
        retVal = builder.CreateSub(lhs, rhs, "sub");
        break;
    case scan::Token::Mult:
        retVal = builder.CreateMul(lhs, rhs, "mul");
        break;
    case scan::Token::Div:
        retVal = builder.CreateSDiv(lhs, rhs, "div");
        break;
    case scan::Token::Mod:
        retVal = builder.CreateSRem(lhs, rhs, "mod");
        break;

    default:
        break;
    }

	return retVal;
}

// Value -->
AST_EMIT(ASTNotExpr)
{
	Value* retVal = nullptr;

    IRBuilder<true, NoFolder> builder(ctx.mBlock);
	auto value = mExpr->emitIR(ctx);
    value = builder.CreateICmpEQ(value, ctx.mZero);
    retVal = builder.CreateZExt(value, llvm::Type::getInt32Ty(ctx.mGlobal));

	return retVal;
}

// Factor -->
AST_EMIT(ASTConstantExpr)
{
	Value* retVal = nullptr;

    if (mType == Type::Char)
        retVal = ConstantInt::get(llvm::Type::getInt8Ty(ctx.mGlobal), mValue);
    else
        retVal = ConstantInt::get(llvm::Type::getInt32Ty(ctx.mGlobal), mValue);

	return retVal;
}

AST_EMIT(ASTStringExpr)
{
	return mString->getValue();
}

AST_EMIT(ASTIdentExpr)
{
	return mIdent.readFrom(ctx);
}

AST_EMIT(ASTArrayExpr)
{
	// Generate the array subscript, which'll give us the address
	Value* addr = mArray->emitIR(ctx);

	IRBuilder<true, NoFolder> build(ctx.mBlock);
	// Now load this value and return

	// NOTE: This still needs to be a load because arrays are in memory
	return build.CreateLoad(addr);
}

AST_EMIT(ASTFuncExpr)
{

	// At this point, we can assume the argument types match
	// Create the list of arguments
	std::vector<Value*> callList;
	for (auto arg : mArgs)
	{
		Value* argValue = arg->emitIR(ctx);
		// If this is an array or ptr, we need to change this to a getelemptr
		// (Provided it already isn't one)
		if (!isa<GetElementPtrInst>(argValue) &&
			argValue->getType()->isPointerTy())
		{
			if (argValue->getType()->getPointerElementType()->isArrayTy())
			{
				IRBuilder<true, NoFolder> build(ctx.mBlock);
				std::vector<llvm::Value*> gepIdx;
				gepIdx.push_back(ctx.mZero);
				gepIdx.push_back(ctx.mZero);

				argValue = build.CreateInBoundsGEP(argValue, gepIdx);
			}
			else
			{
				IRBuilder<true, NoFolder> build(ctx.mBlock);
				// Need to return the address of the specific index in question
				// So need a GEP
				argValue = build.CreateInBoundsGEP(argValue, ctx.mZero);
			}
		}

		callList.push_back(argValue);
	}

	// Now call the function, and return it
	Value* retVal = nullptr;

	IRBuilder<true, NoFolder> build(ctx.mBlock);
	if (mType != Type::Void)
	{
		retVal = build.CreateCall(mIdent.getAddress(), callList, "call");
	}
	else
	{
		retVal = build.CreateCall(mIdent.getAddress(), callList);
	}

	return retVal;
}

AST_EMIT(ASTIncExpr)
{
	Value* retVal = nullptr;

    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    auto value = mIdent.readFrom(ctx);
    value = builder.CreateAdd(value, ConstantInt::get(value->getType(), 1), "inc"); // use the same type as value
    mIdent.writeTo(ctx, value);
    retVal = value;

	return retVal;
}

AST_EMIT(ASTDecExpr)
{
	Value* retVal = nullptr;

    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    auto value = mIdent.readFrom(ctx);
    value = builder.CreateSub(value, ConstantInt::get(value->getType(), 1), "dec");
    mIdent.writeTo(ctx, value);
    retVal = value;

	return retVal;
}

AST_EMIT(ASTAddrOfArray)
{
	return mArray->emitIR(ctx);
}

AST_EMIT(ASTToIntExpr)
{
	Value* exprVal = mExpr->emitIR(ctx);
	IRBuilder<true, NoFolder> build(ctx.mBlock);
	return build.CreateSExt(exprVal, llvm::Type::getInt32Ty(ctx.mGlobal), "conv");
}

AST_EMIT(ASTToCharExpr)
{
	Value* exprVal = mExpr->emitIR(ctx);
	IRBuilder<true, NoFolder> build(ctx.mBlock);
	return build.CreateTrunc(exprVal, llvm::Type::getInt8Ty(ctx.mGlobal), "conv");
}

// Declaration
AST_EMIT(ASTDecl)
{
	// If there's an expression, emit this also and store it in the ident
	if (mExpr)
	{
		Value* declExpr = mExpr->emitIR(ctx);

		IRBuilder<true, NoFolder> build(ctx.mBlock);
		// If this is a string, we have to memcpy
		if (declExpr->getType()->isPointerTy())
		{
			// This address should already be saved
			Value* arrayLoc = mIdent.readFrom(ctx);

			// GEP the address of the src
			std::vector<llvm::Value*> gepIdx;
			gepIdx.push_back(ctx.mZero);
			gepIdx.push_back(ctx.mZero);

			Value*  src = build.CreateGEP(declExpr, gepIdx);

			// Memcpy into the array
			// memcpy(dest, src, size, align, volatile)
			build.CreateMemCpy(arrayLoc, src, mIdent.getArrayCount(), 1);
		}
		else
		{
			// Basic types can just be written
			mIdent.writeTo(ctx, declExpr);
		}
	}

	return nullptr;
}

// Statements
AST_EMIT(ASTCompoundStmt)
{
    for (auto & x : mDecls)
        x->emitIR(ctx);
    for (auto & x : mStmts)
        x->emitIR(ctx);

	return nullptr;
}

AST_EMIT(ASTAssignStmt)
{
	// This is simpler than decl because we don't allow
	// assignments to happen later for full arrays

    auto value = mExpr->emitIR(ctx);
    mIdent.writeTo(ctx, value);

	return nullptr;
}

AST_EMIT(ASTAssignArrayStmt)
{
	// Generate the expression
	Value* exprVal = mExpr->emitIR(ctx);

	// Generate the array subscript, which'll give us the address
	Value* addr = mArray->emitIR(ctx);

	IRBuilder<true, NoFolder> build(ctx.mBlock);

	// NOTE: This is still a create store because arrays are always stack-allocated
	build.CreateStore(exprVal, addr);

	return nullptr;
}

AST_EMIT(ASTIfStmt)
{
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    auto value = this->mExpr->emitIR(ctx);
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    if (!value->getType()->isIntegerTy(1))
        value = builder.CreateICmpNE(value, ctx.mZero);

    auto thenBody = BasicBlock::Create(ctx.mGlobal, "if.then", ctx.mFunc);
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    ctx.mSSA.addBlock(thenBody, true);
    auto end = BasicBlock::Create(ctx.mGlobal, "if.end", ctx.mFunc);
    ctx.mSSA.addBlock(end);
    if (mElseStmt)
    {
        auto elseBody = BasicBlock::Create(ctx.mGlobal, "if.else", ctx.mFunc);
        ctx.mSSA.addBlock(elseBody, true);
        builder.CreateCondBr(value, thenBody, elseBody);

        ctx.mBlock = elseBody;
        mElseStmt->emitIR(ctx);
        // New fix for break & continue statement
        if (!ctx.mBlock->getTerminator())
        {
            IRBuilder<true, NoFolder> builderElse(ctx.mBlock);
            builderElse.CreateBr(end);
        }
    }
    else
        builder.CreateCondBr(value, thenBody, end);

    ctx.mBlock = thenBody;
    mThenStmt->emitIR(ctx);
    if (!ctx.mBlock->getTerminator())
    {
        IRBuilder<true, NoFolder> builderThen(ctx.mBlock);
        builderThen.CreateBr(end);
    }

    ctx.mSSA.sealBlock(end);
    ctx.mBlock = end;

	return nullptr;
}

AST_EMIT(ASTWhileStmt)
{
    this->llvmBasicBlock = ctx.mBlock;
    if (ctx.PeelingEnabled && CanPeel(ctx)) { // Check whether the 1st-loop-iteration peeling is applicable
      emitIR_LoopPeeling(ctx);
      this->mPeeling = true;
    }
    else {
      // PA4: Register each new basic block with ctx.mSSA.addBlock()
      // and seal blocks once all predecessors are known
      auto condBlock = BasicBlock::Create(ctx.mGlobal, "while.cond", ctx.mFunc);
      // PA4: Register each new basic block with ctx.mSSA.addBlock()
      // and seal blocks once all predecessors are known
      ctx.mSSA.addBlock(condBlock);

      IRBuilder<true, NoFolder> builder(ctx.mBlock);
      builder.CreateBr(condBlock); // unconditional branch in predecessor

      ctx.mBlock = condBlock;
      auto value = this->mExpr->emitIR(ctx);
      IRBuilder<true, NoFolder> builderCond(ctx.mBlock);
      if (!value->getType()->isIntegerTy(1))
          value = builderCond.CreateICmpNE(value, ctx.mZero);

      auto body = BasicBlock::Create(ctx.mGlobal, "while.body", ctx.mFunc); // after expr's emitIR
      ctx.mSSA.addBlock(body, true);
      auto endBlock = BasicBlock::Create(ctx.mGlobal, "while.end", ctx.mFunc);
      ctx.mSSA.addBlock(endBlock, true);
      builderCond.CreateCondBr(value, body, endBlock); // conditional branch in while.cond

      ctx.mBlock = body;

      // Push the continue/break targets
      ctx.mContinueBlocks.push(condBlock);
      ctx.mBreakBlocks.push(endBlock);

      this->mLoopStmt->emitIR(ctx);

      // Pop the continue/break targets
      ctx.mContinueBlocks.pop();
      ctx.mBreakBlocks.pop();

      IRBuilder<true, NoFolder> builderBody(ctx.mBlock);
      builderBody.CreateBr(condBlock);
      ctx.mSSA.sealBlock(condBlock);
      ctx.mBlock = endBlock;
    }

	return nullptr;
}

AST_EMIT(ASTReturnStmt)
{
	IRBuilder<true, NoFolder> builder(ctx.mBlock);
    if (mExpr)
        builder.CreateRet(mExpr->emitIR(ctx));
    else
        builder.CreateRetVoid();

	return nullptr;
}

AST_EMIT(ASTExprStmt)
{
	// Emit the expression, just return the value
    auto retVal = mExpr->emitIR(ctx);
	return retVal;
}

AST_EMIT(ASTNullStmt)
{
	// Doesn't do anything (hence empty)
	return nullptr;
}

AST_EMIT(ASTBreakStmt)
{
    // Get the current break target from the context stack
    llvm::BasicBlock* breakTarget = ctx.mBreakBlocks.top();

    // Create an unconditional branch to that target
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    builder.CreateBr(breakTarget);
	return nullptr;
}

AST_EMIT(ASTContinueStmt)
{
    // Get the current continue target from the context stack
    llvm::BasicBlock* continueTarget = ctx.mContinueBlocks.top();

    // Create an unconditional branch to that target
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    builder.CreateBr(continueTarget);
	return nullptr;
}

AST_EMIT(ASTForStmt)
{
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    // 1. Create all the basic blocks needed for the loop structure
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(ctx.mGlobal, "for.cond", ctx.mFunc);
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    ctx.mSSA.addBlock(condBlock);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(ctx.mGlobal, "for.body", ctx.mFunc);
    ctx.mSSA.addBlock(bodyBlock, true);
    llvm::BasicBlock* stepBlock = llvm::BasicBlock::Create(ctx.mGlobal, "for.step", ctx.mFunc);
    ctx.mSSA.addBlock(stepBlock, true);
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx.mGlobal, "for.end", ctx.mFunc);
    ctx.mSSA.addBlock(endBlock, true);

    // 2. Emit the initialization statement in the block preceding the loop
    minitStmt->emitIR(ctx);

    // Unconditionally jump into the condition check to start the loop
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    builder.CreateBr(condBlock);

    // 3. Populate the for.cond block
    ctx.mBlock = condBlock;
    IRBuilder<true, NoFolder> condBuilder(ctx.mBlock);

    auto condValue = this->mcondExpr->emitIR(ctx);

    // Convert condition to a boolean (i1) and create the conditional branch
    if (!condValue->getType()->isIntegerTy(1))
        condValue = condBuilder.CreateICmpNE(condValue, ctx.mZero, "tobool");

    condBuilder.CreateCondBr(condValue, bodyBlock, endBlock);

    // 4. Populate the for.body block
    ctx.mBlock = bodyBlock;

    // PUSH the targets for break and continue
    ctx.mBreakBlocks.push(endBlock);
    ctx.mContinueBlocks.push(stepBlock);

    // Emit the IR for the main loop body
    mbodyStmt->emitIR(ctx);

    // POP the targets after the body is finished
    ctx.mBreakBlocks.pop();
    ctx.mContinueBlocks.pop();

    // After the body, unconditionally jump to the step block
    IRBuilder<true, NoFolder> bodyBuilder(ctx.mBlock);
    bodyBuilder.CreateBr(stepBlock);

    // 5. Populate the for.step block
    ctx.mBlock = stepBlock;

    mstepStmt->emitIR(ctx);

    // After the step, unconditionally jump back to the condition check
    IRBuilder<true, NoFolder> stepBuilder(ctx.mBlock);
    stepBuilder.CreateBr(condBlock);
    ctx.mSSA.sealBlock(condBlock);

    // 6. Set the end block as the new insertion point for subsequent code
    ctx.mBlock = endBlock;

	return nullptr;
}

AST_EMIT(ASTDoWhileStmt)
{
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    // 1. Create the basic blocks for the loop structure
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(ctx.mGlobal, "dowhile.body", ctx.mFunc);
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    ctx.mSSA.addBlock(bodyBlock);
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(ctx.mGlobal, "dowhile.cond", ctx.mFunc);
    ctx.mSSA.addBlock(condBlock, true);
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx.mGlobal, "dowhile.end", ctx.mFunc);
    ctx.mSSA.addBlock(endBlock, true);

    // 2. Unconditionally jump from the preheader into the body
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    builder.CreateBr(bodyBlock);

    // 3. Populate the dowhile.body block
    ctx.mBlock = bodyBlock;

    // Push the continue/break targets
    ctx.mContinueBlocks.push(condBlock);
    ctx.mBreakBlocks.push(endBlock);

    // Emit the IR for the loop's statement(s)
    this->mLoopStmt->emitIR(ctx);

    // Pop the continue/break targets
    ctx.mContinueBlocks.pop();
    ctx.mBreakBlocks.pop();

    // After the body, unconditionally jump to the condition check
    IRBuilder<true, NoFolder> bodyBuilder(ctx.mBlock);
    bodyBuilder.CreateBr(condBlock);

    // 4. Populate the dowhile.cond block
    ctx.mBlock = condBlock;
    IRBuilder<true, NoFolder> condBuilder(ctx.mBlock);

    llvm::Value* condValue = mExpr->emitIR(ctx);

    // Create the conditional branch
    if (!condValue->getType()->isIntegerTy(1))
        condValue = condBuilder.CreateICmpNE(condValue, ctx.mZero, "tobool");

    condBuilder.CreateCondBr(condValue, bodyBlock, endBlock);
    ctx.mSSA.sealBlock(bodyBlock);

    // 5. Set the new current block to dowhile.end
    ctx.mBlock = endBlock;

	return nullptr;
}

AST_EMIT(ASTSwitchStmt)
{
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    // 1. Emit the value we are switching on
    llvm::Value* switchValue = mExpr->emitIR(ctx);

    // 2. Create all the necessary basic blocks
    llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx.mGlobal, "switch.end", ctx.mFunc);
    // PA4: Register each new basic block with ctx.mSSA.addBlock()
    // and seal blocks once all predecessors are known
    ctx.mSSA.addBlock(endBlock);  // unsealed: predecessors from breaks/fallthrough
    llvm::BasicBlock* defaultBlock = nullptr;

    // This map links an AST node to its corresponding LLVM block for easy lookup
    std::map<ASTCaseDefaultStmt*, llvm::BasicBlock*> blockMap;

    // Create blocks for all case statements and find the default statement
    for (const auto& stmt : case_default_stmts)
    {
        if (std::dynamic_pointer_cast<ASTCaseStmt>(stmt))
        {
            llvm::BasicBlock* caseBlock = llvm::BasicBlock::Create(ctx.mGlobal, "switch.case", ctx.mFunc);
            ctx.mSSA.addBlock(caseBlock);
            blockMap[stmt.get()] = caseBlock;
        }
        else if (std::dynamic_pointer_cast<ASTDefaultStmt>(stmt))
        {
            defaultBlock = llvm::BasicBlock::Create(ctx.mGlobal, "switch.default", ctx.mFunc);
            ctx.mSSA.addBlock(defaultBlock);
            blockMap[stmt.get()] = defaultBlock;
        }
    }

    // If no explicit default case exists, non-matching values will jump to the end of the switch
    if (!defaultBlock)
    {
        defaultBlock = endBlock;
    }

    // 3. Create the main 'switch' instruction
    IRBuilder<true, NoFolder> builder(ctx.mBlock);
    llvm::SwitchInst* switchInst = builder.CreateSwitch(switchValue, defaultBlock, case_default_stmts.size());

    // 4. Add each 'case' as a destination to the switch instruction
    for (const auto& stmt : case_default_stmts)
    {
        if (auto caseNode = std::dynamic_pointer_cast<ASTCaseStmt>(stmt))
        {
            llvm::BasicBlock* destBlock = blockMap[caseNode.get()];
            llvm::Value* caseValueRaw = caseNode->getExpr()->emitIR(ctx);
            llvm::ConstantInt* caseValue = llvm::dyn_cast<llvm::ConstantInt>(caseValueRaw);

            if (caseValue)
            {
                switchInst->addCase(caseValue, destBlock);
            }
        }
    }

    // 5. Push the endBlock as the target for any 'break' statements
    ctx.mBreakBlocks.push(endBlock);

    // 6. Emit the IR for each case/default body and handle fall-through
    for (size_t i = 0; i < case_default_stmts.size(); ++i)
    {
        const auto& stmtNode = case_default_stmts[i];

        ctx.mBlock = blockMap[stmtNode.get()];
        ctx.mSSA.sealBlock(blockMap[stmtNode.get()]);

        stmtNode->emitIR(ctx);

        if (!ctx.mBlock->getTerminator())
        {
            IRBuilder<true, NoFolder> fallthroughBuilder(ctx.mBlock);
            llvm::BasicBlock* nextBlock = defaultBlock;
            if (i + 1 < case_default_stmts.size())
            {
                nextBlock = blockMap[case_default_stmts[i+1].get()];
            }
            fallthroughBuilder.CreateBr(nextBlock);
        }
    }

    // 7. Pop the break block target
    ctx.mBreakBlocks.pop();
    ctx.mSSA.sealBlock(endBlock);

    // 8. Set the end block as the new insertion point for subsequent code
    ctx.mBlock = endBlock;

	return nullptr;
}

AST_EMIT(ASTCaseStmt)
{
    // This function is now responsible for emitting all statements within the default block.
    for (const auto& innerStmt : mStmts)
    {
        // If a terminator (like break or return) is hit, stop emitting.
        if (ctx.mBlock->getTerminator())
        {
            break;
        }
        innerStmt->emitIR(ctx);
    }
	return nullptr;
}

AST_EMIT(ASTDefaultStmt)
{
    // This function is now responsible for emitting all statements within the default block.
    for (const auto& innerStmt : mStmts)
    {
        // If a terminator (like break or return) is hit, stop emitting.
        if (ctx.mBlock->getTerminator())
        {
            break;
        }
        innerStmt->emitIR(ctx);
    }
	return nullptr;
}

// If killedDef is found in the use (operands) of any instruction in cond blocks -> return false
// Otherwise -> return true
bool CondBBsNotUseKilledDef(BasicBlock *CondBB, std::list<llvm::Value*> &killedDef, std::set<BasicBlock*> &visited) {
  visited.insert(CondBB);
  for (auto &II : *CondBB) {
    for (unsigned i = 0; i < II.getNumOperands(); i++) {
      // If any operand uses the rewritten variables (operands) in preheader, return false
      if (std::find(killedDef.begin(), killedDef.end(), II.getOperand(i)) != killedDef.end()) {
        return false;
      }
    }
    // 1) If there's any redefinition (rewriting) of any variables which are used in the condition, the loop peeling isn't possible
    // preheader BB: x++
    // condition BB: while (x < 10) { ... }
    // ------------------------------------------
    // preheader BB: a = 10;
    // preheader BB: x = a + 100;
    // condition BB: while (x < 10) { ... }
    // ------------------------------------------
    // 2) If there's declaration which are relevant with the condition, the loop peeling isn't possible
    // preheader BB: int y = 100
    // condition BB: while (y < 10000) { ... }
    // ------------------------------------------
    llvm::Value * rdef = dyn_cast<llvm::Value>(&II);
    if (std::find(killedDef.begin(), killedDef.end(), rdef) != killedDef.end()) {
      killedDef.remove(rdef);
    }
  }
  bool ret = true;
  for (succ_iterator succ = succ_begin(CondBB), succ_endBB = succ_end(CondBB); succ != succ_endBB; ++succ) {
    if (!visited.count(*succ))
      ret = ret && CondBBsNotUseKilledDef(*succ, killedDef, visited);
  }
  return ret;
}

void computePostOrder(BasicBlock *entry, std::set<BasicBlock *> &visited, std::list<BasicBlock *> &order)
{
  visited.insert(entry);
  auto succItr = succ_begin(entry), end = succ_end(entry);
  for (; succItr != end; ++succItr)
    if (!visited.count(*succItr))
      computePostOrder(*succItr, visited, order);
  order.push_back(entry);
}

// Compute BFS traversal order starting from BB
std::list<BasicBlock*> computeBFSOrder(CodeContext& ctx, BasicBlock* BB) {
  std::set<BasicBlock*> visited;
  std::list<BasicBlock *> toBeRemoved;
  computePostOrder(BB, visited, toBeRemoved);
  toBeRemoved.reverse();
  return toBeRemoved;
}

void ASTWhileStmt::getAllocaList(CodeContext& ctx) {
  this->AllocaList.clear();
  auto FirstBB = ctx.mFunc->begin();
  for (auto &II : *FirstBB) {
    if (AllocaInst* inst = dyn_cast<llvm::AllocaInst>(&II)) {
      this->AllocaList.push_back(inst);
    }
  }
}

void ASTWhileStmt::getKilledDefsInPreheader(CodeContext& ctx) {
  this->killedDefInPreheader.clear();
  auto FirstBB = ctx.mFunc->begin();
  if (ctx.mBlock != FirstBB) {
    for (auto &II : *ctx.mBlock) {
      if (isa<llvm::StoreInst>(II)) {
        // Overwriting of the alloca variables: Find the redefined operand
        if (std::find(this->AllocaList.begin(), this->AllocaList.end(), II.getOperand(1)) != this->AllocaList.end()) {
          this->killedDefInPreheader.push_back(II.getOperand(1));
        }
      }
    }
  }
  else { // when preheader is the first basicblock of the program
    std::unordered_map<llvm::Value*, int> defs;
    for (auto value : this->AllocaList)
      defs[value] = 0;
    for (auto &II : *ctx.mBlock) {
      if (isa<llvm::StoreInst>(II)) {
        // Overwriting of the alloca variables: Find the redefined operand
        if (std::find(this->AllocaList.begin(), this->AllocaList.end(), II.getOperand(1)) != this->AllocaList.end()) {
          defs[II.getOperand(1)]++;
        }
      }
    }
    for (auto def : defs) {
      if (def.second > 1) { // second =1: Initial def after alloca. second > 1: This means redef happens
        this->killedDefInPreheader.push_back(def.first);
      }
    }
  }
}


bool ASTWhileStmt::CanPeel(CodeContext& ctx) {
  auto curBlock = ctx.mBlock;
  // 1) Get all alloca from the first basic block in the function
  getAllocaList(ctx);

  // 2) Find the defs (allocas) that are re-defined in the preheader
  getKilledDefsInPreheader(ctx);

  // 3) Find whether the killed allocas (killedDefInPreheader) are used in the condition
  auto CondBB = BasicBlock::Create(ctx.mGlobal, "CondBB", ctx.mFunc);
  ctx.mBlock = CondBB;
  auto value = this->mExpr->emitIR(ctx);

  std::set<BasicBlock*> visited;
  bool ret = CondBBsNotUseKilledDef(CondBB, killedDefInPreheader, visited);

  // 4) Remove CondBB and the successor blocks generated here
  std::list<BasicBlock*> toBeRemoved = computeBFSOrder(ctx, CondBB);
  for (auto BB : toBeRemoved) {
    if (BB == toBeRemoved.back()) break;
    llvm::DeleteDeadBlock(BB);
  }
  toBeRemoved.back()->eraseFromParent();

  ctx.mBlock = curBlock;
  return ret;
}

BasicBlock* replicate_basicblock(CodeContext& ctx) {
  BasicBlock* NewBB = BasicBlock::Create(ctx.mGlobal, "replicated", ctx.mFunc);

  std::unordered_map<Instruction*, Instruction*> fixedInstList;
  for (BasicBlock::iterator II = ctx.mBlock->begin(); II != ctx.mBlock->end(); ++II) {
    if (isa<llvm::AllocaInst>(*II))
      continue;
    Instruction *newI;
    std::set<Instruction*> myusers;
    for (auto U : II->users()) {
      if (auto I = dyn_cast<Instruction>(U))
        myusers.insert(dyn_cast<Instruction>(U));
    }
    if (fixedInstList.find(II) == fixedInstList.end())
      newI = II->clone();
    else
      newI = fixedInstList[II];

    BasicBlock::iterator iter = II;
    iter++;
    while (iter != ctx.mBlock->end()) {
      Instruction* nextI = dyn_cast<Instruction>(iter);
      if (myusers.find(nextI) != myusers.end()) {
        Instruction *newIUser;
        if (fixedInstList.find(nextI) != fixedInstList.end()) {
          newIUser = fixedInstList[nextI];
        }
        else {
          newIUser = nextI->clone();
          fixedInstList[nextI] = newIUser;
        }
        for (unsigned i = 0; i < newIUser->getNumOperands(); i++) {
          if (dyn_cast<Value>(newIUser->getOperand(i)) == dyn_cast<Value>(II)) {
            newIUser->setOperand(i, newI);
          }
        }
      }
      iter++;
    }
    if (II->hasName())
      newI->setName(II->getName());
    NewBB->getInstList().push_back(newI);
  } // for
  return NewBB;
}

void ASTWhileStmt::RemovePreHeader(CodeContext& ctx) {
  // 2-1) AllocaInst: These should be in the preheader
  // 2-2) CallInst: llvm.memcpy should remain here.
  // 2-3) StoreInst: "CanPeel() = true" means there's no re-written operands which are used in the cond
  // But there can be the re-written operands irrelevant with the cond (killedDefInPreheader)
  // If the given StoreInst stores to the operand in killedDefInPreheader, then remove it.
  std::list<Instruction*>toBeRemoved;
  for (auto &I : *ctx.mBlock) {
    if (isa<llvm::AllocaInst>(I)) continue;
    else if (isa<llvm::StoreInst>(I)) {
      if (std::find(this->killedDefInPreheader.begin(), this->killedDefInPreheader.end(), I.getOperand(1)) != this->killedDefInPreheader.end()) {
        toBeRemoved.push_back(&I);
      }
    }
    else if (isa<llvm::CallInst>(I)) {
      StringRef name = cast<llvm::CallInst>(I).getCalledFunction()->getName();
      if (!name.startswith("llvm.memcpy"))
        toBeRemoved.push_back(&I);
    }
  }
  for (auto &I : toBeRemoved)
    I->eraseFromParent();
}


void ASTWhileStmt::emitIR_LoopPeeling(CodeContext& ctx) {
  // PA1: Implement
  // Follow 6 steps in PA1 description.
  // Please refer to the 1st-loop-iteration peeling section 

  // Step 1: replicate 2 copies of current basic block (mm)
  BasicBlock* preheaderCondNo = replicate_basicblock(ctx);
  BasicBlock* preheaderCondYes = replicate_basicblock(ctx);
  

  // Step 2: remove content of mblock 
  RemovePreHeader(ctx);
  

  // Step 3: create a conditional branch instruction at the end of the mblock 
  //         to connect the two copies of the preheader
  auto value = this->mExpr->emitIR(ctx);
  IRBuilder<true, NoFolder> builder(ctx.mBlock);
  if (!value->getType()->isIntegerTy(1))
      value = builder.CreateICmpNE(value, ctx.mZero);

  builder.CreateCondBr(value, preheaderCondYes, preheaderCondNo); // conditional branch in while.cond


  // Step 4: append IR for the 1st loop body iteration
  auto condBlock = BasicBlock::Create(ctx.mGlobal, "while.cond", ctx.mFunc);
  ctx.mBlock = preheaderCondYes;
  this->mLoopStmt->emitIR(ctx);
  IRBuilder<true, NoFolder> builderFirstBody(ctx.mBlock);
  builderFirstBody.CreateBr(condBlock); // unconditional branch in while.cond


  // Step 5: IR lowering for remaining loop body iterations
  //         and unconditional branch to the while.end
  auto body = BasicBlock::Create(ctx.mGlobal, "while.body", ctx.mFunc);
  auto endBlock = BasicBlock::Create(ctx.mGlobal, "while.end", ctx.mFunc);
  ctx.mBlock = condBlock;

  // set Condition for while.cond
  auto valueRemain = this->mExpr->emitIR(ctx);
  IRBuilder<true, NoFolder> builderCond(ctx.mBlock);
  if (!valueRemain->getType()->isIntegerTy(1))
      valueRemain = builderCond.CreateICmpNE(valueRemain, ctx.mZero);
  builderCond.CreateCondBr(valueRemain, body, endBlock);
  
  ctx.mBlock = body;
  // Push the continue/break targets
  ctx.mContinueBlocks.push(condBlock);
  ctx.mBreakBlocks.push(endBlock);
  // Emit remaining loop body iterations
  this->mLoopStmt->emitIR(ctx);
  // Pop the continue/break targets
  ctx.mContinueBlocks.pop();
  ctx.mBreakBlocks.pop();

  IRBuilder<true, NoFolder> builderBody(ctx.mBlock);
  builderBody.CreateBr(condBlock);


  // Step 6: unconditional branch to while.end
  ctx.mBlock = preheaderCondNo;
  IRBuilder<true, NoFolder> builderEnd(ctx.mBlock);
  builderEnd.CreateBr(endBlock);

  ctx.mBlock = endBlock;
}
