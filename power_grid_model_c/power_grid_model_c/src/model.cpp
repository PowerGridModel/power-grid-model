// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/model.h"

#include "handle.hpp"
#include "options.hpp"

#include <power_grid_model/auxiliary/dataset_handler.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/main_model.hpp>

namespace {
using namespace power_grid_model;
} // namespace

// aliases main class
struct PGM_PowerGridModel : public MainModel {
    using MainModel::MainModel;
};

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                     PGM_ConstDataset const* input_dataset) {
    return call_with_catch(
        handle,
        [system_frequency, input_dataset] {
            return new PGM_PowerGridModel{system_frequency, input_dataset->export_dataset<const_dataset_t>(), 0};
        },
        PGM_regular_error);
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset) {
    call_with_catch(
        handle,
        [model, update_dataset] {
            model->update_component<MainModel::permanent_update_t>(update_dataset->export_dataset<const_dataset_t>());
        },
        PGM_regular_error);
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    return call_with_catch(
        handle, [model] { return new PGM_PowerGridModel{*model}; }, PGM_regular_error);
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    call_with_catch(
        handle, [model, component, size, ids, indexer] { model->get_indexer(component, ids, size, indexer); },
        PGM_regular_error);
}

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                   PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset) {
    PGM_clear_error(handle);
    // check dataset integrity
    if ((batch_dataset != nullptr) && (!batch_dataset->is_batch() || !output_dataset->is_batch())) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = "If batch_dataset is provided. Both batch_dataset and output_dataset should be a batch!\n";
        return;
    }

    Dataset const exported_output_dataset = output_dataset->export_dataset<mutable_dataset_t>();
    auto const exported_update_dataset =
        batch_dataset != nullptr ? batch_dataset->export_dataset<const_dataset_t>() : ConstDataset{};

    // call calculation
    try {
        auto const calculation_method = static_cast<CalculationMethod>(opt->calculation_method);
        switch (opt->calculation_type) {
        case PGM_power_flow:
            if (opt->symmetric != 0) {
                handle->batch_parameter = model->calculate_power_flow<symmetric_t>(
                    opt->err_tol, opt->max_iter, calculation_method, exported_output_dataset, exported_update_dataset,
                    opt->threading);
            } else {
                handle->batch_parameter = model->calculate_power_flow<asymmetric_t>(
                    opt->err_tol, opt->max_iter, calculation_method, exported_output_dataset, exported_update_dataset,
                    opt->threading);
            }
            break;
        case PGM_state_estimation:
            if (opt->symmetric != 0) {
                handle->batch_parameter = model->calculate_state_estimation<symmetric_t>(
                    opt->err_tol, opt->max_iter, calculation_method, exported_output_dataset, exported_update_dataset,
                    opt->threading);
            } else {
                handle->batch_parameter = model->calculate_state_estimation<asymmetric_t>(
                    opt->err_tol, opt->max_iter, calculation_method, exported_output_dataset, exported_update_dataset,
                    opt->threading);
            }
            break;
        case PGM_short_circuit: {
            auto const short_circuit_voltage_scaling =
                static_cast<ShortCircuitVoltageScaling>(opt->short_circuit_voltage_scaling);
            handle->batch_parameter =
                model->calculate_short_circuit(short_circuit_voltage_scaling, calculation_method,
                                               exported_output_dataset, exported_update_dataset, opt->threading);
            break;
        }
        default:
            throw MissingCaseForEnumError{"CalculationType", opt->calculation_type};
        }
    } catch (BatchCalculationError& e) {
        handle->err_code = PGM_batch_error;
        handle->err_msg = e.what();
        handle->failed_scenarios = e.failed_scenarios();
        handle->batch_errs = e.err_msgs();
    } catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    } catch (...) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = "Unknown error!\n";
    }
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) { delete model; }
