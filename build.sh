#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

CXX="${CXX:-c++}"
if ! command -v "$CXX" >/dev/null 2>&1; then
    echo "C++ compiler not found: $CXX" >&2
    exit 1
fi

mkdir -p bin

sources=(
    src/Account.cpp
    src/Transaction.cpp
    src/Budget.cpp
    src/FinanceStorage.cpp
    src/FinanceManager.cpp
)

echo "[1/2] Building bin/FinanceApp..."
"$CXX" -std=c++17 -Wall -Iinclude "${sources[@]}" src/main.cpp -o bin/FinanceApp

echo "[2/2] Building bin/test_finance..."
"$CXX" -std=c++17 -Wall -Iinclude "${sources[@]}" tests/test_finance.cpp -o bin/test_finance

echo "Build complete. Run ./bin/FinanceApp get-data to check the backend."
