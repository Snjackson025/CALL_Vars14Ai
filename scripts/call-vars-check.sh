#!/bin/sh
# Local check for CALL-VARS — mirrors .github/workflows/call-vars.yml.
# Usage: sh scripts/call-vars-check.sh
# Compiles call_vars.cpp with the same flags as the workflow, then runs
# the same two smoke tests. Exits non-zero on the first failure.
set -eu

cd "$(dirname "$0")/.."

echo "Compiling call_vars.cpp..."
g++ -std=c++17 -Wall -Wextra -o /tmp/call_vars_check call_vars.cpp
echo "Compile OK."

echo "Smoke test 1: start up and shut down..."
output=$(printf '0\n' | /tmp/call_vars_check)
echo "$output" | grep -q "CALL-VARS AI"
echo "$output" | grep -q "FoxMath Engine: ONLINE"
echo "$output" | grep -q "FoxMail Engine: ONLINE"
echo "$output" | grep -q "CALL-VARS shutdown complete"
echo "Smoke test 1 OK."

echo "Smoke test 2: Fox Math Engine (3 x 4)..."
output=$(printf '2\n3\n4\n0\n' | /tmp/call_vars_check)
echo "$output" | grep -q "Standard multiplication: 12.0000"
echo "$output" | grep -q "Fox multiplication: 24.0000"
echo "Smoke test 2 OK."

echo "Smoke test 3: Fox spatial engine (3D-7D)..."
output=$(printf '6\n9\n5\n7\n0\n2\n8\n0\n1\n90\n6\n0\n' | /tmp/call_vars_check)
echo "$output" | grep -q "Spatial Engine: ONLINE (3D-7D)"
echo "$output" | grep -q "FOX SPATIAL VIEW (3D-7D)"
echo "$output" | grep -q "Dimensional space set to 5D"
echo "$output" | grep -q "Fox moved."
echo "$output" | grep -q "Fox rotated."
echo "$output" | grep -q "F = CALL-VARS"
echo "$output" | grep "Fox position:" | grep -q "2 0 0 0"
echo "$output" | grep -q "CALL-VARS shutdown complete"
echo "Smoke test 3 OK."

rm -f /tmp/call_vars_check
echo "All CALL-VARS checks passed."
