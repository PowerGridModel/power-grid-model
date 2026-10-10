// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "logger.hpp"
#include "math_solver.hpp"
#include "options.hpp" // NOLINT(misc-include-cleaner)

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/model.h"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <exception>
#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/main_model_fwd.hpp>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

struct PGM_StateOutput {
    std::vector<std::optional<power_grid_model::ModelStateOutput>> scenarios;
};

namespace {
using namespace power_grid_model;

using power_grid_model_c::call_with_catch;
using power_grid_model_c::cast_to_c;
using power_grid_model_c::cast_to_cpp;
using power_grid_model_c::get_math_solver_dispatcher;
using power_grid_model_c::safe_enum;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_ptr_maybe_nullptr;
using power_grid_model_c::safe_str_view;
} // namespace

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                     PGM_ConstDataset const* input_dataset) {
    return call_with_catch(handle, [handle, system_frequency, input_dataset] {
        // The handle's composite_logger is kept current by PGM_register/unregister_logger.
        // Bind the model to it so all models from this handle log to the same loggers.
        // The composite lives as long as the handle, satisfying MainModel's requirement that
        // its logger reference outlives it.
        return cast_to_c(
            new MainModel{// NOSONAR(S5025)
                          system_frequency, safe_ptr_get(cast_to_cpp(input_dataset)),
                          get_math_solver_dispatcher(), 0, safe_ptr_get(handle).composite_logger});
    });
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset) {
    call_with_catch(handle, [model, update_dataset] {
        safe_ptr_get(cast_to_cpp(model))
            .update_components<permanent_update_t>(safe_ptr_get(cast_to_cpp(update_dataset)));
    });
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    return call_with_catch(handle, [handle, model] {
        return cast_to_c(new MainModel{safe_ptr_get(cast_to_cpp(model)), safe_ptr_get(handle).composite_logger}); // NOSONAR(S5025)
    });
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    call_with_catch(handle, [model, component, size, ids, indexer] {
        safe_ptr_get(cast_to_cpp(model)).get_indexer(safe_str_view(component), safe_ptr(ids), size, safe_ptr(indexer));
    });
}

// helper functions
namespace {
void check_no_experimental_features_used(MainModel const& model, MainModel::Options const& opt,
                                         ConstDataset const* batch_dataset) {
    // optionally add experimental feature checks here
    using namespace std::string_literals;

    model.check_no_experimental_features_used(opt, batch_dataset);
}

void check_no_future_deprecations(MainModel const& model, MainModel::Options const& opt,
                                  ConstDataset const* batch_dataset) {
    // optionally add deprecation checks here
    using namespace std::string_literals;

    model.check_no_future_deprecations(opt, batch_dataset);
}

void check_experimental_support(Idx experimental_features, MainModel const& model, MainModel::Options const& opt,
                                ConstDataset const* batch_dataset) {
    switch (experimental_features) {
    case PGM_experimental_features_disabled:
        check_no_experimental_features_used(model, opt, batch_dataset);
        break;
    case PGM_experimental_features_enabled:
        check_no_future_deprecations(model, opt, batch_dataset);
        break;
    default:
        throw MissingCaseForEnumError{"calculate_impl", experimental_features};
    }
}

void check_calculate_valid_options(PGM_Options const& opt) {
    if (opt.tap_changing_strategy != PGM_tap_changing_strategy_disabled && opt.calculation_type != PGM_power_flow) {
        // illegal combination of options
        throw InvalidArguments{"PGM_calculate",
                               InvalidArguments::TypeValuePair{.name = "PGM_TapChangingStrategy",
                                                               .value = std::to_string(opt.tap_changing_strategy)}};
    }
}

constexpr auto get_calculation_type(PGM_Options const& opt) { return safe_enum<CalculationType>(opt.calculation_type); }

constexpr auto get_calculation_symmetry(PGM_Options const& opt) {
    switch (opt.symmetric) {
    case PGM_asymmetric:
        return CalculationSymmetry::asymmetric;
    case PGM_symmetric:
        return CalculationSymmetry::symmetric;
    default:
        throw MissingCaseForEnumError{"get_calculation_symmetry", opt.tap_changing_strategy};
    }
}

constexpr auto get_calculation_method(PGM_Options const& opt) {
    return safe_enum<CalculationMethod>(opt.calculation_method);
}

constexpr auto get_optimizer_type(PGM_Options const& opt) {
    using enum OptimizerType;

    switch (opt.tap_changing_strategy) {
    case PGM_tap_changing_strategy_disabled:
        return no_optimization;
    case PGM_tap_changing_strategy_any_valid_tap:
    case PGM_tap_changing_strategy_max_voltage_tap:
    case PGM_tap_changing_strategy_min_voltage_tap:
    case PGM_tap_changing_strategy_fast_any_tap:
        return automatic_tap_adjustment;
    default:
        throw MissingCaseForEnumError{"get_optimizer_type", opt.tap_changing_strategy};
    }
}

constexpr auto get_optimizer_strategy(PGM_Options const& opt) {
    using enum OptimizerStrategy;

    switch (opt.tap_changing_strategy) {
    case PGM_tap_changing_strategy_disabled:
    case PGM_tap_changing_strategy_any_valid_tap:
        return any;
    case PGM_tap_changing_strategy_max_voltage_tap:
        return global_maximum;
    case PGM_tap_changing_strategy_min_voltage_tap:
        return global_minimum;
    case PGM_tap_changing_strategy_fast_any_tap:
        return fast_any;
    default:
        throw MissingCaseForEnumError{"get_optimizer_strategy", opt.tap_changing_strategy};
    }
}

constexpr auto get_short_circuit_voltage_scaling(PGM_Options const& opt) {
    return safe_enum<ShortCircuitVoltageScaling>(opt.short_circuit_voltage_scaling);
}

constexpr auto extract_calculation_options(PGM_Options const& opt) {
    return MainModel::Options{.calculation_type = get_calculation_type(opt),
                              .calculation_symmetry = get_calculation_symmetry(opt),
                              .calculation_method = get_calculation_method(opt),
                              .optimizer_type = get_optimizer_type(opt),
                              .optimizer_strategy = get_optimizer_strategy(opt),
                              .err_tol = opt.err_tol,
                              .max_iter = opt.max_iter,
                              .threading = opt.threading,
                              .short_circuit_voltage_scaling = get_short_circuit_voltage_scaling(opt)};
}

class BadCalculationRequest : public PowerGridError {
  public:
    explicit BadCalculationRequest(std::string msg) : PowerGridError{std::move(msg)} {}
};

void calculate_single_batch_dimension_impl(MainModel& model, MainModel::Options const& options,
                                           MutableDataset const& output_dataset, ConstDataset const* batch_dataset,
                                           std::span<ModelStateRequest const> state_requests = {},
                                           std::span<std::optional<ModelStateOutput>> state_outputs = {}) {
    // check dataset integrity
    if ((batch_dataset != nullptr) && (!batch_dataset->is_batch() || !output_dataset.is_batch())) {
        throw BadCalculationRequest{
            "If batch_dataset is provided. Both batch_dataset and output_dataset should be a batch!\n"};
    }

    ConstDataset const& exported_update_dataset = batch_dataset != nullptr
                                                      ? safe_ptr_get(batch_dataset)
                                                      : ConstDataset{false, 1, "update", output_dataset.meta_data()};

    if (!state_requests.empty() || !state_outputs.empty()) {
        model.calculate_with_state(options, output_dataset, exported_update_dataset, state_requests, state_outputs);
    } else {
        model.calculate(options, output_dataset, exported_update_dataset);
    }
}

struct BatchExceptionHandler : public power_grid_model_c::DefaultExceptionHandler {
    void operator()(PGM_Handle& handle) const {
        std::exception_ptr const ex_ptr = std::current_exception();
        try {
            std::rethrow_exception(ex_ptr);
        } catch (BatchCalculationError const& ex) {
            handle_regular_error(handle, ex, PGM_batch_error);
            handle.failed_scenarios = ex.failed_scenarios();
            handle.batch_errs = ex.err_msgs();
        } catch (std::exception& ex) { // NOSONAR(S1181)
            handle_regular_error(handle, ex, PGM_regular_error);
        } catch (...) { // NOSONAR(S2738)
            handle_unkown_error(handle);
        }
    }
};

constexpr BatchExceptionHandler batch_exception_handler{};

template <typename T, std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, T>
void append_range(std::vector<T>& vec, R&& range) {
    std::ranges::move(std::forward<R>(range), std::back_inserter(vec));
}

class MDBatchExceptionHandler : public power_grid_model_c::DefaultExceptionHandler {
  public:
    MDBatchExceptionHandler(Idx scenario_offset, Idx stride_size)
        : scenario_offset_{scenario_offset}, stride_size_{stride_size} {
        assert(scenario_offset_ >= 0);
        assert(stride_size_ > 0);
    }

    void operator()(PGM_Handle& handle) const noexcept {
        using namespace std::string_literals;

        std::exception_ptr const ex_ptr = std::current_exception();
        try {
            std::rethrow_exception(ex_ptr);
        } catch (BatchCalculationError const& ex) {
            handle_regular_error(handle, ex, PGM_batch_error);
            append_range(handle.failed_scenarios,
                         ex.failed_scenarios() | std::views::transform([scenario_offset = scenario_offset_](Idx idx) {
                             return idx + scenario_offset;
                         }));

            append_range(handle.batch_errs, ex.err_msgs());
        } catch (std::exception const& ex) {
            handle_regular_error(handle, ex, PGM_batch_error);
            append_range(handle.failed_scenarios, IdxRange{stride_size_});
            append_range(handle.batch_errs, std::views::repeat(ex.what(), stride_size_));
        } catch (...) { // NOSONAR(S2738)
            handle_unkown_error(handle);
        }
    }

  private:
    Idx scenario_offset_{};
    Idx stride_size_{};
};

Idx get_batch_dimension(ConstDataset const* batch_dataset) {
    Idx dimension = 0;
    ConstDataset const* safe_batch_dataset = safe_ptr_maybe_nullptr(batch_dataset);
    while (safe_batch_dataset != nullptr) {
        ++dimension;
        safe_batch_dataset =
            safe_ptr_maybe_nullptr(safe_ptr_get(safe_batch_dataset).get_next_cartesian_product_dimension());
    }
    return dimension;
}

Idx get_stride_size(ConstDataset const* batch_dataset) {
    Idx size = 1;
    ConstDataset const* current =
        safe_ptr_maybe_nullptr(safe_ptr_get(batch_dataset).get_next_cartesian_product_dimension());
    while (current != nullptr) {
        auto const& safe_current = safe_ptr_get(current);
        size *= safe_current.batch_size();
        current = safe_current.get_next_cartesian_product_dimension();
    }
    return size;
}

// run calculation
void calculate_multi_dimensional_impl(MainModel& model, MainModel::Options const& options,
                                      MutableDataset const& output_dataset, ConstDataset const* batch_dataset,
                                      std::span<ModelStateRequest const> state_requests = {},
                                      std::span<std::optional<ModelStateOutput>> state_outputs = {}) {
    // for dimension < 2 (one-time or 1D batch), call implementation directly
    if (auto const batch_dimension = get_batch_dimension(batch_dataset); batch_dimension < 2) {
        calculate_single_batch_dimension_impl(model, options, output_dataset, batch_dataset, state_requests,
                               state_outputs);
        return;
    }

    auto const& safe_batch_dataset = safe_ptr_get(batch_dataset);

    // get stride size of the rest of dimensions
    Idx const first_batch_size = safe_batch_dataset.batch_size();
    Idx const stride_size = get_stride_size(batch_dataset);

    PGM_Handle local_handle{};

    // loop over the first dimension batch
    for (Idx i = 0; i < first_batch_size; ++i) {
        // a new handle
        call_with_catch(
            &local_handle,
            [&model, &options, &output_dataset, &safe_batch_dataset, &state_requests, state_outputs, i, stride_size] {
                // create sliced datasets for the rest of dimensions
                ConstDataset const single_update_dataset = safe_batch_dataset.get_individual_scenario(i);
                MutableDataset const sliced_output_dataset =
                    output_dataset.get_slice_scenario(i * stride_size, (i + 1) * stride_size);
                auto const sliced_state_requests = state_requests.empty()
                                                       ? state_requests
                                                       : state_requests.subspan(static_cast<size_t>(i * stride_size),
                                                                                static_cast<size_t>(stride_size));
                auto const sliced_state_outputs = state_outputs.empty()
                                                     ? state_outputs
                                                     : state_outputs.subspan(static_cast<size_t>(i * stride_size),
                                                                             static_cast<size_t>(stride_size));

                // create a model copy
                MainModel local_model{model};

                // apply the update
                local_model.update_components<permanent_update_t>(single_update_dataset);

                // recursive call
                calculate_multi_dimensional_impl(local_model, options, sliced_output_dataset,
                                                 safe_batch_dataset.get_next_cartesian_product_dimension(),
                                                 sliced_state_requests, sliced_state_outputs);
            },
            MDBatchExceptionHandler{i * stride_size, stride_size});
    }

    if (local_handle.err_code != PGM_no_error) {
        throw BatchCalculationError{std::move(local_handle.err_msg), std::move(local_handle.failed_scenarios),
                                    std::move(local_handle.batch_errs)};
    }
}

void calculate_impl(MainModel& model, PGM_Options const& options, MutableDataset const& output_dataset,
                    ConstDataset const* batch_dataset, std::span<ModelStateRequest const> state_requests = {},
                    std::span<std::optional<ModelStateOutput>> state_outputs = {}) {
    check_calculate_valid_options(options);
    auto const extracted_options = extract_calculation_options(options);

    check_experimental_support(options.experimental_features, model, extracted_options, batch_dataset);

    calculate_multi_dimensional_impl(model, extracted_options, output_dataset, batch_dataset, state_requests,
                                     state_outputs);
}

Idx get_total_scenarios(ConstDataset const* batch_dataset) {
    if (batch_dataset == nullptr) {
        return 1;
    }
    Idx total = 1;
    auto current = batch_dataset;
    while (current != nullptr) {
        auto const& dataset = safe_ptr_get(current);
        total *= dataset.batch_size();
        current = safe_ptr_maybe_nullptr(dataset.get_next_cartesian_product_dimension());
    }
    return total;
}

} // namespace

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                   PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
    call_with_catch(
        handle,
        [handle, model, opt, output_dataset, batch_dataset] {
            auto& cpp_model = safe_ptr_get(cast_to_cpp(model));
            // Log to the handle passed to this call, not the handle the model was created with.
            cpp_model.set_logger(safe_ptr_get(handle).composite_logger);
            calculate_impl(cpp_model, safe_ptr_get(opt), safe_ptr_get(cast_to_cpp(output_dataset)),
                           safe_ptr_maybe_nullptr(cast_to_cpp(batch_dataset)));
        },
        batch_exception_handler);
}

void PGM_calculate_with_state(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                              PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset,
                              PGM_StateOutputRequest const* requests, PGM_Idx request_count,
                              PGM_StateOutput** state_output) {
    call_with_catch(
        handle,
        [handle, model, opt, output_dataset, batch_dataset, requests, request_count, state_output] {
            if (get_calculation_type(safe_ptr_get(opt)) != CalculationType::power_flow) {
                throw BadCalculationRequest{"Model state output is only available for power-flow calculations.\n"};
            }
            auto const* cpp_batch_dataset = safe_ptr_maybe_nullptr(cast_to_cpp(batch_dataset));
            Idx const n_scenarios = get_total_scenarios(cpp_batch_dataset);
            if (request_count != n_scenarios) {
                throw DatasetError{"Model state request count must match the calculation scenario count.\n"};
            }
            if (request_count > 0 && requests == nullptr) {
                throw DatasetError{"Model state requests cannot be null when request_count is positive.\n"};
            }

            std::vector<ModelStateRequest> converted_requests;
            converted_requests.reserve(static_cast<size_t>(request_count));
            for (Idx idx = 0; idx < request_count; ++idx) {
                converted_requests.push_back({.y_bus = requests[idx].y_bus != 0,
                                              .jacobian = requests[idx].jacobian != 0});
            }
            auto result = std::make_unique<PGM_StateOutput>();
            result->scenarios.resize(static_cast<size_t>(n_scenarios));

            auto& cpp_model = safe_ptr_get(cast_to_cpp(model));
            cpp_model.set_logger(safe_ptr_get(handle).composite_logger);
            calculate_impl(cpp_model, safe_ptr_get(opt), safe_ptr_get(cast_to_cpp(output_dataset)),
                           cpp_batch_dataset, converted_requests, result->scenarios);
            safe_ptr_get(state_output) = result.release();
        },
        batch_exception_handler);
}

PGM_Idx PGM_state_output_scenario_count(PGM_Handle* handle, PGM_StateOutput const* state_output) {
    return call_with_catch(handle, [state_output] {
        return static_cast<PGM_Idx>(safe_ptr_get(state_output).scenarios.size());
    });
}

void PGM_state_output_get_scenario(PGM_Handle* handle, PGM_StateOutput const* state_output, PGM_Idx scenario_idx,
                                   PGM_StateScenarioView* view) {
    call_with_catch(handle, [state_output, scenario_idx, view] {
        auto const& state_result = safe_ptr_get(state_output).scenarios.at(static_cast<size_t>(scenario_idx));
        auto& output = safe_ptr_get(view);
        output = {};
        if (!state_result.has_value()) {
            return;
        }
        auto const& state = *state_result;
        output.has_state = 1;
        output.y_bus_requested = state.y_bus_requested;
        output.jacobian_requested = state.jacobian_requested;
        output.n_groups = static_cast<PGM_Idx>(state.groups.size());
        output.n_input_nodes = static_cast<PGM_Idx>(state.input_node_id.size());
        output.input_node_group = state.input_node_group.data();
        output.input_node_bus = state.input_node_bus.data();
        output.input_node_id = state.input_node_id.data();
    });
}

void PGM_state_output_get_group(PGM_Handle* handle, PGM_StateOutput const* state_output, PGM_Idx scenario_idx,
                                PGM_Idx group_idx, PGM_StateGroupView* view) {
    call_with_catch(handle, [state_output, scenario_idx, group_idx, view] {
        auto const& scenario = safe_ptr_get(state_output).scenarios.at(static_cast<size_t>(scenario_idx));
        if (!scenario.has_value()) {
            throw DatasetError{"No model state was requested for this scenario.\n"};
        }
        auto const& group = scenario->groups.at(static_cast<size_t>(group_idx));
        auto& output = safe_ptr_get(view);
        output = {};
        auto const& mapping = group.mapping;
        output.group = mapping.group;
        output.n_bus = mapping.n_bus;
        output.is_symmetric = mapping.is_symmetric;
        output.n_user_node_refs = static_cast<PGM_Idx>(mapping.bus_user_sequence.size());
        output.bus_user_indptr = mapping.bus_user_indptr.data();
        output.bus_user_sequence = mapping.bus_user_sequence.data();
        output.bus_user_id = mapping.bus_user_id.data();
        output.bus_kind = reinterpret_cast<int8_t const*>(mapping.bus_kind.data());
        output.origin_branch3_id = mapping.origin_branch3_id.data();
        if (group.y_bus.has_value()) {
            auto const& y_bus = *group.y_bus;
            output.has_y_bus = 1;
            output.y_bus_nnz = static_cast<PGM_Idx>(y_bus.col_indices.size());
            output.y_bus_row_indptr = y_bus.row_indptr.data();
            output.y_bus_col_indices = y_bus.col_indices.data();
            output.admittance_real = y_bus.admittance_real.data();
            output.admittance_imag = y_bus.admittance_imag.data();
            output.n_admittance_values = static_cast<PGM_Idx>(y_bus.admittance_real.size());
        }
        if (group.jacobian.has_value()) {
            auto const& jacobian = *group.jacobian;
            output.has_jacobian = 1;
            output.jacobian_nnz = static_cast<PGM_Idx>(jacobian.col_indices_lu.size());
            output.jacobian_row_indptr = jacobian.row_indptr_lu.data();
            output.jacobian_col_indices = jacobian.col_indices_lu.data();
            output.jacobian_h = jacobian.blocks[0].data();
            output.jacobian_n = jacobian.blocks[1].data();
            output.jacobian_m = jacobian.blocks[2].data();
            output.jacobian_l = jacobian.blocks[3].data();
            output.n_jacobian_values = static_cast<PGM_Idx>(jacobian.blocks[0].size());
        }
    });
}

PGM_Idx PGM_state_output_jacobian_history_count(PGM_Handle* handle, PGM_StateOutput const* state_output,
                                               PGM_Idx scenario_idx, PGM_Idx group_idx) {
    return call_with_catch(handle, [state_output, scenario_idx, group_idx] {
        auto const& scenario = safe_ptr_get(state_output).scenarios.at(static_cast<size_t>(scenario_idx));
        if (!scenario.has_value()) {
            throw DatasetError{"No model state was requested for this scenario.\n"};
        }
        return static_cast<PGM_Idx>(scenario->groups.at(static_cast<size_t>(group_idx)).jacobian_history.size());
    });
}

void PGM_state_output_get_jacobian_history(PGM_Handle* handle, PGM_StateOutput const* state_output,
                                          PGM_Idx scenario_idx, PGM_Idx group_idx, PGM_Idx iteration_idx,
                                          PGM_StateJacobianView* view) {
    call_with_catch(handle, [state_output, scenario_idx, group_idx, iteration_idx, view] {
        auto const& scenario = safe_ptr_get(state_output).scenarios.at(static_cast<size_t>(scenario_idx));
        if (!scenario.has_value()) {
            throw DatasetError{"No model state was requested for this scenario.\n"};
        }
        auto const& group = scenario->groups.at(static_cast<size_t>(group_idx));
        auto const& jacobian = group.jacobian_history.at(static_cast<size_t>(iteration_idx));
        auto& output = safe_ptr_get(view);
        output = {};
        output.iteration = jacobian.iteration;
        output.n_bus = group.mapping.n_bus;
        output.jacobian_nnz = static_cast<PGM_Idx>(jacobian.col_indices_lu.size());
        output.jacobian_row_indptr = jacobian.row_indptr_lu.data();
        output.jacobian_col_indices = jacobian.col_indices_lu.data();
        output.jacobian_h = jacobian.blocks[0].data();
        output.jacobian_n = jacobian.blocks[1].data();
        output.jacobian_m = jacobian.blocks[2].data();
        output.jacobian_l = jacobian.blocks[3].data();
        output.n_jacobian_values = static_cast<PGM_Idx>(jacobian.blocks[0].size());
    });
}

void PGM_destroy_state_output(PGM_StateOutput* state_output) {
    delete state_output;
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) {
    delete cast_to_cpp(model); // NOSONAR(S5025)
}
