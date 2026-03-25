#!/usr/bin/env python3
import subprocess
import os
import unittest

uscc = "../bin/uscc"

__unittest = True

class CopyPropTests(unittest.TestCase):

	def setUp(self):
		self.maxDiff = None
		if not os.path.isfile(uscc):
			raise Exception("Can't run without uscc")

	def checkCopyProp(self, fileName):
		expectFile = open("expected/" + fileName + ".copyprop", "r")
		expectedStr = expectFile.read()
		expectFile.close()
		try:
			result = subprocess.check_output(
				[uscc, "-copyprop", "-p", fileName + ".usc"],
				stderr=subprocess.STDOUT)
			self.assertMultiLineEqual(expectedStr, result.decode('utf-8'))
		except subprocess.CalledProcessError as e:
			self.fail("\n" + e.output.decode('utf-8'))

	def test_CopyProp_simple_store_load(self):
		self.checkCopyProp("copyprop01")

	def test_CopyProp_multiple_elements(self):
		self.checkCopyProp("copyprop02")

	def test_CopyProp_chained_store_load(self):
		self.checkCopyProp("copyprop03")

	def test_CopyProp_variable_index(self):
		self.checkCopyProp("copyprop04")

	def test_CopyProp_overwrite_same_element(self):
		self.checkCopyProp("copyprop05")

	def test_CopyProp_variable_aliasing(self):
		self.checkCopyProp("copyprop06")

	def test_CopyProp_pure_ssa_noop(self):
		self.checkCopyProp("copyprop07")

	def test_CopyProp_loop_forwarding(self):
		self.checkCopyProp("copyprop08")

if __name__ == '__main__':
	unittest.main(verbosity=2)
