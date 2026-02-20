// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_functions.hpp"

#include <power_grid_model_c/basics.h>
#include <power_grid_model_cpp/basics.hpp>
#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/model.hpp>
#include <power_grid_model_cpp/options.hpp>
#include <power_grid_model_cpp/serialization.hpp>

#include <cassert>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

namespace power_grid_model_cpp {

struct BatchDatasets {
    explicit BatchDatasets(ClIOptions const& cli_options) {
        if (!cli_options.is_batch) {
            return;
        }
        for (auto const& [batch_file, format] :
             std::views::zip(cli_options.batch_update_file, cli_options.batch_update_serialization_format)) {
            datasets.emplace_back(load_dataset(batch_file, format));
            dataset_consts.emplace_back(datasets.back().dataset);
        }
        assert(!datasets.empty());
        for (auto it = dataset_consts.begin(); it != dataset_consts.end() - 1; ++it) {
            it->set_next_cartesian_product_dimension(*(it + 1));
        }
        batch_size = std::transform_reduce(datasets.begin(), datasets.end(), Idx{1}, std::multiplies{},
                                           [](OwningDataset const& ds) { return ds.dataset.get_info().batch_size(); });
    }

    DatasetConst const& head() const {
        assert(!dataset_consts.empty());
        return dataset_consts.front();
    }

    Idx batch_size{1};
    std::vector<OwningDataset> datasets;
    std::vector<DatasetConst> dataset_consts;
};

void pgm_calculation(ClIOptions const& cli_options) {
    // Load input dataset
    OwningDataset const input_dataset = load_dataset(cli_options.input_file, cli_options.input_serialization_format);

    // Apply batch updates if provided
    BatchDatasets const batch_datasets{cli_options};

    // create result dataset
    // NOLINTNEXTLINE(misc-const-correctness)
    OwningDataset result_dataset{input_dataset, cli_options.output_dataset_name, cli_options.is_batch,
                                 batch_datasets.batch_size, cli_options.output_component_attribute_filters};
    // create model
    Model model{cli_options.system_frequency, input_dataset.dataset};
    // create calculation options
    Options calc_options{};
    calc_options.set_calculation_type(cli_options.calculation_type);
    calc_options.set_calculation_method(cli_options.calculation_method);
    calc_options.set_symmetric(static_cast<Idx>(cli_options.symmetric_calculation));
    calc_options.set_err_tol(cli_options.error_tolerance);
    calc_options.set_max_iter(cli_options.max_iterations);
    calc_options.set_threading(cli_options.threading);
    calc_options.set_short_circuit_voltage_scaling(cli_options.short_circuit_voltage_scaling);
    calc_options.set_tap_changing_strategy(cli_options.tap_changing_strategy);

    // perform calculation
    if (cli_options.is_batch) {
        model.calculate(calc_options, result_dataset.dataset, batch_datasets.head());
    } else {
        model.calculate(calc_options, result_dataset.dataset);
    }

    // Save output dataset
    save_dataset(cli_options.output_file, result_dataset.dataset,
                 cli_options.use_msgpack_output_serialization ? PGM_msgpack : PGM_json,
                 cli_options.use_compact_serialization ? 1 : 0, cli_options.output_json_indent);
}

} // namespace power_grid_model_cpp
