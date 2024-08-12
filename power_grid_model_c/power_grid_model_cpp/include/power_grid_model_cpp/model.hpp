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
    Model(double system_frequency, PGM_ConstDataset const* input_dataset)
        : model_{PGM_create_model(handle_.get(), system_frequency, input_dataset)} {}
    // copy constructor
    Model(Model const& other) : model_{PGM_copy_model(handle_.get(), other.model_.get())} {
        other.handle_.check_error();
    }

    ~Model() = default;

    static void update_model(Handle const& handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset) {
        PGM_update_model(handle.get(), model, update_dataset);
        handle.check_error();
    }
    void update_model(PGM_ConstDataset const* update_dataset) {
        PGM_update_model(handle_.get(), model_.get(), update_dataset);
        handle_.check_error();
    }

    static void get_indexer(Handle const& handle, PGM_PowerGridModel const* model, std::string const& component,
                            Idx size, ID const* ids, Idx* indexer) {
        PGM_get_indexer(handle.get(), model, component.c_str(), size, ids, indexer);
        handle.check_error();
    }
    void get_indexer(std::string const& component, Idx size, ID const* ids, Idx* indexer) const {
        PGM_get_indexer(handle_.get(), model_.get(), component.c_str(), size, ids, indexer);
        handle_.check_error();
    }

    static void calculate(Handle const& handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                          PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
        PGM_calculate(handle.get(), model, opt, output_dataset, batch_dataset);
        handle.check_error();
    }
    void calculate(PGM_Options const* opt, PGM_MutableDataset const* output_dataset,
                   PGM_ConstDataset const* batch_dataset) {
        PGM_calculate(handle_.get(), model_.get(), opt, output_dataset, batch_dataset);
        handle_.check_error();
    }

  private:
    power_grid_model_cpp::Handle handle_{};
    UniquePtr<PGM_PowerGridModel, PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP