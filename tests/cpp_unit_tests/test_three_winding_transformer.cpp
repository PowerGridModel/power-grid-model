// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/three_winding_transformer.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test three winding transformer") {
    ThreeWindingTransformerInputBasics input_basics{
        {1, 2, 3, 4, true, true, true},  // Create branch3 {{id}, node_1, node_2, node_3, status_1, status_2, status_3}
        138e3,                           // u1
        69e3,                            // u2
        13.8e3,                          // u3
        60e6,                            // s1
        50e6,                            // s2
        10e6,                            // s3
        0.09                             // uk12
        0.06                             // uk13
        0.03                             // uk23
        19e3,                            // pk_12
        16e3,                            // pk_13
        13e3,                            // pk_23
        0.005,                           // i0
        5e3,                             // p0
        WindingType::wye_n,              // winding_12
        WindingType::delta,              // winding_13
        WindingType::delta,              // winding_23
        1,                               // clock_12
        11,                              // clock_13
        Branch3side::side_1,             // tap side
        2,                               // tap_pos
        -8,                              // tap_min
        10,                              // tap_max
        0,                               // tap_nom
        1,                               // tap direction
        0.01                             // tap size
    };

    ThreeWindingTransformerInput input{
        input_basics,
        nan,  // uk_12_min
        nan,  // uk_12_max
        nan,  // uk_13_min
        nan,  // uk_13_max
        nan,  // uk_23_min
        nan,  // uk_23_max
        nan,  // pk_12_min
        nan,  // pk_12_max
        nan,  // pk_13_min
        nan,  // pk_13_max
        nan,  // pk_23_min
        nan,  // pk_23_max
        nan,  // r1
        nan,  // x1
        nan,  // r2
        nan,  // x2
        nan,  // r3
        nan   // x3
    };

    // To add more
}

}  // namespace power_grid_model
