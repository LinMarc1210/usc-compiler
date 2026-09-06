#---------------------------------------------------------
# Copyright (c) 2014, Sanjay Madhav
# All rights reserved.
#
# This file is distributed under the BSD license.
# See LICENSE.TXT for details.
#---------------------------------------------------------
import subprocess
import os
import sys
import re

import unittest
uscc = "../../bin/uscc"

__unittest = True

class PeelTests(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None
        if not os.path.isfile(uscc):
            raise Exception("Can't run without uscc")

    def checkLift(self, fileName):
        # read in expected
        with open("expected/" + fileName + "_lifted.usc", "r") as expectFile:
            expectedStr = expectFile.read()
        expectedStr_ = re.sub(r"[\n\t\s]*", "", expectedStr)
        try:
            result = subprocess.check_output([uscc, "-L", fileName + ".usc"], stderr=subprocess.STDOUT)
            resultStr = result.decode('utf-8') if isinstance(result, bytes) else result
            resultStr_ = re.sub(r"[\n\t\s]*", "", resultStr)
            self.assertMultiLineEqual(expectedStr_, resultStr_)
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("\n" + output)

    def checkPeelLift(self, fileName):
        # read in expected
        with open("expected/" + fileName + "_lifted.usc", "r") as expectFile:
            expectedStr = expectFile.read()
        expectedStr_ = re.sub(r"[\n\t\s]*", "", expectedStr)
        try:
            result = subprocess.check_output([uscc, "-L", "-P", fileName + ".usc"], stderr=subprocess.STDOUT)
            resultStr = result.decode('utf-8') if isinstance(result, bytes) else result
            resultStr_ = re.sub(r"[\n\t\s]*", "", resultStr)
            self.assertMultiLineEqual(expectedStr_, resultStr_)
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("\n" + output)

    # Part 1: Basic Statements (Tests 01-05)
    def test_lift_lifting01(self):
        """Basic: Simple function with return"""
        self.checkLift("lifting01")

    def test_lift_lifting02(self):
        """Basic: Variable declaration and assignment"""
        self.checkLift("lifting02")

    def test_lift_lifting03(self):
        """Basic: Simple if statement"""
        self.checkLift("lifting03")

    def test_lift_lifting04(self):
        """Basic: If-else statement"""
        self.checkLift("lifting04")

    def test_lift_lifting05(self):
        """Basic: While loop"""
        self.checkLift("lifting05")

    # Part 2: Break and Continue (Tests 06-08)
    def test_lift_lifting06(self):
        """Break/Continue: Break in while loop"""
        self.checkLift("lifting06")

    def test_lift_lifting07(self):
        """Break/Continue: Continue in while loop"""
        self.checkLift("lifting07")

    def test_lift_lifting08(self):
        """Break/Continue: Break and continue combined"""
        self.checkLift("lifting08")

    # Part 3: For Loop (Tests 09-12)
    def test_lift_lifting09(self):
        """For Loop: Simple for loop"""
        self.checkLift("lifting09")

    def test_lift_lifting10(self):
        """For Loop: For loop with break"""
        self.checkLift("lifting10")

    def test_lift_lifting11(self):
        """For Loop: For loop with continue"""
        self.checkLift("lifting11")

    def test_lift_lifting12(self):
        """For Loop: Nested for loops"""
        self.checkLift("lifting12")

    # Part 4: Do-While Loop (Tests 13-15)
    def test_lift_lifting13(self):
        """Do-While: Simple do-while loop"""
        self.checkLift("lifting13")

    def test_lift_lifting14(self):
        """Do-While: Do-while with break"""
        self.checkLift("lifting14")

    def test_lift_lifting15(self):
        """Do-While: Nested do-while"""
        self.checkLift("lifting15")

    # Part 5: Switch-Case (Tests 16-19)
    def test_lift_lifting16(self):
        """Switch-Case: Simple switch with cases"""
        self.checkLift("lifting16")

    def test_lift_lifting17(self):
        """Switch-Case: Switch with default"""
        self.checkLift("lifting17")

    def test_lift_lifting18(self):
        """Switch-Case: Switch with break in cases"""
        self.checkLift("lifting18")

    def test_lift_lifting19(self):
        """Switch-Case: Switch with fall-through"""
        self.checkLift("lifting19")

    # Part 6: Comprehensive (Test 20)
    def test_lift_lifting20(self):
        """Comprehensive: for, do-while, switch, break, continue"""
        self.checkLift("lifting20")

    # Part 7: Loop Peeling Tests (peeltest01-05)
    def test_peellift_peeltest01(self):
        """Loop Peeling: Simple peelable loop"""
        self.checkPeelLift("peeltest01")

    def test_peellift_peeltest02(self):
        """Loop Peeling: Non-peelable (variable redefined before loop)"""
        self.checkPeelLift("peeltest02")

    def test_peellift_peeltest03(self):
        """Loop Peeling: Nested loops"""
        self.checkPeelLift("peeltest03")

    def test_peellift_peeltest04(self):
        """Loop Peeling: Sequential loops (second loop peeled)"""
        self.checkPeelLift("peeltest04")

    def test_peellift_peeltest05(self):
        """Loop Peeling: Mixed peelability (first non-peelable, second peeled)"""
        self.checkPeelLift("peeltest05")

if __name__ == '__main__':
    unittest.main(verbosity=2)
