// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_MODEL_HPP
#define POWER_GRID_MODEL_CPP_MODEL_HPP

#include "basics.hpp"
#include "dataset.hpp"
#include "handle.hpp"
#include "options.hpp"

#include "power_grid_model_c/model.h"

namespace power_grid_model_cpp {
class Model {
  public:
    Model(double system_frequency, DatasetConst const& input_dataset)
        : model_{handle_.call_with(PGM_create_model, system_frequency, input_dataset.get())} {}
    Model(Model const& other) : model_{handle_.call_with(PGM_copy_model, other.model_.get())} {}
    Model& operator=(Model const& other) {
        if (this != &other) {
            model_.reset(handle_.call_with(PGM_copy_model, other.model_.get()));
        }
        return *this;
    }
    Model(Model&& other) noexcept : handle_{std::move(other.handle_)}, model_{std::move(other.model_)} {}
    Model& operator=(Model&& other) noexcept {
        if (this != &other) {
            handle_ = std::move(other.handle_);
            model_ = std::move(other.model_);
        }
        return *this;
    }
    ~Model() = default;

    PowerGridModel* get() const { return model_.get(); }

    void update(DatasetConst const& update_dataset) const {
        handle_.call_with(PGM_update_model, model_.get(), update_dataset.get());
    }

    void get_indexer(std::string const& component, Idx size, ID const* ids, Idx* indexer) const {
        handle_.call_with(PGM_get_indexer, model_.get(), component.c_str(), size, ids, indexer);
    }

    void calculate(Options const& opt, DatasetMutable const& output_dataset, DatasetConst const& batch_dataset) const {
        handle_.call_with(PGM_calculate, model_.get(), opt.get(), output_dataset.get(), batch_dataset.get());
    }

    void calculate(Options const& opt, DatasetMutable const& output_dataset) const {
        handle_.call_with(PGM_calculate, model_.get(), opt.get(), output_dataset.get(), nullptr);
    }

  private:
    Handle handle_{};
    detail::UniquePtr<PowerGridModel, &PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP
