#!/usr/bin/env python3
#---------------------------------------------------------
# Copyright (c) 2024, Jianping Zeng.
# All rights reserved.
#
# This file is distributed under the BSD license.
# See LICENSE.TXT for details.
#---------------------------------------------------------
import subprocess
import os
import sys

import unittest
uscc = "../bin/uscc"

__unittest = True

class EmitTests(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None
        if not os.path.isfile(uscc):
            raise Exception("Can't run without uscc")

    def checkOpt(self, fileName):
        # read in expected
        expectFile = open("expected/" + fileName + ".output", "r")
        expectedStr = expectFile.read()
        expectFile.close()
        # Check if the output string is expected.
        try:
            resultStr = subprocess.check_output([uscc, "-p", "-O",
                                                 fileName + ".ll"],
                                                stderr=subprocess.STDOUT,
                                                text=True)
            self.assertMultiLineEqual(expectedStr, resultStr)
        except subprocess.CalledProcessError as e:
            self.fail("\n" + e.output)

    def test_ConstantDeadBlock_opt01(self):
        self.checkOpt("emit09")
    def test_ConstantDeadBlock_opt02(self):
        self.checkOpt("opt01")
    def test_ConstantDeadBlock_opt03(self):
        self.checkOpt("opt02")
    def test_ConstantDeadBlock_opt04(self):
        self.checkOpt("opt03")
    def test_ConstantDeadBlock_opt05(self):
        self.checkOpt("opt04")
    def test_ConstantDeadBlock_cdb_true(self):
        self.checkOpt("cdb_true")
    def test_ConstantDeadBlock_cdb_false(self):
        self.checkOpt("cdb_false")
    def test_ConstantDeadBlock_cdb_nested(self):
        self.checkOpt("cdb_nested")
    def test_ConstantDeadBlock_cdb_chain(self):
        self.checkOpt("cdb_chain")

if __name__ == '__main__':
    unittest.main(verbosity=2)
