// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CLI_BACKEND_CLI_MAIN_H
#define POWER_GRID_MODEL_CLI_BACKEND_CLI_MAIN_H

#ifdef _WIN32
#define PGM_CLI_HELPER_DLL_IMPORT __declspec(dllimport)
#define PGM_CLI_HELPER_DLL_EXPORT __declspec(dllexport)
#define PGM_CLI_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define PGM_CLI_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define PGM_CLI_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define PGM_CLI_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define PGM_CLI_HELPER_DLL_IMPORT
#define PGM_CLI_HELPER_DLL_EXPORT
#define PGM_CLI_HELPER_DLL_LOCAL
#endif
#endif

#ifdef PGM_CLI_BACKEND_EXPORTS
#define PGM_CLI_API PGM_CLI_HELPER_DLL_EXPORT
#else
#define PGM_CLI_API PGM_CLI_HELPER_DLL_IMPORT
#endif
#define PGM_CLI_LOCAL PGM_CLI_HELPER_DLL_LOCAL

#ifdef __cplusplus
#define PGM_CLI_NOEXCEPT noexcept
#else
#define PGM_CLI_NOEXCEPT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CLI entry point for the Power Grid Model command line interface.
 *
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Process exit code.
 */
PGM_CLI_API int PGM_cli_main(int argc, char** argv) PGM_CLI_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif // POWER_GRID_MODEL_CLI_BACKEND_CLI_MAIN_H
