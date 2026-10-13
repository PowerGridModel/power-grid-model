#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/report.hpp>

#include <doctest/doctest.h>

#include <type_traits>

using InputValidationFunction =
    power_grid_model::validation::ValidationReport (*)(power_grid_model::ConstDataset const&, bool);
static_assert(
    std::is_same_v<decltype(&power_grid_model::validation::validate_input_dataset), InputValidationFunction>);
using BatchValidationFunction = power_grid_model::validation::ValidationReport (*)(
    power_grid_model::ConstDataset const&, power_grid_model::ConstDataset const&, bool);
static_assert(
    std::is_same_v<decltype(&power_grid_model::validation::validate_batch_report), BatchValidationFunction>);

TEST_CASE("Validation report validity follows its issues") {
    power_grid_model::validation::ValidationReport report;

    CHECK(report.valid());

    report.scopes.push_back({.scenario = 0,
                             .status = power_grid_model::validation::ValidationStatus::failed,
                             .issues = {{"missing_value", "node", "u_rated", {1}}}});

    CHECK_FALSE(report.valid());
}