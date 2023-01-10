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

update:
    u_ref = 0.5 p.u. (50.0 V)
    q_specified = 100 var (-j1.0A)
u0 = 50.0 V - (j10.0 ohm * -j1.0 A) = 40.0 V
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
    NodeOutput<true> sym_node_output{};
    std::array output_type_names{"node"};
    std::array<void*, 1> sym_output_data{&sym_node_output};

    // create model
    ModelPtr unique_model{
        PGM_create_model(hl, 50.0, 3, input_type_names.data(), input_type_sizes.data(), input_data.data())};
    PGM_PowerGridModel* model = unique_model.get();

    SUBCASE("Simple power flow") {
        PGM_calculate(hl, model, opt, 1, output_type_names.data(), sym_output_data.data(),  // basic parameters
                      0, 0, nullptr, nullptr, nullptr, nullptr);                            // batch parameters
        CHECK(PGM_err_code(hl) == 0);
        CHECK(sym_node_output.id == 0);
        CHECK(sym_node_output.energized == 1);
        CHECK(sym_node_output.u == doctest::Approx(50.0));
        CHECK(sym_node_output.u_pu == doctest::Approx(0.5));
        CHECK(sym_node_output.u_angle == doctest::Approx(0.0));
    }
}

}  // namespace power_grid_model