# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# Fuzzer engine setup. Included from the root CMakeLists.txt only when
# PGM_ENABLE_FUZZER=ON, before the C API target is declared, so a bad engine
# configuration fails early and the C API can pick up the flags set here.
#
# Sets PGM_FUZZER_SANITIZER_OPTIONS: the compile options the C API library needs
# so that the engine's coverage instrumentation reaches the code under test.
# Empty for AFL++, which instruments through its compiler wrapper instead.
# Also sets PGM_AFL_DRIVER for the AFL++ engine.

set(PGM_FUZZER_SANITIZER_OPTIONS "")

if(PGM_FUZZER_ENGINE STREQUAL "afl")
    # AFL++ instrumentation comes from the compiler, so the whole build — the
    # harness and the C API library it links against — must be configured with
    # the AFL++ toolchain.
    if(NOT CMAKE_C_COMPILER MATCHES "afl-clang")
        message(
            FATAL_ERROR
            "PGM_FUZZER_ENGINE=afl requires the AFL++ compiler. Configure with "
            "-DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++."
        )
    endif()

    # aflpp_driver supplies an AFL fork-server main() for LLVMFuzzerTestOneInput
    # harnesses; it ships with AFL++ as libAFLDriver.a. Override the location
    # with -DPGM_AFL_DRIVER=/path/to/libAFLDriver.a or by setting AFL_PATH.
    find_library(
        PGM_AFL_DRIVER
        NAMES AFLDriver
        PATHS /usr/local/lib/afl /usr/lib/afl
        ENV AFL_PATH
        PATH_SUFFIXES lib afl ""
    )
    if(NOT PGM_AFL_DRIVER)
        message(
            FATAL_ERROR
            "Could not find libAFLDriver.a (AFL++ aflpp_driver). Set AFL_PATH or "
            "pass -DPGM_AFL_DRIVER=/path/to/libAFLDriver.a."
        )
    endif()
elseif(PGM_FUZZER_ENGINE STREQUAL "libfuzzer")
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(
            FATAL_ERROR
            "PGM_FUZZER_ENGINE=libfuzzer requires a Clang toolchain "
            "(-fsanitize=fuzzer)."
        )
    endif()

    # The code under test lives in the C API library, not in the harnesses, so
    # libFuzzer's coverage instrumentation has to reach it too. The harnesses
    # get the engine itself at link time (see tests/fuzzer/CMakeLists.txt).
    set(PGM_FUZZER_SANITIZER_OPTIONS -fsanitize=fuzzer-no-link)
else()
    message(
        FATAL_ERROR
        "Unknown PGM_FUZZER_ENGINE '${PGM_FUZZER_ENGINE}' (use afl or libfuzzer)."
    )
endif()
