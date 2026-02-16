//
//  ASTtoCode.cpp
//  uscc
//

#include "ASTNodes.h"
#include "Symbols.h"

using namespace uscc::parse;
using namespace uscc::scan;

using std::shared_ptr;

#define ASTTOCODE(a) void a::ASTtoCode(std::ostream& output, int depth) const noexcept \
{

#define OUTS() for (int i = 0; i < depth; i++) \
{ \
	output << "    "; \
}

ASTTOCODE(ASTProgram)
	for (auto func : mFuncs)
	{
		func->ASTtoCode(output, depth);
	}
}

ASTTOCODE(ASTFunction) // "Function: "
	switch (mReturnType)
	{
		case Type::Void:
			output << "void ";
			break;
		case Type::Int:
			output << "int ";
			break;
		case Type::Char:
			output << "char ";
			break;
		default:
			output <<  "Shouldn't have gotten here. ";
			break;
	}
	output << mIdent.getName();
	output << "(";

    int i = 1;
	for (auto arg : mArgs)
	{
		arg->ASTtoCode(output, depth);
        if (i != mArgs.size())
            output << ", ";
        i++;
	}

    output << ")" << std::endl;
    output << "{" << std::endl;
	
	mBody->ASTtoCode(output, depth+1); // CompoundStmt

    output << "}" << std::endl;
}

ASTTOCODE(ASTArgDecl) // "ArgDecl: "
	switch (mIdent.getType())
	{
		case Type::Void:
			output << "void ";
			break;
		case Type::Int:
			output << "int ";
			break;
		case Type::Char:
			output << "char ";
			break;
		case Type::IntArray:
			output << "int[] ";
			break;
		case Type::CharArray:
			output << "char[] ";
			break;
		default:
			output << "Shouldn't have gotten here...";
			break;
	}
	output << mIdent.getName();
}

ASTTOCODE(ASTArraySub) // "ArraySub: "
	output << mIdent.getName();
    output << "[";
	mExpr->ASTtoCode(output, depth);
    output << "]";
}

ASTTOCODE(ASTBadExpr) // "BadExpr:"
    // nothing to print
}

ASTTOCODE(ASTLogicalAnd)
	mLHS->ASTtoCode(output, depth);
    output << " && ";
	mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTLogicalOr)
	mLHS->ASTtoCode(output, depth);
    output << " || ";
	mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTBinaryCmpOp)
	mLHS->ASTtoCode(output, depth);
    output <<  " " << Token::Values[mOp] << " ";
	mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTBinaryMathOp)
	mLHS->ASTtoCode(output, depth);
	output << " " << Token::Values[mOp] << " ";
	mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTNotExpr)
	output << "!(";
	mExpr->ASTtoCode(output, depth);
	output << ")";
}

ASTTOCODE(ASTConstantExpr)
    output << mValue;
}

ASTTOCODE(ASTStringExpr)
    std::string msg = mString->getText();

    std::string newmsg;
    if (msg.substr(msg.length() - 1) == "\n") {
        newmsg = msg.substr(0,msg.length() - 1) + "\\n";
        output << "\"" << newmsg << "\"";
    }
    else {
        output << "\"" << mString->getText() << "\"";
    }
}

ASTTOCODE(ASTIdentExpr) // IdentExpr:
	output << mIdent.getName();
}

ASTTOCODE(ASTArrayExpr) // "ArrayExpr: "
	mArray->ASTtoCode(output, depth);
}

ASTTOCODE(ASTFuncExpr) // "FuncExpr: "
    output << mIdent.getName() << "(";
    int i = 1;
	for (auto arg : mArgs)
	{
		arg->ASTtoCode(output, depth);
        if (i != mArgs.size())
            output << ", ";
        i++;
	}
    output << ")";
}

ASTTOCODE(ASTIncExpr) // "IncExpr: "
	output << "++" << mIdent.getName();
}

ASTTOCODE(ASTDecExpr) // "DecExpr: "
	output << "--" << mIdent.getName();
}

ASTTOCODE(ASTAddrOfArray) // "AddrOfArray: "
	output << "&";
	mArray->ASTtoCode(output, depth);
}
			
ASTTOCODE(ASTToIntExpr) // "ToIntExpr: "
	mExpr->ASTtoCode(output, depth);
}
			
ASTTOCODE(ASTToCharExpr) // ToCharExpr: "
	mExpr->ASTtoCode(output, depth);
}

// Declaration
ASTTOCODE(ASTDecl) // "Decl: "
    OUTS()
	switch (mIdent.getType())
	{
		case Type::Void:
			output << "void " << mIdent.getName();
			break;
		case Type::Int:
			output << "int " << mIdent.getName();
			break;
		case Type::Char:
			output << "char " << mIdent.getName();
			break;
		case Type::IntArray:
			output << "int " << mIdent.getName() << "[" << mIdent.getArrayCount() << "]";
			break;
		case Type::CharArray:
			output << "char " << mIdent.getName() << "[" << mIdent.getArrayCount() << "]";
			break;
		default:
			output << "Shouldn't have gotten here...";
			break;
	}
	if (mExpr)
	{
        output << " = ";
		mExpr->ASTtoCode(output, depth);
	}
    output << ";" << std::endl;
}

// Statements
ASTTOCODE(ASTCompoundStmt) // CompoundStmt:"
	for (auto decl : mDecls)
	{
		decl->ASTtoCode(output, depth); // ASTDecl
	}
    // ASTStmt (ASTCompoundStmt, ASTAssignStmt, ASTAssignArrayStmt, ASTIfStmt, ASTWhileStmt, ASTReturnStmt, ASTExprStmt, ASTNullStmt)
	for (auto stmt : mStmts)
	{
        if (stmt) {
            if (std::dynamic_pointer_cast<ASTIfStmt>(stmt)) {
                OUTS()
            }
		    stmt->ASTtoCode(output, depth);
        }
	}
}

ASTTOCODE(ASTReturnStmt) // "ReturnStmt:
    OUTS()
	if (!mExpr)
	{
        output << "return;" << std::endl;
	}
	else
	{
        output << "return ";
		mExpr->ASTtoCode(output, depth);
        output << ";" << std::endl;
	}
}

ASTTOCODE(ASTAssignStmt) // "AssignStmt: "
    OUTS()
	output << mIdent.getName() << " = ";
	mExpr->ASTtoCode(output, depth);
    output << ";" << std::endl;
}

ASTTOCODE(ASTAssignArrayStmt) // "AssignArrayStmt:"
    OUTS()
	mArray->ASTtoCode(output, depth); // ASTArraySub
    output << " = ";
	mExpr->ASTtoCode(output, depth);
    output << ";" << std::endl;
}

ASTTOCODE(ASTIfStmt) // "IfStmt: "
    /* OUTS() */
	output << "if (";
	mExpr->ASTtoCode(output, depth);
	output << ")" << std::endl;
    OUTS()
	output << "{" << std::endl;

    // ASTStmt (ASTCompoundStmt, ASTAssignStmt, ASTAssignArrayStmt, ASTIfStmt, ASTWhileStmt, ASTReturnStmt, ASTExprStmt, ASTNullStmt)
	mThenStmt->ASTtoCode(output, depth + 1); // ASTStmt
    OUTS()
	output << "}" << std::endl;
	if (mElseStmt)
	{
        if (std::dynamic_pointer_cast<ASTIfStmt>(mElseStmt)) {
            OUTS()
	        output << "else ";
		    mElseStmt->ASTtoCode(output, depth);
        }
        else {
            OUTS()
	        output << "else" << std::endl;
            OUTS()
	        output << "{" << std::endl;
		    mElseStmt->ASTtoCode(output, depth + 1);
            OUTS()
	        output << "}" << std::endl;
        }
	}
}

ASTTOCODE(ASTWhileStmt) // "WhileStmt"
    OUTS()
    output << "while (";
	mExpr->ASTtoCode(output, depth);
    output << ")" << std::endl;
    OUTS()
    output << "{" << std::endl;
	mLoopStmt->ASTtoCode(output, depth + 1);
    OUTS()
    output << "}" << std::endl;
}

ASTTOCODE(ASTExprStmt) // "ExprStmt"
    if (mExpr) {
        OUTS()
	    mExpr->ASTtoCode(output, depth + 1);
        output << ";" << std::endl;
    }
}

ASTTOCODE(ASTNullStmt) // "NullStmt"
    OUTS()
	output << ";" << std::endl;
}

ASTTOCODE(ASTBreakStmt) // "BreakStmt"
    OUTS()
    output << "break;" << std::endl;
}

ASTTOCODE(ASTContinueStmt) // "ContinueStmt"
    OUTS()
    output << "continue;" << std::endl;
}

ASTTOCODE(ASTForStmt) // "ForStmt"
    OUTS()
    output << "for (";

    // Print init statement (assignment or expression)
    if (minitStmt) {
        if (auto assignStmt = std::dynamic_pointer_cast<ASTAssignStmt>(minitStmt)) {
            output << assignStmt->getIdent().getName() << " = ";
            assignStmt->getExpr()->ASTtoCode(output, 0);
        } else if (auto exprStmt = std::dynamic_pointer_cast<ASTExprStmt>(minitStmt)) {
            if (exprStmt->getExpr()) {
                exprStmt->getExpr()->ASTtoCode(output, 0);
            }
        }
    }
    output << "; ";

    // Print condition expression
    if (mcondExpr) {
        if (auto exprStmt = std::dynamic_pointer_cast<ASTExprStmt>(mcondExpr)) {
            if (exprStmt->getExpr()) {
                exprStmt->getExpr()->ASTtoCode(output, 0);
            }
        }
    }
    output << "; ";

    // Print step expression
    if (mstepStmt) {
        if (auto exprStmt = std::dynamic_pointer_cast<ASTExprStmt>(mstepStmt)) {
            if (exprStmt->getExpr()) {
                exprStmt->getExpr()->ASTtoCode(output, 0);
            }
        }
    }
    output << ")" << std::endl;

    OUTS()
    output << "{" << std::endl;
    mbodyStmt->ASTtoCode(output, depth + 1);
    OUTS()
    output << "}" << std::endl;
}

ASTTOCODE(ASTDoWhileStmt) // "DoWhileStmt"
    OUTS()
    output << "do" << std::endl;
    OUTS()
    output << "{" << std::endl;
    mLoopStmt->ASTtoCode(output, depth + 1);
    OUTS()
    output << "} while (";
    mExpr->ASTtoCode(output, depth);
    output << ");" << std::endl;
}

ASTTOCODE(ASTSwitchStmt) // "SwitchStmt"
    OUTS()
    output << "switch (";
    mExpr->ASTtoCode(output, depth);
    output << ")" << std::endl;
    OUTS()
    output << "{" << std::endl;
    for (auto stmt : case_default_stmts)
    {
        stmt->ASTtoCode(output, depth + 1);
    }
    OUTS()
    output << "}" << std::endl;
}

ASTTOCODE(ASTCaseStmt) // "CaseStmt"
    OUTS()
    output << "case ";
    mExpr->ASTtoCode(output, depth);
    output << ":" << std::endl;
    for (auto stmt : mStmts)
    {
        stmt->ASTtoCode(output, depth + 1);
    }
}

ASTTOCODE(ASTDefaultStmt) // "DefaultStmt"
    OUTS()
    output << "default:" << std::endl;
    for (auto stmt : mStmts)
    {
        stmt->ASTtoCode(output, depth + 1);
    }
}
