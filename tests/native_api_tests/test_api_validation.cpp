// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_c/basics.h>
#include <power_grid_model_cpp/basics.hpp>
#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/handle.hpp>
#include <power_grid_model_cpp/options.hpp>
#include <power_grid_model_cpp/validation.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <utility>
#include <vector>

namespace power_grid_model_cpp {
namespace {
using nlohmann::json;

DatasetConst make_single_node_input_dataset(ID const& node_id, double const& u_rated) {
    DatasetConst dataset{"input", false, 1};
    dataset.add_buffer("node", 1, 1, nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", &node_id);
    dataset.add_attribute_buffer("node", "u_rated", &u_rated);
    return dataset;
}

DatasetConst make_single_line_input_dataset(ID const& line_id, IntS const& from_status, IntS const& to_status,
                                            double const& r1) {
    DatasetConst dataset{"input", false, 1};
    dataset.add_buffer("line", 1, 1, nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", &line_id);
    dataset.add_attribute_buffer("line", "from_status", &from_status);
    dataset.add_attribute_buffer("line", "to_status", &to_status);
    dataset.add_attribute_buffer("line", "r1", &r1);
    return dataset;
}

DatasetConst make_batch_line_update_dataset(std::vector<ID> const& line_ids, std::vector<IntS> const& from_status,
                                            std::vector<IntS> const& to_status) {
    auto const batch_size = static_cast<Idx>(line_ids.size());
    DatasetConst dataset{"update", true, batch_size};
    dataset.add_buffer("line", 1, batch_size, nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", line_ids.data());
    dataset.add_attribute_buffer("line", "from_status", from_status.data());
    dataset.add_attribute_buffer("line", "to_status", to_status.data());
    return dataset;
}

Options make_validation_options() {
    Options options{};
    options.set_experimental_features(PGM_experimental_features_enabled);
    return options;
}
} // namespace

TEST_CASE("C++ validation requires experimental features to be enabled") {
    Options const options{};

    CHECK_THROWS_WITH_AS((ValidationReport{make_single_node_input_dataset(1, 110.0), options}),
                         doctest::Contains("experimental"), PowerGridBadRequest);
}

TEST_CASE("C++ input validation returns a valid report") {
    auto options = make_validation_options();
    ValidationReport report{make_single_node_input_dataset(1, 110.0), options};

    CHECK(report.get() != nullptr);
    CHECK(report.passed());
    auto const report_json = json::parse(report.get_to_zero_terminated_string());
    CHECK(report_json["type"] == "validation_report");
    CHECK(report_json["version"] == "0.1");
    CHECK(report_json["is_batch"] == false);
    CHECK(report_json["valid"] == true);
    CHECK(report_json["results"].empty());
}

TEST_CASE("C++ input validation reports nonpositive u_rated") {
    auto options = make_validation_options();
    ValidationReport report{make_single_node_input_dataset(3, 0.0), options};

    CHECK(report.get() != nullptr);
    CHECK_FALSE(report.passed());
    CHECK(json::parse(report.get_to_zero_terminated_string()) == json::parse(R"json(
        {
            "type": "validation_report",
            "version": "0.1",
            "is_batch": false,
            "valid": false,
            "results": [
                {
                    "scenario": null,
                    "issues": [
                        {
                            "rule": "not_greater_than_zero",
                            "component": "node",
                            "field": "u_rated",
                            "ids": [3]
                        }
                    ]
                }
            ]
        }
    )json"));
}

TEST_CASE("C++ batch validation reports base-input findings independently") {
    auto options = make_validation_options();
    ValidationReport input_report{make_single_node_input_dataset(4, 110.0), options};
    ValidationReport batch_report{make_single_line_input_dataset(10, 2, 1, 1.0),
                                  make_batch_line_update_dataset({10}, {0}, {1}), options};

    CHECK(input_report.passed());
    CHECK_FALSE(batch_report.passed());
    CHECK(json::parse(input_report.get_to_zero_terminated_string())["is_batch"] == false);
    CHECK(json::parse(batch_report.get_to_zero_terminated_string()) == json::parse(R"json(
        {
            "type": "validation_report",
            "version": "0.1",
            "is_batch": true,
            "valid": false,
            "results": [
                {
                    "scenario": null,
                    "issues": [
                        {
                            "rule": "not_boolean",
                            "component": "line",
                            "field": "from_status",
                            "ids": [10]
                        }
                    ]
                }
            ]
        }
    )json"));
}

// TODO (nitbharambe): Address may_fail after implementing update scenario validation
TEST_CASE("C++ batch validation reports findings for update scenarios (not implemented yet)" * doctest::may_fail()) {
    auto options = make_validation_options();
    ValidationReport report{make_single_line_input_dataset(10, 1, 1, 1.0),
                            make_batch_line_update_dataset({10, 10}, {1, 2}, {1, 1}), options};

    CHECK_FALSE(report.passed());
    CHECK(json::parse(report.get_to_zero_terminated_string()) == json::parse(R"json(
        {
            "type": "validation_report",
            "version": "0.1",
            "is_batch": true,
            "valid": false,
            "results": [
                {
                    "scenario": 1,
                    "issues": [
                        {
                            "rule": "not_boolean",
                            "component": "line",
                            "field": "from_status",
                            "ids": [10]
                        }
                    ]
                }
            ]
        }
    )json"));
}

TEST_CASE("C++ validate_data helpers") {
    auto options = make_validation_options();

    SUBCASE("input dataset") {
        auto report = validate_data(make_single_node_input_dataset(1, 110.0), options);
        CHECK(report.passed());
    }

    SUBCASE("batch update dataset") {
        auto report = validate_data(make_single_line_input_dataset(10, 2, 1, 1.0),
                                    make_batch_line_update_dataset({10}, {0}, {1}), options);
        CHECK_FALSE(report.passed());
    }
}

TEST_CASE("C++ ValidationReport can be moved") {
    auto options = make_validation_options();
    ValidationReport report{make_single_node_input_dataset(1, 0.0), options};
    auto const* const raw_report = report.get();

    ValidationReport moved_report{std::move(report)};

    CHECK(moved_report.get() == raw_report);
    CHECK_FALSE(moved_report.passed());
}

} // namespace power_grid_model_cpp
