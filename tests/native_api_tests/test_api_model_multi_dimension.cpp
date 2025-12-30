// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "load_dataset.hpp"

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <array>
#include <complex>
#include <numbers>
#include <string>
#include <vector>

namespace power_grid_model_cpp {
namespace {
using namespace std::string_literals;
using power_grid_model_cpp_test::load_dataset;
using std::numbers::sqrt3;

// input
auto const complete_state_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "sym_load": [
      {"id": 2, "node": 0, "status": 1, "type": 0, "p_specified": 0, "q_specified": 0}
    ],
    "source": [
      {"id": 1, "node": 0, "status": 1, "u_ref": 1, "sk": 1e20}
    ],
    "node": [
      {"id": 0, "u_rated": 10e3}
    ]
  }
})json"s;

} // namespace

TEST_CASE("API Model Multi-Dimension") {
    // model
    auto const owning_input_dataset = load_dataset(complete_state_json);
    auto const& input_dataset = owning_input_dataset.dataset;
    Model model{50.0, input_dataset};

    // 3-D batch update
    double const u_rated = 10e3;
    std::vector<double> const u_ref{0.9, 1.0, 1.1};
    std::vector<double> const p_specified{1e6, 2e6, 3e6, 4e6};
    std::vector<double> const q_specified{0.1e6, 0.2e6, 0.3e6, 0.4e6, 0.5e6};
    Idx const size_u_ref = std::ssize(u_ref);
    Idx const size_p_specified = std::ssize(p_specified);
    Idx const size_q_specified = std::ssize(q_specified);
    Idx const total_batch_size = size_u_ref * size_p_specified * size_q_specified;

    // calculate source current manually
    std::vector<double> i_source_ref(total_batch_size);
    for (Idx i = 0; i < size_u_ref; ++i) {
        for (Idx j = 0; j < size_p_specified; ++j) {
            for (Idx k = 0; k < size_q_specified; ++k) {
                Idx const index = i * size_p_specified * size_q_specified + j * size_q_specified + k;
                double const s = std::abs(std::complex<double>{p_specified[j], q_specified[k]});
                i_source_ref[index] = s / (sqrt3 * u_rated * u_ref[i]);
            }
        }
    }

    // construct batch update dataset
    DatasetConst batch_u_ref{"update", true, size_u_ref};
    batch_u_ref.add_buffer("source", 1, size_u_ref, nullptr, nullptr);
    batch_u_ref.add_attribute_buffer("source", "u_ref", u_ref.data());
    DatasetConst batch_p_specified{"update", true, size_p_specified};
    batch_p_specified.add_buffer("sym_load", 1, size_p_specified, nullptr, nullptr);
    batch_p_specified.add_attribute_buffer("sym_load", "p_specified", p_specified.data());
    DatasetConst batch_q_specified{"update", true, size_q_specified};
    batch_q_specified.add_buffer("sym_load", 1, size_q_specified, nullptr, nullptr);
    batch_q_specified.add_attribute_buffer("sym_load", "q_specified", q_specified.data());
    batch_u_ref.set_next_cartesian_product_dimension(batch_p_specified);
    batch_p_specified.set_next_cartesian_product_dimension(batch_q_specified);

    SUBCASE("Correct cartesian product usage") {
        // output dataset
        std::vector<double> i_source_result(total_batch_size);
        DatasetMutable batch_output_dataset{"sym_output", true, total_batch_size};
        batch_output_dataset.add_buffer("source", 1, total_batch_size, nullptr, nullptr);
        batch_output_dataset.add_attribute_buffer("source", "i", i_source_result.data());

        // options
        Options const options{};

        // calculate
        model.calculate(options, batch_output_dataset, batch_u_ref);

        // check results
        for (Idx idx = 0; idx < total_batch_size; ++idx) {
            CHECK(i_source_result[idx] == doctest::Approx(i_source_ref[idx]));
        }
    }
    SUBCASE("Linked list item referring to itself is not allowed") {
        CHECK_THROWS_AS(batch_u_ref.set_next_cartesian_product_dimension(batch_u_ref), PowerGridRegularError);
    }
    SUBCASE("Cyclic linked list is not allowed") {
        CHECK_THROWS_AS(batch_q_specified.set_next_cartesian_product_dimension(batch_u_ref), PowerGridRegularError);
    }
}

} // namespace power_grid_model_cpp
