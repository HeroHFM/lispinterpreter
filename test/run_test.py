#!/usr/bin/env python3

import subprocess
import glob
import sys
import os

import pathlib

# USAGE ./test/run_test.py ./tmp/lisp test/

if __name__ == "__main__":
	binary = sys.argv[1]
	test_dir = sys.argv[2]

	# Collect tests
	tests = glob.glob(test_dir + "src/*.lsp")

	# Run tests
	n_tests  = len(tests)
	n_passed = 0
	for test in tests:
		test_name = os.path.split(test)[-1]
		print(f"Runnning test {test_name}...", end="")
		
		result = subprocess.check_output([binary, test])
		with open(test_dir + "out/" + test_name[:-3] + "out", "rb") as f:
			expected = f.read()
			if expected != result:
				print("FAILED")
				print("Expected:")
				print(expected.decode())
				print("Got:")
				print(result.decode())
			else:
				print("passed")
				n_passed += 1
	print(f"{n_passed}/{n_tests} passed")
