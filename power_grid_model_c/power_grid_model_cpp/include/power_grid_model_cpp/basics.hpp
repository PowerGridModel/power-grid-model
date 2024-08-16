// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BASICS_HPP
#define POWER_GRID_MODEL_CPP_BASICS_HPP

#ifdef PGM_DLL_EXPORTS
#error "Cannot export dynamic targets from the PGM C API wrapper. Please disable PGM_DLL_EXPORT."
#endif

#include "power_grid_model_c/basics.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace power_grid_model_cpp {

using Idx = PGM_Idx;
using ID = PGM_ID;

using MetaComponent = PGM_MetaComponent;
using MetaAttribute = PGM_MetaAttribute;
using RawDataPtr = void*;            // raw mutable data ptr
using RawDataConstPtr = void const*; // raw read-only data ptr
//using ConstDataset = PGM_ConstDataset;
//using MutableDataset = PGM_MutableDataset; //
//using WritableDataset = PGM_WritableDataset; // 
//using DatasetInfo = PGM_DatasetInfo; //
using MetaDataset = PGM_MetaDataset;
using PowerGridModel = PGM_PowerGridModel;
using OptionsC = PGM_Options; // 
using DeserializerC = PGM_Deserializer; //
using SerializerC = PGM_Serializer; //

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
