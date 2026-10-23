// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/auxiliary/update.hpp"
#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/report.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <type_traits>
#include <vector>

using InputValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, bool, power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_input_dataset), InputValidationFunction>);
using BatchValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, power_grid_model::ConstDataset const&, bool,
    power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_batch_data), BatchValidationFunction>);

TEST_CASE("Batch validation checks line status without node or r1 validation") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using meta_data::meta_data_gen::meta_data;

    std::array<NodeInput, 1> const nodes{{{.id = 1, .u_rated = power_grid_model::nan}}};
    std::array<LineInput, 1> const lines{{{.id = 10,
                                           .from_node = 1,
                                           .to_node = 2,
                                           .from_status = 2,
                                           .to_status = 1,
                                           .r1 = std::numeric_limits<double>::infinity()}}};
    std::array<BranchUpdate, 1> const updates{{{.id = 10, .from_status = 0, .to_status = 1}}};
    ConstDataset input_dataset{false, 1, "input", meta_data};
    input_dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    input_dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    ConstDataset update_dataset{true, 1, "update", meta_data};
    update_dataset.add_buffer("line", updates.size(), updates.size(), nullptr, updates.data());

    auto const report = validate_batch_data(input_dataset, update_dataset, true, CalculationType::power_flow);

    CHECK(report.type == ValidationType::batch);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    CHECK(report.scopes.front().scenario == na_Idx);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues[0].rule == "not_boolean");
    CHECK(report.scopes.front().issues[0].component == "line");
    CHECK(report.scopes.front().issues[0].field == "from_status");
    CHECK(report.scopes.front().issues[0].ids == std::vector<ID>{10});
}
