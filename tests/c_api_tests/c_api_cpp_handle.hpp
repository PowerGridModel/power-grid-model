// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "power_grid_model_c.h"

#include <memory>

namespace power_grid_model {

// custom deleter
template <auto func> struct DeleterFunctor {
    template <typename T> void operator()(T* arg) const { func(arg); }
};

// unique pointers
using HandlePtr = std::unique_ptr<PGM_Handle, DeleterFunctor<&PGM_destroy_handle>>;
using OptionPtr = std::unique_ptr<PGM_Options, DeleterFunctor<&PGM_destroy_options>>;
using ModelPtr = std::unique_ptr<PGM_PowerGridModel, DeleterFunctor<&PGM_destroy_model>>;
using BufferPtr = std::unique_ptr<void, DeleterFunctor<&PGM_destroy_buffer>>;
using SerializerPtr = std::unique_ptr<PGM_Serializer, DeleterFunctor<&PGM_destroy_serializer>>;
using DeserializerPtr = std::unique_ptr<PGM_Deserializer, DeleterFunctor<&PGM_destroy_deserializer>>;
using ConstDatasetPtr = std::unique_ptr<PGM_ConstDataset, DeleterFunctor<&PGM_destroy_dataset_const>>;
using MutableDatasetPtr = std::unique_ptr<PGM_MutableDataset, DeleterFunctor<&PGM_destroy_dataset_mutable>>;

} // namespace power_grid_model
