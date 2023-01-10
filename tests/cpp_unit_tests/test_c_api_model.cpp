// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/auxiliary/output.hpp"
#include "power_grid_model/auxiliary/update.hpp"
#include "power_grid_model_c.h"

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
    SourceInput source_input{{{1}, 0, 1}, 1.0, 0.0, nan, 0.0, 1.0};
    SymLoadGenInput load_input{{{{2}, 0, 1}, LoadGenType::const_i}, 0.0, 0.0};
}

}  // namespace power_grid_model