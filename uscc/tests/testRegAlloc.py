#!/usr/bin/env python3
# testRegAlloc.py
# Test script for PA6 register allocation.
#
# For each profile_split*.usc fixture runs two emit checks:
#   baseline:      uscc -s <fixture>
#                  -> diffed against expected/<stem>.ra.output
#   profile_split: uscc -s -profile-file <profile> <fixture>
#                  -> diffed against expected/<stem>.profile_split.ra.output
#
# The baseline allocator (Part A) runs when -profile-file is absent.
# The profile-split allocator (Part B) runs when -profile-file is
# supplied alongside -s.
#
# Tests compare stderr against golden files; a passing implementation
# produces the exact same sequence of "Found neighbors", "Removal",
# "Assigning", "coalesce", "deploy", etc. log lines.

import glob
import os
import subprocess
import unittest

uscc = "../bin/uscc"

__unittest = True


def fixtures():
    return sorted(os.path.basename(p) for p in glob.glob("profile_split*.usc"))


class PA6RegAllocTests(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None
        if not os.path.isfile(uscc):
            raise Exception("Can't run without uscc")

    def _runUscc(self, args):
        return subprocess.check_output(
            [uscc] + args, stderr=subprocess.STDOUT, text=True)

    def _checkStderrMatch(self, fixture, extra_args, expected_suffix):
        stem = fixture.rsplit(".", 1)[0]
        asm = "out/" + stem + ".s"
        try:
            output = self._runUscc(["-s", "-o", asm] + extra_args + [fixture])
        except subprocess.CalledProcessError as e:
            self.fail("\n" + (e.output or ""))
        expected_path = "expected/" + stem + expected_suffix
        with open(expected_path) as f:
            expected = f.read()
        self.assertMultiLineEqual(expected, output)

    def _mk(fixture):
        stem = fixture.rsplit(".", 1)[0]
        profile = "expected/" + stem + ".profile"

        def test_baseline_emit(self):
            self._checkStderrMatch(fixture, [], ".ra.output")

        def test_profile_split_emit(self):
            self._checkStderrMatch(
                fixture, ["-profile-file", profile], ".profile_split.ra.output")

        return {
            "test_" + stem + "_baseline_emit": test_baseline_emit,
            "test_" + stem + "_profile_split_emit": test_profile_split_emit,
        }

    for _f in fixtures():
        for _name, _fn in _mk(_f).items():
            locals()[_name] = _fn
        del _f, _name, _fn
    del _mk


if __name__ == "__main__":
    os.system("rm -rf out")
    os.makedirs("out", exist_ok=True)
    unittest.main(verbosity=2)
