
// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT mondify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_INPUT_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model { 

struct BaseInput {
    ID id; // ID of the object
};

struct ApplianceInput : BaseInput {
    ID node; // Node ID to which this appliance is connected
    IntS status; // If the appliance is connected
};

struct GenericLoadGenInput : ApplianceInput {
    LoadGenType type; // Type of the load_gen
};

template <bool sym>
struct LoadGenInput : GenericLoadGenInput {
    RealValue<sym> p_specified; // Specified active power
    RealValue<sym> q_specified; // Specified reactive power
};
using SymLoadGenInput = LoadGenInput<true>;
using AsymLoadGenInput = LoadGenInput<false>;



// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BaseInput";
        meta.size = sizeof(BaseInput);
        meta.alignment = alignof(BaseInput);
        meta.attributes.push_back(get_data_attribute<&BaseInput::id>("id"));
    }
};

template<>
struct get_meta<ApplianceInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ApplianceInput";
        meta.size = sizeof(ApplianceInput);
        meta.alignment = alignof(ApplianceInput);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ApplianceInput::node>("node"));
        meta.attributes.push_back(get_data_attribute<&ApplianceInput::status>("status"));
    }
};

template<>
struct get_meta<GenericLoadGenInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "GenericLoadGenInput";
        meta.size = sizeof(GenericLoadGenInput);
        meta.alignment = alignof(GenericLoadGenInput);
        meta.attributes = get_meta<ApplianceInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&GenericLoadGenInput::type>("type"));
    }
};

template<bool sym>
struct get_meta<LoadGenInput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "LoadGenInput";
        meta.size = sizeof(LoadGenInput);
        meta.alignment = alignof(LoadGenInput);
        meta.attributes = get_meta<GenericLoadGenInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&LoadGenInput::p_specified>("p_specified"));
        meta.attributes.push_back(get_data_attribute<&LoadGenInput::q_specified>("q_specified"));
    }
};

} // namespace meta_data


}  // namespace power_grid_model#endif
// clang-format off
