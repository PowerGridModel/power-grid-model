// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include <power_grid_model/auxiliary/dataset_fwd.hpp>

#include "power_grid_model_c/basics.h"

// // forward declare all referenced struct/class in C++ core
// // alias them in the root namespace

namespace power_grid_model::meta_data {

struct MetaAttribute;
struct MetaComponent;
struct MetaDataset;
class Serializer;
class Deserializer;

template <dataset_type_tag dataset_type> class Dataset;
using ConstDataset = Dataset<const_dataset_t>;
using MutableDataset = Dataset<mutable_dataset_t>;
using WritableDataset = Dataset<writable_dataset_t>;

struct DatasetInfo;

} // namespace power_grid_model::meta_data

namespace power_grid_model_c {
using power_grid_model::meta_data::ConstDataset;
using power_grid_model::meta_data::Dataset;
using power_grid_model::meta_data::DatasetInfo;
using power_grid_model::meta_data::Deserializer;
using power_grid_model::meta_data::MetaAttribute;
using power_grid_model::meta_data::MetaComponent;
using power_grid_model::meta_data::MetaDataset;
using power_grid_model::meta_data::MutableDataset;
using power_grid_model::meta_data::Serializer;
using power_grid_model::meta_data::WritableDataset;

MetaComponent const& unwrap(PGM_MetaComponent const&);
MetaAttribute const& unwrap(PGM_MetaAttribute const&);
ConstDataset const& unwrap(PGM_ConstDataset const&);
ConstDataset const* unwrap(PGM_ConstDataset const*);
MutableDataset const& unwrap(PGM_MutableDataset const&);
PGM_WritableDataset& wrap(WritableDataset&);
PGM_DatasetInfo const& wrap(DatasetInfo const&);
PGM_MetaDataset const& wrap(MetaDataset const&);
PGM_MetaComponent const& wrap(MetaComponent const&);
PGM_MetaAttribute const& wrap(MetaAttribute const&);
} // namespace power_grid_model_c
