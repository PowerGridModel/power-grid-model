// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BASICS_HPP
#define POWER_GRID_MODEL_CPP_BASICS_HPP

#ifdef PGM_DLL_EXPORTS
#error                                                                                                                 \
    "Cannot export out-of-line PGM C API symbols from the inline header-only PGM C++ API. Please disable PGM_DLL_EXPORT."
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
using IntS = int8_t;

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

namespace detail {
// custom deleter
template <auto func> struct DeleterFunctor {
    template <typename T> void operator()(T* arg) const { func(arg); }
};

// unique pointer definition
template <typename T, auto func> class UniquePtr : public std::unique_ptr<T, DeleterFunctor<func>> {
  public:
    using std::unique_ptr<T, DeleterFunctor<func>>::unique_ptr;
    using std::unique_ptr<T, DeleterFunctor<func>>::operator=;
    T* get() { return static_cast<std::unique_ptr<T, DeleterFunctor<func>>&>(*this).get(); }
    T const* get() const { return static_cast<std::unique_ptr<T, DeleterFunctor<func>> const&>(*this).get(); }
};
} // namespace detail

} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BASICS_HPP
