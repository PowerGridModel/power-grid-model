// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Shared scaffolding for the Power Grid Model fuzz harnesses.
//
// Header-only and static, so each harness stays a single translation unit and
// the CMake/OSS-Fuzz build does not need an extra library target.
//
// Every harness takes the fuzz input verbatim as one serialized document. None
// of them reads configuration bytes out of the input: which API calls run, and
// with which options, is fixed per harness, so coverage feedback only ever
// reflects how the library handled the document.
//
// What lives here:
//   1. allocation helpers with an explicit, overflow-safe memory budget,
//   2. an error-draining helper that keeps PGM_error_message() in the trace,
//   3. attribute sizing for columnar buffers.

#ifndef PGM_FUZZ_COMMON_H
#define PGM_FUZZ_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "power_grid_model_c.h"

/* --------------------------------------------------------------------------
 * Memory budget
 *
 * Deliberately generous. The point of a cap here is only to stop the engine
 * from spending its time on inputs whose sole property is "asks for a lot of
 * memory" — it is NOT meant to hide unbounded-allocation bugs. Those belong to
 * the deserializer itself (fuzz_deserialize_json, which applies no cap at all)
 * and are surfaced by the engine's own -rss_limit_mb / -malloc_limit_mb and by
 * ASan's allocator_may_return_null=0.
 * -------------------------------------------------------------------------- */
#define PGM_FUZZ_MEM_BUDGET ((size_t)256 * 1024 * 1024)

/* Upper bound on components in one dataset; the metadata has far fewer, so any
 * value above this cannot be a real component list. Keeps the fixed-size buffer
 * arrays in the harnesses honest. */
#define PGM_FUZZ_MAX_COMPONENTS 64

/* A running total of what a single LLVMFuzzerTestOneInput call has handed out.
 * Reset at the top of each harness invocation. */
typedef struct {
    size_t used;
} fuzz_budget;

static inline void budget_reset(fuzz_budget* b) { b->used = 0; }

/* Overflow-safe count * size. Returns 0 on overflow (callers treat 0 as
 * "refuse"), which is safe because every real request here is non-zero. */
static inline size_t safe_mul(size_t count, size_t size) {
    if (count != 0 && size > SIZE_MAX / count) {
        return 0;
    }
    return count * size;
}

/* aligned_alloc() requires size to be a multiple of alignment (and macOS
 * enforces it), so round up. Zero-sized requests are bumped to one alignment
 * unit so the caller always gets a distinct, freeable pointer. */
static inline void* alloc_aligned(fuzz_budget* b, size_t alignment, size_t size) {
    if (alignment == 0) {
        alignment = 1;
    }
    if (size == 0) {
        size = alignment;
    }
    if (size > SIZE_MAX - alignment) {
        return NULL;
    }
    size = ((size + alignment - 1) / alignment) * alignment;

    if (size > PGM_FUZZ_MEM_BUDGET || b->used > PGM_FUZZ_MEM_BUDGET - size) {
        return NULL;
    }
    b->used += size;
    return aligned_alloc(alignment, size);
}

/* Budget-aware malloc for the scratch arrays (indptr, id lists, attribute
 * columns) that do not need component alignment. */
static inline void* alloc_plain(fuzz_budget* b, size_t size) {
    if (size == 0) {
        size = 1;
    }
    if (size > PGM_FUZZ_MEM_BUDGET || b->used > PGM_FUZZ_MEM_BUDGET - size) {
        return NULL;
    }
    b->used += size;
    return malloc(size);
}

/* --------------------------------------------------------------------------
 * Error handling
 *
 * Always read the message before clearing: PGM_error_message() formats the
 * stored exception, so calling it keeps handle.cpp's message path in the
 * coverage trace instead of leaving it permanently cold.
 * -------------------------------------------------------------------------- */
static inline void drain_error(PGM_Handle* handle) {
    if (PGM_error_code(handle) != PGM_no_error) {
        (void)PGM_error_message(handle);
        PGM_clear_error(handle);
    }
}

/* Batch calculations report per-scenario failures out-of-band. Reading them is
 * the only way PGM_n_failed_scenarios / PGM_failed_scenarios / PGM_batch_errors
 * get exercised at all. */
static inline void drain_batch_error(PGM_Handle* handle) {
    if (PGM_error_code(handle) == PGM_batch_error) {
        PGM_Idx const n = PGM_n_failed_scenarios(handle);
        PGM_Idx const* failed = PGM_failed_scenarios(handle);
        char const** messages = PGM_batch_errors(handle);
        if (failed != NULL && messages != NULL) {
            for (PGM_Idx i = 0; i < n; i++) {
                /* Touch both arrays so a bad length or a dangling string shows
                 * up under ASan rather than being silently skipped. */
                volatile PGM_Idx scenario = failed[i];
                (void)scenario;
                if (messages[i] != NULL) {
                    (void)strlen(messages[i]);
                }
            }
        }
    }
    drain_error(handle);
}

/* --------------------------------------------------------------------------
 * Attribute sizing (columnar buffers)
 * -------------------------------------------------------------------------- */
static inline size_t ctype_size(PGM_Idx ctype) {
    switch (ctype) {
    case PGM_int32:
        return 4;
    case PGM_int8:
        return 1;
    case PGM_double:
        return 8;
    case PGM_double3:
        return 24;
    default:
        return 0;
    }
}

#endif /* PGM_FUZZ_COMMON_H */
