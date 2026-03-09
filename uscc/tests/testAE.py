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
ae = "-ae"
cse = "-cse"

__unittest = True

def parseAEOutput(text):
	"""Parse AE output into (header, dict_of_blocks).
	header = the two '**********' lines.
	dict_of_blocks maps block_name -> {'GEN': str, 'KILL': str, 'IN': str, 'OUT': str}
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

	def checkAvailableExpr(self, fileName):
		# read in expected
		expectFile = open("expected/" + fileName + ".output", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# Check if the output string is expected.
		try:
			resultStr = subprocess.check_output([uscc, ae, fileName + ".usc"], stderr=subprocess.STDOUT).decode('utf-8')
			expHeader, expBlocks = parseAEOutput(expectedStr)
			resHeader, resBlocks = parseAEOutput(resultStr)
			self.assertEqual(expHeader, resHeader,
				"Header mismatch:\nExpected:\n%s\nGot:\n%s" % (expHeader, resHeader))
			self.assertEqual(set(expBlocks.keys()), set(resBlocks.keys()),
				"Block name mismatch:\nExpected: %s\nGot: %s" % (
					sorted(expBlocks.keys()), sorted(resBlocks.keys())))
			for block in expBlocks:
				for key in ('GEN', 'KILL', 'IN', 'OUT'):
					self.assertEqual(expBlocks[block].get(key, ''), resBlocks[block].get(key, ''),
						"Block %s %s mismatch:\nExpected: %s\nGot: %s" % (
							block, key,
							expBlocks[block].get(key, ''),
							resBlocks[block].get(key, '')))
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

	def checkCSE(self, fileName):
		# read in expected
		expectFile = open("expected/" + fileName + ".output", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		# Check if the output string is expected.
		try:
			resultStr = subprocess.check_output([uscc, cse, "-p", fileName + ".usc"], stderr=subprocess.STDOUT).decode('utf-8')
			self.assertMultiLineEqual(expectedStr, resultStr)
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

	def test_availableExpr01(self):
		self.checkAvailableExpr("ae01")
	def test_availableExpr02(self):
		self.checkAvailableExpr("ae02")
	def test_availableExpr03(self):
		self.checkAvailableExpr("ae03")
	def test_availableExpr04(self):
		self.checkAvailableExpr("ae04")
	def test_availableExpr05(self):
		self.checkAvailableExpr("ae05")
	def test_availableExpr06(self):
		self.checkAvailableExpr("ae06")
	def test_availableExpr07(self):
		self.checkAvailableExpr("ae07")
	def test_availableExpr08(self):
		self.checkAvailableExpr("ae08")
	def test_availableExpr09(self):
		self.checkAvailableExpr("ae09")
	def test_availableExpr10(self):
		self.checkAvailableExpr("ae10")
	def test_cse01(self):
		self.checkCSE("cse01")
	def test_cse02(self):
		self.checkCSE("cse02")
	def test_cse03(self):
		self.checkCSE("cse03")
	def test_cse04(self):
		self.checkCSE("cse04")
	def test_cse05(self):
		self.checkCSE("cse05")
	def test_cse06(self):
		self.checkCSE("cse06")
	def test_cse07(self):
		self.checkCSE("cse07")
	def test_cse08(self):
		self.checkCSE("cse08")
	def test_cse09(self):
		self.checkCSE("cse09")
	def test_cse10(self):
		self.checkCSE("cse10")

if __name__ == '__main__':
	unittest.main(verbosity=2)
