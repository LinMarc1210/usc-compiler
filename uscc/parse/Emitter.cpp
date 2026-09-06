//
//  Emitter.cpp
//  uscc
//
//  Implements helper classes related to LLVM IR
//  construction.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------


#include "Emitter.h"
#include "Parse.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <llvm/IR/Value.h>
#include <llvm/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/Bitcode/BitcodeWriterPass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Dominators.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support//FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/CodeGen/RegAllocRegistry.h>
#include "../opt/Passes.h"
#include "../opt/ProfileReader.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>
#include <map>
#include <tuple>
#pragma clang diagnostic pop

using namespace uscc::parse;
using namespace llvm;

CodeContext::CodeContext(StringTable& strings, bool ASTOptimized, bool PeelingEnabled)
: mGlobal(getGlobalContext())
, mModule(nullptr)
, mBlock(nullptr)
, mStrings(strings)
, mPrintfIdent(nullptr)
, mZero(nullptr)
, mFunc(nullptr)
, mBreakBlocks()
, mContinueBlocks()
, ASTOptimized(ASTOptimized)
, PeelingEnabled(PeelingEnabled)
{

}

Emitter::Emitter(Parser& parser, bool ASTOptimized, bool PeelingEnabled) noexcept
: mContext(parser.mStrings, ASTOptimized, PeelingEnabled)
{
	if (parser.mNeedPrintf)
	{
		mContext.mPrintfIdent = parser.mSymbols.getIdentifier("printf");
	}

	// Initialize zero
	mContext.mZero = Constant::getNullValue(IntegerType::getInt32Ty(mContext.mGlobal));

	// This is what kicks off the generation of the LLVM IR from the AST.
	// If the input is a pre-compiled .ll/.bc file, skip AST emission and
	// parse the IR directly into the Module instead.
	if (!parser.readBC)
	{
		parser.mRoot->emitIR(mContext);
	}
	else
	{
		llvm::SMDiagnostic err;
		mContext.mModule = llvm::ParseIRFile(parser.mFileName, err,
		                                     llvm::getGlobalContext());
		if (!mContext.mModule)
		{
			err.print("uscc", llvm::errs());
			exit(1);
		}
	}
}

void Emitter::printCode(Parser& parser) noexcept
{
    // Print the .usc code
    parser.mRoot->ASTtoCode(*(parser.mASTStream),0);
}

void Emitter::optimize() noexcept
{
	legacy::PassManager pm;
	uscc::opt::registerOptPasses(pm);
	pm.run(*mContext.mModule);
}

void Emitter::edgeProfile() noexcept
{
	legacy::PassManager pm;
	uscc::opt::registerEdgeProfilingPass(pm);
	pm.run(*mContext.mModule);
}

void Emitter::edgeProfileNaive() noexcept
{
	legacy::PassManager pm;
	uscc::opt::registerEdgeProfilingPass(pm);
	pm.run(*mContext.mModule);
}

void Emitter::edgeProfileOpt() noexcept
{
	legacy::PassManager pm;
	uscc::opt::registerEdgeProfilingOptPass(pm);
	pm.run(*mContext.mModule);
}

void Emitter::enableNaturalLoop() noexcept
{
	legacy::PassManager pm;
	uscc::opt::registerNaturalLoopPasses(pm);
	pm.run(*mContext.mModule);
}

void Emitter::registerAnalysis()
{
	PassRegistry *registry = PassRegistry::getPassRegistry();
	uscc::opt::registerAnalysisPasses(*registry);
}

void Emitter::doDCE()
{
	legacy::PassManager pm;
	pm.add(createDCEPass());
	pm.run(*mContext.mModule);
}

void Emitter::doLiveness()
{
	legacy::PassManager pm;
	pm.add(createLivenessPass());
	pm.run(*mContext.mModule);
}

void Emitter::doAE()
{
	legacy::PassManager pm;
	pm.add(createAEPass());
	pm.run(*mContext.mModule);
}

void Emitter::doCSE()
{
	legacy::PassManager pm;
	pm.add(createCSEPass());
	pm.run(*mContext.mModule);
}

void Emitter::doCopyProp()
{
	legacy::PassManager pm;
	pm.add(createCopyPropagationPass());
	pm.run(*mContext.mModule);
}

// ----------------------------------------------------------------------
// Profile-driven SpecLICM support.
//
// Phase 1 (unchanged): an instrumented build emits edge counts to stdout
// via the PA2 -EN / -EO passes.  Running that binary + splitting out the
// `EDGE_PROFILE: ... END_PROFILE` records yields a plain text file.
//
// Phase 2 (this code): we parse that text file into an in-memory edge-count
// map, then walk the Module and stamp LLVM's standard !prof metadata
// (`!{!"branch_weights", i32 W0, i32 W1, ...}`) onto each conditional
// terminator whose successor edges are covered by the profile.
//
// After stamping, SpecLICM reads the same metadata via BranchProbabilityInfo
// exactly as if it had come from -fprofile-instr-use, so the pass itself is
// independent of this file format.
// ----------------------------------------------------------------------
namespace {

// Parser factored out — see uscc/opt/ProfileReader.{h,cpp}
using uscc::opt::EdgeKey;
using uscc::opt::ProfileMap;
using uscc::opt::parseProfileFile;

// For each conditional terminator whose successor edges are covered by the
// profile, stamp !prof branch_weights using LLVM's own MDBuilder.  Any
// downstream pass (in our case SpecLICM via BranchProbabilityInfo) sees the
// stamped weights exactly as if they had come from -fprofile-instr-use.
static void stampBranchWeights(llvm::Module &M, const ProfileMap &profile)
{
	MDBuilder MDB(M.getContext());
	for (llvm::Function &F : M)
	{
		if (!F.hasName()) continue;
		std::string fn = F.getName().str();
		for (llvm::BasicBlock &BB : F)
		{
			TerminatorInst *T = BB.getTerminator();
			unsigned n = T->getNumSuccessors();
			if (n < 2) continue; // only conditional br / switch
			SmallVector<uint32_t, 4> weights;
			weights.reserve(n);
			bool anyFound = false;
			uint32_t cap = UINT32_MAX / n;
			for (unsigned i = 0; i < n; ++i)
			{
				BasicBlock *succ = T->getSuccessor(i);
				EdgeKey k;
				k.fn = fn;
				k.src = BB.getName().str();
				k.dst = succ->getName().str();
				ProfileMap::const_iterator it = profile.find(k);
				uint64_t c = (it == profile.end()) ? 0 : it->second;
				if (c > 0) anyFound = true;
				// BPI clamps its own weights to [1, UINT32_MAX/numSuccessors].
				uint32_t w = (c == 0) ? 1u
				                      : static_cast<uint32_t>(std::min<uint64_t>(c, cap));
				weights.push_back(w);
			}
			if (!anyFound) continue;
			T->setMetadata(llvm::LLVMContext::MD_prof,
			               MDB.createBranchWeights(weights));
		}
	}
}

} // anonymous namespace

void Emitter::doSpecLICM(const std::string &profileFile)
{
	// Phase 2 step (a): parse the text profile and stamp !prof metadata.
	// When no profile file is supplied, this is skipped entirely and SpecLICM
	// falls back to its pure alias-analysis-driven behavior (zero regression
	// from the default `-spec-licm` invocation).
	if (!profileFile.empty())
	{
		ProfileMap profile = parseProfileFile(profileFile);
		if (!profile.empty())
			stampBranchWeights(*mContext.mModule, profile);
	}

	// Phase 2 step (b): run the pass pipeline.  SpecLICM picks up !prof
	// metadata via BranchProbabilityInfo and narrows its speculative hoists
	// to loads whose aliasing stores all live on the infrequent path.
	// Dependent-chain hoisting is handled explicitly inside SpecLICM itself
	// (see collectDependentChain in SpecLICM.cpp) — no external LICM needed.
	legacy::PassManager pm;
	pm.add(createTypeBasedAliasAnalysisPass());
	pm.add(createBasicAliasAnalysisPass());
	pm.add(createLoopRotatePass());
	pm.add(createSpecLICMPass());
	pm.add(createCFGSimplificationPass());
	pm.run(*mContext.mModule);
}

void Emitter::doPhiRemoval(const char* fileName) noexcept
{
  llvm::SMDiagnostic Err;
  llvm::LLVMContext myContext;
  llvm::Module* mCurrentModel = ParseIRFile(fileName, Err, myContext);
  if (!mCurrentModel) {
    Err.print("uscc", llvm::errs());
  }
  legacy::PassManager pm;
    pm.add(createRedundantPhiRemovalPass());
	pm.add(createPrintModulePass(outs()));
	pm.run(*mCurrentModel);
}

void Emitter::print() noexcept
{
	legacy::PassManager pm;
	pm.add(createPrintModulePass(outs()));
	pm.run(*mContext.mModule);
}

void Emitter::writeBitcode(const char* fileName) noexcept
{
	legacy::PassManager pm;
	std::string err;
	raw_fd_ostream file(fileName, err, sys::fs::F_None);
	pm.add(createBitcodeWriterPass(file));
	pm.run(*mContext.mModule);
}

bool Emitter::verify() noexcept
{
	return !verifyModule(*mContext.mModule);
}

// This function will take the bitcode emitted by uscc and convert it to assembly
bool Emitter::writeAsm(const char *fileName, bool profileSplitMode,
                       const std::string &profilePath) noexcept
{
  Module *mod = mContext.mModule;
  if (!mod) {
    assert("The pointer to Module must not be null!");
    return false;
  }

  // This code is copied over from llc
  InitializeNativeTarget();
  InitializeNativeTargetAsmParser();
  InitializeNativeTargetAsmPrinter();

  PassRegistry *Registry = PassRegistry::getPassRegistry();
  initializeCore(*Registry);
  initializeCodeGen(*Registry);
  initializeLoopStrengthReducePass(*Registry);
  initializeLowerIntrinsicsPass(*Registry);
  initializeUnreachableBlockElimPass(*Registry);

  RegisterRegAlloc usccRegAlloc("uscc", "USCC register allocator",
                                createUSCCRegisterAllocator);
  RegisterRegAlloc usccProfileSplit("uscc-profile-split",
                              "USCC profile-split register allocator",
                              createUSCCRegisterAllocatorProfileSplit);

  // If -profile-file was supplied, parse it and stamp the edge
  // counts onto the Module as !prof branch_weights.  The
  // profile-split allocator (selected by -regalloc below) picks
  // them up via MachineBlockFrequencyInfo.
  if (profileSplitMode && !profilePath.empty()) {
    uscc::opt::ProfileMap profile =
        uscc::opt::parseProfileFile(profilePath);
    if (!profile.empty())
      stampBranchWeights(*mod, profile);
  }

  const char *regallocArg =
      profileSplitMode ? "-regalloc=uscc-profile-split" : "-regalloc=uscc";
  const char *argv[] = {
      fileName,
      "-optimize-regalloc=true",
      regallocArg
  };
  cl::ParseCommandLineOptions(3, argv, "USC compiler\n");

  Triple TheTriple;
  TheTriple.setTriple(sys::getDefaultTargetTriple());

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget("", TheTriple,
                                                         Error);
  if (!TheTarget) {
    errs() << fileName << ": " << Error;
    return false;
  }

  // Package up features to be passed to target/subtarget
  CodeGenOpt::Level OLvl = CodeGenOpt::Less;

  TargetOptions Options;
  Options.DisableIntegratedAS = false;
  Options.MCOptions.ShowMCEncoding = false;
  Options.MCOptions.MCUseDwarfDirectory = false;
  Options.MCOptions.AsmVerbose = true;
  Options.PositionIndependentExecutable = 1;

  std::unique_ptr<TargetMachine> target(
      TheTarget->createTargetMachine(TheTriple.getTriple(), sys::getHostCPUName(), "",
                                     Options, Reloc::PIC_,
                                     CodeModel::Default, OLvl));
  assert(target.get() && "Could not allocate target machine!");

  assert(mod && "Should have exited if we didn't have a module!");
  TargetMachine &Target = *target.get();

  sys::fs::OpenFlags OpenFlags = sys::fs::F_None;
  OpenFlags |= sys::fs::F_Text;
  tool_output_file *FDOut = new tool_output_file(fileName, Error,
                                                 OpenFlags);
  // Figure out where we are going to send the output.
  std::unique_ptr<tool_output_file> Out(FDOut);
  if (!Out)
    return 1;

  // Build up all of the passes that we want to do to the module.
  PassManager PM;

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfo *TLI = new TargetLibraryInfo(TheTriple);
  PM.add(TLI);

  // Add the target data from the target machine, if it exists, or the module.
  if (const DataLayout *DL = Target.getDataLayout())
    mod->setDataLayout(DL);

  PM.add(new DataLayoutPass(mod));
  {
    formatted_raw_ostream FOS(Out->os());

    // Ask the target to add backend passes as necessary.
    if (Target.addPassesToEmitFile(PM, FOS, TargetMachine::CGFT_AssemblyFile, false,
                                   nullptr, nullptr)) {
      errs() << fileName << ": target does not support generation of this"
             << " file type!\n";
      exit(1);
    }

    PM.run(*mod);
  }

  // Declare success.
  Out->keep();
  return true;
}
