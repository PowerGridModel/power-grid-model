// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_H
#define POWER_GRID_MODEL_C_H

// Generic helper definitions for shared library support
#if defined _WIN32
#define POWER_GRID_MODEL_HELPER_DLL_IMPORT __declspec(dllimport)
#define POWER_GRID_MODEL_HELPER_DLL_EXPORT __declspec(dllexport)
#define POWER_GRID_MODEL_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define POWER_GRID_MODEL_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define POWER_GRID_MODEL_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define POWER_GRID_MODEL_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define POWER_GRID_MODEL_HELPER_DLL_IMPORT
#define POWER_GRID_MODEL_HELPER_DLL_EXPORT
#define POWER_GRID_MODEL_HELPER_DLL_LOCAL
#endif
#endif
// Now we use the generic helper definitions above to define POWER_GRID_MODEL_API and POWER_GRID_MODEL_LOCAL.
// POWER_GRID_MODEL_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for
// static build) POWER_GRID_MODEL_LOCAL is used for non-api symbols.
#ifndef POWER_GRID_MODEL_STATIC      // not defined if POWER_GRID_MODEL is compiled as a DLL/SO
#ifdef POWER_GRID_MODEL_DLL_EXPORTS  // defined if we are building the POWER_GRID_MODEL DLL (instead of using it)
#define POWER_GRID_MODEL_API POWER_GRID_MODEL_HELPER_DLL_EXPORT
#else
#define POWER_GRID_MODEL_API POWER_GRID_MODEL_HELPER_DLL_IMPORT
#endif  // POWER_GRID_MODEL_DLL_EXPORTS
#define POWER_GRID_MODEL_LOCAL POWER_GRID_MODEL_HELPER_DLL_LOCAL
#else  // POWER_GRID_MODEL_STATIC is defined: this means POWER_GRID_MODEL is a static lib.
#define POWER_GRID_MODEL_API
#define POWER_GRID_MODEL_LOCAL
#endif  // POWER_GRID_MODEL_STATIC

#include <stdint.h>
// if build in progress, include the power grid model c++ header
#ifdef POWER_GRID_MODEL_BUILD_IN_PROGRESS
#include "power_grid_model/main_model.hpp"
#include "power_grid_model/power_grid_model.hpp"
#endif

// C linkage
#ifdef __cplusplus
extern "C" {
#endif

#ifdef POWER_GRID_MODEL_BUILD_IN_PROGRESS
// if build in progress, include the main model as alias
using POWER_GRID_MODEL_PowerGridModel = power_grid_model::MainModel;
// context handle
struct POWER_GRID_MODEL_Handle;
// include index type
using Idx = power_grid_model::Idx;
#else
// else define opaque pointer
typedef struct POWER_GRID_MODEL_PowerGridModel POWER_GRID_MODEL_PowerGridModel;
// context handle
typedef struct POWER_GRID_MODEL_Handle POWER_GRID_MODEL_Handle;
// index type
typedef int64_t Idx;
#endif

// create and release handle
POWER_GRID_MODEL_API POWER_GRID_MODEL_Handle* POWER_GRID_MODEL_create_handle();
POWER_GRID_MODEL_API void POWER_GRID_MODEL_destroy_handle(POWER_GRID_MODEL_Handle* handle);

// get error code and error messsage
POWER_GRID_MODEL_API Idx POWER_GRID_MODEL_err_code(POWER_GRID_MODEL_Handle const* handle);
POWER_GRID_MODEL_API char const* POWER_GRID_MODEL_err_msg(POWER_GRID_MODEL_Handle const* handle);

#ifdef __cplusplus
}
#endif

#endif  // POWER_GRID_MODEL_C_H