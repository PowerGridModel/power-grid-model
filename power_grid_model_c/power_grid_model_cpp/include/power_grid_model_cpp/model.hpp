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
    Model(double system_frequency, ConstDataset const* input_dataset)
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

    PowerGridModel* get() const { return model_.get(); }

    static void update_model(Model& model, ConstDataset const* update_dataset) {
        PGM_update_model(model.handle_.get(), model.get(), update_dataset);
        model.handle_.check_error();
    }
    void update_model(ConstDataset const* update_dataset) { update_model(*this, update_dataset); }

    static void get_indexer(Model const& model, std::string const& component, Idx size, ID const* ids, Idx* indexer) {
        PGM_get_indexer(model.handle_.get(), model.get(), component.c_str(), size, ids, indexer);
        model.handle_.check_error();
    }
    void get_indexer(std::string const& component, Idx size, ID const* ids, Idx* indexer) const {
        get_indexer(*this, component, size, ids, indexer);
    }

    static void calculate(Model& model, OptionsC const* opt, MutableDataset const* output_dataset,
                          ConstDataset const* batch_dataset) {
        PGM_calculate(model.handle_.get(), model.get(), opt, output_dataset, batch_dataset);
        model.handle_.check_error();
    }
    void calculate(OptionsC const* opt, MutableDataset const* output_dataset, ConstDataset const* batch_dataset) {
        calculate(*this, opt, output_dataset, batch_dataset);
    }

  private:
    Handle handle_{};
    detail::UniquePtr<PowerGridModel, PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP
