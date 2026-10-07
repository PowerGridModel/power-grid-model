<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Fuzz harnesses

Coverage-guided fuzz harnesses for the Power Grid Model C API. They run under
**AFL++** by default; libFuzzer is also supported, and both drive the same
sources unchanged.

## Why several harnesses

The library's untrusted-input path is a funnel: a document must parse before it
can be written into buffers, buffers must be populated before a model can be
built, and a model must exist before a solver runs. Driving that whole chain
from one target means every malformed-JSON execution pays for a solver that it
will never reach, while every execution that *does* reach the solver had to be a
fully valid document first. The parser gets slow executions; the solver gets a
narrow slice of inputs; and coverage feedback cannot tell which stage a new edge
came from.

Splitting on the funnel's natural boundaries fixes all three. It also makes
whole regions of the API reachable that a single end-to-end target must reject
in order to keep going — non-`input` datasets, batch data, ragged (non-uniform)
scenarios, columnar buffers, and the batch error channel.

| Harness | Entry point | What it drives |
|---------|-------------|----------------|
| `fuzz_deserialize_json` | `PGM_create_deserializer_from_binary_buffer` | JSON parsing and the complete dataset-info surface. No buffers, no model, no solver. |
| `fuzz_parse_to_buffer` | `PGM_deserializer_parse_to_buffer` | Writing parsed values into caller-owned memory. Every input is parsed twice: into row buffers and into columnar buffers. Any dataset kind, any batch size, including ragged batches that require an `indptr`. |
| `fuzz_model_create` | `PGM_create_model` | Model construction, `PGM_copy_model`, and `PGM_get_indexer`. No solver, so it is fast. |
| `fuzz_power_flow` | `PGM_calculate` | Every power-flow method in both symmetries, plus every automatic tap-changing strategy. |
| `fuzz_state_estimation` | `PGM_calculate` | Both state-estimation methods in both symmetries. |
| `fuzz_short_circuit` | `PGM_calculate` | IEC 60909 in both symmetries and both voltage scalings. |
| `fuzz_model_batch_update` | batch `PGM_calculate` + `PGM_update_model` | Applying an untrusted update document to a live model: per-scenario batch dispatch, the `PGM_n_failed_scenarios` / `PGM_failed_scenarios` / `PGM_batch_errors` channel, and in-place update. |

`fuzz_common.h` holds what every harness shares (memory budget, error
draining); `fuzz_model.h` holds document parsing, model construction, and the
calculation runner used by the harnesses past the parser.

### The batch target's fixed grid

A batch calculation needs two documents: the grid and the update applied to it.
Carrying both in one input requires framing — a length prefix or a separator —
which is exactly the kind of structural control the harnesses avoid; it also
means most mutations destroy one of the two documents. So the grid is fixed
instead: `update_grid.json` (rendered into `update_grid.h`) is a small meshed
grid with every updatable component type the corpus targets — sources, lines,
a transformer with a tap regulator, symmetric and asymmetric loads and
generators, and a shunt. The fuzz input is only the update document.

Grid-side variety is not lost: the single-calculation harnesses already fuzz the
grid. What this harness adds is the update path, and it now gets every byte of
the input.

The batch calculation runs before `PGM_update_model` because it leaves the model
as it found it; the in-place update then starts from the same grid on every
input. Whichever of the two does not fit the document's shape (batch vs.
single) is rejected by the library, and that rejection path is fuzzed too.

### The batch document shape

A batch document's `data` is a **list of scenarios**, each scenario a map of
component to elements:

```json
{"is_batch": true, "data": [ {"source": [{"id": 1, "u_ref": 1.05}]},
                             {"source": [{"id": 1, "u_ref": 0.95}]} ]}
```

It is *not* a map of component to per-scenario lists. The deserializer checks
this and rejects the mismatch outright ("Map/Array type of data does not match
is_batch!"), so a document with the wrong shape never gets past the parser — and
a corpus built that way silently exercises nothing. `corpus_json` keeps one
deliberately wrong-shaped document (`20_batch_shape_mismatch.json`) so that
rejection path stays covered.

## Corpora and dictionary

- `corpus_json/` — hand-maintained documents of every kind: valid grids,
  boundary values, columnar documents, grids with sensors, faults and tap
  regulators, and deliberately malformed input. Seed corpus for every harness
  except the batch target.
- `corpus_update/` — **generated** by `tools/make_corpus.py`: uniform, ragged,
  and single (non-batch) update documents for the fixed grid in
  `update_grid.json`. Seed corpus for `fuzz_model_batch_update`.
- `update_grid.h` — **generated** from `update_grid.json` by the same script.
- `pgm.dict` — dictionary of PGM JSON envelope keys, component and attribute
  names, and common numeric literals. Shared by all harnesses.

Add new documents to `corpus_json/`, edit the grid in `update_grid.json`, then
regenerate:

```bash
python3 tools/make_corpus.py
```

## Building and running with AFL++ (default)

Requires the AFL++ toolchain (`afl-clang-fast` / `afl-clang-fast++`), which
instruments both the harnesses and the C API library they link against.

```bash
export AFL_USE_ASAN=1   # optional: also build with AddressSanitizer
cmake -S . -B build -G Ninja \
    -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++ \
    -DPGM_ENABLE_FUZZER=ON
cmake --build build

afl-fuzz -i tests/fuzzer/corpus_json -o fuzz_out \
    -x tests/fuzzer/pgm.dict -- ./build/bin/pgm_fuzz_deserialize_json @@
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
cmake --build build-libfuzzer

./build-libfuzzer/bin/pgm_fuzz_power_flow \
    -dict=tests/fuzzer/pgm.dict tests/fuzzer/corpus_json
```

Every harness uses `corpus_json` except `pgm_fuzz_model_batch_update`, which
uses `corpus_update`.

## OSS-Fuzz

All harnesses expose the standard libFuzzer entry point
(`LLVMFuzzerTestOneInput`), which is what lets AFL++
(via `aflpp_driver`) and libFuzzer share the same sources. The
[OSS-Fuzz](https://github.com/google/oss-fuzz) `power-grid-model` project
compiles these sources against `$LIB_FUZZING_ENGINE` — building for both `afl`
and `libfuzzer` — and ships each corpus directory and `pgm.dict` as that
target's seed corpus and dictionary.
