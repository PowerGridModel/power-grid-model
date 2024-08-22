// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_cpp.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/auxiliary/update.hpp>

#include <doctest/doctest.h>

#include <power_grid_model_c/dataset_definitions.h>

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
TEST_CASE("C API Model") {
    Handle handle;
    Options options;

    // input data
    DatasetConst input_dataset{"input", 0, 1};
    NodeInput node_input{.id = 0, .u_rated = 100.0};
    SourceInput source_input{.id = 1,
                             .node = 0,
                             .status = 1,
                             .u_ref = 1.0,
                             .u_ref_angle = 0.0,
                             .sk = 1000.0,
                             .rx_ratio = 0.0,
                             .z01_ratio = 1.0};
    SymLoadGenInput load_input{
        .id = 2, .node = 0, .status = 1, .type = LoadGenType::const_i, .p_specified = 0.0, .q_specified = 500.0};

    // create one buffer and set attr, leave angle to nan as default zero, leave z01 ratio to nan
    Buffer source_buffer{PGM_def_input_source, 1};
    source_buffer.set_nan(0);
    source_buffer.set_value(PGM_def_input_source_id, &source_input.id, 0, -1);
    source_buffer.set_value(PGM_def_input_source_node, &source_input.node, 0, sizeof(ID));
    source_buffer.set_value(PGM_def_input_source_status, &source_input.status, 0, -1);
    source_buffer.set_value(PGM_def_input_source_u_ref, &source_input.u_ref, 0, -1);
    source_buffer.set_value(PGM_def_input_source_sk, &source_input.sk, 0, -1);
    source_buffer.set_value(PGM_def_input_source_rx_ratio, &source_input.rx_ratio, 0, -1);

    // add buffer
    source_buffer.add_buffer("node", 1, 1, nullptr, &node_input);
    source_buffer.add_buffer("sym_load", 1, 1, nullptr, &load_input);
    source_buffer.add_buffer("source", 1, 1, nullptr, source_buffer);

    // output data
    std::array<NodeOutput<symmetric_t>, 2> sym_node_outputs{};
    NodeOutput<symmetric_t>& node_result_0 = sym_node_outputs[0];
    NodeOutput<symmetric_t>& node_result_1 = sym_node_outputs[1];
    DatasetMutable single_output_dataset{"sym_output", 0, 1};
    single_output_dataset.add_buffer("node", 1, 1, nullptr, sym_node_outputs.data());
    DatasetMutable batch_output_dataset{"sym_output", 1, 2};
    batch_output_dataset.add_buffer("node", 1, 2, nullptr, sym_node_outputs.data());

    // update data
    SourceUpdate source_update{.id = 1, .status = na_IntS, .u_ref = 0.5, .u_ref_angle = nan};
    std::array<Idx, 3> source_update_indptr{0, 1, 1};
    std::array<SymLoadGenUpdate, 2> load_updates{};
    // set nan twice with offset
    Buffer::set_nan(PGM_def_update_sym_load, load_updates.data(), 0, 1);
    Buffer::set_nan(PGM_def_update_sym_load, load_updates.data(), 1, 1);
    // set value
    load_updates[0].id = 2;
    load_updates[0].q_specified = 100.0;
    load_updates[1].id = 2;
    load_updates[1].q_specified = 300.0;
    // dataset
    DatasetConst single_update_dataset{"update", 0, 1};
    single_update_dataset.add_buffer("source", 1, 1, nullptr, &source_update);
    single_update_dataset.add_buffer("sym_load", 1, 1, nullptr, load_updates.data());
    DatasetConst batch_update_dataset{"update", 1, 2};
    batch_update_dataset.add_buffer("source", -1, 1, source_update_indptr.data(), &source_update);
    batch_update_dataset.add_buffer("sym_load", 1, 2, nullptr, load_updates.data());

    // create model
    Model model{50.0, input_dataset};

    SUBCASE("Simple power flow") {
        model.calculate(options, single_output_dataset, nullptr);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(50.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.5));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Simple update") {
        model.update(single_update_dataset);
        model.calculate(options, single_output_dataset, nullptr);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Copy model") {
        Model model_copy{model};
        model_copy.calculate(options, single_output_dataset, nullptr);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(50.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.5));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Get indexer") {
        std::array<ID, 2> ids{2, 2};
        std::array<Idx, 2> indexer{3, 3};
        model.get_indexer"sym_load", 2, ids.data(), indexer.data()();
        CHECK(indexer[0] == 0);
        CHECK(indexer[1] == 0);
        ids[1] = 6;
        model.get_indexer"sym_load", 2, ids.data(), indexer.data()();
    }

    SUBCASE("Batch power flow") {
        model.calculate(options, batch_output_dataset, batch_update_dataset);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
        CHECK(node_result_1.id == 0);
        CHECK(node_result_1.energized == 1);
        CHECK(node_result_1.u == doctest::Approx(70.0));
        CHECK(node_result_1.u_pu == doctest::Approx(0.7));
        CHECK(node_result_1.u_angle == doctest::Approx(0.0));
        // check via get attribute for u_pu and u
        std::array<double, 2> u_pu{};
        Buffer::get_value(PGM_def_sym_output_node_u_pu, sym_node_outputs.data(), u_pu.data(), 0, 2, -1);
        CHECK(u_pu[0] == doctest::Approx(0.4));
        CHECK(u_pu[1] == doctest::Approx(0.7));
        std::array<double, 4> u{};
        Buffer::get_value(PGM_def_sym_output_node_u, sym_node_outputs.data(), u.data(), 0, 2,
                             2 * sizeof(double)); // stride of two double
        CHECK(u[0] == doctest::Approx(40.0));
        CHECK(u[2] == doctest::Approx(70.0));
    }

    SUBCASE("Input error handling") {
        using namespace std::string_literals;

        std::string expected_error;

        SUBCASE("Construction error") {
            expected_error = "Conflicting id detected:"s;

            load_input.id = 0;
            CHECKS_THROWS_WITH_AS(Model wrong_model{50.0, input_dataset}, expected_error, PowerGridRegularError);
        }

        SUBCASE("Update error") {
            expected_error = "The id cannot be found:"s;

            source_update.id = 5;
            CHECKS_THROWS_WITH_AS(model.update(single_update_dataset), expected_error, PowerGridRegularError);
        }

        SUBCASE("Invalid calculation type error") {
            expected_error = "CalculationType is not implemented for"s;

            CHECKS_THROWS_WITH_AS(options.set_calculation_type(-128), expected_error, PowerGridRegularError);
        }

        SUBCASE("Invalid tap changing strategy error") {
            expected_error = "get_optimizer_type is not implemented for"s;

            options = Options{};
            options.set_tap_changing_strategy(-128);
            CHECKS_THROWS_WITH_AS(model.calculate(options, single_output_dataset, nullptr), expected_error, PowerGridRegularError);
        }

        SUBCASE("Tap changing strategy") {
            options.set_tap_changing_strategy(PGM_tap_changing_strategy_min_voltage_tap);
            CHECK_NOTHROW(model.calculate(options, single_output_dataset, nullptr));
        }
    }

    SUBCASE("Calculation error") {
        SUBCASE("Single calculation error") {
            // not converging
            options.set_max_iter(1);
            options.set_err_tol(1e-100);
            options.set_symmetric(0);
            options.set_threading(1);
            CHECKS_THROWS_WITH_AS(model.calculate(options, single_output_dataset, nullptr), "Iteration failed to converge after"s, PowerGridRegularError);

            // wrong method
            options.set_calculation_type(PGM_state_estimation);
            options.set_calculation_method(PGM_iterative_current);
            CHECKS_THROWS_WITH_AS(model.calculate(options, single_output_dataset, nullptr), "The calculation method is invalid for this calculation!"s, PowerGridRegularError);
        }

        SUBCASE("Batch calculation error") { // This part still needs work. Also, checking error strings needs to be fixed as well
            // wrong id
            load_updates[1].id = 5;
            model.calculate(options, batch_output_dataset, batch_update_dataset);
            // failed in batch 1
            CHECK(PGM_error_code(hl) == PGM_batch_error);
            CHECK(PGM_n_failed_scenarios(hl) == 1);
            CHECK(PGM_failed_scenarios(hl)[0] == 1);
            std::string const err_msg{PGM_batch_errors(hl)[0]};
            CHECK(err_msg.find("The id cannot be found:"s) != std::string::npos); //how to deal with this here. the several scenario stuff, specially without a manual try catch
            // valid results for batch 0
            CHECK(node_result_0.id == 0);
            CHECK(node_result_0.energized == 1);
            CHECK(node_result_0.u == doctest::Approx(40.0));
            CHECK(node_result_0.u_pu == doctest::Approx(0.4));
            CHECK(node_result_0.u_angle == doctest::Approx(0.0));
        }
    }
}

} // namespace power_grid_model_cpp
