#!/bin/bash
set -e

echo "=== Compiling Auto-Vectorized Baseline ==="
# Ensure main entry point or harness if building standalone executable
clang -O3 -march=icelake-server -mprefer-vector-width=512 -fno-unroll-loops -c -o baseline_auto_vec.o benchmarks/sparse_kernel.c || \
clang -O3 -mprefer-vector-width=512 -fno-unroll-loops -c -o baseline_auto_vec.o benchmarks/sparse_kernel.c

echo "=== Compiling Probare Predicated ==="
PROBARE_BIN="./target/release/probare"
if [ ! -f "$PROBARE_BIN" ]; then
    cargo build --release --bin probare
fi

$PROBARE_BIN audit benchmarks/sparse_kernel.c --output probare_out --unwind-bound 1 || [ $? -eq 10 ]
clang -O3 -c probare_out/target.ll -o probare_predicated.o

echo "=== Auto-vectorized and Predicated object files successfully emitted ==="
