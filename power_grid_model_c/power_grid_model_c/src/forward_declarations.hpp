// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include <power_grid_model/auxiliary/dataset_fwd.hpp>

// forward declare all referenced struct/class in C++ core
// alias them in the root namespace

namespace power_grid_model::meta_data {

struct MetaAttribute;
struct MetaComponent;
struct MetaDataset;
class Serializer;
class Deserializer;

template <dataset_handler_tag dataset_handler_type> class DatasetHandler;

struct DatasetInfo;

} // namespace power_grid_model::meta_data

using PGM_MetaAttribute = power_grid_model::meta_data::MetaAttribute;
using PGM_MetaComponent = power_grid_model::meta_data::MetaComponent;
using PGM_MetaDataset = power_grid_model::meta_data::MetaDataset;
using PGM_Serializer = power_grid_model::meta_data::Serializer;
using PGM_Deserializer = power_grid_model::meta_data::Deserializer;
using PGM_ConstDataset = power_grid_model::meta_data::DatasetHandler<power_grid_model::const_dataset_t>;
using PGM_MutableDataset = power_grid_model::meta_data::DatasetHandler<power_grid_model::mutable_dataset_t>;
using PGM_WritableDataset = power_grid_model::meta_data::DatasetHandler<power_grid_model::writable_dataset_t>;
using PGM_DatasetInfo = power_grid_model::meta_data::DatasetInfo;
