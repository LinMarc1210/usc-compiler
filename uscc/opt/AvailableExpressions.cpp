
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
    // PA3: Implement
    uint32_t ID = 0;
    for (auto &bb : F) {
        for (auto &inst : bb) {
            if (isa<BinaryOperator>(&inst) || isa<CmpInst>(&inst)) { // same with dyn_cast
                expr2Id[&inst] = ID;
                id2Expr.push_back(&inst);
                universalSet.insert(ID);
                ID++;
            }
        }
    }
    
    // Step 2: Calculate GEN and KILL sets
    // PA3: Implement
    for (auto &bb : F) {
        set<uint32_t> GEN = {};
        set<uint32_t> KILL = {};

        for (auto &inst : bb) {
            set<uint32_t> G;    // GEN[X] = G + (GEN[X] - K)
            set<uint32_t> K;    // KILL[X] = K + (KILL[X] - G)

            // case 1: when inst produces a result value
            if (!inst.getType()->isVoidTy() && !isa<AllocaInst>(&inst)) {
                // example: %result = add i32 %x, %y
                // instruction 'I' defined the Value '%result'
                Instruction *I = &inst;    // Access the entire instruction (add)
                Value *result_I = I;       // Access the result of operation (%result) by Upcasting (Instruction -> Value)
                for (uint32_t id : universalSet) {
                    Instruction *expr = id2Expr[id];
                    for (User::op_iterator op = expr->op_begin() ; op != expr->op_end() ; ++op) {  // iterate all operand for each instruction (expr)
                        Value *operandValue = op->get(); // Access actual operand value(llvm::Value) from op_iterator (%a, %b)
                        // means '%result' is used as some expr's operand, so expr is killed by 'I'
                        if (operandValue == result_I) {
                            K.insert(id);
                        }
                    }
                }
            }
            // case 2: when inst is the StoreInst
            else if (StoreInst *SI = dyn_cast<StoreInst>(&inst)) {
                Value *val = SI->getPointerOperand();    // store/load location 
                for (uint32_t id : universalSet) {
                    Instruction *expr = id2Expr[id];
                    for (User::op_iterator op = expr->op_begin() ; op != expr->op_end() ; ++op) {  // iterate all operand for each instruction (expr)
                        Value *operandValue = op->get();
                        if (LoadInst *LI = dyn_cast<LoadInst>(operandValue)) {
                            if (LI->getPointerOperand() == val) {
                                K.insert(id);
                            }
                        }
                    }
                }
            }

            // Compute G
            if (expr2Id.count(&inst)) {    // if it's generated (expr)
                G.insert(expr2Id[&inst]);
            }

            // Compute GEN, KILL set
            GEN = set_union(G, set_difference(GEN, K));
            KILL = set_union(K, set_difference(KILL, G));
        }
        bb2Gen[&bb] = GEN;
        bb2Kill[&bb] = KILL;
    }


    // Step 3: Initialize IN and OUT sets
    // PA3: Implement
    for (auto &bb : F) {
        if (&bb == &F.front()) {  // cannot compare BasicBlock, so take address to compare
            bb2In[&bb] = {};
            bb2Out[&bb] = {};
        }
        else {
            bb2In[&bb] = {};
            bb2Out[&bb] = bb2Gen[&bb];
        }
    }

    // Step 4: Worklist algorithm 
    // PA3: Implement
    std::deque<BasicBlock*> worklist;
    std::set<BasicBlock*> visited;
    computePostOrder(&(F.front()), visited, worklist);

    unsigned iterationCount = 0;
    while (!worklist.empty()) {
        BasicBlock *curr = worklist.back();  // reverse-postorder, so pop from back
        worklist.pop_back();

        // Compute IN set
        set<uint32_t> newIn = {};
        bool first = true;
        for (auto it = pred_begin(curr), end = pred_end(curr) ; it != end ; it++) {
            if (first) {
                newIn = bb2Out[*it];
                first = false;
            }
            newIn = intersect(newIn, bb2Out[*it]);
        }
        bb2In[curr] = newIn;

        // Compute OUT set
        set<uint32_t> newOut = set_union(bb2Gen[curr], set_difference(newIn, bb2Kill[curr]));
        
        // Check fixed-point
        if (newOut != bb2Out[curr]) {
            bb2Out[curr] = newOut;
            // add successors back to postorder worklist
            for (auto it = succ_begin(curr), end = succ_end(curr) ; it != end ; it++) {
                worklist.push_front(*it);
            }
        }

        iterationCount++;
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

    // PA3: Implement
    uint32_t target_id = expr2Id[&expr];
    set<uint32_t> available = bb2In[bb];

    for (auto &inst : *bb) {

        // Apply KILL effect on available
        // case 1: when inst produces a result value
        if (!inst.getType()->isVoidTy() && !isa<AllocaInst>(&inst)) {
            // example: %result = add i32 %x, %y
            // instruction 'I' defined the Value '%result'
            Instruction *I = &inst;    // Access the entire instruction (add)
            Value *result_I = I;       // Access the result of operation (%result) by Upcasting (Instruction -> Value)
            for (uint32_t id : universalSet) {
                Instruction *expr = id2Expr[id];
                for (User::op_iterator op = expr->op_begin() ; op != expr->op_end() ; ++op) {  // iterate all operand for each instruction (expr)
                    Value *operandValue = op->get(); // Access actual operand value(llvm::Value) from op_iterator (%a, %b)
                    // means '%result' is used as some expr's operand, so expr is killed by 'I'
                    if (operandValue == result_I) {
                        available.erase(id);    // KILL effect on available
                    }
                }
            }
        }
        // case 2: when inst is the StoreInst
        else if (StoreInst *SI = dyn_cast<StoreInst>(&inst)) {
            Value *val = SI->getPointerOperand();    // store/load location 
            for (uint32_t id : universalSet) {
                Instruction *expr = id2Expr[id];
                for (User::op_iterator op = expr->op_begin() ; op != expr->op_end() ; ++op) {  // iterate all operand for each instruction (expr)
                    Value *operandValue = op->get();
                    if (LoadInst *LI = dyn_cast<LoadInst>(operandValue)) {
                        if (LI->getPointerOperand() == val) {
                            available.erase(id);
                        }
                    }
                }
            }
        }


        // Apply GEN effect on available
        if (expr2Id.count(&inst)) {    // if it's generated (expr)
            available.insert(expr2Id[&inst]);
        }


        // when Inst == point, return whether expr's ID is in available set or not.
        if (&inst == &point) {
            break;
        }
    }
    return available.count(target_id);
}
