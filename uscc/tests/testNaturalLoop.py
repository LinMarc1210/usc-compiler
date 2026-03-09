#---------------------------------------------------------
# Copyright (c) 2023, Jianping Zeng.
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
nld = "--natural-loop"

__unittest = True

class EmitTests(unittest.TestCase):

	def setUp(self):
		self.maxDiff = None
		if not os.path.isfile(uscc):
			raise Exception("Can't run without uscc")

	def checkNaturalLoop(self, fileName, opt=False):
		# read in expected
		expectFile = open("expected/" + fileName + ".loop.output", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# Check if the output string is expected.
		try:
			resultStr = subprocess.check_output([uscc, nld, fileName + ".bc"], stderr=subprocess.STDOUT)
			self.assertMultiLineEqual(expectedStr, resultStr.decode('utf-8'))
		except subprocess.CalledProcessError as e:
			outputStr = e.output.decode('utf-8')
			outputStr = outputStr.replace('\r\n','\n')
			self.assertMultiLineEqual(expectedStr, outputStr)
			self.fail("\n" + outputStr)


	def test_loop01(self):
		self.checkNaturalLoop("loop01")
	def test_loop02(self):
		self.checkNaturalLoop("loop02")
	def test_loop03(self):
		self.checkNaturalLoop("loop03")
	def test_loop04(self):
		self.checkNaturalLoop("loop04")
	def test_loop05(self):
		self.checkNaturalLoop("loop05")
	def test_loop06(self):
		self.checkNaturalLoop("loop06")
	def test_loop07(self):
		self.checkNaturalLoop("kruscal", True)
	def test_loop08(self):
		self.checkNaturalLoop("bubble_sort", True)
	def test_loop9(self):
		self.checkNaturalLoop("quick_sort", True)
	def test_loop10(self):
		self.checkNaturalLoop("merge_sort", True)
	def test_loop11(self):
		self.checkNaturalLoop("insertion_sort", True)
	def test_loop12(self):
		self.checkNaturalLoop("heap_sort", True)
	def test_loop13(self):
		self.checkNaturalLoop("topological_sort", True)
	def test_loop14(self):
		self.checkNaturalLoop("bfs", True)
	def test_loop15(self):
		self.checkNaturalLoop("dfs", True)
	def test_loop16(self):
		self.checkNaturalLoop("find_median", True)
	def test_loop17(self):
		self.checkNaturalLoop("binary_search", True)
	def test_loop18(self):
		self.checkNaturalLoop("dijkstra", True)
	def test_loop19(self):
		self.checkNaturalLoop("kmp", True)
	def test_loop20(self):
		self.checkNaturalLoop("traverse_tree", True)


if __name__ == '__main__':
	unittest.main(verbosity=2)
