// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "job_adapter.hpp"
#include "job_dispatch.hpp"
#include "main_model_impl.hpp"

#include "common/calculation_info.hpp"

#include <memory>

namespace power_grid_model {

// main model class

class MainModel {
  private:
    using Impl = MainModelImpl<main_core::MainModelType<AllExtraRetrievableTypes, AllComponents>>;

  public:
    using Options = MainModelOptions;

    explicit MainModel(double system_frequency, ConstDataset const& input_data,
                       MathSolverDispatcher const& math_solver_dispatcher, Idx pos = 0)
        : impl_{std::make_unique<Impl>(system_frequency, input_data, math_solver_dispatcher, pos)} {}
    explicit MainModel(double system_frequency, meta_data::MetaData const& meta_data,
                       MathSolverDispatcher const& math_solver_dispatcher)
        : impl_{std::make_unique<Impl>(system_frequency, meta_data, math_solver_dispatcher)} {};

    // deep copy
    MainModel(MainModel const& other) {
        if (other.impl_ != nullptr) {
            impl_ = std::make_unique<Impl>(*other.impl_);
        }
    }
    MainModel& operator=(MainModel const& other) {
        if (this != &other) {
            impl_.reset();
            if (other.impl_ != nullptr) {
                impl_ = std::make_unique<Impl>(*other.impl_);
            }
        }
        return *this;
    }
    MainModel(MainModel&& other) noexcept : impl_{std::move(other.impl_)} {}
    MainModel& operator=(MainModel&& other) noexcept {
        if (this != &other) {
            impl_ = std::move(other.impl_);
        }
        return *this;
    };
    ~MainModel() { impl_.reset(); }

    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        impl().get_indexer(component_type, id_begin, size, indexer_begin);
    }

    template <cache_type_c CacheType> void update_components(ConstDataset const& update_data) {
        impl().update_components<CacheType>(update_data.get_individual_scenario(0));
    }

    /*
    Batch calculation, propagating the results to result_data

    Run the calculation function in batch on the provided update data.

    The calculation function should be able to run standalone.

    threading
        < 0 sequential
        = 0 parallel, use number of hardware threads
        > 0 specify number of parallel threads
    raise a BatchCalculationError if any of the calculations in the batch raised an exception
    */
    BatchParameter calculate(Options const& options, MutableDataset const& result_data,
                             ConstDataset const& update_data) {
        info_.clear();
        JobAdapter<Impl> adapter{std::ref(impl()), std::ref(options)};
        return JobDispatch::batch_calculation(adapter, result_data, update_data, options.threading, info_);
    }

    CalculationInfo calculation_info() const { return info_.get(); }

    void check_no_experimental_features_used(Options const& options) const {
        impl().check_no_experimental_features_used(options);
    }

  private:
    Impl& impl() {
        assert(impl_ != nullptr);
        return *impl_;
    }
    Impl const& impl() const {
        assert(impl_ != nullptr);
        return *impl_;
    }

    std::unique_ptr<Impl> impl_;
    MultiThreadedCalculationInfo info_;
};

} // namespace power_grid_model
