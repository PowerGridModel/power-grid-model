// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_MODEL_HPP
#define POWER_GRID_MODEL_CPP_MODEL_HPP

#include "power_grid_model_c/model.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Model {
  public:
    // constructor
    Model(double system_frequency, PGM_ConstDataset const* input_dataset)
        : model_{PGM_create_model(handle_.get(), system_frequency, input_dataset)} {}
    // copy constructor
    Model(Model const& other) : model_{PGM_copy_model(handle_.get(), other.model_.get())} {
        other.handle_.check_error();
    }
    // copy assignment operator
    Model& operator=(Model const& other) {
        if (this != &other) {
            model_.reset(PGM_copy_model(handle_.get(), other.model_.get()));
            other.handle_.check_error();
        }
        return *this;
    }
    // destructor
    ~Model() = default;

    PGM_PowerGridModel* get() const { return model_.get(); }

    static void update_model(Handle const& handle, Model& model, PGM_ConstDataset const* update_dataset) {
        PGM_update_model(handle.get(), model.get(), update_dataset);
        handle.check_error();
    }
    void update_model(PGM_ConstDataset const* update_dataset) { update_model(handle_, *this, update_dataset); }

    static void get_indexer(Handle const& handle, Model const& model, std::string const& component, Idx size,
                            ID const* ids, Idx* indexer) {
        PGM_get_indexer(handle.get(), model.get(), component.c_str(), size, ids, indexer);
        handle.check_error();
    }
    void get_indexer(std::string const& component, Idx size, ID const* ids, Idx* indexer) const {
        get_indexer(handle_, *this, component, size, ids, indexer);
    }

    static void calculate(Handle const& handle, Model model, PGM_Options const* opt,
                          PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
        PGM_calculate(handle.get(), model.get(), opt, output_dataset, batch_dataset);
        handle.check_error();
    }
    void calculate(PGM_Options const* opt, PGM_MutableDataset const* output_dataset,
                   PGM_ConstDataset const* batch_dataset) {
        calculate(handle_, *this, opt, output_dataset, batch_dataset);
    }

  private:
    Handle handle_{};
    UniquePtr<PGM_PowerGridModel, PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP
