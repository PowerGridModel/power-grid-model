// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_FORWARD_DECLARATION_HPP
#define POWER_GRID_MODEL_C_FORWARD_DECLARATION_HPP

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

// forward declare all referenced struct/class in C++ core
// alias them in the root namespace

namespace power_grid_model {

namespace meta_data {

struct MetaAttribute;
struct MetaComponent;
struct MetaDataset;
class Serializer;
class Deserializer;

} // namespace meta_data

} // namespace power_grid_model

using PGM_MetaAttribute = power_grid_model::meta_data::MetaAttribute;
using PGM_MetaComponent = power_grid_model::meta_data::MetaComponent;
using PGM_MetaDataset = power_grid_model::meta_data::MetaDataset;
using PGM_Serializer = power_grid_model::meta_data::Serializer;
using PGM_Deserializer = power_grid_model::meta_data::Deserializer;

#endif