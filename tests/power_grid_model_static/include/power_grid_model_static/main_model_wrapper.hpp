// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model_fwd.hpp>

#include <map>
#include <memory>

namespace power_grid_model::pgm_static {

// main model wrapper class

class MainModelWrapper {
  private:
    class Impl;

  public:
    using Options = MainModelOptions;

    explicit MainModelWrapper(double system_frequency, ConstDataset const& input_data, Idx pos = 0);
    explicit MainModelWrapper(double system_frequency, meta_data::MetaData const& meta_data);

    MainModelWrapper(MainModelWrapper const& /* other */);
    MainModelWrapper(MainModelWrapper&& /* other */);
    MainModelWrapper& operator=(MainModelWrapper& /* other */);
    MainModelWrapper& operator=(MainModelWrapper&& /* other */);
    ~MainModelWrapper();

    static bool is_update_independent(ConstDataset const& update_data);

    std::map<std::string, Idx> all_component_count() const;
    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const;

    void set_construction_complete();
    void restore_components(ConstDataset const& update_data);

    // template forward declarations.
    // If you get linking errors, it is likely a missing template instantiations in main_model_wrapper.cpp
    template <class CompType> void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(std::span<typename CompType::InputType const>{components});
    }
    template <class CompType> void add_component(std::span<typename CompType::InputType const> components) {
        ConstDataset dataset{false, 1, "input", meta_data::meta_data_gen::meta_data};
        dataset.add_buffer(CompType::name, components.size(), components.size(), nullptr, components.data());
        add_components(dataset);
    }

    template <cache_type_c CacheType> void update_component(ConstDataset const& update_data, Idx pos = 0);

    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::vector<OutputType>& target) const {
        output_result<CompType>(math_output, std::span<OutputType>{target});
    }
    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_result(MathOutputType const& math_output, std::span<OutputType> target) const {
        if constexpr (IsInList<CompType, AllComponents>::value) {
            std::string const dataset_name = [] {
                using namespace std::string_literals;
                if constexpr (std::same_as<OutputType, typename CompType::template OutputType<symmetric_t>>) {
                    return "sym_output"s;
                }
                if constexpr (std::same_as<OutputType, typename CompType::template OutputType<asymmetric_t>>) {
                    return "asym_output"s;
                }
                if constexpr (std::same_as<OutputType, typename CompType::ShortCircuitOutputType>) {
                    return "sc_output"s;
                }
                throw UnreachableHit{"MainModelWrapper::output_result", "Unknown output type"};
            }();
            MutableDataset dataset{true, 1, dataset_name, meta_data::meta_data_gen::meta_data};
            dataset.add_buffer(CompType::name, target.size(), target.size(), nullptr, target.data());
            output_result(math_output, dataset);
        } else {
            output_extra_retreivable_result<CompType>(math_output, target);
        }
    }

    template <symmetry_tag sym> MathOutput<std::vector<SolverOutput<sym>>> calculate_power_flow(Options const& options);
    template <symmetry_tag sym>
    void calculate_power_flow(Options const& options, MutableDataset const& result_data, Idx pos = 0);
    template <symmetry_tag sym>
    BatchParameter calculate_power_flow(Options const& options, MutableDataset const& result_data,
                                        ConstDataset const& update_data);
    template <symmetry_tag sym>
    MathOutput<std::vector<SolverOutput<sym>>> calculate_state_estimation(Options const& options);
    template <symmetry_tag sym>
    BatchParameter calculate_state_estimation(Options const& options, MutableDataset const& result_data,
                                              ConstDataset const& update_data);
    template <symmetry_tag sym>
    MathOutput<std::vector<ShortCircuitSolverOutput<sym>>> calculate_short_circuit(Options const& options);
    void calculate_short_circuit(Options const& options, MutableDataset const& result_data, Idx pos = 0);
    BatchParameter calculate_short_circuit(Options const& options, MutableDataset const& result_data,
                                           ConstDataset const& update_data);

  private:
    std::unique_ptr<Impl> impl_;

    void add_components(ConstDataset const& input_data, Idx pos = 0);

    template <solver_output_type SolverOutputType>
    void output_result(MathOutput<std::vector<SolverOutputType>> const& math_output, MutableDataset const& result_data,
                       Idx pos = 0) const;
    template <typename CompType, typename MathOutputType, typename OutputType>
    void output_extra_retreivable_result(MathOutputType const& math_output, std::span<OutputType> target) const;
};

} // namespace power_grid_model::pgm_static

namespace pgm_static = power_grid_model::pgm_static;
