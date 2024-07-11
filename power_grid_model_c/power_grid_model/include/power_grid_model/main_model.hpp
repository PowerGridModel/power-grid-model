// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model_impl.hpp>

#include <map>
#include <memory>

namespace power_grid_model {

// main model wrapper class

class MainModelWrapper {
  private:
    class Impl : public MainModel {
      public:
        using MainModel::MainModel;
    };

  public:
    using Options = MainModelOptions;

    explicit MainModelWrapper(double system_frequency, ConstDataset const& input_data, Idx pos = 0)
        : impl_{std::make_unique<Impl>(system_frequency, input_data, pos)} {}
    explicit MainModelWrapper(double system_frequency, meta_data::MetaData const& meta_data)
        : impl_{std::make_unique<Impl>(system_frequency, meta_data)} {};

    // deep copy
    MainModelWrapper(MainModelWrapper const& other)
        : impl_{other.impl_ == nullptr ? nullptr : new Impl{*other.impl_}} {}
    MainModelWrapper& operator=(MainModelWrapper& other) {
        impl_.reset(other.impl_ == nullptr ? nullptr : new Impl{*other.impl_});
        return *this;
    }
    MainModelWrapper(MainModelWrapper&& other) = default;
    MainModelWrapper& operator=(MainModelWrapper&& /* other */) = default;
    ~MainModelWrapper() = default;

    static bool is_update_independent(ConstDataset const& update_data) {
        return Impl::is_update_independent(update_data);
    }

    std::map<std::string, Idx> all_component_count() const {
        assert(impl_ != nullptr);
        return impl_->all_component_count();
    }
    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        assert(impl_ != nullptr);
        impl_->get_indexer(component_type, id_begin, size, indexer_begin);
    }

    void set_construction_complete() {
        assert(impl_ != nullptr);
        impl_->set_construction_complete();
    }
    void restore_components(ConstDataset const& update_data) {
        assert(impl_ != nullptr);
        impl_->restore_components(impl_->get_sequence_idx_map(update_data));
    }

    template <class CompType> void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(std::span<typename CompType::InputType const>{components});
    }
    template <class CompType> void add_component(std::span<typename CompType::InputType const> components) {
        assert(impl_ != nullptr);
        impl_->add_component<CompType>(components);
    }

    template <cache_type_c CacheType> void update_component(ConstDataset const& update_data, Idx pos = 0) {
        assert(impl_ != nullptr);
        impl_->update_component<CacheType>(update_data);
    }

    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::vector<OutputType>& target) const {
        output_result<CompType>(math_output, std::span<OutputType>{target});
    }
    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::span<OutputType> target) const {
        assert(impl_ != nullptr);
        impl_->output_result<CompType>(math_output, target);
    }

    template <symmetry_tag sym>
    MathOutput<std::vector<SolverOutput<sym>>> calculate_power_flow(Options const& options) {
        assert(impl_ != nullptr);
        return impl_->calculate_power_flow<sym>(options);
    }
    template <symmetry_tag sym>
    void calculate_power_flow(Options const& options, MutableDataset const& result_data, Idx pos = 0) {
        assert(impl_ != nullptr);
        return impl_->calculate_power_flow<sym>(options, result_data, pos);
    }
    template <symmetry_tag sym>
    BatchParameter calculate_power_flow(Options const& options, MutableDataset const& result_data,
                                        ConstDataset const& update_data) {
        assert(impl_ != nullptr);
        return impl_->calculate_power_flow<sym>(options, result_data, update_data);
    }
    template <symmetry_tag sym>
    MathOutput<std::vector<SolverOutput<sym>>> calculate_state_estimation(Options const& options) {
        assert(impl_ != nullptr);
        return impl_->calculate_state_estimation<sym>(options);
    }
    template <symmetry_tag sym>
    BatchParameter calculate_state_estimation(Options const& options, MutableDataset const& result_data,
                                              ConstDataset const& update_data) {

        assert(impl_ != nullptr);
        return impl_->calculate_state_estimation<sym>(options, result_data, update_data);
    }
    template <symmetry_tag sym>
    MathOutput<std::vector<ShortCircuitSolverOutput<sym>>> calculate_short_circuit(Options const& options) {

        assert(impl_ != nullptr);
        return impl_->calculate_short_circuit<sym>(options);
    }
    void calculate_short_circuit(Options const& options, MutableDataset const& result_data, Idx pos = 0);
    BatchParameter calculate_short_circuit(Options const& options, MutableDataset const& result_data,
                                           ConstDataset const& update_data) {
        assert(impl_ != nullptr);
        return impl_->calculate_short_circuit(options, result_data, update_data);
    }

  private:
    std::unique_ptr<Impl> impl_;
};

} // namespace power_grid_model
