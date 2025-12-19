// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_functions.hpp"

#include <optional>

namespace power_grid_model_cpp {

void pgm_calculation(ClIOptions const& cli_options) {
    // Load input dataset
    OwningDataset input_dataset = load_dataset(cli_options.input_file, cli_options.input_serialization_format);

    // Apply batch updates if provided
    std::optional<OwningDataset> batch_update_dataset{std::nullopt};
    if (cli_options.is_batch) {
        batch_update_dataset =
            load_dataset(cli_options.batch_update_file, cli_options.batch_update_serialization_format);
    }
    Idx const batch_size = cli_options.is_batch ? batch_update_dataset->dataset.get_info().batch_size() : 1;

    // create result dataset
    OwningDataset result_dataset{input_dataset, cli_options.output_dataset_name, cli_options.is_batch, batch_size,
                                 cli_options.output_component_attribute_filters};
    // create model
    Model model{cli_options.system_frequency, input_dataset.dataset};
    // create calculation options
    Options calc_options{};
    calc_options.set_calculation_type(cli_options.calculation_type);
    calc_options.set_calculation_method(cli_options.calculation_method);
    calc_options.set_symmetric(cli_options.symmetric_calculation);
    calc_options.set_err_tol(cli_options.error_tolerance);
    calc_options.set_max_iter(cli_options.max_iterations);
    calc_options.set_threading(cli_options.threading);
    calc_options.set_short_circuit_voltage_scaling(cli_options.short_circuit_voltage_scaling);
    calc_options.set_tap_changing_strategy(cli_options.tap_changing_strategy);

    // perform calculation
    if (cli_options.is_batch) {
        model.calculate(calc_options, result_dataset.dataset, batch_update_dataset->dataset);
    } else {
        model.calculate(calc_options, result_dataset.dataset);
    }

    // Save output dataset
    save_dataset(cli_options.output_file, result_dataset.dataset,
                 cli_options.use_msgpack_output_serialization ? PGM_msgpack : PGM_json,
                 cli_options.use_compact_serialization ? 1 : 0, cli_options.output_json_indent);
}

} // namespace power_grid_model_cpp
