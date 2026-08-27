<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Fuzz harnesses

Coverage-guided fuzz harnesses for the Power Grid Model C API. They exercise the
untrusted-input entry points of the library and run under **AFL++** by default;
libFuzzer is also supported (both drive the same harness sources).

| Harness | Entry point | What it drives |
|---------|-------------|----------------|
| `harness_deserializer_json.c` | `PGM_create_deserializer_from_binary_buffer(..., PGM_json)` | JSON deserialization → dataset inspection → buffer alloc/parse → buffer get/set round-trips → `PGM_create_model` → symmetric / asymmetric power flow + short-circuit `PGM_calculate` |

Supporting material:

- `corpus_json/` — seed corpus (valid grids, edge cases, and malformed
  documents) for `harness_deserializer_json`.
- `pgm.dict` — AFL++/libFuzzer dictionary of the PGM JSON envelope keys,
  component names, attribute names, and common numeric literals.

## Building and running with AFL++ (default)

Requires the AFL++ toolchain (`afl-clang-fast` / `afl-clang-fast++`), which
instruments both the harness and the C API library it links against.

```bash
export AFL_USE_ASAN=1   # optional: also build with AddressSanitizer
cmake -S . -B build -G Ninja \
    -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++ \
    -DPGM_ENABLE_FUZZER=ON
cmake --build build --target pgm_fuzz_deserializer_json

afl-fuzz -i tests/fuzzer/corpus_json -o fuzz_out \
    -x tests/fuzzer/pgm.dict -- ./build/bin/pgm_fuzz_deserializer_json @@
```

If `libAFLDriver.a` is not on a standard path, point the build at it with
`-DPGM_AFL_DRIVER=/path/to/libAFLDriver.a` or by exporting `AFL_PATH`.

## Building and running with libFuzzer (alternative)

Requires a Clang toolchain (this path uses `-fsanitize=fuzzer`).

```bash
cmake -S . -B build-libfuzzer -G Ninja \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DPGM_ENABLE_FUZZER=ON -DPGM_FUZZER_ENGINE=libfuzzer \
    -DCMAKE_C_FLAGS="-fsanitize=address,undefined" \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"
cmake --build build-libfuzzer --target pgm_fuzz_deserializer_json

./build-libfuzzer/bin/pgm_fuzz_deserializer_json \
    -dict=tests/fuzzer/pgm.dict tests/fuzzer/corpus_json
```

The harnesses expose the standard libFuzzer entry points
(`LLVMFuzzerTestOneInput` / `LLVMFuzzerInitialize`), which is what lets AFL++
(via `aflpp_driver`) and libFuzzer share the same sources with no changes. The
[OSS-Fuzz](https://github.com/google/oss-fuzz) `power-grid-model` project
compiles these same sources against `$LIB_FUZZING_ENGINE` — building for `afl`
and `libfuzzer` — and ships `corpus_json/` and `pgm.dict` as the seed corpus and
dictionary.
