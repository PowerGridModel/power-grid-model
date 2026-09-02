# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# Fuzz harnesses for the Power Grid Model C API.
#
# These targets are only configured when PGM_ENABLE_FUZZER=ON. The default
# fuzzing engine is AFL++ (build with the afl-clang-fast toolchain); libFuzzer
# is also supported via -DPGM_FUZZER_ENGINE=libfuzzer. 

set(PGM_FUZZER_ENGINE "afl" CACHE STRING
    "Fuzzing engine for the harnesses: afl (default) or libfuzzer")
set_property(CACHE PGM_FUZZER_ENGINE PROPERTY STRINGS afl libfuzzer)

add_executable(pgm_fuzz_deserializer_json harness_deserializer_json.c)

# power_grid_model_c is defined as SHARED and exports its public include dir,
# so linking against it is enough to resolve <power_grid_model_c.h>.
target_link_libraries(pgm_fuzz_deserializer_json PRIVATE power_grid_model_c)

set_target_properties(pgm_fuzz_deserializer_json PROPERTIES LINKER_LANGUAGE CXX)

if(PGM_FUZZER_ENGINE STREQUAL "afl")
    # AFL++: instrumentation comes from the compiler itself, so this requires
    # configuring with the AFL++ toolchain, e.g.
    #   -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_CXX_COMPILER=afl-clang-fast++
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
    target_link_libraries(pgm_fuzz_deserializer_json PRIVATE "${PGM_AFL_DRIVER}")

elseif(PGM_FUZZER_ENGINE STREQUAL "libfuzzer")
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "PGM_FUZZER_ENGINE=libfuzzer requires a Clang toolchain "
            "(-fsanitize=fuzzer).")
    endif()
    # Coverage instrumentation + the libFuzzer runtime (provides main()).
    target_compile_options(pgm_fuzz_deserializer_json PRIVATE -fsanitize=fuzzer-no-link)
    target_link_options(pgm_fuzz_deserializer_json PRIVATE -fsanitize=fuzzer)

else()
    message(FATAL_ERROR
        "Unknown PGM_FUZZER_ENGINE '${PGM_FUZZER_ENGINE}' (use afl or libfuzzer).")
endif()
