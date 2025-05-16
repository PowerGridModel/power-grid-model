// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "load_dataset.hpp"

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <exception>
#include <limits>
#include <map>
#include <string>
#include <utility>

/*
Testing network

source_1 -- node_0 |---- line_5 ----| node_4
              |    |---- line_6 ----|
              |
              load_2

source_1: 1.0 p.u., 100.0 V, internal_impedance(j10.0 ohm, sk=1000.0 VA, rx_ratio=0.0)
node_0: 100.0 V
oad_2: const_i, -j5.0A, 0.0 W, 500.0 var

update_0 voltage calculation:
    u_ref = 0.5 p.u. (50.0 V)
    q_specified = 100 var (-j1.0A)
u0 = 50.0 V - (j10.0 ohm * -j1.0 A) = 40.0 V

update_1 voltage calculation:
    q_specified = 300 var (-j3.0A)
u0 = 100.0 V - (j10.0 ohm * -j3.0 A) = 70.0 V

Dataset created with the following buffers:

|                | Row Based | Columnar | Dense | Sparse |
|----------------|-----------|----------|-------|--------|
| input data     |           |          |       |        |
| - node         |           |    Y     |       |        |
| - line         |           |    Y     |       |        |
| - load         |     Y     |          |       |        |
| - source       |     Y     |          |       |        |
| single update  |           |          |       |        |
| - line         |           |    Y     |       |        |
| - load         |     Y     |          |       |        |
| - source       |     Y     |          |       |        |
| batch update   |           |          |       |        |
| - line         |           |    Y     |   Y   |        |
| - load         |     Y     |          |   Y   |        |
| - source       |     Y     |          |       |    Y   |

*/

namespace power_grid_model_cpp {
namespace {
using namespace std::string_literals;
using power_grid_model_cpp_test::load_dataset;

enum class MeasuredTerminalType : IntS {
    branch_from = 0,
    branch_to = 1,
    source = 2,
    shunt = 3,
    load = 4,
    generator = 5,
    branch3_1 = 6,
    branch3_2 = 7,
    branch3_3 = 8,
    node = 9
};

void check_exception(PowerGridError const& e, PGM_ErrorCode const& reference_error,
                     std::string_view reference_err_msg) {
    CHECK(e.error_code() == reference_error);
    std::string const err_msg{e.what()};
    REQUIRE(std::in_range<doctest::String::size_type>(reference_err_msg.size()));
    doctest::String const ref_err_msg{reference_err_msg.data(),
                                      static_cast<doctest::String::size_type>(reference_err_msg.size())};
    REQUIRE(err_msg.c_str() == doctest::Contains(ref_err_msg));
}

template <typename Func, class... Args>
void check_throws_with(Func&& func, PGM_ErrorCode const& reference_error, std::string_view reference_err_msg,
                       Args&&... args) {
    try {
        std::forward<Func>(func)(std::forward<Args>(args)...);
        FAIL("Expected error not thrown.");
    } catch (PowerGridError const& e) {
        check_exception(e, reference_error, reference_err_msg);
    }
}

auto const complete_state_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "sym_load": [
      {"id": 2, "node": 0, "status": 1, "type": 2, "p_specified": 0, "q_specified": 500}
    ],
    "source": [
      {"id": 1, "node": 0, "status": 1, "u_ref": 1, "sk": 1000, "rx_ratio": 0}
    ],
    "node": [
      {"id": 0, "u_rated": 100},
      {"id": 4, "u_rated": 100}
    ],
    "line": [
      {"id": 5, "from_node": 0, "to_node": 4, "from_status": 0, "to_status": 1},
      {"id": 6, "from_node": 4, "to_node": 0, "from_status": 0, "to_status": 0}
    ]
  }
})json"s;

auto const single_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": false,
  "attributes": {},
  "data": {
    "source": [
      {"id": 1, "u_ref": 0.5}
    ],
    "sym_load": [
      {"id": 2, "q_specified": 100}
    ],
    "line": [
      {"id": 5, "from_status": 0, "to_status": 1},
      {"id": 6, "from_status": 0, "to_status": 0}
    ]
  }
})json"s;

auto const batch_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"id": 1, "u_ref": 0.5}
      ],
      "sym_load": [
        {"id": 2, "q_specified": 100}
      ],
      "line": [
        {"id": 5, "from_status": 0, "to_status": 1},
        {"id": 6, "from_status": 0, "to_status": 0}
      ]
    },
    {
      "sym_load": [
        {"id": 2, "q_specified": 300}
      ],
      "line": [
        {"id": 5, "from_status": 0, "to_status": 0},
        {"id": 6, "from_status": 0, "to_status": 0}
      ]
    }
  ]
})json"s;
} // namespace

TEST_CASE("API Model") {
    using namespace std::string_literals;

    Options options{};

    auto const owning_input_dataset = load_dataset(complete_state_json);
    auto const& input_dataset = owning_input_dataset.dataset;

    auto const single_owning_update_dataset = load_dataset(single_update_json);
    auto const& single_update_dataset = single_owning_update_dataset.dataset;

    auto const batch_owning_update_dataset = load_dataset(batch_update_json);
    auto const& batch_update_dataset = batch_owning_update_dataset.dataset;

    // output data
    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable single_output_dataset{"sym_output", false, 1};
    single_output_dataset.add_buffer("node", 2, 2, nullptr, node_output);
    Buffer node_batch_output{PGM_def_sym_output_node, 4};
    node_batch_output.set_nan();
    DatasetMutable batch_output_dataset{"sym_output", true, 2};
    batch_output_dataset.add_buffer("node", 2, 4, nullptr, node_batch_output);

    std::vector<ID> node_result_id(2);
    std::vector<int8_t> node_result_energized(2);
    std::vector<double> node_result_u(2);
    std::vector<double> node_result_u_pu(2);
    std::vector<double> node_result_u_angle(2);
    std::vector<ID> batch_node_result_id(4);
    std::vector<int8_t> batch_node_result_energized(4);
    std::vector<double> batch_node_result_u(4);
    std::vector<double> batch_node_result_u_pu(4);
    std::vector<double> batch_node_result_u_angle(4);

    // create model
    Model model{50.0, input_dataset};

    SUBCASE("Test Movability") {
        Model model_dummy{std::move(model)};
        model = std::move(model_dummy);
    }
    SUBCASE("Test Copyability") {
        Model const model_dummy{std::move(model)};
        model = Model{model_dummy};
    }
    SUBCASE("Single power flow") {
        // Common checker used for all single power flow test subcases
        auto check_common_node_results = [&]() {
            node_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
            node_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), 0, -1);
            node_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), 0, 1, -1);

            CHECK(node_result_id[0] == 0);
            CHECK(node_result_energized[0] == 1);
            CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
            CHECK(node_result_id[1] == 4);
            CHECK(node_result_energized[1] == 0);
            CHECK(node_result_u[1] == doctest::Approx(0.0));
            CHECK(node_result_u_pu[1] == doctest::Approx(0.0));
            CHECK(node_result_u_angle[1] == doctest::Approx(0.0));
        };

        SUBCASE("Single power flow") {
            model.calculate(options, single_output_dataset);
            node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), -1);
            node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
            CHECK(node_result_u[0] == doctest::Approx(50.0));
            CHECK(node_result_u_pu[0] == doctest::Approx(0.5));
            check_common_node_results();
        }

        SUBCASE("Permanent update") {
            model.update(single_update_dataset);
            model.calculate(options, single_output_dataset);
            node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), -1);
            node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
            CHECK(node_result_u[0] == doctest::Approx(40.0));
            CHECK(node_result_u_pu[0] == doctest::Approx(0.4));
            check_common_node_results();
        }

        SUBCASE("Copy model") {
            auto model_copy = Model{model};
            model_copy.calculate(options, single_output_dataset);
            node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), -1);
            node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
            CHECK(node_result_u[0] == doctest::Approx(50.0));
            CHECK(node_result_u_pu[0] == doctest::Approx(0.5));
            check_common_node_results();
        }
    }

    SUBCASE("Test get indexer") {
        std::vector<ID> const node_id_2{1, 2, 3};
        std::vector<double> const node_u_rated_2{10.0e3, 10.0e3, 10.0e3};

        DatasetConst input_dataset_2{"input", false, 1};
        input_dataset_2.add_buffer("node", std::ssize(node_id_2), std::ssize(node_id_2), nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("node", "id", node_id_2.data());
        input_dataset_2.add_attribute_buffer("node", "u_rated", node_u_rated_2.data());

        auto model_2 = Model{50.0, input_dataset_2};

        SUBCASE("Good weather") {
            std::vector<ID> const ids_to_index{2, 1, 3, 2};
            std::vector<Idx> const expected_indexer{1, 0, 2, 1};
            std::vector<Idx> indexer(ids_to_index.size());
            model_2.get_indexer("node", std::ssize(ids_to_index), ids_to_index.data(), indexer.data());
            CHECK(indexer == expected_indexer);
        }
        SUBCASE("Bad weather: wrong id") {
            std::vector<ID> const ids_to_index{2, 1, 3, 4};
            std::vector<Idx> indexer(ids_to_index.size());
            CHECK_THROWS_WITH_AS(
                model_2.get_indexer("node", std::ssize(ids_to_index), ids_to_index.data(), indexer.data()),
                doctest::Contains("The id cannot be found: 4"), PowerGridRegularError);
        }
        SUBCASE("Bad weather: wrong type") {
            std::vector<ID> const ids_to_index{2, 1, 3, 2};
            std::vector<Idx> indexer(ids_to_index.size());
            CHECK_THROWS_WITH_AS(
                model_2.get_indexer("sym_load", std::ssize(ids_to_index), ids_to_index.data(), indexer.data()),
                doctest::Contains("Wrong type for object with id 2"), PowerGridRegularError);
        }
    }

    SUBCASE("Batch power flow") {
        model.calculate(options, batch_output_dataset, batch_update_dataset);
        node_batch_output.get_value(PGM_def_sym_output_node_id, batch_node_result_id.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_energized, batch_node_result_energized.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u, batch_node_result_u.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u_pu, batch_node_result_u_pu.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u_angle, batch_node_result_u_angle.data(), -1);
        CHECK(batch_node_result_id[0] == 0);
        CHECK(batch_node_result_energized[0] == 1);
        CHECK(batch_node_result_u[0] == doctest::Approx(40.0));
        CHECK(batch_node_result_u_pu[0] == doctest::Approx(0.4));
        CHECK(batch_node_result_u_angle[0] == doctest::Approx(0.0));
        CHECK(batch_node_result_id[1] == 4);
        CHECK(batch_node_result_energized[1] == 0);
        CHECK(batch_node_result_u[1] == doctest::Approx(0.0));
        CHECK(batch_node_result_u_pu[1] == doctest::Approx(0.0));
        CHECK(batch_node_result_u_angle[1] == doctest::Approx(0.0));
        CHECK(batch_node_result_id[2] == 0);
        CHECK(batch_node_result_energized[2] == 1);
        CHECK(batch_node_result_u[2] == doctest::Approx(70.0));
        CHECK(batch_node_result_u_pu[2] == doctest::Approx(0.7));
        CHECK(batch_node_result_u_angle[2] == doctest::Approx(0.0));
        CHECK(batch_node_result_id[3] == 4);
        CHECK(batch_node_result_energized[3] == 0);
        CHECK(batch_node_result_u[3] == doctest::Approx(0.0));
        CHECK(batch_node_result_u_pu[3] == doctest::Approx(0.0));
        CHECK(batch_node_result_u_angle[3] == doctest::Approx(0.0));
    }

    SUBCASE("Input error handling") {
        SUBCASE("Construction error") {
            auto const bad_load_id_state_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "sym_load": [
      {"id": 0, "node": 0, "status": 1, "type": 2, "p_specified": 0, "q_specified": 500}
    ],
    "source": [
      {"id": 1, "node": 0, "status": 1, "u_ref": 1, "sk": 1000, "rx_ratio": 0}
    ],
    "node": [
      {"id": 0, "u_rated": 100},
      {"id": 4, "u_rated": 100}
    ],
    "line": [
      {"id": 5, "from_node": 0, "to_node": 4, "from_status": 0, "to_status": 1},
      {"id": 6, "from_node": 4, "to_node": 0, "from_status": 0, "to_status": 0}
    ]
  }
})json"s;

            auto const bad_owning_input_dataset = load_dataset(bad_load_id_state_json);
            auto const& bad_input_dataset = bad_owning_input_dataset.dataset;

            auto const bad_model_lambda = [&bad_input_dataset]() { Model const wrong_model{50.0, bad_input_dataset}; };

            check_throws_with(bad_model_lambda, PGM_regular_error, "Conflicting id detected:"s);
        }

        SUBCASE("Update error") {
            auto const bad_source_id_single_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": false,
  "attributes": {},
  "data": {
    "source": [
      {"id": 99, "u_ref": 0.5}
    ],
    "sym_load": [
      {"id": 2, "q_specified": 100}
    ],
    "line": [
      {"id": 5, "from_status": 0, "to_status": 1},
      {"id": 6, "from_status": 0, "to_status": 0}
    ]
  }
})json"s;

            auto const bad_single_owning_update_dataset = load_dataset(bad_source_id_single_update_json);
            auto const bad_update_lambda = [&model, &bad_single_owning_update_dataset]() {
                model.update(bad_single_owning_update_dataset.dataset);
            };

            check_throws_with(bad_update_lambda, PGM_regular_error, "The id cannot be found:"s);
        }

        SUBCASE("Update error in calculation") {
            auto const bad_load_id_batch_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"id": 1, "u_ref": 0.5}
      ],
      "sym_load": [
        {"id": 2, "q_specified": 100}
      ],
      "line": [
        {"id": 99, "from_status": 0, "to_status": 1},
        {"id": 999, "from_status": 0, "to_status": 0}
      ]
    },
    {
      "sym_load": [
        {"id": 2, "q_specified": 300}
      ],
      "line": [
        {"id": 9999, "from_status": 0, "to_status": 0},
        {"id": 99999, "from_status": 0, "to_status": 0}
      ]
    }
  ]
})json"s;

            auto const bad_batch_owning_update_dataset = load_dataset(bad_load_id_batch_update_json);
            auto const bad_calc_with_update_lambda = [&model, &options, &batch_output_dataset,
                                                      &bad_batch_owning_update_dataset]() {
                model.calculate(options, batch_output_dataset, bad_batch_owning_update_dataset.dataset);
            };
            check_throws_with(bad_calc_with_update_lambda, PGM_batch_error, "The id cannot be found:"s);
        }

        SUBCASE("Invalid calculation type error") {
            auto const bad_calc_type_lambda = [&options, &model, &single_output_dataset]() {
                options.set_calculation_type(-128);
                model.calculate(options, single_output_dataset);
            };
            check_throws_with(bad_calc_type_lambda, PGM_regular_error, "CalculationType is not implemented for"s);
        }

        SUBCASE("Invalid tap changing strategy error") {
            auto const bad_tap_strat_lambda = [&options, &model, &single_output_dataset]() {
                options.set_tap_changing_strategy(-128);
                model.calculate(options, single_output_dataset);
            };
            check_throws_with(bad_tap_strat_lambda, PGM_regular_error, "get_optimizer_type is not implemented for"s);
        }

        SUBCASE("Tap changing strategy") {
            options.set_tap_changing_strategy(PGM_tap_changing_strategy_min_voltage_tap);
            CHECK_NOTHROW(model.calculate(options, single_output_dataset));
        }
    }

    SUBCASE("Calculation error") {
        SUBCASE("Single calculation error") {
            // not converging
            options.set_max_iter(1);
            options.set_err_tol(1e-100);
            options.set_symmetric(0);
            options.set_threading(1);
            auto const calc_error_lambda = [&model, &single_output_dataset](auto const& opt) {
                model.calculate(opt, single_output_dataset);
            };
            check_throws_with(calc_error_lambda, PGM_regular_error, "Iteration failed to converge after"s, options);

            // wrong method
            options.set_calculation_type(PGM_state_estimation);
            options.set_calculation_method(PGM_iterative_current);
            check_throws_with(calc_error_lambda, PGM_regular_error,
                              "The calculation method is invalid for this calculation!"s, options);
        }

        SUBCASE("Batch calculation error") {
            SUBCASE("Line bad line id") {
                auto const bad_line_id_batch_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"id": 1, "u_ref": 0.5}
      ],
      "sym_load": [
        {"id": 2, "q_specified": 100}
      ],
      "line": [
        {"id": 5, "from_status": 0, "to_status": 1},
        {"id": 6, "from_status": 0, "to_status": 0}
      ]
    },
    {
      "sym_load": [
        {"id": 999, "q_specified": 300}
      ],
      "line": [
        {"id": 5, "from_status": 0, "to_status": 0},
        {"id": 6, "from_status": 0, "to_status": 0}
      ]
    }
  ]
})json"s;

                auto const bad_batch_owning_update_dataset = load_dataset(bad_line_id_batch_update_json);

                // failed in batch scenario 1
                try {
                    model.calculate(options, batch_output_dataset, bad_batch_owning_update_dataset.dataset);
                    FAIL("Expected batch calculation error not thrown.");
                } catch (PowerGridBatchError const& e) {
                    CHECK(e.error_code() == PGM_batch_error);
                    auto const& failed_scenarios = e.failed_scenarios();
                    CHECK(failed_scenarios.size() == 1);
                    CHECK(failed_scenarios[0].scenario == 1);
                    std::string const err_msg{failed_scenarios[0].error_message};
                    CHECK(err_msg.find("The id cannot be found:"s) != std::string::npos);
                }
                // valid results for batch 0
                node_batch_output.get_value(PGM_def_sym_output_node_id, batch_node_result_id.data(), -1);
                node_batch_output.get_value(PGM_def_sym_output_node_energized, batch_node_result_energized.data(), -1);
                node_batch_output.get_value(PGM_def_sym_output_node_u, batch_node_result_u.data(), -1);
                node_batch_output.get_value(PGM_def_sym_output_node_u_pu, batch_node_result_u_pu.data(), -1);
                node_batch_output.get_value(PGM_def_sym_output_node_u_angle, batch_node_result_u_angle.data(), -1);
                CHECK(batch_node_result_id[0] == 0);
                CHECK(batch_node_result_energized[0] == 1);
                CHECK(batch_node_result_u[0] == doctest::Approx(40.0));
                CHECK(batch_node_result_u_pu[0] == doctest::Approx(0.4));
                CHECK(batch_node_result_u_angle[0] == doctest::Approx(0.0));
                CHECK(batch_node_result_id[1] == 4);
                CHECK(batch_node_result_energized[1] == 0);
                CHECK(batch_node_result_u[1] == doctest::Approx(0.0));
                CHECK(batch_node_result_u_pu[1] == doctest::Approx(0.0));
                CHECK(batch_node_result_u_angle[1] == doctest::Approx(0.0));
            }
        }
    }

    SUBCASE("Model update optional id") {
        std::vector<ID> const input_node_id{0};
        std::vector<double> const input_node_u_rated{100.0};
        Buffer input_node_buffer{PGM_def_input_node, 1};
        input_node_buffer.set_nan();
        input_node_buffer.set_value(PGM_def_input_node_id, input_node_id.data(), -1);
        input_node_buffer.set_value(PGM_def_input_node_u_rated, input_node_u_rated.data(), -1);

        std::vector<ID> const input_source_id{1};
        std::vector<ID> const input_source_node{0};
        std::vector<int8_t> const input_source_status{1};
        std::vector<double> const input_source_u_ref{1.0};
        std::vector<double> const input_source_sk{1000.0};
        std::vector<double> const input_source_rx_ratio{0.0};
        Buffer input_source_buffer{PGM_def_input_source, 1};
        input_source_buffer.set_nan();
        input_source_buffer.set_value(PGM_def_input_source_id, input_source_id.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_node, input_source_node.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_status, input_source_status.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_u_ref, input_source_u_ref.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_sk, input_source_sk.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_rx_ratio, input_source_rx_ratio.data(), -1);

        std::vector<ID> const input_sym_load_id{2};
        std::vector<ID> const input_sym_load_node{0};
        std::vector<int8_t> const input_sym_load_status{1};
        std::vector<int8_t> const input_sym_load_type{2};
        std::vector<double> const input_sym_load_p_specified{0.0};
        std::vector<double> const input_sym_load_q_specified{500.0};
        Buffer input_sym_load_buffer{PGM_def_input_sym_load, 1};
        input_sym_load_buffer.set_nan();
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_id, input_sym_load_id.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_node, input_sym_load_node.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_status, input_sym_load_status.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_type, input_sym_load_type.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_p_specified, input_sym_load_p_specified.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_q_specified, input_sym_load_q_specified.data(), -1);

        // input dataset - row
        DatasetConst input_dataset_row{"input", false, 1};
        input_dataset_row.add_buffer("node", 1, 1, nullptr, input_node_buffer);
        input_dataset_row.add_buffer("source", 1, 1, nullptr, input_source_buffer);
        input_dataset_row.add_buffer("sym_load", 1, 1, nullptr, input_sym_load_buffer);

        // input dataset - col
        DatasetConst input_dataset_col{"input", false, 1};
        input_dataset_col.add_buffer("node", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("node", "id", input_node_id.data());
        input_dataset_col.add_attribute_buffer("node", "u_rated", input_node_u_rated.data());

        input_dataset_col.add_buffer("source", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("source", "id", input_source_id.data());
        input_dataset_col.add_attribute_buffer("source", "node", input_source_node.data());
        input_dataset_col.add_attribute_buffer("source", "status", input_source_status.data());
        input_dataset_col.add_attribute_buffer("source", "u_ref", input_source_u_ref.data());
        input_dataset_col.add_attribute_buffer("source", "sk", input_source_sk.data());
        input_dataset_col.add_attribute_buffer("source", "rx_ratio", input_source_rx_ratio.data());

        input_dataset_col.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("sym_load", "id", input_sym_load_id.data());
        input_dataset_col.add_attribute_buffer("sym_load", "node", input_sym_load_node.data());
        input_dataset_col.add_attribute_buffer("sym_load", "status", input_sym_load_status.data());
        input_dataset_col.add_attribute_buffer("sym_load", "type", input_sym_load_type.data());
        input_dataset_col.add_attribute_buffer("sym_load", "p_specified", input_sym_load_p_specified.data());
        input_dataset_col.add_attribute_buffer("sym_load", "q_specified", input_sym_load_q_specified.data());

        // update dataset
        std::vector<Idx> update_source_indptr{0, 1, 2};
        std::vector<ID> const update_source_id{1, 1};
        std::vector<double> const update_source_u_ref{0.5, 1.0};
        Buffer update_source_buffer{PGM_def_update_source, 2};
        update_source_buffer.set_nan();
        update_source_buffer.set_value(PGM_def_update_source_id, update_source_id.data(), -1);
        update_source_buffer.set_value(PGM_def_update_source_u_ref, update_source_u_ref.data(), -1);

        std::vector<Idx> update_sym_load_indptr{0, 1, 2};
        std::vector<ID> const update_sym_load_id{2, 5};
        std::vector<double> const update_sym_load_q_specified{100.0, 300.0};
        Buffer update_sym_load_buffer{PGM_def_update_sym_load, 2};
        update_sym_load_buffer.set_nan();
        update_sym_load_buffer.set_value(PGM_def_update_sym_load_id, update_sym_load_id.data(), -1);
        update_sym_load_buffer.set_value(PGM_def_update_sym_load_q_specified, update_sym_load_q_specified.data(), -1);

        // update dataset buffers - no ids
        Buffer update_source_buffer_no_id{PGM_def_update_source, 2};
        update_source_buffer_no_id.set_nan();
        update_source_buffer_no_id.set_value(PGM_def_update_source_u_ref, update_source_u_ref.data(), -1);

        Buffer update_sym_load_buffer_no_id{PGM_def_update_sym_load, 2};
        update_sym_load_buffer_no_id.set_nan();
        update_sym_load_buffer_no_id.set_value(PGM_def_update_sym_load_q_specified, update_sym_load_q_specified.data(),
                                               -1);

        // update dataset - row
        DatasetConst update_dataset_row{"update", true, 2};
        update_dataset_row.add_buffer("source", -1, 2, update_source_indptr.data(), update_source_buffer);
        update_dataset_row.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), update_sym_load_buffer);

        // update dataset - col
        DatasetConst update_dataset_col{"update", true, 2};

        update_dataset_col.add_buffer("source", -1, 2, update_source_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("source", "id", update_source_id.data());
        update_dataset_col.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("sym_load", "id", update_sym_load_id.data());
        update_dataset_col.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // update dataset - row no ids
        DatasetConst update_dataset_row_no_id{"update", true, 2};
        update_dataset_row_no_id.add_buffer("source", -1, 2, update_source_indptr.data(), update_source_buffer_no_id);
        update_dataset_row_no_id.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(),
                                            update_sym_load_buffer_no_id);

        // update dataset - col no ids
        DatasetConst update_dataset_col_no_id{"update", true, 2};
        update_dataset_col_no_id.add_buffer("source", -1, 2, update_source_indptr.data(), nullptr);

        update_dataset_col_no_id.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col_no_id.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), nullptr);
        update_dataset_col_no_id.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // output data
        Buffer batch_node_output{PGM_def_sym_output_node, 2};
        batch_node_output.set_nan();
        DatasetMutable batch_output{"sym_output", true, 2};
        batch_output.add_buffer("node", 1, 2, nullptr, batch_node_output);

        // options
        Options const batch_options{};

        SUBCASE("Row-based input dataset") {
            Model row_model{50.0, input_dataset_row};

            SUBCASE("Row-based update dataset error") {
                CHECK_THROWS_AS(row_model.calculate(batch_options, batch_output, update_dataset_row),
                                PowerGridBatchError);
            }
            SUBCASE("Row-based update dataset wo id") {
                row_model.calculate(batch_options, batch_output_dataset, update_dataset_row_no_id);
            }
            SUBCASE("Columnar update dataset error") {
                CHECK_THROWS_AS(row_model.calculate(batch_options, batch_output, update_dataset_col),
                                PowerGridBatchError);
            }
            SUBCASE("Columnar update dataset wo id") {
                row_model.calculate(batch_options, batch_output, update_dataset_col_no_id);
            }
            SUBCASE("Columnar update dataset wo id - non-uniform") {
                update_source_indptr = {0, 1, 1};
                CHECK_THROWS_AS(row_model.calculate(batch_options, batch_output, update_dataset_col_no_id),
                                PowerGridBatchError);
            }
        }

        SUBCASE("Columnar input dataset") {
            Model col_model{50.0, input_dataset_col};

            SUBCASE("Row-based update dataset error") {
                CHECK_THROWS_AS(col_model.calculate(batch_options, batch_output, update_dataset_row),
                                PowerGridBatchError);
            }
            SUBCASE("Row-based update dataset wo id") {
                col_model.calculate(batch_options, batch_output_dataset, update_dataset_row_no_id);
            }
            SUBCASE("Columnar update dataset error") {
                CHECK_THROWS_AS(col_model.calculate(batch_options, batch_output, update_dataset_col),
                                PowerGridBatchError);
            }
            SUBCASE("Columnar update dataset wo id") {
                col_model.calculate(batch_options, batch_output, update_dataset_col_no_id);
            }
            SUBCASE("Columnar update dataset wo id - non-uniform") {
                update_source_indptr = {0, 1, 1};
                CHECK_THROWS_AS(col_model.calculate(batch_options, batch_output, update_dataset_col_no_id),
                                PowerGridBatchError);
            }
        }
    }

    SUBCASE("Self contained model update error") {
        std::vector<ID> const input_node_id{0};
        std::vector<double> const input_node_u_rated{100.0};
        Buffer input_node_buffer{PGM_def_input_node, 1};
        input_node_buffer.set_nan();
        input_node_buffer.set_value(PGM_def_input_node_id, input_node_id.data(), -1);
        input_node_buffer.set_value(PGM_def_input_node_u_rated, input_node_u_rated.data(), -1);

        std::vector<ID> const input_source_id{1};
        std::vector<ID> const input_source_node{0};
        std::vector<int8_t> const input_source_status{1};
        std::vector<double> const input_source_u_ref{1.0};
        std::vector<double> const input_source_sk{1000.0};
        std::vector<double> const input_source_rx_ratio{0.0};
        Buffer input_source_buffer{PGM_def_input_source, 1};
        input_source_buffer.set_nan();
        input_source_buffer.set_value(PGM_def_input_source_id, input_source_id.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_node, input_source_node.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_status, input_source_status.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_u_ref, input_source_u_ref.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_sk, input_source_sk.data(), -1);
        input_source_buffer.set_value(PGM_def_input_source_rx_ratio, input_source_rx_ratio.data(), -1);

        std::vector<ID> const input_sym_load_id{2};
        std::vector<ID> const input_sym_load_node{0};
        std::vector<int8_t> const input_sym_load_status{1};
        std::vector<int8_t> const input_sym_load_type{2};
        std::vector<double> const input_sym_load_p_specified{0.0};
        std::vector<double> const input_sym_load_q_specified{500.0};
        Buffer input_sym_load_buffer{PGM_def_input_sym_load, 1};
        input_sym_load_buffer.set_nan();
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_id, input_sym_load_id.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_node, input_sym_load_node.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_status, input_sym_load_status.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_type, input_sym_load_type.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_p_specified, input_sym_load_p_specified.data(), -1);
        input_sym_load_buffer.set_value(PGM_def_input_sym_load_q_specified, input_sym_load_q_specified.data(), -1);

        // input dataset - row
        DatasetConst input_dataset_row{"input", false, 1};
        input_dataset_row.add_buffer("node", 1, 1, nullptr, input_node_buffer);
        input_dataset_row.add_buffer("source", 1, 1, nullptr, input_source_buffer);
        input_dataset_row.add_buffer("sym_load", 1, 1, nullptr, input_sym_load_buffer);

        // input dataset - col
        DatasetConst input_dataset_col{"input", false, 1};
        input_dataset_col.add_buffer("node", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("node", "id", input_node_id.data());
        input_dataset_col.add_attribute_buffer("node", "u_rated", input_node_u_rated.data());

        input_dataset_col.add_buffer("source", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("source", "id", input_source_id.data());
        input_dataset_col.add_attribute_buffer("source", "node", input_source_node.data());
        input_dataset_col.add_attribute_buffer("source", "status", input_source_status.data());
        input_dataset_col.add_attribute_buffer("source", "u_ref", input_source_u_ref.data());
        input_dataset_col.add_attribute_buffer("source", "sk", input_source_sk.data());
        input_dataset_col.add_attribute_buffer("source", "rx_ratio", input_source_rx_ratio.data());

        input_dataset_col.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        input_dataset_col.add_attribute_buffer("sym_load", "id", input_sym_load_id.data());
        input_dataset_col.add_attribute_buffer("sym_load", "node", input_sym_load_node.data());
        input_dataset_col.add_attribute_buffer("sym_load", "status", input_sym_load_status.data());
        input_dataset_col.add_attribute_buffer("sym_load", "type", input_sym_load_type.data());
        input_dataset_col.add_attribute_buffer("sym_load", "p_specified", input_sym_load_p_specified.data());
        input_dataset_col.add_attribute_buffer("sym_load", "q_specified", input_sym_load_q_specified.data());

        // update dataset
        std::vector<Idx> source_indptr{0, 1, 1};
        std::vector<ID> const update_source_id{1};
        std::vector<double> const update_source_u_ref{0.5};
        Buffer update_source_buffer{PGM_def_update_source, 1};
        update_source_buffer.set_nan();
        update_source_buffer.set_value(PGM_def_update_source_id, update_source_id.data(), -1);
        update_source_buffer.set_value(PGM_def_update_source_u_ref, update_source_u_ref.data(), -1);

        std::vector<Idx> sym_load_indptr{0, 1, 2};
        std::vector<ID> const update_sym_load_id{2, 5};
        std::vector<double> const update_sym_load_q_specified{100.0, 300.0};
        Buffer update_sym_load_buffer{PGM_def_update_sym_load, 2};
        update_sym_load_buffer.set_nan();
        update_sym_load_buffer.set_value(PGM_def_update_sym_load_id, update_sym_load_id.data(), -1);
        update_sym_load_buffer.set_value(PGM_def_update_sym_load_q_specified, update_sym_load_q_specified.data(), -1);

        // update dataset - row
        DatasetConst update_dataset_row{"update", true, 2};
        update_dataset_row.add_buffer("source", -1, 1, source_indptr.data(), update_source_buffer);
        update_dataset_row.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), update_sym_load_buffer);

        // update dataset - col
        DatasetConst update_dataset_col{"update", true, 2};

        update_dataset_col.add_buffer("source", -1, 1, source_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("source", "id", update_source_id.data());
        update_dataset_col.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("sym_load", "id", update_sym_load_id.data());
        update_dataset_col.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // output data
        Buffer output_node_batch{PGM_def_sym_output_node, 2};
        output_node_batch.set_nan();
        DatasetMutable output_batch_dataset{"sym_output", true, 2};
        output_batch_dataset.add_buffer("node", 1, 2, nullptr, output_node_batch);

        // options
        Options const opt{};

        SUBCASE("Row-based input dataset") {
            Model row_model{50.0, input_dataset_row};

            SUBCASE("Row-based update dataset") {
                CHECK_THROWS_AS(row_model.calculate(opt, output_batch_dataset, update_dataset_row),
                                PowerGridBatchError);
            }
        }

        SUBCASE("Columnar input dataset") {
            Model col_model{50.0, input_dataset_col};

            SUBCASE("Row-based update dataset") {
                CHECK_THROWS_AS(col_model.calculate(opt, output_batch_dataset, update_dataset_row),
                                PowerGridBatchError);
            }
        }
    }

    SUBCASE("Wrong calculation methods") {
        using namespace std::string_view_literals;

        constexpr auto invalid_calculation_method_pattern = "The calculation method is invalid for this calculation!";
        constexpr auto all_types = std::array{PGM_power_flow, PGM_state_estimation, PGM_short_circuit};
        constexpr auto all_methods =
            std::array{PGM_default_method,    PGM_linear,           PGM_newton_raphson, PGM_linear_current,
                       PGM_iterative_current, PGM_iterative_linear, PGM_iec60909};

        auto supported_methods = std::map<PGM_CalculationType, std::vector<PGM_CalculationMethod>>{
            {PGM_power_flow, std::vector{PGM_default_method, PGM_newton_raphson, PGM_linear, PGM_linear_current,
                                         PGM_iterative_current}},
            {PGM_state_estimation, std::vector{PGM_default_method, PGM_iterative_linear, PGM_newton_raphson}},
            {PGM_short_circuit, std::vector{PGM_default_method, PGM_iec60909}}};

        auto output_dataset_types = std::map<PGM_CalculationType, std::string>{
            {PGM_power_flow, "sym_output"s}, {PGM_state_estimation, "sym_output"s}, {PGM_short_circuit, "sc_output"s}};

        for (auto calculation_type : all_types) {
            CAPTURE(calculation_type);
            auto const& supported_type_methods = supported_methods.at(calculation_type);

            DatasetMutable const output_dataset{output_dataset_types.at(calculation_type), false, 1};

            for (auto calculation_method : all_methods) {
                CAPTURE(calculation_method);
                options.set_calculation_type(calculation_type);
                options.set_calculation_method(calculation_method);

                if (std::ranges::find(supported_type_methods, calculation_method) == std::end(supported_type_methods)) {
                    CHECK_THROWS_WITH_AS(model.calculate(options, output_dataset), invalid_calculation_method_pattern,
                                         PowerGridRegularError);
                } else {
                    try {
                        model.calculate(options, output_dataset);
                    } catch (std::exception const& e) {
                        CHECK(e.what() != doctest::Contains(invalid_calculation_method_pattern));
                    }
                }
            }
        }
    }

    SUBCASE("Forbid link power measurements") {
        SUBCASE("SymPowerSensor") {
            auto const input_data_se_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1, "u_rated": 10000},
      {"id": 2, "u_rated": 10000}
    ],
    "link": [
      {"id": 3, "from_node": 1, "to_node": 2}
    ],
    "sym_power_sensor": [
      {"id": 4, "measured_object": 3, "measured_terminal_type": 0}
    ]
  }
})json"s;

            auto const owning_input_dataset_se = load_dataset(input_data_se_json);
            auto const& input_dataset_se = owning_input_dataset_se.dataset;

            auto const construct_model = [&input_dataset_se] { return Model{50.0, input_dataset_se}; };

            CHECK_THROWS_WITH_AS(construct_model(), "PowerSensor measurement is not supported for object of type Link",
                                 PowerGridRegularError);
        }

        SUBCASE("AsymPowerSensor") {
            auto const input_data_se_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1, "u_rated": 10000},
      {"id": 2, "u_rated": 10000}
    ],
    "link": [
      {"id": 3, "from_node": 1, "to_node": 2}
    ],
    "asym_power_sensor": [
      {"id": 4, "measured_object": 3, "measured_terminal_type": 0}
    ]
  }
})json"s;

            auto const owning_input_dataset_se = load_dataset(input_data_se_json);
            auto const& input_dataset_se = owning_input_dataset_se.dataset;

            auto const construct_model = [&input_dataset_se] { return Model{50.0, input_dataset_se}; };

            CHECK_THROWS_WITH_AS(construct_model(), "PowerSensor measurement is not supported for object of type Link",
                                 PowerGridRegularError);
        }
    }

    SUBCASE("Test duplicated id") {
        auto const input_data_2_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1},
      {"id": 1},
      {"id": 3}
    ]
  }
})json"s;

        auto const owning_input_dataset_2 = load_dataset(input_data_2_json);
        auto const& input_dataset_2 = owning_input_dataset_2.dataset;

        auto construct_model = [&] { Model{50.0, input_dataset_2}; };
        CHECK_THROWS_WITH_AS(construct_model(), "Conflicting id detected: 1\n", PowerGridRegularError);
    }

    SUBCASE("Test non-existing id") {
        auto const input_data_2_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1, "u_rated": 10000},
      {"id": 2, "u_rated": 10000}
    ],
    "link": [
      {"id": 5, "from_node": 99, "to_node": 2}
    ]
  }
})json"s;
        auto const owning_input_dataset_2 = load_dataset(input_data_2_json);
        auto const& input_dataset_2 = owning_input_dataset_2.dataset;

        auto construct_model = [&] { Model{50.0, input_dataset_2}; };
        CHECK_THROWS_WITH_AS(construct_model(), "The id cannot be found: 99\n", PowerGridRegularError);
    }

    SUBCASE("Test id for wrong type") {
        std::vector<ID> const node_id_2{1, 2, 3};
        std::vector<double> const node_u_rated_2{10.0e3, 10.0e3, 10.0e3};

        std::vector<ID> line_id_2{9};
        std::vector<ID> line_from_node_2{1};
        std::vector<ID> line_to_node_2{2};

        std::vector<ID> link_id{5};
        std::vector<ID> link_from_node{2};
        std::vector<ID> link_to_node{3};

        std::vector<ID> sym_voltage_sensor_id{25};
        std::vector<ID> sym_voltage_sensor_measured_object{1};

        std::vector<ID> sym_power_sensor_id{28};
        std::vector<ID> sym_power_sensor_measured_object{3};
        std::vector<MeasuredTerminalType> sym_power_sensor_measured_terminal_type{MeasuredTerminalType::node};

        DatasetConst input_dataset_2{"input", false, 1};

        input_dataset_2.add_buffer("node", std::ssize(node_id_2), std::ssize(node_id_2), nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("node", "id", node_id_2.data());
        input_dataset_2.add_attribute_buffer("node", "u_rated", node_u_rated_2.data());

        input_dataset_2.add_buffer("line", std::ssize(line_id_2), std::ssize(line_id_2), nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("line", "id", line_id_2.data());
        input_dataset_2.add_attribute_buffer("line", "from_node", line_from_node_2.data());
        input_dataset_2.add_attribute_buffer("line", "to_node", line_to_node_2.data());

        input_dataset_2.add_buffer("link", std::ssize(link_id), std::ssize(link_id), nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("link", "id", link_id.data());
        input_dataset_2.add_attribute_buffer("link", "from_node", link_from_node.data());
        input_dataset_2.add_attribute_buffer("link", "to_node", link_to_node.data());

        input_dataset_2.add_buffer("sym_voltage_sensor", std::ssize(sym_voltage_sensor_id),
                                   std::ssize(sym_voltage_sensor_id), nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("sym_voltage_sensor", "id", sym_voltage_sensor_id.data());
        input_dataset_2.add_attribute_buffer("sym_voltage_sensor", "measured_object",
                                             sym_voltage_sensor_measured_object.data());

        input_dataset_2.add_buffer("sym_power_sensor", std::ssize(sym_power_sensor_id), std::ssize(sym_power_sensor_id),
                                   nullptr, nullptr);
        input_dataset_2.add_attribute_buffer("sym_power_sensor", "id", sym_power_sensor_id.data());
        input_dataset_2.add_attribute_buffer("sym_power_sensor", "measured_object",
                                             sym_power_sensor_measured_object.data());
        input_dataset_2.add_attribute_buffer("sym_power_sensor", "measured_terminal_type",
                                             sym_power_sensor_measured_terminal_type.data());

        auto construct_model = [&] { Model{50.0, input_dataset_2}; };

        SUBCASE("Correct type") { CHECK_NOTHROW(construct_model()); }
        SUBCASE("Wrong branch terminal node") {
            link_from_node[0] = 9;
            CHECK_THROWS_WITH_AS(construct_model(), "Wrong type for object with id 9\n", PowerGridRegularError);
        }
        SUBCASE("Wrong voltage sensor measured object") {
            sym_voltage_sensor_measured_object[0] = 5;
            CHECK_THROWS_WITH_AS(construct_model(), "Wrong type for object with id 5\n", PowerGridRegularError);
        }
        SUBCASE("Wrong power sensor measured object type") {
            using enum MeasuredTerminalType;
            std::vector<MeasuredTerminalType> const mt_types{branch_from, branch_to, generator, load, shunt, source};

            for (auto const& mt_type : mt_types) {
                CAPTURE(mt_type);
                sym_power_sensor_measured_terminal_type[0] = mt_type;
                CHECK_THROWS_WITH_AS(construct_model(), "Wrong type for object with id 3\n", PowerGridRegularError);
            }
        }
    }
}

} // namespace power_grid_model_cpp
