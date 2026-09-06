#!/usr/bin/env python3
#---------------------------------------------------------
# InstCombine Optimization Test Suite
# Tests the InstCombine peephole optimization pass
#
# Tests are organized by optimization part so students can
# progressively pass more tests as they implement each part:
#   Part 1: Constant Folding (tests 01-04)
#   Part 2: Identity/Zero/Strength Reduction (tests 05-11)
#   Part 3: Reassociation (tests 12, 12a, 12b, 13, 13a, 13b)
#           - 12/13: store in entry block
#           - 12a/13a: store in if.then block
#           - 12b/13b: store in if.else block
#   Part 4: LHS Identity Patterns (tests 14-15)
#   Part 5: Same Operand Patterns (tests 16-17)
#   Comprehensive (tests 18-20)
#---------------------------------------------------------
import subprocess
import os
import sys
import unittest

uscc = "../../bin/uscc"

__unittest = True

class InstCombineTests(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None
        if not os.path.isfile(uscc):
            raise Exception("Can't run without uscc")

    def checkInstCombine(self, fileName):
        # Read expected optimized IR
        with open("expected/" + fileName + ".instcomb", "r") as expectFile:
            expectedStr = expectFile.read()
        try:
            # Compile with -O (optimizations) and -p (print IR)
            result = subprocess.check_output([uscc, "-O", "-p", fileName + ".usc"],
                                            stderr=subprocess.STDOUT)
            resultStr = result.decode('utf-8') if isinstance(result, bytes) else result
            self.assertMultiLineEqual(expectedStr, resultStr)
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("\n" + output)

    # Part 1: Constant Folding (tests 01-04)
    def test_Part1_ConstFold_01(self):
        """Constant folding: add, sub, mul"""
        self.checkInstCombine("instcomb01")

    def test_Part1_ConstFold_02(self):
        """Constant folding: div, mod"""
        self.checkInstCombine("instcomb02")

    def test_Part1_ConstFold_03(self):
        """Constant folding: chained operations"""
        self.checkInstCombine("instcomb03")

    def test_Part1_ConstFold_04(self):
        """Constant folding: more division and modulo"""
        self.checkInstCombine("instcomb04")

    # Part 2: Identity/Zero/Strength Reduction (tests 05-11)
    def test_Part2_Identity_05(self):
        """Identity: x + 0 -> x, x - 0 -> x"""
        self.checkInstCombine("instcomb05")

    def test_Part2_Zero_06(self):
        """Zero absorbing: x * 0 -> 0"""
        self.checkInstCombine("instcomb06")

    def test_Part2_Identity_07(self):
        """Identity: x * 1 -> x"""
        self.checkInstCombine("instcomb07")

    def test_Part2_StrengthRed_08(self):
        """Strength reduction: x * 2^k -> shl x, k"""
        self.checkInstCombine("instcomb08")

    def test_Part2_Identity_09(self):
        """Identity: multiple variables"""
        self.checkInstCombine("instcomb09")

    def test_Part2_StrengthRed_10(self):
        """Strength reduction: various powers of 2"""
        self.checkInstCombine("instcomb10")

    def test_Part2_Zero_11(self):
        """Zero absorbing: multiple patterns"""
        self.checkInstCombine("instcomb11")

    # Part 3: Reassociation (tests 12-13)
    # Tests various scenarios: store in entry, if.then, or if.else (no PHI needed)
    def test_Part3_Reassoc_12(self):
        """Reassociation: (x + C1) + C2 -> x + (C1+C2), store in entry"""
        self.checkInstCombine("instcomb12")

    def test_Part3_Reassoc_12a(self):
        """Reassociation: (x + C1) + C2 -> x + (C1+C2), store in if.then"""
        self.checkInstCombine("instcomb12a")

    def test_Part3_Reassoc_12b(self):
        """Reassociation: (x + C1) + C2 -> x + (C1+C2), store in if.else"""
        self.checkInstCombine("instcomb12b")

    def test_Part3_Reassoc_13(self):
        """Reassociation: (x * C1) * C2 -> x * (C1*C2), store in entry"""
        self.checkInstCombine("instcomb13")

    def test_Part3_Reassoc_13a(self):
        """Reassociation: (x * C1) * C2 -> x * (C1*C2), store in if.then"""
        self.checkInstCombine("instcomb13a")

    def test_Part3_Reassoc_13b(self):
        """Reassociation: (x * C1) * C2 -> x * (C1*C2), store in if.else"""
        self.checkInstCombine("instcomb13b")

    # Part 4: LHS Identity Patterns (tests 14-15)
    def test_Part4_Identity_14(self):
        """LHS identity: 0 + x -> x, 1 * x -> x"""
        self.checkInstCombine("instcomb14")

    def test_Part4_Zero_15(self):
        """LHS zero: 0 * x -> 0"""
        self.checkInstCombine("instcomb15")

    # Part 5: Same Operand Patterns (tests 16-17)
    def test_Part5_SameOp_16(self):
        """Same operand: x - x -> 0"""
        self.checkInstCombine("instcomb16")

    def test_Part5_SameOp_17(self):
        """Same operand: combined with other optimizations"""
        self.checkInstCombine("instcomb17")

    # Comprehensive Tests (tests 18-20)
    def test_Comprehensive_18(self):
        """Comprehensive: Part 1 + Part 2"""
        self.checkInstCombine("instcomb18")

    def test_Comprehensive_19(self):
        """Comprehensive: Part 3 + Part 4 + Part 5"""
        self.checkInstCombine("instcomb19")

    def test_Comprehensive_20(self):
        """Comprehensive: Full optimization pipeline"""
        self.checkInstCombine("instcomb20")


if __name__ == '__main__':
    unittest.main(verbosity=2)
