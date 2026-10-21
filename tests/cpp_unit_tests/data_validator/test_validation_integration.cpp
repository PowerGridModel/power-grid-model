// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

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

using InputValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, bool, power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_input_dataset), InputValidationFunction>);
using BatchValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, power_grid_model::ConstDataset const&, bool,
    power_grid_model::CalculationType);
static_assert(std::is_same_v<decltype(&power_grid_model::validation::validate_batch_report), BatchValidationFunction>);

TEST_CASE("Node input validation checks rated voltage") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;
    using power_grid_model::validation::ValidationStatus;

    std::array<NodeInput, 4> const nodes{
        {{1, power_grid_model::nan}, {2, std::numeric_limits<double>::infinity()}, {3, 0.0}, {4, -1.0}}};
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

    std::array<ID, 4> const ids{1, 2, 3, 4};
    std::array<double, 4> const u_rated{power_grid_model::nan, std::numeric_limits<double>::infinity(), 0.0, -1.0};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", ids.data());
    dataset.add_attribute_buffer("node", "u_rated", u_rated.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    REQUIRE(report.scopes.front().issues.size() == 3);
    CHECK(report.scopes.front().issues[0].rule == "missing_value");
    CHECK(report.scopes.front().issues[0].ids[0] == 1);
    CHECK(report.scopes.front().issues[1].rule == "infinity");
    CHECK(report.scopes.front().issues[1].ids[0] == 2);
    CHECK(report.scopes.front().issues[2].rule == "not_greater_than_zero");
    REQUIRE(report.scopes.front().issues[2].ids.size() == 2);
    CHECK(report.scopes.front().issues[2].ids[0] == 3);
    CHECK(report.scopes.front().issues[2].ids[1] == 4);
}

TEST_CASE("Node input validation treats missing columnar attributes as missing values") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::ID;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;

    std::array<ID, 1> const ids{1};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", ids.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues.front().rule == "missing_value");
    CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{1});
}

TEST_CASE("Line input validation checks from_status and r1 attributes") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;

    std::array<LineInput, 2> const lines{{{11, 0, 1, power_grid_model::na_IntS, 1, power_grid_model::nan},
                                          {12, 1, 2, 2, 1, std::numeric_limits<double>::infinity()}}};
    std::array<NodeInput, 3> const nodes{{{0, 1.0}, {1, 1.0}, {2, 1.0}}};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    auto const& issues = report.scopes.front().issues;
    REQUIRE(issues.size() == 4);
    CHECK(issues[0].rule == "missing_value");
    CHECK(issues[0].component == "line");
    CHECK(issues[0].field == "from_status");
    CHECK(issues[0].ids == std::vector<power_grid_model::ID>{11});
    CHECK(issues[1].rule == "missing_value");
    CHECK(issues[1].field == "r1");
    CHECK(issues[1].ids == std::vector<power_grid_model::ID>{11});
    CHECK(issues[2].rule == "infinity");
    CHECK(issues[2].field == "r1");
    CHECK(issues[2].ids == std::vector<power_grid_model::ID>{12});
    CHECK(issues[3].rule == "not_boolean");
    CHECK(issues[3].field == "from_status");
    CHECK(issues[3].ids == (std::vector<power_grid_model::ID>{11, 12}));
}

TEST_CASE("Line input validation checks columnar from_status and r1 attributes") {
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::ID;
    using power_grid_model::IntS;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_input_dataset;

    std::array<ID, 2> const ids{21, 22};
    std::array<ID, 2> const from_node{0, 1};
    std::array<IntS, 2> const from_status{0, 2};
    std::array<double, 2> const r1{0.1, std::numeric_limits<double>::infinity()};
    std::array<NodeInput, 2> const nodes{{{0, 1.0}, {1, 1.0}}};
    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", ids.data());
    dataset.add_attribute_buffer("line", "from_node", from_node.data());
    dataset.add_attribute_buffer("line", "from_status", from_status.data());
    dataset.add_attribute_buffer("line", "r1", r1.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    auto const& issues = report.scopes.front().issues;
    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == "infinity");
    CHECK(issues[0].component == "line");
    CHECK(issues[0].field == "r1");
    CHECK(issues[0].ids == std::vector<ID>{22});
    CHECK(issues[1].rule == "not_boolean");
    CHECK(issues[1].field == "from_status");
    CHECK(issues[1].ids == std::vector<ID>{22});
}

TEST_CASE("Batch validation checks line status without node or r1 validation") {
    using power_grid_model::BranchUpdate;
    using power_grid_model::CalculationType;
    using power_grid_model::ConstDataset;
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::validate_batch_report;
    using power_grid_model::validation::ValidationStatus;
    using power_grid_model::validation::ValidationType;

    std::array<NodeInput, 1> const nodes{{{1, power_grid_model::nan}}};
    std::array<LineInput, 1> const lines{{{10, 1, 2, 2, 1, std::numeric_limits<double>::infinity()}}};
    std::array<BranchUpdate, 1> const updates{{{10, 0, 1}}};
    ConstDataset input_dataset{false, 1, "input", meta_data};
    input_dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    input_dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    ConstDataset update_dataset{true, 1, "update", meta_data};
    update_dataset.add_buffer("line", updates.size(), updates.size(), nullptr, updates.data());

    auto const report = validate_batch_report(input_dataset, update_dataset, true, CalculationType::power_flow);

    CHECK(report.type == ValidationType::batch);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    CHECK(report.scopes.front().scenario == power_grid_model::na_Idx);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues[0].rule == "not_boolean");
    CHECK(report.scopes.front().issues[0].component == "line");
    CHECK(report.scopes.front().issues[0].field == "from_status");
    CHECK(report.scopes.front().issues[0].ids == std::vector<power_grid_model::ID>{10});
}
