#!/usr/bin/env bash
set -uo pipefail

# Advanced test runner for tane
# Usage: ./test.sh
# Each test: 
# 1. Calls build/tane "code" to generate out.s
# 2. Compiles out.s to executable
# 3. Runs executable and shows result

BIN="build/tane"
ASM_FILE="out.s"
TEST_EXE="test_program"

if [[ ! -x "$BIN" ]]; then
  echo "Error: $BIN is not built yet. Run 'make' first." >&2
  exit 1
fi

pass=0
fail=0

echo "Running assembly compilation tests..."

run_test() {
  local code="$1"
  local expected="${2:-}"
  
  echo "----------------------------------------"
  echo "Testing: $code"
  
  # Step 1: Generate assembly
  echo "> $BIN \"$code\""
  if ! $BIN "$code" 2>/dev/null; then
    echo "❌ Failed to generate assembly"
    ((fail++))
    return
  fi
  
  # Step 2: Check if out.s was generated
  if [[ ! -f "$ASM_FILE" ]]; then
    echo "❌ Assembly file $ASM_FILE was not generated"
    ((fail++))
    return
  fi
  
  echo "✅ Assembly generated successfully"
  
  # Step 3: Compile assembly to executable
  echo "> gcc -o $TEST_EXE $ASM_FILE"
  if ! gcc -o "$TEST_EXE" "$ASM_FILE" 2>/dev/null; then
    echo "❌ Failed to compile assembly"
    ((fail++))
    return
  fi
  
  echo "✅ Assembly compiled successfully"
  
  # Step 4: Run executable and capture exit code
  echo "> ./$TEST_EXE"
  ./"$TEST_EXE"
  local exit_code=$?
  # Handle the exit code properly (|| true was resetting $? to 0)
  true  # Reset to prevent script exit
  
  echo "✅ Program executed with exit code: $exit_code"
  
  # Step 5: Check expected result if provided
  if [[ -n "$expected" ]]; then
    if [[ "$exit_code" == "$expected" ]]; then
      echo "✅ Expected result: $expected, Got: $exit_code"
      ((pass++))
    else
      echo "❌ Expected result: $expected, Got: $exit_code"
      ((fail++))
    fi
  else
    ((pass++))
  fi
  
  # Cleanup
  rm -f "$ASM_FILE" "$TEST_EXE"
}

# Test cases
# Testing basic return 1 functionality (current implementation generates ret 1 which gives exit code 41)
run_test "return 1;" "1"

# Additional tests (commented out until implementation supports them)
run_test "return 0;" "0"
run_test "return 42;" "42"
run_test "return 5+3;" "8"
run_test "return 10-4;" "6"
run_test "return 6*7;" "42"
run_test "return 20/4;" "5"
run_test "return 10%3;" "1"
run_test "return 2+3*4;" "14"
run_test "return (2+3)*4;" "20"
run_test "return 10-2*3;" "4"
run_test "return (10-2)*3;" "24"
run_test "return 18/2+4;" "13"
run_test "return 18/(2+4);" "3"

#run_test "return 2+3*4;" "14"
# More complex tests (commented out until parser supports them)
# run_test "return 1+2*3;" "7"
# run_test "return 10/2+5;" "10"

echo "----------------------------------------"
echo "Test Summary:"
echo "✅ Passed: $pass"
echo "❌ Failed: $fail"

if [[ $fail -eq 0 ]]; then
  echo "🎉 All tests passed!"
  exit 0
else
  echo "💥 Some tests failed."
  exit 1
fi
