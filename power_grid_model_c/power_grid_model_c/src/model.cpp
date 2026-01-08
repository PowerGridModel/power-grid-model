// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/model.h"

#include "handle.hpp"
#include "input_sanitization.hpp"
#include "math_solver.hpp"
#include "options.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/main_model.hpp>

namespace {
using namespace power_grid_model;

using power_grid_model_c::call_with_catch;
using power_grid_model_c::get_math_solver_dispatcher;
using power_grid_model_c::safe_enum;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_ptr_maybe_nullptr;
using power_grid_model_c::safe_str_view;
} // namespace

// aliases main class
struct PGM_PowerGridModel : public MainModel {
    using MainModel::MainModel;
};

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                     PGM_ConstDataset const* input_dataset) {
    return call_with_catch(handle, [system_frequency, input_dataset] {
        return new PGM_PowerGridModel{// NOSONAR(S5025)
                                      system_frequency, safe_ptr_get(input_dataset), get_math_solver_dispatcher(), 0};
    });
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset) {
    call_with_catch(handle, [model, update_dataset] {
        model->update_components<permanent_update_t>(safe_ptr_get(update_dataset));
    });
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    return call_with_catch(handle, [model] {
        return new PGM_PowerGridModel{safe_ptr_get(model)}; // NOSONAR(S5025)
    });
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    call_with_catch(handle, [model, component, size, ids, indexer] {
        safe_ptr_get(model).get_indexer(safe_str_view(component), safe_ptr(ids), size, safe_ptr(indexer));
    });
}

namespace {
void check_no_experimental_features_used(MainModel const& model, MainModel::Options const& opt) {
    // optionally add experimental feature checks here
    using namespace std::string_literals;

    model.check_no_experimental_features_used(opt);
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

void calculate_impl(PGM_PowerGridModel& model, PGM_Options const& opt, MutableDataset const& output_dataset,
                    ConstDataset const* batch_dataset) {
    // check dataset integrity
    if ((batch_dataset != nullptr) && (!batch_dataset->is_batch() || !output_dataset.is_batch())) {
        throw BadCalculationRequest{
            "If batch_dataset is provided. Both batch_dataset and output_dataset should be a batch!\n"};
    }

    ConstDataset const& exported_update_dataset =
        batch_dataset != nullptr ? safe_ptr_get(batch_dataset)
                                 : PGM_ConstDataset{false, 1, "update", output_dataset.meta_data()};

    check_calculate_valid_options(opt);
    auto const options = extract_calculation_options(opt);

    if (opt.experimental_features == PGM_experimental_features_disabled) {
        check_no_experimental_features_used(model, options);
    }

    model.calculate(options, output_dataset, exported_update_dataset);
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
} // namespace

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                   PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
    call_with_catch(
        handle,
        [model, opt, output_dataset, batch_dataset] {
            calculate_impl(safe_ptr_get(model), safe_ptr_get(opt), safe_ptr_get(output_dataset),
                           safe_ptr_maybe_nullptr(batch_dataset));
        },
        batch_exception_handler);
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) {
    delete model; // NOSONAR(S5025)
}
