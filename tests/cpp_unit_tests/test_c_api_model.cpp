// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/auxiliary/output.hpp"
#include "power_grid_model/auxiliary/update.hpp"
#include "power_grid_model_c.h"

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

TEST_CASE("C API Model") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    // get options
    OptionPtr const unique_options{PGM_create_options(hl)};
    PGM_Options* opt = unique_options.get();

    // input data
    NodeInput node_input{{0}, 100.0};
    SourceInput source_input{{{1}, 0, 1}, 1.0, 0.0, 1000.0, 0.0, 1.0};
    SymLoadGenInput load_input{{{{2}, 0, 1}, LoadGenType::const_i}, 0.0, 500.0};
    std::array input_type_names{"node", "source", "sym_load"};
    std::array<Idx, 3> input_type_sizes{1, 1, 1};
    // create one buffer
    BufferPtr const unique_node_buffer{PGM_create_buffer(hl, "input", "node", 1)};
    std::memcpy(unique_node_buffer.get(), &node_input, sizeof(NodeInput));
    std::array<void const*, 3> input_data{unique_node_buffer.get(), &source_input, &load_input};

    // output data
    std::array<NodeOutput<true>, 2> sym_node_outputs{};
    NodeOutput<true>& node_result_0 = sym_node_outputs[0];
    NodeOutput<true>& node_result_1 = sym_node_outputs[1];
    std::array output_type_names{"node"};
    std::array<void*, 1> sym_output_data{sym_node_outputs.data()};

    // update data
    SourceUpdate source_update{{{1}, na_IntS}, 0.5, nan};
    std::array<SymLoadGenUpdate, 2> load_updates{{{{{2}, na_IntS}, nan, 100.0}, {{{2}, na_IntS}, nan, 300.0}}};
    std::array update_type_names{"source", "sym_load"};
    std::array<Idx, 2> update_type_sizes{1, 1};
    std::array<void const*, 2> update_data{&source_update, load_updates.data()};
    std::array<Idx, 2> sizes_per_batch{-1, 1};
    std::array<Idx, 3> source_update_indptr{0, 1, 1};
    std::array<Idx const*, 2> indptrs_per_type{source_update_indptr.data(), nullptr};

    // create model
    ModelPtr unique_model{
        PGM_create_model(hl, 50.0, 3, input_type_names.data(), input_type_sizes.data(), input_data.data())};
    PGM_PowerGridModel* model = unique_model.get();

    SUBCASE("Simple power flow") {
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      0, 0, nullptr, nullptr, nullptr, nullptr);                            // batch parameters
        CHECK(PGM_err_code(hl) == 0);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(50.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.5));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Simple update") {
        PGM_update_model(hl, model, 2, update_type_names.data(), update_type_sizes.data(), update_data.data());
        CHECK(PGM_err_code(hl) == 0);
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      0, 0, nullptr, nullptr, nullptr, nullptr);                            // batch parameters
        CHECK(PGM_err_code(hl) == 0);
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }

    SUBCASE("Batch power flow") {
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      2, 2, update_type_names.data(), sizes_per_batch.data(), indptrs_per_type.data(),
                      update_data.data());  // batch parameters
        CHECK(PGM_err_code(hl) == 0);
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
    }

    SUBCASE("Construction error") {
        source_input.id = 0;
        ModelPtr wrong_model{
            PGM_create_model(hl, 50.0, 3, input_type_names.data(), input_type_sizes.data(), input_data.data())};
        CHECK(wrong_model.get() == nullptr);
        CHECK(PGM_err_code(hl) == 1);
        std::string err_msg{PGM_err_msg(hl)};
        CHECK(err_msg.find("Conflicting id detected:") != std::string::npos);
    }

    SUBCASE("Update error") {
        source_update.id = 5;
        PGM_update_model(hl, model, 2, update_type_names.data(), update_type_sizes.data(), update_data.data());
        CHECK(PGM_err_code(hl) == 1);
        std::string err_msg{PGM_err_msg(hl)};
        CHECK(err_msg.find("The id cannot be found:") != std::string::npos);
    }

    SUBCASE("Single calculation error") {
        // not converging
        PGM_set_max_iter(hl, opt, 1);
        PGM_set_err_tol(hl, opt, 1e-100);
        PGM_set_symmetric(hl, opt, 0);
        PGM_set_threading(hl, opt, 1);
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      0, 0, nullptr, nullptr, nullptr, nullptr);                            // batch parameters
        CHECK(PGM_err_code(hl) == 1);
        std::string err_msg{PGM_err_msg(hl)};
        CHECK(err_msg.find("Iteration failed to converge after") != std::string::npos);
        // wrong method
        PGM_set_calculation_type(hl, opt, PGM_state_estimation);
        PGM_set_calculation_method(hl, opt, PGM_iterative_current);
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      0, 0, nullptr, nullptr, nullptr, nullptr);                            // batch parameters
        CHECK(PGM_err_code(hl) == 1);
        err_msg = PGM_err_msg(hl);
        CHECK(err_msg.find("The calculation method is invalid for this calculation!") != std::string::npos);
    }

    SUBCASE("Batch calculation error") {
        // wrong id
        load_updates[1].id = 5;
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      2, 2, update_type_names.data(), sizes_per_batch.data(), indptrs_per_type.data(),
                      update_data.data());  // batch parameters
        // failed in batch 1
        CHECK(PGM_err_code(hl) == 2);
        CHECK(PGM_n_failed_batches(hl) == 1);
        CHECK(PGM_failed_batches(hl)[0] == 1);
        std::string err_msg{PGM_batch_errs(hl)[0]};
        CHECK(err_msg.find("The id cannot be found:") != std::string::npos);
        // valid results for batch 0
        CHECK(node_result_0.id == 0);
        CHECK(node_result_0.energized == 1);
        CHECK(node_result_0.u == doctest::Approx(40.0));
        CHECK(node_result_0.u_pu == doctest::Approx(0.4));
        CHECK(node_result_0.u_angle == doctest::Approx(0.0));
    }
}

}  // namespace power_grid_model