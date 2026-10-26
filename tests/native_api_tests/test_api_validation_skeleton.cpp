// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_c/basics.h>
#include <power_grid_model_c/dataset.h>
#include <power_grid_model_c/handle.h>
#include <power_grid_model_c/options.h>
#include <power_grid_model_c/validation.h>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

using CDestroyValidationReportFunction = void (*)(PGM_ValidationReport*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_destroy_validation_report), CDestroyValidationReportFunction>);
using CValidationFunction = PGM_ValidationReport* (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_ConstDataset const*,
                                                      PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_data), CValidationFunction>);
using CValidationReportIsValidFunction = PGM_Idx (*)(PGM_Handle*, PGM_ValidationReport*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validation_report_is_valid), CValidationReportIsValidFunction>);
using CValidationReportGetStringFunction = char const* (*)(PGM_Handle*, PGM_ValidationReport*) noexcept;
static_assert(
    std::is_same_v<decltype(&PGM_validation_report_get_to_zero_terminated_string), CValidationReportGetStringFunction>);

namespace {
using OwnedHandle = std::unique_ptr<PGM_Handle, decltype(&PGM_destroy_handle)>;
using OwnedValidationReport = std::unique_ptr<PGM_ValidationReport, decltype(&PGM_destroy_validation_report)>;
using OwnedOptions = std::unique_ptr<PGM_Options, decltype(&PGM_destroy_options)>;
using OwnedConstDataset = std::unique_ptr<PGM_ConstDataset, decltype(&PGM_destroy_dataset_const)>;

void require_successful_api_call(PGM_Handle* handle) { REQUIRE(PGM_error_code(handle) == PGM_no_error); }

OwnedConstDataset create_single_node_input_dataset(PGM_Handle* handle, PGM_ID const* node_id, double const* u_rated) {
    auto dataset = OwnedConstDataset{PGM_create_dataset_const(handle, "input", 0, 1), &PGM_destroy_dataset_const};
    REQUIRE(dataset != nullptr);
    require_successful_api_call(handle);

    PGM_dataset_const_add_buffer(handle, dataset.get(), "node", 1, 1, nullptr, nullptr);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "node", "id", node_id);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "node", "u_rated", u_rated);
    require_successful_api_call(handle);
    return dataset;
}

OwnedConstDataset create_single_line_input_dataset(PGM_Handle* handle, PGM_ID const* line_id,
                                                   std::int8_t const* from_status, std::int8_t const* to_status,
                                                   double const* r1) {
    auto dataset = OwnedConstDataset{PGM_create_dataset_const(handle, "input", 0, 1), &PGM_destroy_dataset_const};
    REQUIRE(dataset != nullptr);
    require_successful_api_call(handle);

    PGM_dataset_const_add_buffer(handle, dataset.get(), "line", 1, 1, nullptr, nullptr);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "id", line_id);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "from_status", from_status);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "to_status", to_status);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "r1", r1);
    require_successful_api_call(handle);
    return dataset;
}

OwnedConstDataset create_batch_line_update_dataset(PGM_Handle* handle, std::vector<PGM_ID> const& line_ids,
                                                   std::vector<std::int8_t> const& from_status,
                                                   std::vector<std::int8_t> const& to_status) {
    REQUIRE(!line_ids.empty());
    REQUIRE(from_status.size() == line_ids.size());
    REQUIRE(to_status.size() == line_ids.size());
    auto const batch_size = static_cast<PGM_Idx>(line_ids.size());
    auto dataset =
        OwnedConstDataset{PGM_create_dataset_const(handle, "update", 1, batch_size), &PGM_destroy_dataset_const};
    REQUIRE(dataset != nullptr);
    require_successful_api_call(handle);

    PGM_dataset_const_add_buffer(handle, dataset.get(), "line", 1, batch_size, nullptr, nullptr);
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "id", line_ids.data());
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "from_status", from_status.data());
    require_successful_api_call(handle);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "line", "to_status", to_status.data());
    require_successful_api_call(handle);
    return dataset;
}

struct ValidationApiFixture {
    OwnedHandle handle{PGM_create_handle(), &PGM_destroy_handle};
    OwnedOptions options{nullptr, &PGM_destroy_options};

    ValidationApiFixture() {
        REQUIRE(handle != nullptr);
        require_successful_api_call(handle.get());

        options.reset(PGM_create_options(handle.get()));
        REQUIRE(options != nullptr);
        require_successful_api_call(handle.get());
        PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);
        require_successful_api_call(handle.get());
    }

    OwnedValidationReport validate(PGM_ConstDataset const* input_dataset,
                                   PGM_ConstDataset const* update_dataset = nullptr) const {
        return OwnedValidationReport{PGM_validate_data(handle.get(), input_dataset, update_dataset, options.get()),
                                     &PGM_destroy_validation_report};
    }
};
} // namespace


TEST_CASE_FIXTURE(ValidationApiFixture, "Validation API requires experimental features to be enabled") {
    PGM_ID node_id = 1;
    double u_rated = 110.0;
    auto const dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);

    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_disabled);
    require_successful_api_call(handle.get());

    auto const report = validate(dataset.get());
    CHECK(report == nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental") != std::string_view::npos);
}


TEST_CASE_FIXTURE(ValidationApiFixture, "Input validation returns a valid report") {
    PGM_ID node_id = 1;
    double u_rated = 110.0;
    auto const dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);

    auto const report = validate(dataset.get());
    REQUIRE(report != nullptr);
    require_successful_api_call(handle.get());
    CHECK(PGM_validation_report_is_valid(handle.get(), report.get()) == 1);
    require_successful_api_call(handle.get());

    auto const* data = PGM_validation_report_get_to_zero_terminated_string(handle.get(), report.get());
    REQUIRE(data != nullptr);
    require_successful_api_call(handle.get());
    CHECK(nlohmann::json::parse(data)["validation_type"] == "input");
}

TEST_CASE_FIXTURE(ValidationApiFixture, "Input validation reports nonpositive u_rated") {
    PGM_ID node_id = 3;
    double u_rated = 0.0;
    auto const dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);

    auto const report = validate(dataset.get());
    REQUIRE(report != nullptr);
    CHECK(PGM_validation_report_is_valid(handle.get(), report.get()) == 0);
    require_successful_api_call(handle.get());

    auto const* data = PGM_validation_report_get_to_zero_terminated_string(handle.get(), report.get());
    REQUIRE(data != nullptr);
    require_successful_api_call(handle.get());
    CHECK(nlohmann::json::parse(data) == nlohmann::json::parse(R"json(
        {
            "validation_type": "input",
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

TEST_CASE_FIXTURE(ValidationApiFixture, "Batch validation reports base-input findings independently") {
    PGM_ID node_id = 4;
    double u_rated = 110.0;
    auto const valid_dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);

    PGM_ID line_id = 10;
    std::int8_t invalid_from_status = 2;
    std::int8_t valid_to_status = 1;
    double r1 = 1.0;
    auto const batch_input_dataset =
        create_single_line_input_dataset(handle.get(), &line_id, &invalid_from_status, &valid_to_status, &r1);
    std::vector<PGM_ID> const update_line_ids{line_id};
    std::vector<std::int8_t> const update_from_status{0};
    std::vector<std::int8_t> const update_to_status{1};
    auto const update_dataset =
        create_batch_line_update_dataset(handle.get(), update_line_ids, update_from_status, update_to_status);

    auto const input_report = validate(valid_dataset.get());
    REQUIRE(input_report != nullptr);
    require_successful_api_call(handle.get());
    auto const batch_report = validate(batch_input_dataset.get(), update_dataset.get());
    REQUIRE(batch_report != nullptr);
    require_successful_api_call(handle.get());

    CHECK(PGM_validation_report_is_valid(handle.get(), input_report.get()) == 1);
    require_successful_api_call(handle.get());
    CHECK(PGM_validation_report_is_valid(handle.get(), batch_report.get()) == 0);
    require_successful_api_call(handle.get());

    auto const* input_data = PGM_validation_report_get_to_zero_terminated_string(handle.get(), input_report.get());
    REQUIRE(input_data != nullptr);
    require_successful_api_call(handle.get());
    auto const* batch_data = PGM_validation_report_get_to_zero_terminated_string(handle.get(), batch_report.get());
    REQUIRE(batch_data != nullptr);
    require_successful_api_call(handle.get());

    CHECK(nlohmann::json::parse(input_data)["validation_type"] == "input");
    CHECK(nlohmann::json::parse(batch_data) == nlohmann::json::parse(R"json(
        {
            "validation_type": "batch",
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

TEST_CASE_FIXTURE(ValidationApiFixture, "Batch validation reports findings for update scenarios (not implemented yet)" *
                                            doctest::may_fail()) {
    PGM_ID line_id = 10;
    std::int8_t valid_from_status = 1;
    std::int8_t valid_to_status = 1;
    double r1 = 1.0;
    auto const input_dataset =
        create_single_line_input_dataset(handle.get(), &line_id, &valid_from_status, &valid_to_status, &r1);

    std::vector<PGM_ID> const update_line_ids{10, 10};
    std::vector<std::int8_t> const update_from_status{1, 2};
    std::vector<std::int8_t> const update_to_status{1, 1};
    auto const update_dataset =
        create_batch_line_update_dataset(handle.get(), update_line_ids, update_from_status, update_to_status);

    auto const report = validate(input_dataset.get(), update_dataset.get());
    REQUIRE(report != nullptr);
    require_successful_api_call(handle.get());
    CHECK(PGM_validation_report_is_valid(handle.get(), report.get()) == 0);
    require_successful_api_call(handle.get());

    auto const* data = PGM_validation_report_get_to_zero_terminated_string(handle.get(), report.get());
    REQUIRE(data != nullptr);
    require_successful_api_call(handle.get());
    CHECK(nlohmann::json::parse(data) == nlohmann::json::parse(R"json(
        {
            "validation_type": "batch",
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
