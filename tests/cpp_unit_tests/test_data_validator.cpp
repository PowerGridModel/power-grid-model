// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/report.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <type_traits>

using InputValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, bool, power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_input_dataset), InputValidationFunction>);
using BatchValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, power_grid_model::ConstDataset const&, bool,
    power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_batch_report), BatchValidationFunction>);

TEST_CASE("Validation report validity follows its issues") {
    power_grid_model::validation::ValidationReport report;

    CHECK(report.valid());

    report.scopes.push_back({.scenario = 0,
                             .status = power_grid_model::validation::ValidationStatus::failed,
                             .issues = {{"missing_value", "node", "u_rated", {1}}}});

    CHECK_FALSE(report.valid());
}

TEST_CASE("Issue helper appends report issues") {
    using power_grid_model::validation::add_issue;
    using power_grid_model::validation::Issues;

    Issues issues;
    add_issue(issues, "missing_value", "node", "u_rated", {3});

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "missing_value");
    CHECK(issues.front().component == "node");
    CHECK(issues.front().field == "u_rated");
    REQUIRE(issues.front().ids.size() == 1);
    CHECK(issues.front().ids.front() == 3);
}

TEST_CASE("Node input validation checks rated voltage") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;
    using power_grid_model::validation::ValidationStatus;

    std::array<NodeInput, 4> const nodes{{{1, std::numeric_limits<double>::quiet_NaN()},
                                          {2, std::numeric_limits<double>::infinity()},
                                          {3, 0.0},
                                          {4, -1.0}}};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    auto const& scope = report.scopes.front();
    CHECK(scope.status == ValidationStatus::failed);
    REQUIRE(scope.issues.size() == 3);
    CHECK(scope.issues[0].rule == "missing_value");
    CHECK(scope.issues[0].field == "u_rated");
    REQUIRE(scope.issues[0].ids.size() == 1);
    CHECK(scope.issues[0].ids[0] == 1);
    CHECK(scope.issues[1].rule == "infinity");
    REQUIRE(scope.issues[1].ids.size() == 1);
    CHECK(scope.issues[1].ids[0] == 2);
    CHECK(scope.issues[2].rule == "not_greater_than_zero");
    REQUIRE(scope.issues[2].ids.size() == 2);
    CHECK(scope.issues[2].ids[0] == 3);
    CHECK(scope.issues[2].ids[1] == 4);
}

TEST_CASE("Node input validation supports columnar data") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::ID;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;
    using power_grid_model::validation::ValidationStatus;

    std::array<ID, 2> const ids{1, 2};
    std::array<double, 2> const u_rated{std::numeric_limits<double>::quiet_NaN(), -1.0};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", ids.data());
    dataset.add_attribute_buffer("node", "u_rated", u_rated.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    REQUIRE(report.scopes.front().issues.size() == 2);
    CHECK(report.scopes.front().issues[0].rule == "missing_value");
    CHECK(report.scopes.front().issues[0].ids[0] == 1);
    CHECK(report.scopes.front().issues[1].rule == "not_greater_than_zero");
    CHECK(report.scopes.front().issues[1].ids[0] == 2);
}
