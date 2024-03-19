// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/transformer_tap_regulator.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Test transformer tap regulator") {
    TransformerTapRegulatorInput input{.id = 1,
                                       .regulated_object = 2,
                                       .control_side = ControlSide::from,
                                       .u_set = 10.5e3,
                                       .u_band = 1e3,
                                       .enabled = true,
                                       .line_drop_compensation_r = 1.0,
                                       .line_drop_compensation_x = 2.0};

    double u_rated{10.5e3};

    TransformerTapRegulator transformer_tap_regulator{input, u_rated};

    // To test:
    // update
    // get_output
    // calc_param
    // control_side
    // energized
    // math_model_type

    SUBCASE("test energized") {
        CHECK(transformer_tap_regulator.energized(true) == true);
        CHECK(transformer_tap_regulator.energized(false) == true);
    }

    // SUBCASE("test get_output") {
    //     TransformerTapRegulatorOutput output = transformer_tap_regulator.get_output(10);
    //     // TODO
    // }
}

} // namespace power_grid_model