#!/usr/bin/env python3
#---------------------------------------------------------
# Edge Profiling Test Suite
# Tests both Naive and Optimized (MST-based) Edge Profiling passes.
#
# 40 tests: 20 naive + 20 optimized (split for clearer grading).
# Each test requires a matching expected output file in tests/expected/.
#
# Naive Mode (-EN): Instruments ALL edges directly
# Optimized Mode (-EO): Uses Knuth's algorithm - instruments only chord
#   destinations via logsrc/logdest. Prints raw matrix values (no offline
#   recovery). Tree edges without logdest show count 0.
#
# All comparisons are order-independent (dict/set equality).
#---------------------------------------------------------
import subprocess
import os
import sys
import time
import unittest
import re

uscc = "../bin/uscc"
lli = "../../bin/lli"

__unittest = True


def parse_profile_output(output):
    """Parse edge profiling output into structured data.

    Accumulates across multiple EDGE_PROFILE sections (multi-function).
    Returns dict with edges (dict), spanning_tree (set), instrumented (set).
    """
    result = {
        'function': None,
        'spanning_tree': set(),
        'instrumented': set(),
        'edges': {},
    }

    lines = output.strip().split('\n')
    current_section = None

    for line in lines:
        line = line.strip()
        if line.startswith('EDGE_PROFILE:'):
            result['function'] = line.split(':')[1].strip()
        elif line.startswith('SPANNING_TREE:'):
            edges_str = line.split(':', 1)[1].strip()
            if edges_str:
                for edge in edges_str.split(','):
                    edge = edge.strip()
                    if ' -> ' in edge:
                        src, dest = edge.split(' -> ')
                        result['spanning_tree'].add((src.strip(), dest.strip()))
        elif line.startswith('INSTRUMENTED:'):
            edges_str = line.split(':', 1)[1].strip()
            if edges_str:
                for edge in edges_str.split(','):
                    edge = edge.strip()
                    if ' -> ' in edge:
                        src, dest = edge.split(' -> ')
                        result['instrumented'].add((src.strip(), dest.strip()))
        elif line.startswith('EDGES:'):
            current_section = 'edges'
        elif line.startswith('END_PROFILE'):
            current_section = None
        elif current_section == 'edges' and ' -> ' in line and ' : ' in line:
            match = re.match(r'(.+) -> (.+) : (\d+)', line)
            if match:
                src = match.group(1).strip()
                dest = match.group(2).strip()
                count = int(match.group(3))
                result['edges'][(src, dest)] = count

    return result


def parse_expected_file(filepath):
    """Parse expected profile file into structured data."""
    with open(filepath, 'r') as f:
        return parse_profile_output(f.read())


class EdgeProfilingTests(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None
        if not os.path.isfile(uscc):
            raise Exception("Can't run without uscc")

    def checkEdgeProfilingNaive(self, fileName):
        """Test naive edge profiling (-EN) for a given test file."""
        bcFile = fileName + "_naive.bc"
        uscFile = fileName + ".usc"
        expectedFile = "expected/" + fileName + "_naive.profile"

        # Require expected file
        self.assertTrue(os.path.exists(expectedFile),
            f"Expected file {expectedFile} not found. "
            "Run generate_expected_profiles.py first.")

        # Compile
        try:
            subprocess.check_output([uscc, "-EN", uscFile],
                stderr=subprocess.STDOUT)
            defaultBcFile = uscFile.replace('.usc', '.bc')
            if os.path.exists(defaultBcFile) and defaultBcFile != bcFile:
                os.rename(defaultBcFile, bcFile)
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("Compilation (naive) failed:\n" + output)

        self.assertTrue(os.path.exists(bcFile),
            f"Bitcode file {bcFile} was not created")

        # Execute
        try:
            t0 = time.perf_counter()
            result = subprocess.check_output([lli, bcFile],
                stderr=subprocess.STDOUT, timeout=60)
            elapsed = time.perf_counter() - t0
            actualOutput = result.decode('utf-8') if isinstance(result, bytes) else result
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("Execution (naive) failed:\n" + output)
        except subprocess.TimeoutExpired:
            self.fail("Execution (naive) timed out")
        except FileNotFoundError:
            self.skipTest("lli not found - cannot execute bitcode")

        # Parse and compare
        actual = parse_profile_output(actualOutput)
        expected = parse_expected_file(expectedFile)

        self.assertIsNotNone(actual['function'],
            "Missing function name in naive output")

        self.compare_edge_counts(actual, expected, fileName, 'naive')

        print(f"  [{elapsed:.3f}s]", end="", flush=True)

    def checkEdgeProfilingOpt(self, fileName):
        """Test optimized (MST) edge profiling (-EO) for a given test file."""
        bcFile = fileName + "_opt.bc"
        uscFile = fileName + ".usc"
        expectedFile = "expected/" + fileName + "_opt.profile"

        # Require expected file
        self.assertTrue(os.path.exists(expectedFile),
            f"Expected file {expectedFile} not found. "
            "Run generate_expected_profiles.py first.")

        # Compile
        try:
            subprocess.check_output([uscc, "-EO", uscFile],
                stderr=subprocess.STDOUT)
            defaultBcFile = uscFile.replace('.usc', '.bc')
            if os.path.exists(defaultBcFile) and defaultBcFile != bcFile:
                os.rename(defaultBcFile, bcFile)
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("Compilation (opt) failed:\n" + output)

        self.assertTrue(os.path.exists(bcFile),
            f"Bitcode file {bcFile} was not created")

        # Execute
        try:
            t0 = time.perf_counter()
            result = subprocess.check_output([lli, bcFile],
                stderr=subprocess.STDOUT, timeout=60)
            elapsed = time.perf_counter() - t0
            actualOutput = result.decode('utf-8') if isinstance(result, bytes) else result
        except subprocess.CalledProcessError as e:
            output = e.output.decode('utf-8') if isinstance(e.output, bytes) else e.output
            self.fail("Execution (opt) failed:\n" + output)
        except subprocess.TimeoutExpired:
            self.fail("Execution (opt) timed out")
        except FileNotFoundError:
            self.skipTest("lli not found - cannot execute bitcode")

        # Parse and compare
        actual = parse_profile_output(actualOutput)
        expected = parse_expected_file(expectedFile)

        self.assertIsNotNone(actual['function'],
            "Missing function name in optimized output")

        # Check edges (dict equality — order-independent)
        self.compare_edge_counts(actual, expected, fileName, 'opt')

        # Check spanning tree (set equality — order-independent)
        self.assertEqual(actual['spanning_tree'], expected['spanning_tree'],
            f"Test {fileName} (opt): spanning tree mismatch.\n"
            f"Missing: {expected['spanning_tree'] - actual['spanning_tree']}\n"
            f"Extra: {actual['spanning_tree'] - expected['spanning_tree']}")

        # Check instrumented edges (set equality — order-independent)
        self.assertEqual(actual['instrumented'], expected['instrumented'],
            f"Test {fileName} (opt): instrumented edges mismatch.\n"
            f"Missing: {expected['instrumented'] - actual['instrumented']}\n"
            f"Extra: {actual['instrumented'] - expected['instrumented']}")

        print(f"  [{elapsed:.3f}s]", end="", flush=True)

    def compare_edge_counts(self, actual, expected, fileName, mode):
        """Compare edge counts between actual and expected (order-independent)."""
        errors = []

        for edge, count in expected['edges'].items():
            if edge not in actual['edges']:
                errors.append(f"Missing edge count for {edge}")
            elif actual['edges'][edge] != count:
                errors.append(
                    f"Wrong count for {edge}: expected {count}, got {actual['edges'][edge]}")

        for edge in actual['edges']:
            if edge not in expected['edges']:
                errors.append(f"Unexpected edge in output: {edge}")

        if errors:
            self.fail(f"Test {fileName} ({mode}) failed:\n" + "\n".join(errors))

    # =============================================
    # 40 Test Methods: 20 naive + 20 optimized
    # =============================================

    # Test 01: Deeply Nested Loops (4 levels)
    def test_01_naive_DeeplyNestedLoops(self):
        """Naive: Deeply Nested Loops"""
        self.checkEdgeProfilingNaive("edgeprof01")

    def test_01_opt_DeeplyNestedLoops(self):
        """Opt: Deeply Nested Loops"""
        self.checkEdgeProfilingOpt("edgeprof01")

    # Test 02: Wide Branch Tree (8-way)
    def test_02_naive_WideBranchTree(self):
        """Naive: Wide Branch Tree"""
        self.checkEdgeProfilingNaive("edgeprof02")

    def test_02_opt_WideBranchTree(self):
        """Opt: Wide Branch Tree"""
        self.checkEdgeProfilingOpt("edgeprof02")

    # Test 03: Loop with 5 Conditionals
    def test_03_naive_LoopWithConditionals(self):
        """Naive: Loop with 5 Conditionals"""
        self.checkEdgeProfilingNaive("edgeprof03")

    def test_03_opt_LoopWithConditionals(self):
        """Opt: Loop with 5 Conditionals"""
        self.checkEdgeProfilingOpt("edgeprof03")

    # Test 04: Nested If-Else Chain (6 levels)
    def test_04_naive_NestedIfElseChain(self):
        """Naive: Nested If-Else Chain"""
        self.checkEdgeProfilingNaive("edgeprof04")

    def test_04_opt_NestedIfElseChain(self):
        """Opt: Nested If-Else Chain"""
        self.checkEdgeProfilingOpt("edgeprof04")

    # Test 05: Multiple Nested Loops + Branches
    def test_05_naive_MultipleNestedLoopsBranches(self):
        """Naive: Multiple Nested Loops + Branches"""
        self.checkEdgeProfilingNaive("edgeprof05")

    def test_05_opt_MultipleNestedLoopsBranches(self):
        """Opt: Multiple Nested Loops + Branches"""
        self.checkEdgeProfilingOpt("edgeprof05")

    # Test 06: Loop with Early Exit (10 conditions)
    def test_06_naive_LoopWithEarlyExit(self):
        """Naive: Loop with Early Exit"""
        self.checkEdgeProfilingNaive("edgeprof06")

    def test_06_opt_LoopWithEarlyExit(self):
        """Opt: Loop with Early Exit"""
        self.checkEdgeProfilingOpt("edgeprof06")

    # Test 07: Cascading Conditionals (8 sequential)
    def test_07_naive_CascadingConditionals(self):
        """Naive: Cascading Conditionals"""
        self.checkEdgeProfilingNaive("edgeprof07")

    def test_07_opt_CascadingConditionals(self):
        """Opt: Cascading Conditionals"""
        self.checkEdgeProfilingOpt("edgeprof07")

    # Test 08: Triple Nested with Cross-Branches
    def test_08_naive_TripleNestedCrossBranches(self):
        """Naive: Triple Nested with Cross-Branches"""
        self.checkEdgeProfilingNaive("edgeprof08")

    def test_08_opt_TripleNestedCrossBranches(self):
        """Opt: Triple Nested with Cross-Branches"""
        self.checkEdgeProfilingOpt("edgeprof08")

    # Test 09: Loop with Continue Conditions (5)
    def test_09_naive_LoopWithContinue(self):
        """Naive: Loop with Continue Conditions"""
        self.checkEdgeProfilingNaive("edgeprof09")

    def test_09_opt_LoopWithContinue(self):
        """Opt: Loop with Continue Conditions"""
        self.checkEdgeProfilingOpt("edgeprof09")

    # Test 10: Alternating Loop-Branch Pattern
    def test_10_naive_AlternatingLoopBranch(self):
        """Naive: Alternating Loop-Branch Pattern"""
        self.checkEdgeProfilingNaive("edgeprof10")

    def test_10_opt_AlternatingLoopBranch(self):
        """Opt: Alternating Loop-Branch Pattern"""
        self.checkEdgeProfilingOpt("edgeprof10")

    # Test 11: Deep Recursive-Style Nesting (5)
    def test_11_naive_DeepRecursiveNesting(self):
        """Naive: Deep Recursive-Style Nesting"""
        self.checkEdgeProfilingNaive("edgeprof11")

    def test_11_opt_DeepRecursiveNesting(self):
        """Opt: Deep Recursive-Style Nesting"""
        self.checkEdgeProfilingOpt("edgeprof11")

    # Test 12: Matrix-Style Double Loop + Checks
    def test_12_naive_MatrixDoubleLoop(self):
        """Naive: Matrix-Style Double Loop"""
        self.checkEdgeProfilingNaive("edgeprof12")

    def test_12_opt_MatrixDoubleLoop(self):
        """Opt: Matrix-Style Double Loop"""
        self.checkEdgeProfilingOpt("edgeprof12")

    # Test 13: State Machine Simulation (6 states)
    def test_13_naive_StateMachine(self):
        """Naive: State Machine Simulation"""
        self.checkEdgeProfilingNaive("edgeprof13")

    def test_13_opt_StateMachine(self):
        """Opt: State Machine Simulation"""
        self.checkEdgeProfilingOpt("edgeprof13")

    # Test 14: Binary Search Tree Traversal
    def test_14_naive_BinarySearchTree(self):
        """Naive: Binary Search Tree Traversal"""
        self.checkEdgeProfilingNaive("edgeprof14")

    def test_14_opt_BinarySearchTree(self):
        """Opt: Binary Search Tree Traversal"""
        self.checkEdgeProfilingOpt("edgeprof14")

    # Test 15: Bubble Sort Simulation
    def test_15_naive_BubbleSort(self):
        """Naive: Bubble Sort Simulation"""
        self.checkEdgeProfilingNaive("edgeprof15")

    def test_15_opt_BubbleSort(self):
        """Opt: Bubble Sort Simulation"""
        self.checkEdgeProfilingOpt("edgeprof15")

    # Test 16: Multi-Function Complex (3 funcs)
    def test_16_naive_MultiFunction(self):
        """Naive: Multi-Function Complex"""
        self.checkEdgeProfilingNaive("edgeprof16")

    def test_16_opt_MultiFunction(self):
        """Opt: Multi-Function Complex"""
        self.checkEdgeProfilingOpt("edgeprof16")

    # Test 17: Diamond Patterns Chained (5)
    def test_17_naive_DiamondPatterns(self):
        """Naive: Diamond Patterns Chained"""
        self.checkEdgeProfilingNaive("edgeprof17")

    def test_17_opt_DiamondPatterns(self):
        """Opt: Diamond Patterns Chained"""
        self.checkEdgeProfilingOpt("edgeprof17")

    # Test 18: Loop Unrolling Simulation (4x)
    def test_18_naive_LoopUnrolling(self):
        """Naive: Loop Unrolling Simulation"""
        self.checkEdgeProfilingNaive("edgeprof18")

    def test_18_opt_LoopUnrolling(self):
        """Opt: Loop Unrolling Simulation"""
        self.checkEdgeProfilingOpt("edgeprof18")

    # Test 19: Worst-Case CFG (max edges)
    def test_19_naive_WorstCaseCFG(self):
        """Naive: Worst-Case CFG"""
        self.checkEdgeProfilingNaive("edgeprof19")

    def test_19_opt_WorstCaseCFG(self):
        """Opt: Worst-Case CFG"""
        self.checkEdgeProfilingOpt("edgeprof19")

    # Test 20: Comprehensive Benchmark
    def test_20_naive_ComprehensiveBenchmark(self):
        """Naive: Comprehensive Benchmark"""
        self.checkEdgeProfilingNaive("edgeprof20")

    def test_20_opt_ComprehensiveBenchmark(self):
        """Opt: Comprehensive Benchmark"""
        self.checkEdgeProfilingOpt("edgeprof20")


if __name__ == '__main__':
    unittest.main(verbosity=2)
