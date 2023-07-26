// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/model.h"

#include "handle.hpp"
#include "options.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/power_grid_model.hpp>

namespace {
using namespace power_grid_model;

using meta_data::RawDataConstPtr;
using meta_data::RawDataPtr;
}  // namespace

// aliases main class
struct PGM_PowerGridModel : public MainModel {
    using MainModel::MainModel;
};

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_components,
                                     char const** components, PGM_Idx const* component_sizes,
                                     RawDataConstPtr* input_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_components; ++i) {
        dataset[components[i]] = ConstDataPointer{input_data[i], component_sizes[i]};
    }
    try {
        return new PGM_PowerGridModel{system_frequency, dataset, 0};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Idx n_components, char const** components,
                      PGM_Idx const* component_sizes, RawDataConstPtr* update_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_components; ++i) {
        dataset[components[i]] = ConstDataPointer{update_data[i], component_sizes[i]};
    }
    try {
        model->update_component<MainModel::permanent_update_t>(dataset);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    try {
        return new PGM_PowerGridModel{*model};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    try {
        model->get_indexer(component, ids, size, indexer);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt, PGM_Idx n_output_components,
                   char const** output_components, RawDataPtr* output_data, PGM_Idx n_scenarios,
                   PGM_Idx n_update_components, char const** update_components,
                   PGM_Idx const* n_component_elements_per_scenario, PGM_Idx const** indptrs_per_component,
                   RawDataConstPtr* update_data) {
    PGM_clear_error(handle);
    std::map<std::string, Idx> const n_component = model->all_component_count();
    // prepare output dataset
    Dataset output_dataset{};
    // set n_output_batch to one for single calculation
    Idx const n_output_scenarios = std::max(Idx{1}, n_scenarios);
    for (Idx i = 0; i != n_output_components; ++i) {
        auto const found = n_component.find(output_components[i]);
        if (found != n_component.cend()) {
            output_dataset[output_components[i]] =
                MutableDataPointer{output_data[i], n_output_scenarios, found->second};
        }
    }
    // prepare update dataset
    ConstDataset update_dataset{};
    for (Idx i = 0; i != n_update_components; ++i) {
        if (n_component_elements_per_scenario[i] < 0) {
            // use indptr as sparse batch
            update_dataset[update_components[i]] =
                ConstDataPointer(update_data[i], indptrs_per_component[i], n_scenarios);
        }
        else {
            // use dense batch
            update_dataset[update_components[i]] =
                ConstDataPointer(update_data[i], n_scenarios, n_component_elements_per_scenario[i]);
        }
    }
    // call calculation
    try {
        auto const calculation_method = static_cast<CalculationMethod>(opt->calculation_method);
        switch (opt->calculation_type) {
            case PGM_power_flow:
                if (opt->symmetric) {
                    handle->batch_parameter =
                        model->calculate_power_flow<true>(opt->err_tol, opt->max_iter, calculation_method,
                                                          output_dataset, update_dataset, opt->threading);
                }
                else {
                    handle->batch_parameter =
                        model->calculate_power_flow<false>(opt->err_tol, opt->max_iter, calculation_method,
                                                           output_dataset, update_dataset, opt->threading);
                }
                break;
            case PGM_state_estimation:
                if (opt->symmetric) {
                    handle->batch_parameter =
                        model->calculate_state_estimation<true>(opt->err_tol, opt->max_iter, calculation_method,
                                                                output_dataset, update_dataset, opt->threading);
                }
                else {
                    handle->batch_parameter =
                        model->calculate_state_estimation<false>(opt->err_tol, opt->max_iter, calculation_method,
                                                                 output_dataset, update_dataset, opt->threading);
                }
                break;
            case PGM_short_circuit: {
                [[fallthrough]];  // TODO(mgovers) remove
                // constexpr double voltage_scaling_factor_c{1.1};
                // handle->batch_parameter = model->calculate_short_circuit(
                //     voltage_scaling_factor_c, calculation_method, output_dataset, update_dataset, opt->threading);
                // break;
            }
            default:
                throw MissingCaseForEnumError{"CalculationType", opt->calculation_type};
        }
    }
    catch (BatchCalculationError& e) {
        handle->err_code = PGM_batch_error;
        handle->err_msg = e.what();
        handle->failed_scenarios = e.failed_scenarios();
        handle->batch_errs = e.err_msgs();
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
    catch (...) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = "Unknown error!\n";
    }
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) {
    delete model;
}
