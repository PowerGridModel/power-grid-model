// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BASICS_HPP
#define POWER_GRID_MODEL_CPP_BASICS_HPP

#ifdef PGM_DLL_EXPORTS
#error "Cannot export dynamic targets from the PGM C API wrapper. Please disable PGM_DLL_EXPORT."
#endif

#ifndef PGM_ENABLE_EXPERIMENTAL
#error "This is an experimental feature. Please #define PGM_ENABLE_EXPERIMENTAL to use this."
#endif

#include "power_grid_model_c/basics.h"

#include <cassert>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace power_grid_model_cpp {

using Idx = PGM_Idx;
using ID = PGM_ID;

using PowerGridModel = PGM_PowerGridModel;
using MetaDataset = PGM_MetaDataset;
using MetaComponent = PGM_MetaComponent;
using MetaAttribute = PGM_MetaAttribute;
using RawHandle = PGM_Handle;
using RawDataPtr = void*;            // raw mutable data ptr
using RawDataConstPtr = void const*; // raw read-only data ptr
using RawConstDataset = PGM_ConstDataset;
using RawMutableDataset = PGM_MutableDataset;
using RawWritableDataset = PGM_WritableDataset;
using RawDatasetInfo = PGM_DatasetInfo;
using RawOptions = PGM_Options;
using RawDeserializer = PGM_Deserializer;
using RawSerializer = PGM_Serializer;

enum CalculationType {
    power_flow = 0,       // power flow calculation
    state_estimation = 1, // state estimation calculation
    short_circuit = 2     // short circuit calculation
};

enum CalculationMethod {
    default_method = -128, // the default method for each calculation type, e.g. Newton-Raphson for power flow
    linear = 0,            // linear constant impedance method for power flow
    newton_raphson = 1,    // Newton-Raphson method for power flow or state estimation
    iterative_linear = 2,  // iterative linear method for state estimation
    iterative_current = 3, // linear current method for power flow
    linear_current = 4,    // iterative constant impedance method for power flow
    iec60909 = 5           // fault analysis for short circuits using the iec60909 standard
};

enum ErrorCode {
    no_error = 0,           // no error occurred
    regular_error = 1,      // some error occurred which is not in the batch calculation
    batch_error = 2,        // some error occurred which is in the batch calculation
    serialization_error = 3 // some error occurred which is in the (de)serialization process
};

enum CType {
    Cint32 = 0,   // int32_t
    Cint8 = 1,    // int8_t
    Cdouble = 2,  // double
    Cdouble3 = 3, // double[3]
};

enum SerializationFormat {
    json = 0,    // JSON serialization format
    msgpack = 1, // msgpack serialization format
};

enum ShortCircuitVoltageScaling {
    short_circuit_voltage_scaling_minimum = 0, // voltage scaling for minimum short circuit currents
    short_circuit_voltage_scaling_maximum = 1, // voltage scaling for maximum short circuit currents
};

enum TapChangingStrategy {
    tap_changing_strategy_disabled = 0, // disable automatic tap adjustment
    tap_changing_strategy_any_valid_tap =
        1, // adjust tap position automatically; optimize for any value in the voltage band
    tap_changing_strategy_min_voltage_tap =
        2, // adjust tap position automatically; optimize for the lower end of the voltage band
    tap_changing_strategy_max_voltage_tap =
        3, // adjust tap position automatically; optimize for the higher end of the voltage band
    tap_changing_strategy_fast_any_tap =
        4, // adjust tap position automatically; optimize for any value in the voltage band; binary search
};

enum ExperimentalFeatures {
    experimental_features_disabled = 0, // disable experimental features
    experimental_features_enabled = 1,  // enable experimental features
};

namespace detail {
// custom deleter
template <auto func> struct DeleterFunctor {
    template <typename T> void operator()(T* arg) const { func(arg); }
};

// unique pointer definition
template <typename T, auto func> using UniquePtr = std::unique_ptr<T, DeleterFunctor<func>>;
} // namespace detail

} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BASICS_HPP
