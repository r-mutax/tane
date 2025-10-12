#!/usr/bin/env bash
set -euo pipefail

# Simple test runner for tane
# Usage: ./test.sh
# Each test calls: build/tane "code"

BIN="build/tane"

if [[ ! -x "$BIN" ]]; then
  echo "Error: $BIN is not built yet. Run 'make' first." >&2
  exit 1
fi

pass=0
fail=0

echo "Running sample tests..."

run() {
  local code="$1"
  echo "> $BIN \"$code\""
  $BIN "$code" || true
}

# Add your cases below
run "return 1+2*3;"
run "return 10/2+5;"
run "return 4*(3+2);" # if parentheses unsupported, expected to fail
run "return 42;"

echo "Done."
