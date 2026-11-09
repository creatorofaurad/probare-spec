# Probare: Formal Specification & Verification Artifacts

[![Paper](https://img.shields.io/badge/Paper-PDF-red.svg)](Probare_with_proof.pdf)
[![Specification](https://img.shields.io/badge/Spec-v0.1.3-blue.svg)](spec/SMT_OBLIGATION_SCHEMA.md)
[![License](https://img.shields.io/badge/License-Apache_2.0-green.svg)](LICENSE)

**Author:** Srijan Mandal (`creatorofaurad`)  
**Institution:** Leibniz Systems  
**Date:** October / November 2026  

---

## Overview

This repository contains the public formal specification, container format definitions, empirical telemetry harnesses, and academic whitepaper for **Probare**: a mechanized compiler verification and lowering pipeline.

Probare translates safety-critical C and LLVM IR loops into hardware-predicated AVX-512 EVEX machine code backed by an out-of-process **7-Solver SMT Consensus Pantheon** (Bitwuzla, Z3, cvc5, Yices 2, STP, Boolector, OpenSMT). Unanimous `unsat` verdicts emit verified machine code sealed within an Ed25519-signed **Proof-Carrying Hardware (`.pch`)** container.

---

## Repository Contents

- **`Probare_with_proof.pdf`**: Full academic whitepaper including formal semantics, cycle benchmarks, bootstrap confidence intervals, and two vendor disclosure autopsies (Google Android Trusty TEE and Tesla TTPoE).
- **`spec/SMT_OBLIGATION_SCHEMA.md`**: Complete bit-precise definedness conditions $\mathrm{Def}(I)$, path condition scoping, and disjoint array memory models.
- **`src/pch_header.rs`**: Zero-dependency Rust parser and verifier for the 256-byte `.pch` version 2 binary container.
- **`benchmarks/`**:
  - `fp_dot_product.c`: Pure IEEE-754 FP32 vector dot product kernel yielding **5.81× speedup** (700 cycles vs. 4,072 cycles).
  - `sparse_kernel.c`: Conditional sparse accumulation loop benchmarking predication against Clang `-O3`.
  - `benchmark_auto_vec.sh`: Automated compilation and cycle counter execution harness.
  - `analyze_tail_stats.py`: Bootstrap 95% confidence interval estimator (100,000 samples, 1,000 resamples).
  - `tune_linux_noise.sh`: Hardware core-isolation and OS tail-jitter diagnostic utility.

---

## Verified Disclosures & Empirical Results

1. **Google Android Trusty TEE (`lib/trusty/tipc_virtio_dev.c`):**
   - Independent LLVM-to-SMT discovery of a 64-bit integer underflow in VirtIO receive path.
   - Solved and refuted in **12 ms**.
   - Confirmed by Google Android Security against proprietary internal kernel source.
2. **Physical Intel Xeon AVX-512 Telemetry (100,000 iterations per kernel):**
   - **FP32 Scalar Baseline:** 4,072 cycles min, 4,530 cycles mean, 419,150 cycles max.
   - **Probare EVEX FMA:** 700 cycles min (**5.81× speedup**), 779 cycles mean, 48,580 cycles max (**8.6× tail reduction**).

---

## Citation

```bibtex
@article{mandal2026probare,
  author    = {Srijan Mandal},
  title     = {Probare: Solver-Consensus Verification and Predicated AVX-512 Lowering of Loops},
  journal   = {Leibniz Systems Technical Report},
  year      = {2026},
  month     = {October}
}
```

---
*The complete multi-crate compiler and 7-solver Zig orchestrator runtime remain proprietary to Leibniz Systems.*
