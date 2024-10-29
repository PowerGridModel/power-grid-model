// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "main_model_impl.hpp"

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

    explicit MainModel(double system_frequency, ConstDataset const& input_data, Idx pos = 0)
        : impl_{std::make_unique<Impl>(system_frequency, input_data, pos)} {}
    explicit MainModel(double system_frequency, meta_data::MetaData const& meta_data)
        : impl_{std::make_unique<Impl>(system_frequency, meta_data)} {};

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

    std::map<std::string, Idx, std::less<>> all_component_count() const { return impl().all_component_count(); }
    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        impl().get_indexer(component_type, id_begin, size, indexer_begin);
    }

    void set_construction_complete() { impl().set_construction_complete(); }
    void restore_components(ConstDataset const& update_data) {
        impl().restore_components(impl().get_sequence_idx_map(update_data));
    }

    template <class CompType> void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(std::span<typename CompType::InputType const>{components});
    }
    template <class CompType> void add_component(std::span<typename CompType::InputType const> components) {
        impl().add_component<CompType>(components);
    }

    template <cache_type_c CacheType> void update_component(ConstDataset const& update_data) {
        impl().update_component<CacheType>(update_data);
    }

    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::vector<OutputType>& target) const {
        output_result<CompType>(math_output, std::span<OutputType>{target});
    }
    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::span<OutputType> target) const {
        impl().output_result<CompType>(math_output, target.begin());
    }

    template <calculation_type_tag calculation_type, symmetry_tag sym> auto calculate(Options const& options) {
        return impl().calculate<calculation_type, sym>(options);
    }
    void calculate(Options const& options, MutableDataset const& result_data) {
        return impl().calculate(options, result_data);
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
