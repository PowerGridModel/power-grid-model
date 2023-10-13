// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "power_grid_model_c.h"

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

namespace power_grid_model {
namespace {
using meta_data::RawDataConstPtr;
using meta_data::RawDataPtr;
} // namespace

TEST_CASE("C API Model") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    // get options
    OptionPtr const unique_options{PGM_create_options(hl)};
    PGM_Options* opt = unique_options.get();

    // input data
    ConstDatasetPtr const unique_input_dataset{PGM_create_dataset_const(hl, "input", 0, 1)};
    PGM_ConstDataset* input_dataset = unique_input_dataset.get();
    NodeInput node_input{{0}, 100.0};
    SourceInput source_input{{{1}, 0, 1}, 1.0, 0.0, 1000.0, 0.0, 1.0};
    SymLoadGenInput load_input{{{{2}, 0, 1}, LoadGenType::const_i}, 0.0, 500.0};
    // create one buffer and set attr, leave angle to nan as default zero, leave z01 ratio to nan
    BufferPtr const unique_source_buffer{PGM_create_buffer(hl, PGM_def_input_source, 1)};
    RawDataPtr source_buffer = unique_source_buffer.get();
    PGM_buffer_set_nan(hl, PGM_def_input_source, source_buffer, 0, 1);
    PGM_buffer_set_value(hl, PGM_def_input_source_id, source_buffer, &source_input.id, 0, 1, -1);
    PGM_buffer_set_value(hl, PGM_def_input_source_node, source_buffer, &source_input.node, 0, 1, sizeof(ID));
    PGM_buffer_set_value(hl, PGM_def_input_source_status, source_buffer, &source_input.status, 0, 1, -1);
    PGM_buffer_set_value(hl, PGM_def_input_source_u_ref, source_buffer, &source_input.u_ref, 0, 1, -1);
    PGM_buffer_set_value(hl, PGM_def_input_source_sk, source_buffer, &source_input.sk, 0, 1, -1);
    PGM_buffer_set_value(hl, PGM_def_input_source_rx_ratio, source_buffer, &source_input.rx_ratio, 0, 1, -1);
    // add buffer
    PGM_dataset_const_add_buffer(hl, input_dataset, "node", 1, 1, nullptr, &node_input);
    PGM_dataset_const_add_buffer(hl, input_dataset, "sym_load", 1, 1, nullptr, &load_input);
    PGM_dataset_const_add_buffer(hl, input_dataset, "source", 1, 1, nullptr, source_buffer);

    // output data
    std::array<NodeOutput<true>, 2> sym_node_outputs{};
    NodeOutput<true>& node_result_0 = sym_node_outputs[0];
    NodeOutput<true>& node_result_1 = sym_node_outputs[1];
    MutableDatasetPtr const unique_single_output_dataset{PGM_create_dataset_mutable(hl, "sym_output", 0, 1)};
    PGM_MutableDataset* single_output_dataset = unique_single_output_dataset.get();
    PGM_dataset_mutable_add_buffer(hl, single_output_dataset, "node", 1, 1, nullptr, sym_node_outputs.data());
    MutableDatasetPtr const unique_batch_output_dataset{PGM_create_dataset_mutable(hl, "sym_output", 1, 2)};
    PGM_MutableDataset* batch_output_dataset = unique_batch_output_dataset.get();
    PGM_dataset_mutable_add_buffer(hl, batch_output_dataset, "node", 1, 2, nullptr, sym_node_outputs.data());

    // update data
    SourceUpdate source_update{{{1}, na_IntS}, 0.5, nan};
    std::array<Idx, 3> source_update_indptr{0, 1, 1};
    std::array<SymLoadGenUpdate, 2> load_updates{};
    // set nan twice with offset
    PGM_buffer_set_nan(hl, PGM_def_update_sym_load, load_updates.data(), 0, 1);
    PGM_buffer_set_nan(hl, PGM_def_update_sym_load, load_updates.data(), 1, 1);
    // set value
    load_updates[0].id = 2;
    load_updates[0].q_specified = 100.0;
    load_updates[1].id = 2;
    load_updates[1].q_specified = 300.0;
    // dataset
    ConstDatasetPtr const unique_single_update_dataset{PGM_create_dataset_const(hl, "update", 0, 1)};
    PGM_ConstDataset* single_update_dataset = unique_single_update_dataset.get();
    PGM_dataset_const_add_buffer(hl, single_update_dataset, "source", 1, 1, nullptr, &source_update);
    PGM_dataset_const_add_buffer(hl, single_update_dataset, "sym_load", 1, 1, nullptr, load_updates.data());
    ConstDatasetPtr const unique_batch_update_dataset{PGM_create_dataset_const(hl, "update", 1, 2)};
    PGM_ConstDataset* batch_update_dataset = unique_batch_update_dataset.get();
    PGM_dataset_const_add_buffer(hl, batch_update_dataset, "source", -1, 1, source_update_indptr.data(),
                                 &source_update);
    PGM_dataset_const_add_buffer(hl, batch_update_dataset, "sym_load", 1, 2, nullptr, load_updates.data());

    // create model
    ModelPtr const unique_model{PGM_create_model(hl, 50.0, input_dataset)};
    PGM_PowerGridModel* model = unique_model.get();

    SUBCASE("Simple power flow") {
        PGM_calculate(hl, model, opt, single_output_dataset, nullptr);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(50.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.5));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Simple update") {
        PGM_update_model(hl, model, single_update_dataset);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        PGM_calculate(hl, model, opt, single_output_dataset, nullptr);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Copy model") {
        ModelPtr const model_copy{PGM_copy_model(hl, model)};
        CHECK(PGM_error_code(hl) == PGM_no_error);
        PGM_calculate(hl, model_copy.get(), opt, single_output_dataset, nullptr);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(50.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.5));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Get indexer") {
        std::array<ID, 2> ids{2, 2};
        std::array<Idx, 2> indexer{3, 3};
        PGM_get_indexer(hl, model, "sym_load", 2, ids.data(), indexer.data());
        CHECK(PGM_error_code(hl) == PGM_no_error);
        CHECK(indexer[0] == 0);
        CHECK(indexer[1] == 0);
        ids[1] = 6;
        PGM_get_indexer(hl, model, "sym_load", 2, ids.data(), indexer.data());
        CHECK(PGM_error_code(hl) == PGM_regular_error);
    }

    SUBCASE("Batch power flow") {
        PGM_calculate(hl, model, opt, batch_output_dataset, batch_update_dataset);
        CHECK(PGM_error_code(hl) == PGM_no_error);
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
        PGM_buffer_get_value(hl, PGM_def_sym_output_node_u_pu, sym_node_outputs.data(), u_pu.data(), 0, 2, -1);
        CHECK(u_pu[0] == doctest::Approx(0.4));
        CHECK(u_pu[1] == doctest::Approx(0.7));
        std::array<double, 4> u{};
        PGM_buffer_get_value(hl, PGM_def_sym_output_node_u, sym_node_outputs.data(), u.data(), 0, 2,
                             2 * sizeof(double)); // stride of two double
        CHECK(u[0] == doctest::Approx(40.0));
        CHECK(u[2] == doctest::Approx(70.0));
    }

    SUBCASE("Construction error") {
        load_input.id = 0;
        ModelPtr const wrong_model{PGM_create_model(hl, 50.0, input_dataset)};
        CHECK(wrong_model.get() == nullptr);
        CHECK(PGM_error_code(hl) == PGM_regular_error);
        std::string const err_msg{PGM_error_message(hl)};
        CHECK(err_msg.find("Conflicting id detected:") != std::string::npos);
    }

    SUBCASE("Update error") {
        source_update.id = 5;
        PGM_update_model(hl, model, single_update_dataset);
        CHECK(PGM_error_code(hl) == PGM_regular_error);
        std::string const err_msg{PGM_error_message(hl)};
        CHECK(err_msg.find("The id cannot be found:") != std::string::npos);
    }

    SUBCASE("Single calculation error") {
        // not converging
        PGM_set_max_iter(hl, opt, 1);
        PGM_set_err_tol(hl, opt, 1e-100);
        PGM_set_symmetric(hl, opt, 0);
        PGM_set_threading(hl, opt, 1);
        PGM_calculate(hl, model, opt, single_output_dataset, nullptr);
        CHECK(PGM_error_code(hl) == PGM_regular_error);
        std::string err_msg{PGM_error_message(hl)};
        CHECK(err_msg.find("Iteration failed to converge after") != std::string::npos);
        // wrong method
        PGM_set_calculation_type(hl, opt, PGM_state_estimation);
        PGM_set_calculation_method(hl, opt, PGM_iterative_current);
        PGM_calculate(hl, model, opt, single_output_dataset, nullptr);
        CHECK(PGM_error_code(hl) == PGM_regular_error);
        err_msg = PGM_error_message(hl);
        CHECK(err_msg.find("The calculation method is invalid for this calculation!") != std::string::npos);
    }

    SUBCASE("Batch calculation error") {
        // wrong id
        load_updates[1].id = 5;
        PGM_calculate(hl, model, opt, batch_output_dataset, batch_update_dataset);
        // failed in batch 1
        CHECK(PGM_error_code(hl) == PGM_batch_error);
        CHECK(PGM_n_failed_scenarios(hl) == 1);
        CHECK(PGM_failed_scenarios(hl)[0] == 1);
        std::string const err_msg{PGM_batch_errors(hl)[0]};
        CHECK(err_msg.find("The id cannot be found:") != std::string::npos);
        // valid results for batch 0
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }
}

} // namespace power_grid_model