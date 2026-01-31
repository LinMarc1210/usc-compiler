//
//  IRLifting.cpp
//  uscc (AST Reconstruction)
//

#include "ASTNodes.h"
#include "Emitter.h"
#include "Symbols.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/CFG.h>

using namespace uscc::parse;
using namespace uscc::scan;
using namespace llvm;

using std::shared_ptr;
using std::string;
using std::to_string;
using std::make_shared;

#define AST_RECONSTRUCT(a) ASTNode* a::reconstruct(CodeContext& ctx, bool ssa) noexcept

AST_RECONSTRUCT(ASTProgram) {
    // Never be called
    return nullptr;
}

AST_RECONSTRUCT(ASTFunction) {
    this->setBody(shared_ptr<ASTCompoundStmt>(dynamic_cast<ASTCompoundStmt*>(this->mBody->reconstruct(ctx))));
    return this;
}

AST_RECONSTRUCT(ASTArgDecl) {
    auto ret = new ASTArgDecl(this->mIdent);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTArraySub) {
    auto ArraySubmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTArraySub(this->mIdent, ArraySubmExpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTBadExpr) {
    // Nothing to do
    return nullptr;
}

AST_RECONSTRUCT(ASTLogicalAnd) {
    auto ret = new ASTLogicalAnd();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    bool left = false;
    bool right = false;
	auto leftExpr = mLHS->reconstruct(ctx);
    ASTConstantExpr* leftConstExpr;
    ASTConstantExpr* rightConstExpr;
    if (leftConstExpr = dynamic_cast<ASTConstantExpr*>(leftExpr)) {
        ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        left = true;
    }
	auto rightExpr = mRHS->reconstruct(ctx);
    if (rightConstExpr = dynamic_cast<ASTConstantExpr*>(rightExpr)) {
        ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        right = true;
    }
    if (left && right) {
        int logicalAndValue = leftConstExpr->getValue() && rightConstExpr->getValue();
        auto ret = new ASTConstantExpr(to_string(logicalAndValue));
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
        if (!left) ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        if (!right) ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        return ret;
    }
}

AST_RECONSTRUCT(ASTLogicalOr) {
    auto ret = new ASTLogicalOr();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    bool left = false; 
    bool right = false;
    ASTConstantExpr* leftConstExpr;
    ASTConstantExpr* rightConstExpr;
	auto leftExpr = this->mLHS->reconstruct(ctx);
    if (leftConstExpr = dynamic_cast<ASTConstantExpr*>(leftExpr)) {
        ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        left = true;
    }
	auto rightExpr = this->mRHS->reconstruct(ctx);
    if (rightConstExpr = dynamic_cast<ASTConstantExpr*>(rightExpr)) {
        ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        right = true;
    }
    if (left && right) {
        int logicalOrValue = leftConstExpr->getValue() || rightConstExpr->getValue();
        auto retC = new ASTConstantExpr(to_string(logicalOrValue));
        retC->llvmBasicBlock = this->llvmBasicBlock;
        return retC;
    }
    else {
        if (!left) ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        if (!right) ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        return ret;
    }
}

AST_RECONSTRUCT(ASTBinaryCmpOp) {
    auto ret = new ASTBinaryCmpOp(this->mOp);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    bool left = false; 
    bool right = false;
    ASTConstantExpr* leftConstExpr;
    ASTConstantExpr* rightConstExpr;
	auto leftExpr = this->mLHS->reconstruct(ctx);
    if (leftConstExpr = dynamic_cast<ASTConstantExpr*>(leftExpr)) {
        ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        left = true;
    }
	auto rightExpr = this->mRHS->reconstruct(ctx);
    if (rightConstExpr = dynamic_cast<ASTConstantExpr*>(rightExpr)) {
        ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        right = true;
    }
    if (left && right) {
        int retVal;
        int rvalue = rightConstExpr->getValue();
        int lvalue = leftConstExpr->getValue();
        switch (mOp) {
            case scan::Token::EqualTo:
                retVal = (lvalue == rvalue);
                break;
            case scan::Token::NotEqual:
                retVal = (lvalue != rvalue);
                break;
            case scan::Token::GreaterThan:
                retVal = (lvalue > rvalue);
                break;
            case scan::Token::LessThan:
                retVal = (lvalue < rvalue);
                break;
            default:
                break;
        }
        auto ret = new ASTConstantExpr(to_string(retVal));
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
        if (!left) ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        if (!right) ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        return ret;
    }
}

AST_RECONSTRUCT(ASTBinaryMathOp) {
    auto ret = new ASTBinaryMathOp(this->mOp);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    bool left = false;
    bool right = false;
    ASTConstantExpr* leftConstExpr = nullptr;
    ASTConstantExpr* rightConstExpr = nullptr;
	auto leftExpr = this->mLHS->reconstruct(ctx);
    if (ctx.ASTOptimized) {
        if ((leftConstExpr = dynamic_cast<ASTConstantExpr*>(leftExpr))) {
            left = true;
        }
    }
	auto rightExpr = this->mRHS->reconstruct(ctx);
    if (ctx.ASTOptimized) {
        if ((rightConstExpr = dynamic_cast<ASTConstantExpr*>(rightExpr))) {
            right = true;
        }
    }
    if (ctx.ASTOptimized && left && right) {
        int retVal;
        int rvalue = rightConstExpr->getValue();
        int lvalue = leftConstExpr->getValue();
        switch (mOp) {
            case scan::Token::Plus:
                retVal = lvalue + rvalue;
                break;
            case scan::Token::Minus:
                retVal = lvalue - rvalue;
                break;
            case scan::Token::Mult:
                retVal = lvalue * rvalue;
                break;
            case scan::Token::Div:
                retVal = lvalue / rvalue;
                break;
            case scan::Token::Mod:
                retVal = lvalue % rvalue;
                break;
            default:
                retVal = 0;
                break;
        }
        auto retConst = new ASTConstantExpr(to_string(retVal));
        retConst->llvmBasicBlock = this->llvmBasicBlock;
        return retConst;
    }
    else {
        ret->setLHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(leftExpr)));
        ret->setRHS(shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(rightExpr)));
        return ret;
    }
}

AST_RECONSTRUCT(ASTNotExpr) {
	auto notExpr = this->mExpr->reconstruct(ctx);
    if (ASTConstantExpr* notConstExpr = dynamic_cast<ASTConstantExpr*>(notExpr)) {
        int retVal = !(notConstExpr->getValue());
        auto ret = new ASTConstantExpr(to_string(retVal));
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
	    auto NotmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(notExpr));
        auto ret = new ASTNotExpr(NotmExpr);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
}

AST_RECONSTRUCT(ASTConstantExpr) {
    auto ret = new ASTConstantExpr(to_string(mValue));
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTStringExpr) {
    const string padded = " " + this->mString->getText() + " ";
    auto ret = new ASTStringExpr(padded, ctx.mStrings);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTIdentExpr) {
    if (ctx.ASTOptimized) {
        if (ctx.ASTConstMap.find(mIdent.getName()) != ctx.ASTConstMap.end()) {
            if (ssa) {
                ctx.ASTConstMap.erase(mIdent.getName());
            }
            else {
                int retVal = ctx.ASTConstMap[mIdent.getName()];
                auto ret = new ASTConstantExpr(to_string(retVal));
                ret->llvmBasicBlock = this->llvmBasicBlock;
                return ret;
            }
        }
    }
    auto ret = new ASTIdentExpr(this->mIdent);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTArrayExpr) {
    auto ArraymArray = shared_ptr<ASTArraySub>(dynamic_cast<ASTArraySub*>(this->mArray->reconstruct(ctx)));
    auto ret = new ASTArrayExpr(ArraymArray);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTFuncExpr) {
    auto ret = new ASTFuncExpr(this->mIdent);
    ret->llvmBasicBlock = this->llvmBasicBlock;
	for (auto a : this->mArgs) {
		auto arg = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(a->reconstruct(ctx)));
        ret->addArg(arg);
	}
    return ret;
}

AST_RECONSTRUCT(ASTIncExpr) {
    // Invalidate constant map when variable is modified
    if (ctx.ASTOptimized || ssa) {
        if (ctx.ASTConstMap.find(mIdent.getName()) != ctx.ASTConstMap.end()) {
            ctx.ASTConstMap.erase(mIdent.getName());
        }
    }
    auto ret = new ASTIncExpr(this->mIdent);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTDecExpr) {
    // Invalidate constant map when variable is modified
    if (ctx.ASTOptimized || ssa) {
        if (ctx.ASTConstMap.find(mIdent.getName()) != ctx.ASTConstMap.end()) {
            ctx.ASTConstMap.erase(mIdent.getName());
        }
    }
    auto ret = new ASTDecExpr(this->mIdent);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTAddrOfArray) {
    auto AddrOfArraymArray = shared_ptr<ASTArraySub>(dynamic_cast<ASTArraySub*>(this->mArray->reconstruct(ctx)));
    auto ret = new ASTAddrOfArray(AddrOfArraymArray);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}
			
AST_RECONSTRUCT(ASTToIntExpr) {
	auto ToIntExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTToIntExpr(ToIntExpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}
			
AST_RECONSTRUCT(ASTToCharExpr) {
	auto ToCharmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTToCharExpr(ToCharmExpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTDecl) {
    if (mExpr) {
        auto retExpr = this->mExpr->reconstruct(ctx);
        if (ctx.ASTOptimized) {
            if (ASTConstantExpr* constExpr = dynamic_cast<ASTConstantExpr*>(retExpr)) {
                if (ssa) {
                    ctx.ASTConstMap.erase(mIdent.getName());
                }
                else {
                    ctx.ASTConstMap[mIdent.getName()] = constExpr->getValue();
                }
            }
            else {
                // Non-constant initializer - don't add to constant map
                ctx.ASTConstMap.erase(mIdent.getName());
            }
        }
	    auto DeclmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(retExpr));
        auto ret = new ASTDecl(this->mIdent, DeclmExpr);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
        auto ret = new ASTDecl(this->mIdent, nullptr);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
}

AST_RECONSTRUCT(ASTCompoundStmt) {
    auto ret = new ASTCompoundStmt();
    ret->llvmBasicBlock = this->llvmBasicBlock;
	for (auto d : mDecls) {
        auto decl = shared_ptr<ASTDecl>(dynamic_cast<ASTDecl*>(d->reconstruct(ctx)));
        ret->addDecl(decl);
	}

    // Collect every preheader basicblock of the while loop which is peeled.
    std::set<llvm::BasicBlock*> PreHeaderBB;
    ASTWhileStmt* whileStmt;
    for (auto s : mStmts) {
        if ((whileStmt = dynamic_cast<ASTWhileStmt*>(s.get())) && whileStmt->mPeeling) {
            PreHeaderBB.insert(whileStmt->llvmBasicBlock);
        }
    }

    std::set<shared_ptr<ASTStmt>> stmts;
	for (auto s : mStmts) {
        ASTWhileStmt* whileStmt;
        if ((whileStmt = dynamic_cast<ASTWhileStmt*>(s.get())) && whileStmt->mPeeling) {
            shared_ptr<ASTStmt> peeledStmt = whileStmt->PeelingToAST(ctx, stmts);
            for (auto stmt : stmts) {
                auto i = std::find(ret->mStmts.begin(), ret->mStmts.end(), stmt);
                ret->mStmts.erase(i);
            }
            stmts.clear();
            ret->addStmt(peeledStmt);
            // Clear constant map after peeled while loop - values are unknown after loop executes
            if (ctx.ASTOptimized) {
                ctx.ASTConstMap.clear();
            }
        }
        else {
            shared_ptr<ASTStmt> stmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(s->reconstruct(ctx)));
            if (PreHeaderBB.find(stmt->llvmBasicBlock) != PreHeaderBB.end())
                stmts.insert(stmt);
            ret->addStmt(stmt);
        }
	}
    return ret;
}

AST_RECONSTRUCT(ASTReturnStmt) {
    if (mExpr) {
        auto ReturnmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
        auto ret = new ASTReturnStmt(ReturnmExpr);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
        auto ret = new ASTReturnStmt(nullptr);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
}

AST_RECONSTRUCT(ASTAssignStmt) {
    auto assignExpr = this->mExpr->reconstruct(ctx);
    auto AssignmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(assignExpr));
    if (ctx.ASTOptimized) {
        if (ASTConstantExpr* assignConstExpr = dynamic_cast<ASTConstantExpr*>(assignExpr)) {
            if (ssa) {
                ctx.ASTConstMap.erase(mIdent.getName());
            }
            else {
                ctx.ASTConstMap[mIdent.getName()] = assignConstExpr->getValue();
            }
        }
        else {
            // Non-constant assignment - invalidate the constant map entry
            ctx.ASTConstMap.erase(mIdent.getName());
        }
    }
    auto ret = new ASTAssignStmt(this->mIdent, AssignmExpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTAssignArrayStmt) {
    auto AssignmArray = this->mArray->reconstruct(ctx);
    auto AssignArraymArray = shared_ptr<ASTArraySub>(dynamic_cast<ASTArraySub*>(AssignmArray));
    auto assignExpr = this->mExpr->reconstruct(ctx);
    auto AssignArraymExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(assignExpr));

    auto ret = new ASTAssignArrayStmt(AssignArraymArray, AssignArraymExpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTIfStmt) {
	auto IfmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto IfmThenStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mThenStmt->reconstruct(ctx)));
    shared_ptr<ASTStmt> elseStmt;
    if (this->mElseStmt) {
        elseStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mElseStmt->reconstruct(ctx)));
    }
    auto ret = new ASTIfStmt(IfmExpr, IfmThenStmt, elseStmt);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

std::shared_ptr<ASTIfStmt> ASTWhileStmt::PeelingToAST(CodeContext& ctx, std::set<std::shared_ptr<ASTStmt>> preHeader) {

    auto peelingmExpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));

    // 1) IfStmt(if.then): ASTCompoundStmt: This is same as the preheader
    shared_ptr<ASTCompoundStmt> retValThen = make_shared<ASTCompoundStmt>();
    for (auto & p : preHeader) {
        retValThen->addStmt(p);
    }

    // 2) IfStmt(if.then): This is the 1st peeling (mLoopStmt) - with constant substitution
	auto RmLoopStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(mLoopStmt->reconstruct(ctx)));
    retValThen->addStmt(RmLoopStmt);

    // 3) (while.rest_cond, while.rest_body) Rest of the while loops - use original body without constant substitution
    shared_ptr<ASTWhileStmt> RestWhileloop = make_shared<ASTWhileStmt>(this->mExpr, this->mLoopStmt);
    retValThen->addStmt(RestWhileloop);

    // 4) IfStmt (if.else): This is same as the preheader
    shared_ptr<ASTIfStmt> retVal;
    if (!preHeader.empty()) {
        shared_ptr<ASTCompoundStmt> retValElse = make_shared<ASTCompoundStmt>();
        for (auto & p : preHeader) {
            retValElse->addStmt(p);
        }
        retVal = make_shared<ASTIfStmt>(peelingmExpr, retValThen, retValElse);
    }
    else {
        shared_ptr<ASTStmt> retValNoElse;
        retVal = make_shared<ASTIfStmt>(peelingmExpr, retValThen, retValNoElse);
    }

    // If condition is to check whether the first iteration of while loop is possible
    // This if condition is while condition
    return retVal;
}

AST_RECONSTRUCT(ASTWhileStmt) {
    // I don't want it to be optimized
	/* mExpr->reconstruct(ctx); */

    // Peeling happened
    if (this->mPeeling) {
        shared_ptr<ASTStmt> peeledStmt = this->PeelingToAST(ctx);
        auto ret = new ASTWhileStmt(this->mExpr, peeledStmt);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
    else {
	    /* auto RmLoopStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mLoopStmt->lifting(ctx))); */
        auto ret = new ASTWhileStmt(this->mExpr, this->mLoopStmt);
        ret->llvmBasicBlock = this->llvmBasicBlock;
        return ret;
    }
}

AST_RECONSTRUCT(ASTExprStmt) {
    auto Exprexpr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTExprStmt(Exprexpr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTNullStmt) {
    auto ret = new ASTNullStmt();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTBreakStmt) {
    auto ret = new ASTBreakStmt();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTContinueStmt) {
    auto ret = new ASTContinueStmt();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTForStmt) {
    auto initStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->minitStmt->reconstruct(ctx)));
    auto condExpr = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mcondExpr->reconstruct(ctx)));
    auto stepStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mstepStmt->reconstruct(ctx)));
    auto bodyStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mbodyStmt->reconstruct(ctx)));
    auto ret = new ASTForStmt(initStmt, condExpr, stepStmt, bodyStmt);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTDoWhileStmt) {
    auto loopStmt = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(this->mLoopStmt->reconstruct(ctx)));
    auto expr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTDoWhileStmt(expr, loopStmt);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    return ret;
}

AST_RECONSTRUCT(ASTSwitchStmt) {
    auto expr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTSwitchStmt(expr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    for (auto stmt : this->case_default_stmts) {
        auto caseStmt = shared_ptr<ASTCaseDefaultStmt>(dynamic_cast<ASTCaseDefaultStmt*>(stmt->reconstruct(ctx)));
        ret->addStmt(caseStmt);
    }
    return ret;
}

AST_RECONSTRUCT(ASTCaseStmt) {
    auto expr = shared_ptr<ASTExpr>(dynamic_cast<ASTExpr*>(this->mExpr->reconstruct(ctx)));
    auto ret = new ASTCaseStmt(expr);
    ret->llvmBasicBlock = this->llvmBasicBlock;
    for (auto stmt : this->mStmts) {
        auto s = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(stmt->reconstruct(ctx)));
        ret->addStmt(s);
    }
    return ret;
}

AST_RECONSTRUCT(ASTDefaultStmt) {
    auto ret = new ASTDefaultStmt();
    ret->llvmBasicBlock = this->llvmBasicBlock;
    for (auto stmt : this->mStmts) {
        auto s = shared_ptr<ASTStmt>(dynamic_cast<ASTStmt*>(stmt->reconstruct(ctx)));
        ret->addStmt(s);
    }
    return ret;
}
