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
    // PA1: Implement
    // output << "Program:" << std::endl;
    for (auto& func : mFuncs) {  // member name is from ASTNodes.h class ASTProgram
        func->ASTtoCode(output, depth + 1);
    }
}

ASTTOCODE(ASTFunction) // "Function: "
    // PA1: Implement
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
			output << "Shouldn't have gotten here. ";
			break;
	}

	output << mIdent.getName() << " (";
	for (auto arg : mArgs)
	{
		arg->ASTtoCode(output, depth + 1);
	}
	output << ") {" << std::endl;
	mBody->ASTtoCode(output, depth + 1);
	output << "}" << std::endl;
}

ASTTOCODE(ASTArgDecl) // "ArgDecl: "
    // PA1: Implement
    OUTS();
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
	output << mIdent.getName() << std::endl;
}

ASTTOCODE(ASTArraySub) // "ArraySub: "
    // PA1: Implement
}

ASTTOCODE(ASTBadExpr) // "BadExpr:"
    // nothing to print
}

ASTTOCODE(ASTLogicalAnd)
    // PA1: Implement
}

ASTTOCODE(ASTLogicalOr)
    // PA1: Implement
}

ASTTOCODE(ASTBinaryCmpOp)
    // PA1: Implement
    mLHS->ASTtoCode(output, depth);
    output << " " << Token::Values[mOp] << " ";
    mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTBinaryMathOp)
    // PA1: Implement
    mLHS->ASTtoCode(output, depth);
    output << " " << Token::Values[mOp] << " ";
    mRHS->ASTtoCode(output, depth);
}

ASTTOCODE(ASTNotExpr)
    // PA1: Implement
}

ASTTOCODE(ASTConstantExpr)
    // PA1: Implement
	output << mValue;
}

ASTTOCODE(ASTStringExpr)
    // PA1: Implement
}

ASTTOCODE(ASTIdentExpr) // IdentExpr:
    // PA1: Implement
    output << mIdent.getName();
}

ASTTOCODE(ASTArrayExpr) // "ArrayExpr: "
    // PA1: Implement
}

ASTTOCODE(ASTFuncExpr) // "FuncExpr: "
    // PA1: Implement
}

ASTTOCODE(ASTIncExpr) // "IncExpr: "
    // PA1: Implement
}

ASTTOCODE(ASTDecExpr) // "DecExpr: "
    // PA1: Implement
}

ASTTOCODE(ASTAddrOfArray) // "AddrOfArray: "
    // PA1: Implement
}
			
ASTTOCODE(ASTToIntExpr) // "ToIntExpr: "
    // PA1: Implement
}
			
ASTTOCODE(ASTToCharExpr) // ToCharExpr: "
    // PA1: Implement
}

// Declaration
ASTTOCODE(ASTDecl) // "Decl: "
    // PA1: Implement
    OUTS();
    switch(mIdent.getType())
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
			output << "int[" << mIdent.getArrayCount() << "]";
			break;
		case Type::CharArray:
			output << "char[" << mIdent.getArrayCount() << "]";
			break;
		default:
			output << "Shouldn't have gotten here...";
			break;
	}
	output << mIdent.getName();

    if (mExpr)
    {
        output << " = ";
        mExpr->ASTtoCode(output, depth + 1);
    }
    output << ";" << std::endl;
}

// Statements
ASTTOCODE(ASTCompoundStmt) // CompoundStmt:"
    // PA1: Implement
	for (auto decl : mDecls)
	{
		decl->ASTtoCode(output, depth + 1);
	}
	for (auto stmt : mStmts)
	{
		stmt->ASTtoCode(output, depth + 1);
	}
}

ASTTOCODE(ASTReturnStmt) // "ReturnStmt:
    // PA1: Implement
    OUTS();
	if (!mExpr)
	{
		output << "return;" << std::endl;
	}
	else
	{
		output << "return ";
		mExpr->ASTtoCode(output, depth + 1);
		output << ";" << std::endl;
	}
}

ASTTOCODE(ASTAssignStmt) // "AssignStmt: "
    // PA1: Implement
    OUTS();
    output << mIdent.getName();
    output << " = ";
    mExpr->ASTtoCode(output, depth + 1);
    output << ";" << std::endl;
}

ASTTOCODE(ASTAssignArrayStmt) // "AssignArrayStmt:"
    // PA1: Implement
}

ASTTOCODE(ASTIfStmt) // "IfStmt: "
    // PA1: Implement
    OUTS();
    output << "if (";
    mExpr->ASTtoCode(output, depth + 1);
    output << ") {" << std::endl;
    mThenStmt->ASTtoCode(output, depth + 2);
    if (mElseStmt)
    {
        OUTS();
        output << "} else {" << std::endl;
        mElseStmt->ASTtoCode(output, depth + 2);
    }
    OUTS();
    output << "}" << std::endl;
}

ASTTOCODE(ASTWhileStmt) // "WhileStmt"
    // PA1: Implement
    OUTS();
    output << "while (";
    mExpr->ASTtoCode(output, depth + 1);
    output << ") {" << std::endl;
    mLoopStmt->ASTtoCode(output, depth + 2);
    OUTS();
    output << "}" << std::endl;
}

ASTTOCODE(ASTExprStmt) // "ExprStmt"
    // PA1: Implement
}

ASTTOCODE(ASTNullStmt) // "NullStmt"
    // PA1: Implement
}

ASTTOCODE(ASTBreakStmt) // "BreakStmt"
    // PA1: Implement
    OUTS();
    output << "break;" << std::endl;
}

ASTTOCODE(ASTContinueStmt) // "ContinueStmt"
    // PA1: Implement
    OUTS();
    output << "continue;" << std::endl;
}

ASTTOCODE(ASTForStmt) // "ForStmt"
    // PA1: Implement
}

ASTTOCODE(ASTDoWhileStmt) // "DoWhileStmt"
    // PA1: Implement
}

ASTTOCODE(ASTSwitchStmt) // "SwitchStmt"
    // PA1: Implement
}

ASTTOCODE(ASTCaseStmt) // "CaseStmt"
    // PA1: Implement
}

ASTTOCODE(ASTDefaultStmt) // "DefaultStmt"
    // PA1: Implement
}
