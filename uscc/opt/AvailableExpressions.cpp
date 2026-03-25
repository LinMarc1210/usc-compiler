
/**
 * USC Compiler
 *
 * An iterative forward available expressions analysis.
 */

#include "Passes.h"
#include "AvailableExpressions.h"
#include "llvm/IR/CFG.h"

using namespace std;
using namespace llvm;

bool enableAE;

namespace {
// Helper for set intersection
std::set<uint32_t> intersect(const std::set<uint32_t>& A, const std::set<uint32_t>& B) {
    std::set<uint32_t> result;
    std::set_intersection(A.begin(), A.end(),
                          B.begin(), B.end(),
                          std::inserter(result, result.begin()));
    return result;
}

// Helper for set union (needed for transfer function)
std::set<uint32_t> set_union(const std::set<uint32_t>& A, const std::set<uint32_t>& B) {
    std::set<uint32_t> result = A;
    result.insert(B.begin(), B.end());
    return result;
}

// Helper for set difference (needed for transfer function)
std::set<uint32_t> set_difference(const std::set<uint32_t>& A, const std::set<uint32_t>& B) {
    std::set<uint32_t> result;
    std::set_difference(A.begin(), A.end(),
                        B.begin(), B.end(),
                        std::inserter(result, result.begin()));
    return result;
}

// Helper for comparing sets
bool areSetsEqual(const std::set<uint32_t>& A, const std::set<uint32_t>& B) {
    return A == B;
}

// Helper for computing post order
void computePostOrder(BasicBlock *entry, set<BasicBlock *> &visited, deque<BasicBlock *> &order) {
  visited.insert(entry);
  auto succItr = succ_begin(entry), end = succ_end(entry);
  for (; succItr != end; ++succItr) {
    if (!visited.count(*succItr))
      computePostOrder(*succItr, visited, order);
  }
  order.push_back(entry);
}
}

char AvailableExpressions::ID = 0;
INITIALIZE_PASS(AvailableExpressions, "ae", "Available Expressions Analysis", true, true)

AvailableExpressions::AvailableExpressions() : FunctionPass(ID) {
    initializeAvailableExpressionsPass(*PassRegistry::getPassRegistry());
}

FunctionPass *llvm::createAEPass() {
    return new AvailableExpressions();
}

std::vector<Instruction*> AvailableExpressions::getInSet(BasicBlock* BB) {
    std::vector<Instruction*> result;

    // 1. Find the IN set (of IDs) for the requested block
    if (bb2In.find(BB) == bb2In.end()) {
        // This block has no IN set (e.g., unreachable)
        return result;
    }

    // Get the set of uint32_t IDs
    const std::set<uint32_t>& inSetIDs = bb2In.at(BB);
    if (inSetIDs.empty()) {
        for (auto &Inst : *BB) {
            if (isa<BinaryOperator>(&Inst) || isa<CmpInst>(&Inst)) {
                result.push_back(&Inst);
            }
        }
    }
    else {
        // 2. Convert IDs back to Instruction*
        for (uint32_t id : inSetIDs) {
            // 3. Find Instruction* from id2Expr
            if (id2Expr[id] != nullptr) {
                // 4. Add to result vector
                result.push_back(id2Expr[id]);
            }
        }
    }

    // 5. Return the vector of available instructions
    return result;
}

void AvailableExpressions::dumpAvailableExpressions(Function &F, unsigned iterationCount) {
    llvm::outs() << "********** Available Expressions Information **********\n";
    llvm::outs() << "********** Function: " << F.getName() << ", analysis iterates " << iterationCount << " times\n";

    for (auto &bb : F) {
        llvm::outs() << bb.getName() << ":\n";
        llvm::outs() << "  GEN:";
        for (uint32_t id : this->bb2Gen[&bb]) {
            if (id < this->id2Expr.size() && this->id2Expr[id]) {
                 llvm::outs() << " [" << id << ": " << *this->id2Expr[id] << "]";
            } else {
                 llvm::outs() << " [ID:" << id << "?]";
            }
        }
        llvm::outs() << "\n";

        llvm::outs() << "  KILL:";
        for (uint32_t id : this->bb2Kill[&bb]) {
            if (id < this->id2Expr.size() && this->id2Expr[id]) {
                 llvm::outs() << " [" << id << ": " << *this->id2Expr[id] << "]";
            } else {
                 llvm::outs() << " [ID:" << id << "?]";
            }
        }
        llvm::outs() << "\n";

        // Print IN set
        llvm::outs() << "  IN:";
        if (this->bb2In.count(&bb)) {
            for (uint32_t id : this->bb2In[&bb]) {
                if (id < this->id2Expr.size() && this->id2Expr[id]) {
                     llvm::outs() << " [" << id << ": " << *this->id2Expr[id] << "]";
                } else {
                     llvm::outs() << " [ID:" << id << "?]";
                }
            }
        }
        llvm::outs() << "\n";

        // Print OUT set
        llvm::outs() << "  OUT:";
         if (this->bb2Out.count(&bb)) {
            for (uint32_t id : this->bb2Out[&bb]) {
                 if (id < this->id2Expr.size() && this->id2Expr[id]) {
                     llvm::outs() << " [" << id << ": " << *this->id2Expr[id] << "]";
                } else {
                     llvm::outs() << " [ID:" << id << "?]";
                }
            }
        }
        llvm::outs() << "\n";
    }
    llvm::outs() << "\n";
}


bool AvailableExpressions::runOnFunction(Function &F) {
    if (F.empty())
        return false;

    // Step 1: Identify all expressions and create mappings
    universalSet.clear();
    expr2Id.clear();
    id2Expr.clear();
    uint32_t idCounter = 0;
    for (auto &BB : F) {
        for (auto &Inst : BB) {
            if (isa<BinaryOperator>(&Inst) || isa<CmpInst>(&Inst)) {
                expr2Id[&Inst] = idCounter;
                id2Expr.push_back(&Inst);
                universalSet.insert(idCounter);
                // llvm::outs() << "ID: " << idCounter << " " << Inst << "\n";
                idCounter++;
            }
        }
    }

    // Step 2: Calculate GEN and KILL sets
    for (auto &BB : F) {
        set<uint32_t> currentGen;
        set<uint32_t> currentKill;

        for (auto &Inst : BB) {
            set<uint32_t> G_inst = {}; // G
            set<uint32_t> K_inst = {}; // K

            // Update KILL Set
            if (auto* SI = dyn_cast<StoreInst>(&Inst)) {
                Value* pointerOp = SI->getPointerOperand();
                for (uint32_t id : universalSet) {
                    Instruction* expr = id2Expr[id];

                    for (User::op_iterator op = expr->op_begin(), E = expr->op_end(); op != E; ++op) {
                        if (auto* LI = dyn_cast<LoadInst>(op->get())) {
                            if (LI->getPointerOperand() == pointerOp) {
                                K_inst.insert(id);
                                break;
                            }
                        }
                    }
                }
            }
            else if (!Inst.getType()->isVoidTy() && !isa<AllocaInst>(&Inst)) {
                Value* def = &Inst;
                for (uint32_t id : universalSet) {
                    Instruction* expr = id2Expr[id];
                    for (User::op_iterator op = expr->op_begin(), E = expr->op_end(); op != E; ++op) {
                        if (op->get() == def) {
                            K_inst.insert(id);
                            break;
                        }
                    }
                }
            }

            // Update GEN Set
            if (expr2Id.count(&Inst)) {
                G_inst.insert(expr2Id[&Inst]);
            }

            // // GEN = G + (GEN - K)
            currentGen = set_union(G_inst, set_difference(currentGen, K_inst));
            // // KILL = K + (KILL - G)
            currentKill = set_union(K_inst, set_difference(currentKill, G_inst));
        }
        bb2Gen[&BB] = currentGen;
        bb2Kill[&BB] = currentKill;
    }

    // Step 3: Initialize IN and OUT sets
    bb2In.clear();
    bb2Out.clear();
    BasicBlock& entryBlock = F.getEntryBlock();
    bb2Out[&entryBlock] = set<uint32_t>();
    for (auto &BB : F) {
        if (&BB != &entryBlock) {
            bb2Out[&BB] = bb2Gen[&BB]; // Optimistically assume GEN set
        }
        bb2In[&BB] = set<uint32_t>();
    }

    // Step 4: Worklist algorithm
    // Step 4.1) Compute reverse post order traversal
    set<BasicBlock *> visited;
    std::deque<BasicBlock*> worklist;
    computePostOrder(&F.front(), visited, worklist);

    unsigned iterationCount = 0;
    while (!worklist.empty()) {
        iterationCount++;
        // Reverse post order
        BasicBlock* BB = worklist.back();
        worklist.pop_back();

        // Step 4.2) Meet Operator (Intersection)
        set<uint32_t> newIn;
        bool firstPred = true;
        if (pred_begin(BB) != pred_end(BB)) {
            for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
                 BasicBlock* Pred = *PI;
                 if (firstPred) {
                    newIn = bb2Out.at(Pred); // Get OUT set of predecessor
                    firstPred = false;
                 } else {
                    newIn = intersect(newIn, bb2Out.at(Pred));
                 }
            }
        }
        else {
            newIn.clear();
        }
        bb2In[BB] = newIn;

        // Step 4.3) Transfer Function
        set<uint32_t> oldOut = bb2Out[BB];
        set<uint32_t> inMinusKill = set_difference(bb2In[BB], bb2Kill[BB]);
        set<uint32_t> newOut = set_union(bb2Gen[BB], inMinusKill);

        // Step 4.4) Change Detection and Worklist Update
        if (!areSetsEqual(oldOut, newOut)) {
            bb2Out[BB] = newOut;
            for (succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI) {
                BasicBlock* Succ = *SI;
                worklist.push_front(Succ);
            }
        }
    }

    if (enableAE) {
        dumpAvailableExpressions(F, iterationCount);
    }

    return false;
}

// Helper function to use for CSE
bool AvailableExpressions::isAvailableAfter(Instruction &expr, Instruction &point) {
    if (expr2Id.find(&expr) == expr2Id.end()) {
        return false; // Not an expression we are tracking
    }
    BasicBlock* bb = point.getParent();
    if (!bb || bb2In.find(bb) == bb2In.end()) {
        return false; // Should not happen
    }

    // Perform intra-block analysis starting from the block's IN set
    set<uint32_t> available = bb2In[bb];

    for (auto &Inst : *bb) {
        // Apply KILL logic
        if (auto* SI = dyn_cast<StoreInst>(&Inst)) {
            Value* pointerOp = SI->getPointerOperand();
            for (auto it = available.begin(); it != available.end(); ) {
                Instruction* currentExpr = id2Expr[*it];
                bool killed = false;
                for (User::op_iterator op = currentExpr->op_begin(), E = currentExpr->op_end(); op != E; ++op) {
                    if (auto* LI = dyn_cast<LoadInst>(op->get())) {
                        if (LI->getPointerOperand() == pointerOp) {
                            it = available.erase(it);
                            killed = true;
                            break;
                        }
                    }
                }
                if (!killed) ++it;
            }
        } else if (!Inst.getType()->isVoidTy() && !isa<AllocaInst>(&Inst)) {
            Value* def = &Inst;
            for (auto it = available.begin(); it != available.end(); ) {
                 Instruction* currentExpr = id2Expr[*it];
                 bool killed = false;
                 for (User::op_iterator op = currentExpr->op_begin(), E = currentExpr->op_end(); op != E; ++op) {
                     if (op->get() == def) {
                         it = available.erase(it);
                         killed = true;
                         break;
                     }
                 }
                 if (!killed) ++it;
            }
        }

        // Apply GEN logic
        if (expr2Id.count(&Inst)) {
            available.insert(expr2Id[&Inst]);
        }

        uint32_t targetId = expr2Id[&expr];
        if (&Inst == &point) {
            return available.count(targetId);
        }
    }
    return false;
}
