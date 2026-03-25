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
uscc = "../bin/uscc"
lli = "../../bin/lli"

__unittest = True

class PhiTests(unittest.TestCase):

	def setUp(self):
		self.maxDiff = None
		if not os.path.isfile(uscc):
			raise Exception("Can't run without uscc")
		if not os.path.isfile(lli):
			raise Exception("lli not found at ../../bin/lli")

	def checkPhi(self, fileName):
		# read in expected
		expectFile = open("expected/" + fileName + ".ll", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# first compile the .bc using uscc
		try:
			result = subprocess.check_output([uscc, "-rpr", fileName + ".ll"])
			self.assertMultiLineEqual(expectedStr, result.decode('utf-8'))
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

		# now run it in lli and compare the output


	def test_Phi_phi01(self):
		self.checkPhi("phi1")

	def test_Phi_phi02(self):
		self.checkPhi("phi2")

	def test_Phi_phi03(self):
		self.checkPhi("phi3")

	def test_Phi_phi04(self):
		self.checkPhi("phi4")

	def test_Phi_phi06(self):
		self.checkPhi("phi6")

	def test_Phi_phi07(self):
		self.checkPhi("phi7")

	def test_Phi_phi08(self):
		self.checkPhi("phi8")

	def test_Phi_trivial_same(self):
		self.checkPhi("phi_trivial_same")

	def test_Phi_self_loop(self):
		self.checkPhi("phi_self_loop")

	def test_Phi_switch_merge(self):
		self.checkPhi("phi_switch_merge")

	def test_Phi_cycle_two_nodes(self):
		self.checkPhi("phi_cycle_two_nodes")

	def test_Phi_for_loop(self):
		self.checkPhi("phi_for_loop")


if __name__ == '__main__':
	unittest.main(verbosity=2)
