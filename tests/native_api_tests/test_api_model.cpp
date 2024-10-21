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

TEST_CASE("API Model") {
    using namespace std::string_literals;

    Options const options{};

    // input data
    DatasetConst input_dataset{"input", 0, 1};

    // node buffer
    std::vector<ID> const node_id{0, 4};
    std::vector<double> const node_u_rated{100.0, 100.0};
    Buffer node_buffer{PGM_def_input_node, 2};
    node_buffer.set_nan(0, node_buffer.size());
    node_buffer.set_value(PGM_def_input_node_id, node_id.data(), -1);
    node_buffer.set_value(PGM_def_input_node_u_rated, node_u_rated.data(), -1);

    ID const line_id{5};
    ID const line_from_node{0};
    ID const line_to_node{4};
    Idx const line_from_status{0};
    Idx const line_to_status{1};
    Buffer line_buffer{PGM_def_input_line, 1};
    line_buffer.set_nan();
    line_buffer.set_value(PGM_def_input_line_id, &line_id, 0);
    line_buffer.set_value(PGM_def_input_line_from_node, &line_from_node, 0);
    line_buffer.set_value(PGM_def_input_line_to_node, &line_to_node, 0);
    line_buffer.set_value(PGM_def_input_line_from_status, &line_from_status, 0);
    line_buffer.set_value(PGM_def_input_line_to_status, &line_to_status, 0);

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
    ID load_id = 2;
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
    input_dataset.add_buffer("line", 1, 1, nullptr, line_buffer);

    // add buffers - columnar
    input_dataset.add_buffer("node", 2, 2, nullptr, nullptr);
    input_dataset.add_attribute_buffer("node", "id", node_id.data());
    input_dataset.add_attribute_buffer("node", "u_rated", node_u_rated.data());

    // update data
    ID source_update_id = 1;
    int8_t const source_update_status = std::numeric_limits<int8_t>::min();
    double const source_update_u_ref = 0.5;
    double const source_update_u_ref_angle = std::numeric_limits<double>::quiet_NaN();
    Buffer source_update_buffer{PGM_def_update_source, 1};
    source_update_buffer.set_nan();
    source_update_buffer.set_value(PGM_def_update_source_id, &source_update_id, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_status, &source_update_status, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_u_ref, &source_update_u_ref, 0, -1);
    source_update_buffer.set_value(PGM_def_update_source_u_ref_angle, &source_update_u_ref_angle, 0, -1);

    Buffer line_update_buffer{PGM_def_update_line, 1};
    line_update_buffer.set_nan();
    line_update_buffer.set_value(PGM_def_update_line_id, &line_id, 0);
    line_update_buffer.set_value(PGM_def_update_line_from_status, &line_from_status, 0);
    line_update_buffer.set_value(PGM_def_update_line_to_status, &line_to_status, 0);

    // dataset
    DatasetConst single_update_dataset{"update", 0, 1};
    single_update_dataset.add_buffer("source", 1, 1, nullptr, source_update_buffer);
    single_update_dataset.add_buffer("line", 1, 1, nullptr, line_update_buffer);

    // create model
    Model model{50.0, input_dataset};

    SUBCASE("Input error handling") {
        SUBCASE("Update error") {
            load_id = 2;
            load_buffer.set_value(PGM_def_input_sym_load_id, &load_id, -1);
            source_update_id = 99;
            source_update_buffer.set_value(PGM_def_update_source_id, &source_update_id, 0, -1);
            try {
                model.update(single_update_dataset);
                throw std::exception{};
            } catch (PowerGridRegularError const& e) {
                check_exception(e, PGM_regular_error, "The id cannot be found:"s);
            }
        }
    }
}

} // namespace power_grid_model_cpp
