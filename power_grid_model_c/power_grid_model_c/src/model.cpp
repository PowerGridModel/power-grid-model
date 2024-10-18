// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/model.h"

#include "handle.hpp"
#include "options.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/main_model.hpp>

namespace {
using namespace power_grid_model;
} // namespace

// aliases main class
struct PGM_PowerGridModel : public MainModel {
    using MainModel::MainModel;
};

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                     PGM_ConstDataset const* input_dataset) {
    return call_with_catch(
        handle,
        [system_frequency, input_dataset] { return new PGM_PowerGridModel{system_frequency, *input_dataset, 0}; },
        PGM_regular_error);
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset) {
    call_with_catch(
        handle, [model, update_dataset] { model->update_component<permanent_update_t>(*update_dataset); },
        PGM_regular_error);
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    return call_with_catch(handle, [model] { return new PGM_PowerGridModel{*model}; }, PGM_regular_error);
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    call_with_catch(
        handle, [model, component, size, ids, indexer] { model->get_indexer(component, ids, size, indexer); },
        PGM_regular_error);
}

// run calculation
void PGM_calculate(PGM_Handle* /*handle*/, PGM_PowerGridModel* /*model*/, PGM_Options const* /*opt*/,
                   PGM_MutableDataset const* /*output_dataset*/, PGM_ConstDataset const* /*batch_dataset*/) {
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) { delete model; }
