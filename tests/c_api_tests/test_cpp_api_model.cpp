// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <array>
#include <exception>
#include <limits>
#include <string>

/*
Testing network

source_1(1.0 p.u., 100.0 V) --internal_impedance(j10.0 ohm, sk=1000.0 VA, rx_ratio=0.0)--
-- node_0 (100.0 V) --load_2(const_i, -j5.0A, 0.0 W, 500.0 var)

u0 = 100.0 V - (j10.0 ohm * -j5.0 A) = 50.0 V

update_0:
    u_ref = 0.5 p.u. (50.0 V)
    q_specified = 100 var (-j1.0A)
u0 = 50.0 V - (j10.0 ohm * -j1.0 A) = 40.0 V

update_1:
    q_specified = 300 var (-j3.0A)
u0 = 100.0 V - (j10.0 ohm * -j3.0 A) = 70.0 V
*/

namespace power_grid_model_cpp {
namespace {
void check_exception(PowerGridError const& e, PGM_ErrorCode const& reference_error,
                     std::string_view reference_err_msg) {
    CHECK(e.error_code() == reference_error);
    std::string const err_msg{e.what()};
    CHECK(err_msg.find(reference_err_msg) != std::string::npos);
}
} // namespace

TEST_CASE("C++ API Model") {
    using namespace std::string_literals;

    Options const options{};

    // input data
    DatasetConst const input_dataset{"input", 0, 1};

    // node buffer
    ID node_id = 0;
    double node_u_rated = 100.0;
    Buffer node_buffer{PGM_def_input_node, 1};
    // set nan from offset to size
    node_buffer.set_nan(0, node_buffer.size());
    node_buffer.set_value(PGM_def_input_node_id, &node_id, -1);
    node_buffer.set_value(PGM_def_input_node_u_rated, &node_u_rated, -1);

    // source buffer
    ID source_id = 1;
    ID source_node = 0;
    int8_t source_status = 1;
    double source_u_ref = 1.0;
    double source_sk = 1000.0;
    double source_rx_ratio = 0.0;
    Buffer source_buffer{PGM_def_input_source, 1};
    source_buffer.set_nan();
    source_buffer.set_value(PGM_def_input_source_id, &source_id, -1);
    source_buffer.set_value(PGM_def_input_source_node, &source_node, 0, sizeof(ID));
    source_buffer.set_value(PGM_def_input_source_status, &source_status, -1);
    source_buffer.set_value(PGM_def_input_source_u_ref, &source_u_ref, -1);
    source_buffer.set_value(PGM_def_input_source_sk, &source_sk, -1);
    source_buffer.set_value(PGM_def_input_source_rx_ratio, &source_rx_ratio, -1);

    // load buffer
    ID load_id = 2;
    ID load_node = 0;
    int8_t load_status = 1;
    int8_t load_type = 2;
    double load_p_specified = 0.0;
    double load_q_specified = 500.0;
    Buffer const load_buffer{PGM_def_input_sym_load, 1};
    load_buffer.set_value(PGM_def_input_sym_load_id, &load_id, -1);
    load_buffer.set_value(PGM_def_input_sym_load_node, &load_node, -1);
    load_buffer.set_value(PGM_def_input_sym_load_status, &load_status, -1);
    load_buffer.set_value(PGM_def_input_sym_load_type, &load_type, -1);
    load_buffer.set_value(PGM_def_input_sym_load_p_specified, &load_p_specified, -1);
    load_buffer.set_value(PGM_def_input_sym_load_q_specified, &load_q_specified, -1);

    // gen buffer (columnar)
    std::vector<ID> gen_id = {3, 4};
    std::vector<ID> gen_node = {0, 0};
    std::vector<int8_t> gen_status = {0, 0};

    // add buffers - row
    input_dataset.add_buffer("node", 1, 1, nullptr, node_buffer);
    input_dataset.add_buffer("sym_load", 1, 1, nullptr, load_buffer);
    input_dataset.add_buffer("source", 1, 1, nullptr, source_buffer);
    // add buffers - columnar
    input_dataset.add_buffer("sym_gen", 2, 2, nullptr, nullptr);
    input_dataset.add_attribute_buffer("sym_gen", "id", &gen_id);
    input_dataset.add_attribute_buffer("sym_gen", "node", &gen_node);
    input_dataset.add_attribute_buffer("sym_gen", "status", &gen_status);

    // output data
    Buffer node_output{PGM_def_sym_output_node, 1};
    node_output.set_nan();
    DatasetMutable const single_output_dataset{"sym_output", 0, 1};
    single_output_dataset.add_buffer("node", 1, 1, nullptr, node_output);
    Buffer node_batch_output{PGM_def_sym_output_node, 2};
    node_batch_output.set_nan();
    DatasetMutable const batch_output_dataset{"sym_output", 1, 2};
    batch_output_dataset.add_buffer("node", 1, 2, nullptr, node_batch_output);

    std::vector<ID> node_result_id(2);
    std::vector<int8_t> node_result_energized(2);
    std::vector<double> node_result_u(2);
    std::vector<double> node_result_u_pu(2);
    std::vector<double> node_result_u_angle(2);

    // update data
    ID source_update_id = 1;
    int8_t source_update_status = std::numeric_limits<int8_t>::min();
    double source_update_u_ref = 0.5;
    double source_update_u_ref_angle = std::numeric_limits<double>::quiet_NaN();
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
    DatasetConst const single_update_dataset{"update", 0, 1};
    single_update_dataset.add_buffer("source", 1, 1, nullptr, source_update_buffer);
    single_update_dataset.add_buffer("sym_load", 1, 1, nullptr, load_updates_buffer.get());
    single_update_dataset.add_buffer("sym_gen", 2, 2, nullptr, nullptr);
    single_update_dataset.add_attribute_buffer("sym_gen", "status", &gen_status);
    DatasetConst const batch_update_dataset{"update", 1, 2};
    batch_update_dataset.add_buffer("source", -1, 1, source_update_indptr.data(), source_update_buffer.get());
    batch_update_dataset.add_buffer("sym_load", 1, 2, nullptr, load_updates_buffer);
    batch_update_dataset.add_buffer("sym_gen", 1, 2, nullptr, nullptr);
    batch_update_dataset.add_attribute_buffer("sym_gen", "status", &gen_status);

    // create model
    // FAIL("This test will fail here due to columnar data input.");
    Model model{50.0, input_dataset};

    // test move-ability
    Model model_dummy{std::move(model)};
    model = std::move(model_dummy);

    SUBCASE("Simple power flow") {
        model.calculate(options, single_output_dataset);
        node_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), 0, -1);
        node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), 0, 1, -1);
        node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), -1);
        CHECK(node_result_id[0] == 0);
        CHECK(node_result_energized[0] == 1);
        CHECK(node_result_u[0] == doctest::Approx(50.0));
        CHECK(node_result_u_pu[0] == doctest::Approx(0.5));
        CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
    }

    SUBCASE("Simple update") {
        model.update(single_update_dataset);
        model.calculate(options, single_output_dataset);
        node_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), 0, -1);
        node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), 0, 1, -1);
        node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), -1);
        CHECK(node_result_id[0] == 0);
        CHECK(node_result_energized[0] == 1);
        CHECK(node_result_u[0] == doctest::Approx(40.0));
        CHECK(node_result_u_pu[0] == doctest::Approx(0.4));
        CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
    }

    SUBCASE("Copy model") {
        Model model_copy{model};
        model_copy.calculate(options, single_output_dataset);
        node_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), 0, -1);
        node_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), 0, 1, -1);
        node_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
        node_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), -1);
        CHECK(node_result_id[0] == 0);
        CHECK(node_result_energized[0] == 1);
        CHECK(node_result_u[0] == doctest::Approx(50.0));
        CHECK(node_result_u_pu[0] == doctest::Approx(0.5));
        CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
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
        node_batch_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
        node_batch_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), -1);
        CHECK(node_result_id[0] == 0);
        CHECK(node_result_energized[0] == 1);
        CHECK(node_result_u[0] == doctest::Approx(40.0));
        CHECK(node_result_u_pu[0] == doctest::Approx(0.4));
        CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
        CHECK(node_result_id[1] == 0);
        CHECK(node_result_energized[1] == 1);
        CHECK(node_result_u[1] == doctest::Approx(70.0));
        CHECK(node_result_u_pu[1] == doctest::Approx(0.7));
        CHECK(node_result_u_angle[1] == doctest::Approx(0.0));
    }

    SUBCASE("Input error handling") {
        SUBCASE("Construction error") {
            load_id = 0;
            try {
                Model const wrong_model{50.0, input_dataset};
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "Conflicting id detected:"s);
            }
        }

        SUBCASE("Update error") {
            source_update_id = 5;
            try {
                model.update(single_update_dataset);
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "The id cannot be found:"s);
            }
        }

        SUBCASE("Invalid calculation type error") {
            try {
                options.set_calculation_type(-128);
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "CalculationType is not implemented for"s);
            }
        }

        SUBCASE("Invalid tap changing strategy error") {
            try {
                options.set_tap_changing_strategy(-128);
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "get_optimizer_type is not implemented for"s);
            }
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
            try {
                model.calculate(options, single_output_dataset);
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "Iteration failed to converge after"s);
            }

            // wrong method
            options.set_calculation_type(PGM_state_estimation);
            options.set_calculation_method(PGM_iterative_current);
            try {
                model.calculate(options, single_output_dataset);
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "The calculation method is invalid for this calculation!"s);
            }
        }

        SUBCASE("Batch calculation error") {
            // wrong id
            load_updates_id[1] = 5;
            // failed in batch 1
            try {
                model.calculate(options, batch_output_dataset, batch_update_dataset);
            } catch (PowerGridBatchError const& e) {
                CHECK(e.error_code() == PGM_batch_error);
                auto const& failed_scenarios = e.failed_scenarios();
                CHECK(failed_scenarios.size() == 1);
                CHECK(failed_scenarios[0].scenario == 1);
                std::string const err_msg{failed_scenarios[0].error_message};
                CHECK(err_msg.find("The id cannot be found:"s) != std::string::npos);
            }
            // valid results for batch 0
            node_batch_output.get_value(PGM_def_sym_output_node_id, node_result_id.data(), -1);
            node_batch_output.get_value(PGM_def_sym_output_node_energized, node_result_energized.data(), -1);
            node_batch_output.get_value(PGM_def_sym_output_node_u, node_result_u.data(), -1);
            node_batch_output.get_value(PGM_def_sym_output_node_u_pu, node_result_u_pu.data(), -1);
            node_batch_output.get_value(PGM_def_sym_output_node_u_angle, node_result_u_angle.data(), -1);
            CHECK(node_result_id[0] == 0);
            CHECK(node_result_energized[0] == 1);
            CHECK(node_result_u[0] == doctest::Approx(40.0));
            CHECK(node_result_u_pu[0] == doctest::Approx(0.4));
            CHECK(node_result_u_angle[0] == doctest::Approx(0.0));
        }
    }
}

} // namespace power_grid_model_cpp
