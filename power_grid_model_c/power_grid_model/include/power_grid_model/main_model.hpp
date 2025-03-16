// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "main_model_impl.hpp"

#include <memory>

namespace power_grid_model {

// main model class

class MainModel {
  private:
    using Impl =
        MainModelImpl<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, GenericLoadGen, GenericLoad,
                                            GenericGenerator, GenericPowerSensor, GenericVoltageSensor, Regulator>,
                      AllComponents>;

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

    BatchParameter calculate(Options const& options, MutableDataset const& result_data,
                             ConstDataset const& update_data) {
        return impl().calculate(options, result_data, update_data);
    }

    CalculationInfo calculation_info() const { return impl().calculation_info(); }

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
};

} // namespace power_grid_model
