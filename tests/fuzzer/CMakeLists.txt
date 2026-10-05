# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# Fuzz harnesses for the Power Grid Model C API.
#
# Configured only when PGM_ENABLE_FUZZER=ON. The default engine is AFL++ (build
# with the afl-clang-fast toolchain); libFuzzer is available via
# -DPGM_FUZZER_ENGINE=libfuzzer. Every harness exposes the libFuzzer entry
# points, so both engines — and OSS-Fuzz's $LIB_FUZZING_ENGINE — build the same
# sources unchanged.
#
# PGM_FUZZER_ENGINE is declared in the root CMakeLists.txt, because the
# libFuzzer engine's coverage flag has to be applied before the C API library
# target exists; this file validates it and wires up the harnesses.

# Resolve the engine's runtime once, rather than per target.
if(PGM_FUZZER_ENGINE STREQUAL "afl")
    # AFL++ instrumentation comes from the compiler, so the whole build — the
    # harness and the C API library it links against — must be configured with
    # the AFL++ toolchain.
    if(NOT CMAKE_C_COMPILER MATCHES "afl-clang")
        message(FATAL_ERROR
            "PGM_FUZZER_ENGINE=afl requires the AFL++ compiler. Configure with "
            "-DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++.")
    endif()

    # aflpp_driver supplies an AFL fork-server main() for LLVMFuzzerTestOneInput
    # harnesses; it ships with AFL++ as libAFLDriver.a. Override the location
    # with -DPGM_AFL_DRIVER=/path/to/libAFLDriver.a or by setting AFL_PATH.
    find_library(PGM_AFL_DRIVER
        NAMES AFLDriver
        PATHS /usr/local/lib/afl /usr/lib/afl ENV AFL_PATH
        PATH_SUFFIXES lib afl "")
    if(NOT PGM_AFL_DRIVER)
        message(FATAL_ERROR
            "Could not find libAFLDriver.a (AFL++ aflpp_driver). Set AFL_PATH or "
            "pass -DPGM_AFL_DRIVER=/path/to/libAFLDriver.a.")
    endif()
elseif(PGM_FUZZER_ENGINE STREQUAL "libfuzzer")
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "PGM_FUZZER_ENGINE=libfuzzer requires a Clang toolchain "
            "(-fsanitize=fuzzer).")
    endif()
else()
    message(FATAL_ERROR
        "Unknown PGM_FUZZER_ENGINE '${PGM_FUZZER_ENGINE}' (use afl or libfuzzer).")
endif()

# Declare one harness. The engine plumbing is identical for all of them, so it
# is written once here instead of being repeated per target.
function(pgm_add_fuzz_target name source)
    add_executable(${name} ${source})

    # power_grid_model_c is SHARED and exports its public include directory, so
    # linking against it resolves <power_grid_model_c.h>.
    target_link_libraries(${name} PRIVATE power_grid_model_c)

    # fuzz_common.h lives next to the harness sources.
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # The C API is C++ internally; the harnesses are C. Link as C++.
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)

    if(PGM_FUZZER_ENGINE STREQUAL "afl")
        target_link_libraries(${name} PRIVATE "${PGM_AFL_DRIVER}")
    else()
        # -fsanitize=fuzzer-no-link is already applied globally (root
        # CMakeLists.txt); only the link step needs the engine itself.
        target_link_options(${name} PRIVATE -fsanitize=fuzzer)
    endif()
endfunction()

# Every harness takes one document verbatim; see README.md for what each runs.

# JSON deserialization front-end. Fast; no buffers, no model.
pgm_add_fuzz_target(pgm_fuzz_deserialize_json    fuzz_deserialize_json.c)

# Parse into caller-owned buffers, both row and columnar. Any dataset, any batch
# size including non-uniform.
pgm_add_fuzz_target(pgm_fuzz_parse_to_buffer     fuzz_parse_to_buffer.c)

# Model construction, copy and ID indexing. No solver.
pgm_add_fuzz_target(pgm_fuzz_model_create        fuzz_model_create.c)

# One target per calculation type, each running a fixed list of option sets.
pgm_add_fuzz_target(pgm_fuzz_power_flow          fuzz_power_flow.c)
pgm_add_fuzz_target(pgm_fuzz_state_estimation    fuzz_state_estimation.c)
pgm_add_fuzz_target(pgm_fuzz_short_circuit       fuzz_short_circuit.c)

# Update documents against a fixed grid: batch dispatch, the per-scenario batch
# error channel, and in-place model update.
pgm_add_fuzz_target(pgm_fuzz_model_batch_update  fuzz_model_batch_update.c)
