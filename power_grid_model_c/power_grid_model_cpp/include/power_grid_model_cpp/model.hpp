// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_MODEL_HPP
#define POWER_GRID_MODEL_CPP_MODEL_HPP

#include "model.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Model {
  public:
    power_grid_model_cpp::Handle handle;

    Model(double system_frequency, PGM_ConstDataset const* input_dataset)
        : handle(), model_{PGM_create_model(handle.get(), system_frequency, input_dataset)} {}
    Model(Model const& other) : handle(), model_{PGM_copy_model(handle.get(), other.model_.get())} {}

    ~Model() = default;

    static PGM_PowerGridModel* copy_model(PGM_Handle* provided_handle, PGM_PowerGridModel const* model) {
        return PGM_copy_model(provided_handle, model);
    }

    static void update_model(PGM_Handle* provided_handle, PGM_PowerGridModel* model,
                             PGM_ConstDataset const* update_dataset) {
        return PGM_update_model(provided_handle, model, update_dataset);
    }
    void update_model(PGM_ConstDataset const* update_dataset) {
        return PGM_update_model(handle.get(), model_.get(), update_dataset);
    }

    static void get_indexer(PGM_Handle* provided_handle, PGM_PowerGridModel const* model, char const* component,
                            Idx size, ID const* ids, Idx* indexer) {
        PGM_get_indexer(provided_handle, model, component, size, ids, indexer);
    }
    void get_indexer(char const* component, Idx size, ID const* ids, Idx* indexer) const {
        PGM_get_indexer(handle.get(), model_.get(), component, size, ids, indexer);
    }

    static void calculate(PGM_Handle* provided_handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                          PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
        PGM_calculate(provided_handle, model, opt, output_dataset, batch_dataset);
    }
    void calculate(PGM_Options const* opt, PGM_MutableDataset const* output_dataset,
                   PGM_ConstDataset const* batch_dataset) {
        PGM_calculate(handle.get(), model_.get(), opt, output_dataset, batch_dataset);
    }

  private:
    UniquePtr<PGM_PowerGridModel, PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP