#!/usr/bin/env python3
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

import unittest
uscc = "../../bin/uscc"
lli = "../../../bin/lli"

__unittest = True

class EmitTests(unittest.TestCase):

	def setUp(self):
		self.maxDiff = None
		if not os.path.isfile(uscc):
			raise Exception("Can't run without uscc")
		if not os.path.isfile(lli):
			raise Exception("lli not found at ../../bin/lli")

	def checkEmit(self, fileName):
		# read in expected
		expectFile = open("expected/" + fileName + ".ssa", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# first compile the .bc using uscc
		try:
			result = subprocess.check_output([uscc, "-p", fileName + ".usc"], stderr=subprocess.STDOUT)
			self.assertMultiLineEqual(expectedStr, result.decode('utf-8'))
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

		# now run it in lli and compare the output

	def test_Emit_emit04(self):
		self.checkEmit("emit04")

	def test_Emit_emit06(self):
		self.checkEmit("emit06")

	def test_Emit_emit07(self):
		self.checkEmit("emit07")

	def test_Emit_emit08(self):
		self.checkEmit("emit08")

	def test_Emit_emit09(self):
		self.checkEmit("emit09")

	def test_Emit_emit11(self):
		self.checkEmit("emit11")

	def test_Emit_emit12(self):
		self.checkEmit("emit12")

	def test_Emit_quicksort(self):
		self.checkEmit("quicksort")

	def test_Emit_015(self):
		self.checkEmit("test015")

	def test_Emit_016(self):
		self.checkEmit("test016")

	def test_Emit_opt01(self):
		self.checkEmit("opt01")

	def test_Emit_opt02(self):
		self.checkEmit("opt02")

	def test_Emit_opt03(self):
		self.checkEmit("opt03")

	def test_Emit_opt04(self):
		self.checkEmit("opt04")

	def test_Emit_opt05(self):
		self.checkEmit("opt05")

	def test_Emit_opt06(self):
		self.checkEmit("opt06")

	def test_Emit_opt07(self):
		self.checkEmit("opt07")

	def test_Emit_ssa_simple_var(self):
		self.checkEmit("ssa_simple_var")

	def test_Emit_ssa_func_args(self):
		self.checkEmit("ssa_func_args")

	def test_Emit_ssa_if_merge(self):
		self.checkEmit("ssa_if_merge")

	def test_Emit_ssa_while_loop(self):
		self.checkEmit("ssa_while_loop")

	def test_Emit_ssa_for_loop(self):
		self.checkEmit("ssa_for_loop")

	def test_Emit_ssa_do_while(self):
		self.checkEmit("ssa_do_while")

	def test_Emit_ssa_switch_basic(self):
		self.checkEmit("ssa_switch_basic")

	def test_Emit_ssa_switch_default(self):
		self.checkEmit("ssa_switch_default")

	def test_Emit_ssa_switch_fallthrough(self):
		self.checkEmit("ssa_switch_fallthrough")

	def test_Emit_ssa_nested_if_while(self):
		self.checkEmit("ssa_nested_if_while")

	def test_Emit_ssa_break_in_loop(self):
		self.checkEmit("ssa_break_in_loop")

	def test_Emit_ssa_for_break_continue(self):
		self.checkEmit("ssa_for_break_continue")

if __name__ == '__main__':
	unittest.main(verbosity=2)
