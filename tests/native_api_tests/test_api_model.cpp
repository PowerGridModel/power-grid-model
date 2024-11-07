// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <array>
#include <exception>
#include <limits>
#include <string>

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
void check_exception(PowerGridError const& e, PGM_ErrorCode const& reference_error,
                     std::string_view reference_err_msg) {
    CHECK(e.error_code() == reference_error);
    std::string const err_msg{e.what()};
    doctest::String const ref_err_msg{reference_err_msg.data()};
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

// Types for template parameters
struct row_t {};
struct columnar_t {};
struct sparse_t {};
struct dense_t {};
struct with_id_t {};
struct optional_id_t {};
struct mixed_optional_id_t {};
struct invalid_id_t {};

template <typename first, typename second, typename third, typename fourth> struct TypeCombo {
    using input_type = first;
    using update_type = second;
    using sparsity_type = third;
    using id_check_type = fourth;
};

} // namespace

TEST_CASE("API Model") {
    using namespace std::string_literals;

    Options options{};

    // input data
    DatasetConst input_dataset{"input", 0, 1};

    // node buffer
    std::vector<ID> const node_id{0, 4};
    std::vector<double> const node_u_rated{100.0, 100.0};

    // line buffer
    std::vector<ID> const line_id{5, 6};
    std::vector<ID> const line_from_node{0, 4};
    std::vector<ID> const line_to_node{4, 0};
    std::vector<Idx> const line_from_status{0, 1};
    std::vector<Idx> const line_to_status{1, 0};
    std::vector<ID> const batch_line_id{5, 6, 5, 6};
    std::vector<ID> const batch_line_from_node{0, 4, 0, 4};
    std::vector<ID> const batch_line_to_node{4, 0, 4, 0};
    std::vector<Idx> const batch_line_from_status{0, 1, 0, 1};
    std::vector<Idx> const batch_line_to_status{1, 0, 1, 0};

    // source buffer
    ID const source_id = 1;
    ID const source_node = 0;
    int8_t const source_status = 1;
    double const source_u_ref = 1.0;
    double const source_sk = 1000.0;
    double const source_rx_ratio = 0.0;
    Buffer source_buffer{PGM_def_input_source, 1};
    source_buffer.set_nan();
    source_buffer.set_value(PGM_def_input_source_id, &source_id, -1);
    source_buffer.set_value(PGM_def_input_source_node, &source_node, 0, sizeof(ID));
    source_buffer.set_value(PGM_def_input_source_status, &source_status, -1);
    source_buffer.set_value(PGM_def_input_source_u_ref, &source_u_ref, -1);
    source_buffer.set_value(PGM_def_input_source_sk, &source_sk, -1);
    source_buffer.set_value(PGM_def_input_source_rx_ratio, &source_rx_ratio, -1);

    // load buffer
    ID const load_id = 2;
    ID const load_node = 0;
    int8_t const load_status = 1;
    int8_t const load_type = 2;
    double const load_p_specified = 0.0;
    double const load_q_specified = 500.0;
    Buffer load_buffer{PGM_def_input_sym_load, 1};
    load_buffer.set_value(PGM_def_input_sym_load_id, &load_id, -1);
    load_buffer.set_value(PGM_def_input_sym_load_node, &load_node, -1);
    load_buffer.set_value(PGM_def_input_sym_load_status, &load_status, -1);
    load_buffer.set_value(PGM_def_input_sym_load_type, &load_type, -1);
    load_buffer.set_value(PGM_def_input_sym_load_p_specified, &load_p_specified, -1);
    load_buffer.set_value(PGM_def_input_sym_load_q_specified, &load_q_specified, -1);

    // add buffers - row
    input_dataset.add_buffer("sym_load", 1, 1, nullptr, load_buffer);
    input_dataset.add_buffer("source", 1, 1, nullptr, source_buffer);

    // add buffers - columnar
    input_dataset.add_buffer("node", 2, 2, nullptr, nullptr);
    input_dataset.add_attribute_buffer("node", "id", node_id.data());
    input_dataset.add_attribute_buffer("node", "u_rated", node_u_rated.data());
    input_dataset.add_buffer("line", 2, 2, nullptr, nullptr);
    input_dataset.add_attribute_buffer("line", "id", line_id.data());
    input_dataset.add_attribute_buffer("line", "from_node", line_from_node.data());
    input_dataset.add_attribute_buffer("line", "to_node", line_to_node.data());
    input_dataset.add_attribute_buffer("line", "from_status", line_from_status.data());
    input_dataset.add_attribute_buffer("line", "to_status", line_to_status.data());

    // output data
    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable single_output_dataset{"sym_output", 0, 1};
    single_output_dataset.add_buffer("node", 2, 2, nullptr, node_output);
    Buffer node_batch_output{PGM_def_sym_output_node, 4};
    node_batch_output.set_nan();
    DatasetMutable batch_output_dataset{"sym_output", 1, 2};
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

    // update data
    ID const source_update_id = 1;
    int8_t const source_update_status = std::numeric_limits<int8_t>::min();
    double const source_update_u_ref = 0.5;
    double const source_update_u_ref_angle = std::numeric_limits<double>::quiet_NaN();
    Buffer source_update_buffer{PGM_def_update_source, 1};
    source_update_buffer.set_nan();
    source_update_buffer.set_value(PGM_def_update_source_id, &source_update_id, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_status, &source_update_status, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_u_ref, &source_update_u_ref, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_u_ref_angle, &source_update_u_ref_angle, 0, -1);
    std::array<Idx, 3> source_update_indptr{0, 1, 1};

    std::vector<ID> load_updates_id = {2, 2};
    std::vector<double> load_updates_q_specified = {100.0, 300.0};
    Buffer load_updates_buffer{PGM_def_update_sym_load, 2};
    // set nan twice with offset
    load_updates_buffer.set_nan(0);
    load_updates_buffer.set_nan(1);
    load_updates_buffer.set_value(PGM_def_update_sym_load_id, load_updates_id.data(), -1);
    load_updates_buffer.set_value(PGM_def_update_sym_load_q_specified, load_updates_q_specified.data(), 0, -1);
    load_updates_buffer.set_value(PGM_def_update_sym_load_q_specified, load_updates_q_specified.data(), 1, -1);
    // dataset
    DatasetConst single_update_dataset{"update", 0, 1};
    single_update_dataset.add_buffer("source", 1, 1, nullptr, source_update_buffer);
    single_update_dataset.add_buffer("sym_load", 1, 1, nullptr, load_updates_buffer.get());
    single_update_dataset.add_buffer("line", 2, 2, nullptr, nullptr);
    single_update_dataset.add_attribute_buffer("line", "id", line_id.data());
    single_update_dataset.add_attribute_buffer("line", "from_status", line_from_status.data());
    single_update_dataset.add_attribute_buffer("line", "to_status", line_to_status.data());
    DatasetConst batch_update_dataset{"update", 1, 2};
    batch_update_dataset.add_buffer("source", -1, 1, source_update_indptr.data(), source_update_buffer.get());
    batch_update_dataset.add_buffer("sym_load", 1, 2, nullptr, load_updates_buffer);
    batch_update_dataset.add_buffer("line", 2, 4, nullptr, nullptr);
    batch_update_dataset.add_attribute_buffer("line", "id", batch_line_id.data());
    batch_update_dataset.add_attribute_buffer("line", "from_status", batch_line_from_status.data());
    batch_update_dataset.add_attribute_buffer("line", "to_status", batch_line_to_status.data());

    // create model
    Model model{50.0, input_dataset};

    SUBCASE("Test Movability") {
        Model model_dummy{std::move(model)};
        model = std::move(model_dummy);
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

    SUBCASE("Get indexer") {
        std::array<ID, 2> ids{2, 2};
        std::array<Idx, 2> indexer{3, 3};
        model.get_indexer("sym_load", 2, ids.data(), indexer.data());
        CHECK(indexer[0] == 0);
        CHECK(indexer[1] == 0);
        ids[1] = 6;
        CHECK_THROWS_AS(model.get_indexer("sym_load", 2, ids.data(), indexer.data()), PowerGridRegularError);
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
            ID const bad_load_id = 0;
            ID const good_source_update_id = 1;
            load_buffer.set_value(PGM_def_input_sym_load_id, &bad_load_id, -1);
            source_update_buffer.set_value(PGM_def_update_source_id, &good_source_update_id, 0, -1);

            auto const wrong_model_lambda = [&input_dataset]() { Model const wrong_model{50.0, input_dataset}; };

            check_throws_with(wrong_model_lambda, PGM_regular_error, "Conflicting id detected:"s);
        }

        SUBCASE("Update error") {
            ID const good_load_id = 2;
            ID const bad_source_update_id = 99;
            load_buffer.set_value(PGM_def_input_sym_load_id, &good_load_id, -1);
            source_update_buffer.set_value(PGM_def_update_source_id, &bad_source_update_id, 0, -1);

            auto const bad_update_lambda = [&model, &single_update_dataset]() { model.update(single_update_dataset); };

            check_throws_with(bad_update_lambda, PGM_regular_error, "The id cannot be found:"s);
        }

        SUBCASE("Update error in calculation") {
            ID const bad_load_id = 2;
            load_buffer.set_value(PGM_def_input_sym_load_id, &bad_load_id, -1);
            DatasetConst bad_batch_update_dataset{"update", 1, 2};
            bad_batch_update_dataset.add_buffer("source", -1, 1, source_update_indptr.data(),
                                                source_update_buffer.get());
            bad_batch_update_dataset.add_buffer("sym_load", 1, 2, nullptr, load_updates_buffer);
            bad_batch_update_dataset.add_buffer("line", 2, 4, nullptr, nullptr); // columnar input for line
            std::vector<ID> const bad_batch_line_id{99, 999, 9999, 99999};
            bad_batch_update_dataset.add_attribute_buffer("line", "id", bad_batch_line_id.data());
            bad_batch_update_dataset.add_attribute_buffer("line", "from_status", batch_line_from_status.data());
            bad_batch_update_dataset.add_attribute_buffer("line", "to_status", batch_line_to_status.data());

            auto const bad_calc_with_update_lambda = [&model, &options, &batch_output_dataset,
                                                      &bad_batch_update_dataset]() {
                model.calculate(options, batch_output_dataset, bad_batch_update_dataset);
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
                // wrong id
                load_updates_id[1] = 999;
                load_updates_buffer.set_value(PGM_def_update_sym_load_id, load_updates_id.data(), 1, -1);
                // failed in batch 1
                try {
                    model.calculate(options, batch_output_dataset, batch_update_dataset);
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
        DatasetConst input_dataset_row{"input", 0, 1};
        input_dataset_row.add_buffer("node", 1, 1, nullptr, input_node_buffer);
        input_dataset_row.add_buffer("source", 1, 1, nullptr, input_source_buffer);
        input_dataset_row.add_buffer("sym_load", 1, 1, nullptr, input_sym_load_buffer);

        // input dataset - col
        DatasetConst input_dataset_col{"input", 0, 1};
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
        DatasetConst update_dataset_row{"update", 1, 2};
        update_dataset_row.add_buffer("source", -1, 2, update_source_indptr.data(), update_source_buffer);
        update_dataset_row.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), update_sym_load_buffer);

        // update dataset - col
        DatasetConst update_dataset_col{"update", 1, 2};

        update_dataset_col.add_buffer("source", -1, 2, update_source_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("source", "id", update_source_id.data());
        update_dataset_col.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("sym_load", "id", update_sym_load_id.data());
        update_dataset_col.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // update dataset - row no ids
        DatasetConst update_dataset_row_no_id{"update", 1, 2};
        update_dataset_row_no_id.add_buffer("source", -1, 2, update_source_indptr.data(), update_source_buffer_no_id);
        update_dataset_row_no_id.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(),
                                            update_sym_load_buffer_no_id);

        // update dataset - col no ids
        DatasetConst update_dataset_col_no_id{"update", 1, 2};
        update_dataset_col_no_id.add_buffer("source", -1, 2, update_source_indptr.data(), nullptr);

        update_dataset_col_no_id.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col_no_id.add_buffer("sym_load", -1, 2, update_sym_load_indptr.data(), nullptr);
        update_dataset_col_no_id.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // output data
        Buffer batch_node_output{PGM_def_sym_output_node, 2};
        batch_node_output.set_nan();
        DatasetMutable batch_output{"sym_output", 1, 2};
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
        DatasetConst input_dataset_row{"input", 0, 1};
        input_dataset_row.add_buffer("node", 1, 1, nullptr, input_node_buffer);
        input_dataset_row.add_buffer("source", 1, 1, nullptr, input_source_buffer);
        input_dataset_row.add_buffer("sym_load", 1, 1, nullptr, input_sym_load_buffer);

        // input dataset - col
        DatasetConst input_dataset_col{"input", 0, 1};
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
        DatasetConst update_dataset_row{"update", 1, 2};
        update_dataset_row.add_buffer("source", -1, 1, source_indptr.data(), update_source_buffer);
        update_dataset_row.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), update_sym_load_buffer);

        // update dataset - col
        DatasetConst update_dataset_col{"update", 1, 2};

        update_dataset_col.add_buffer("source", -1, 1, source_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("source", "id", update_source_id.data());
        update_dataset_col.add_attribute_buffer("source", "u_ref", update_source_u_ref.data());

        update_dataset_col.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), nullptr);
        update_dataset_col.add_attribute_buffer("sym_load", "id", update_sym_load_id.data());
        update_dataset_col.add_attribute_buffer("sym_load", "q_specified", update_sym_load_q_specified.data());

        // output data
        Buffer output_node_batch{PGM_def_sym_output_node, 2};
        output_node_batch.set_nan();
        DatasetMutable output_batch_dataset{"sym_output", 1, 2};
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
}

/*

source_1 -- node_0 --load_2

source and node inputs are row based.
load input is either row based or columnar.
load update is row based / columnar and dense / sparse.

invalid_id_t tests are for testing the error handling of the model when the id is not found in the update dataset.
optional_id_t tests are for testing the model when the id is not added to the update dataset.

*/
TEST_CASE_TEMPLATE(
    "API update id tests", T, TypeCombo<row_t, row_t, dense_t, with_id_t>, TypeCombo<row_t, row_t, sparse_t, with_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, with_id_t>, TypeCombo<columnar_t, columnar_t, sparse_t, with_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, with_id_t>, TypeCombo<columnar_t, row_t, sparse_t, with_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, with_id_t>, TypeCombo<row_t, columnar_t, sparse_t, with_id_t>,
    TypeCombo<row_t, row_t, dense_t, optional_id_t>, TypeCombo<row_t, row_t, sparse_t, optional_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, optional_id_t>,
    TypeCombo<columnar_t, columnar_t, sparse_t, optional_id_t>, TypeCombo<columnar_t, row_t, dense_t, optional_id_t>,
    TypeCombo<columnar_t, row_t, sparse_t, optional_id_t>, TypeCombo<row_t, columnar_t, dense_t, optional_id_t>,
    TypeCombo<row_t, columnar_t, sparse_t, optional_id_t>, TypeCombo<row_t, row_t, dense_t, invalid_id_t>,
    TypeCombo<row_t, row_t, dense_t, mixed_optional_id_t>, TypeCombo<row_t, row_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, columnar_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, row_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, mixed_optional_id_t>, TypeCombo<row_t, columnar_t, sparse_t, optional_id_t>,
    TypeCombo<row_t, row_t, dense_t, invalid_id_t>, TypeCombo<row_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, columnar_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<row_t, columnar_t, sparse_t, invalid_id_t>) {

    using namespace std::string_literals;
    using input_type = typename T::input_type;
    using update_type = typename T::update_type;
    using sparsity_type = typename T::sparsity_type;
    using id_check_type = typename T::id_check_type;

    DatasetConst input_dataset{"input", 0, 1};
    DatasetConst update_dataset{"update", 1, 2};

    std::vector<ID> const node_id{0};
    std::vector<double> const node_u_rated{100.0};
    Buffer node_buffer{PGM_def_input_node, 1};
    node_buffer.set_nan();
    node_buffer.set_value(PGM_def_input_node_id, node_id.data(), -1);
    node_buffer.set_value(PGM_def_input_node_u_rated, node_u_rated.data(), -1);
    input_dataset.add_buffer("node", 1, 1, nullptr, node_buffer);

    std::vector<ID> const source_id{1};
    std::vector<ID> const source_node{0};
    std::vector<int8_t> const source_status{1};
    std::vector<double> const source_u_ref{1.0};
    std::vector<double> const source_sk{1000.0};
    std::vector<double> const source_rx_ratio{0.0};
    Buffer source_buffer{PGM_def_input_source, 1};
    source_buffer.set_nan();
    source_buffer.set_value(PGM_def_input_source_id, source_id.data(), -1);
    source_buffer.set_value(PGM_def_input_source_node, source_node.data(), -1);
    source_buffer.set_value(PGM_def_input_source_status, source_status.data(), -1);
    source_buffer.set_value(PGM_def_input_source_u_ref, source_u_ref.data(), -1);
    source_buffer.set_value(PGM_def_input_source_sk, source_sk.data(), -1);
    source_buffer.set_value(PGM_def_input_source_rx_ratio, source_rx_ratio.data(), -1);
    input_dataset.add_buffer("source", 1, 1, nullptr, source_buffer);

    std::vector<ID> const sym_load_id{2};
    std::vector<ID> const sym_load_node{0};
    std::vector<int8_t> const sym_load_status{1};
    std::vector<int8_t> const sym_load_type{2};
    std::vector<double> const sym_load_p_specified{0.0};
    std::vector<double> const sym_load_q_specified{500.0};
    Buffer sym_load_buffer{PGM_def_input_sym_load, 1};
    sym_load_buffer.set_nan();

    if constexpr (std::is_same_v<input_type, row_t>) {
        sym_load_buffer.set_value(PGM_def_input_sym_load_id, sym_load_id.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_node, sym_load_node.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_status, sym_load_status.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_type, sym_load_type.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_p_specified, sym_load_p_specified.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_q_specified, sym_load_q_specified.data(), -1);
        input_dataset.add_buffer("sym_load", 1, 1, nullptr, sym_load_buffer);
    } else {
        input_dataset.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        input_dataset.add_attribute_buffer("sym_load", "id", sym_load_id.data());
        input_dataset.add_attribute_buffer("sym_load", "node", sym_load_node.data());
        input_dataset.add_attribute_buffer("sym_load", "status", sym_load_status.data());
        input_dataset.add_attribute_buffer("sym_load", "type", sym_load_type.data());
        input_dataset.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
        input_dataset.add_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());
    }

    std::vector<Idx> const sym_load_indptr{0, 1, 2};
    std::vector<double> const load_updates_q_specified = {100.0, 300.0};
    auto load_updates_id = [] {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            return std::vector<ID>{99, 2};
        }
        return std::vector<ID>{2, 2};
    }();

    auto source_indptr = [] {
        if constexpr (std::is_same_v<id_check_type, mixed_optional_id_t>) {
            return std::vector<Idx>{0, 1, 1};
        }
        return std::vector<Idx>{0, 0, 0};
    }();
    std::vector<ID> const source_updates_id = {1};

    Buffer source_update_buffer{PGM_def_update_sym_load, 1};
    source_update_buffer.set_nan();
    source_update_buffer.set_value(PGM_def_update_source_id, source_updates_id.data(), -1);

    Buffer sym_load_update_buffer{PGM_def_update_sym_load, 2};
    sym_load_update_buffer.set_nan();
    if constexpr (!std::is_same_v<id_check_type, optional_id_t>) {
        sym_load_update_buffer.set_value(PGM_def_update_sym_load_id, load_updates_id.data(), -1);
    }
    sym_load_update_buffer.set_value(PGM_def_update_sym_load_q_specified, load_updates_q_specified.data(), -1);

    if constexpr (std::is_same_v<update_type, row_t>) {
        if constexpr (std::is_same_v<sparsity_type, dense_t>) {
            update_dataset.add_buffer("sym_load", 1, 2, nullptr, sym_load_update_buffer);
        } else {
            update_dataset.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), sym_load_update_buffer);
        }
        // source is always sparse. the sparsity tag affects the sym_load
        update_dataset.add_buffer("source", -1, source_indptr.back(), source_indptr.data(), source_update_buffer);
    } else {
        if constexpr (std::is_same_v<sparsity_type, dense_t>) {
            update_dataset.add_buffer("sym_load", 1, 2, nullptr, nullptr);
        } else {
            update_dataset.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), nullptr);
        }
        // source is always sparse. the sparsity tag affects the sym_load
        update_dataset.add_buffer("source", -1, source_indptr.back(), source_indptr.data(), nullptr);

        if constexpr (std::is_same_v<id_check_type, mixed_optional_id_t>) {
            update_dataset.add_attribute_buffer("source", "id", source_updates_id.data());
        } else if constexpr (!std::is_same_v<id_check_type, optional_id_t>) {
            update_dataset.add_attribute_buffer("sym_load", "id", load_updates_id.data());
        }
        update_dataset.add_attribute_buffer("sym_load", "q_specified", load_updates_q_specified.data());
    }

    // output dataset
    Buffer batch_node_output{PGM_def_sym_output_node, 2};
    batch_node_output.set_nan();
    DatasetMutable batch_output_dataset{"sym_output", 1, 2};
    batch_output_dataset.add_buffer("node", 1, 2, nullptr, batch_node_output);

    Options const batch_options{};
    Model model{50.0, input_dataset};

    SUBCASE("Permanent update") {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            CHECK_THROWS_AS(model.update(update_dataset), PowerGridError);
        } else {
            CHECK_NOTHROW(model.update(update_dataset));
        }
    }
    SUBCASE("Batch update") {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            CHECK_THROWS_AS(model.calculate(batch_options, batch_output_dataset, update_dataset), PowerGridBatchError);
        } else {
            CHECK_NOTHROW(model.calculate(batch_options, batch_output_dataset, update_dataset));
        }
    }
}

} // namespace power_grid_model_cpp
