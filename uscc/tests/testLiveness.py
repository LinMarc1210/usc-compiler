#!/usr/bin/env python3
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
import re

import unittest
uscc = "../bin/uscc"
live = "-liveness"
dce = "-dce"

__unittest = True

def parseLivenessOutput(text):
	"""Parse liveness output into (header, dict_of_blocks).
	header = the two '**********' lines.
	dict_of_blocks maps block_name -> {'IN': str, 'OUT': str}
	"""
	lines = text.rstrip('\n').split('\n')
	header_lines = []
	blocks = {}
	current_block = None
	for line in lines:
		if line.startswith('**********'):
			header_lines.append(line)
		elif re.match(r'^\S.*:$', line):
			current_block = line
			blocks[current_block] = {}
		elif current_block is not None and line.startswith('  '):
			key = line.split(':')[0].strip()
			blocks[current_block][key] = line
	return '\n'.join(header_lines), blocks

class EmitTests(unittest.TestCase):

	def setUp(self):
		self.maxDiff = None
		if not os.path.isfile(uscc):
			raise Exception("Can't run without uscc")

	def checkLiveness(self, fileName, opt=False):
		# read in expected
		if opt:
			expectFile = open("expected/" + fileName + ".liveness.output", "r")
		else:
			expectFile = open("expected/" + fileName + ".output", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# Check if the output string is expected.
		try:
			resultStr = subprocess.check_output([uscc, live, fileName + ".usc"], stderr=subprocess.STDOUT).decode('utf-8')
			expHeader, expBlocks = parseLivenessOutput(expectedStr)
			resHeader, resBlocks = parseLivenessOutput(resultStr)
			self.assertEqual(expHeader, resHeader,
				"Header mismatch:\nExpected:\n%s\nGot:\n%s" % (expHeader, resHeader))
			self.assertEqual(set(expBlocks.keys()), set(resBlocks.keys()),
				"Block name mismatch:\nExpected: %s\nGot: %s" % (
					sorted(expBlocks.keys()), sorted(resBlocks.keys())))
			for block in expBlocks:
				for key in ('IN', 'OUT'):
					self.assertEqual(expBlocks[block].get(key, ''), resBlocks[block].get(key, ''),
						"Block %s %s mismatch:\nExpected: %s\nGot: %s" % (
							block, key,
							expBlocks[block].get(key, ''),
							resBlocks[block].get(key, '')))
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

	def checkDCE(self, fileName):
		# read in expected
		expectFile = open("expected/" + fileName + ".output", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# Check if the output string is expected.
		try:
			resultStr = subprocess.check_output([uscc, dce, "-p", fileName + ".usc"], stderr=subprocess.STDOUT).decode('utf-8')
			self.assertMultiLineEqual(expectedStr, resultStr)
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

	def test_liveness01(self):
		self.checkLiveness("liveness01")
	def test_liveness02(self):
		self.checkLiveness("liveness02")
	def test_liveness03(self):
		self.checkLiveness("liveness03")
	def test_liveness04(self):
		self.checkLiveness("liveness04")
	def test_liveness05(self):
		self.checkLiveness("liveness05")
	def test_liveness06(self):
		self.checkLiveness("liveness06")
	def test_liveness07(self):
		self.checkLiveness("liveness07")
	def test_liveness08(self):
		self.checkLiveness("opt03", True)
	def test_liveness09(self):
		self.checkLiveness("opt04", True)
	def test_liveness10(self):
		self.checkLiveness("opt05", True)
	def test_liveness11(self):
		self.checkLiveness("opt06", True)
	def test_liveness12(self):
		self.checkLiveness("opt07", True)
	def test_liveness13(self):
		self.checkLiveness("emit09", True)
	def test_liveness14(self):
		self.checkLiveness("emit11", True)
	def test_liveness15(self):
		self.checkLiveness("emit12", True)
	def test_dce01(self):
		self.checkDCE("dce01")
	def test_dce02(self):
		self.checkDCE("dce02")
	def test_dce03(self):
		self.checkDCE("dce03")
	def test_dce04(self):
		self.checkDCE("dce04")
	def test_dce05(self):
		self.checkDCE("dce05")

if __name__ == '__main__':
	unittest.main(verbosity=2)
