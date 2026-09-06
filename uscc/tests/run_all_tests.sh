#!/bin/bash
#---------------------------------------------------------
# Run all test*.py scripts under each pa* directory
#---------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PASS=0
FAIL=0
TOTAL=0
RESULTS=()

echo "========================================"
echo "  Running all tests"
echo "========================================"
echo ""

for pa_dir in "$SCRIPT_DIR"/pa*; do
    [ -d "$pa_dir" ] || continue
    pa_name="$(basename "$pa_dir")"

    for test_file in "$pa_dir"/test*.py; do
        [ -f "$test_file" ] || continue
        test_name="$(basename "$test_file")"
        TOTAL=$((TOTAL + 1))

        echo "----------------------------------------"
        echo "[$pa_name] $test_name"
        echo "----------------------------------------"

        (cd "$pa_dir" && python3 "$test_name" 2>&1)
        exit_code=$?

        if [ $exit_code -eq 0 ]; then
            PASS=$((PASS + 1))
            RESULTS+=("  [PASS] [$pa_name] $test_name")
            echo ">>> RESULT: PASSED"
        else
            FAIL=$((FAIL + 1))
            RESULTS+=("  [FAIL] [$pa_name] $test_name (exit code $exit_code)")
            echo ">>> RESULT: FAILED (exit code $exit_code)"
        fi
        echo ""
    done
done

echo "========================================"
echo "  Test Results Breakdown:"
for res in "${RESULTS[@]}"; do
    echo "$res"
done
echo "----------------------------------------"
echo "  Summary: $PASS/$TOTAL passed, $FAIL failed"
echo "========================================"
exit $FAIL
