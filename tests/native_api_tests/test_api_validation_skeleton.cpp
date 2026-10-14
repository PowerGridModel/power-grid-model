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

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

using CInputValidationFunction = bool (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_input_data), CInputValidationFunction>);
using CBatchValidationFunction = bool (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_ConstDataset const*,
                                          PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_batch_data), CBatchValidationFunction>);
using CValidationReportGetJsonFunction = void (*)(PGM_Handle*, char const**, PGM_Idx*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validation_report_get_json), CValidationReportGetJsonFunction>);

TEST_CASE("Data validation requires experimental features") {
    PGM_Handle* handle = PGM_create_handle();
    PGM_Options* options = PGM_create_options(handle);

    CHECK_FALSE(PGM_validate_input_data(handle, nullptr, options));
    CHECK_FALSE(PGM_validate_batch_data(handle, nullptr, nullptr, options));

    CHECK(PGM_error_code(handle) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle)}.find("experimental features") != std::string_view::npos);

    PGM_destroy_options(options);
    PGM_destroy_handle(handle);
}

TEST_CASE("Input validation functions return report validity") {
    PGM_Handle* handle = PGM_create_handle();
    REQUIRE(handle != nullptr);
    PGM_Options* options = PGM_create_options(handle);
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle, options, PGM_experimental_features_enabled);

    PGM_ID valid_id = 1;
    double valid_u_rated = 110.0;
    PGM_ConstDataset* valid_dataset = PGM_create_dataset_const(handle, "input", 0, 1);
    REQUIRE(valid_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle, valid_dataset, "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle, valid_dataset, "node", "id", &valid_id);
    PGM_dataset_const_add_attribute_buffer(handle, valid_dataset, "node", "u_rated", &valid_u_rated);
    CHECK(PGM_validate_input_data(handle, valid_dataset, options));
    CHECK(PGM_error_code(handle) == PGM_no_error);

    PGM_ID invalid_id = 2;
    double invalid_u_rated = 0.0;
    PGM_ConstDataset* invalid_dataset = PGM_create_dataset_const(handle, "input", 0, 1);
    REQUIRE(invalid_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle, invalid_dataset, "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle, invalid_dataset, "node", "id", &invalid_id);
    PGM_dataset_const_add_attribute_buffer(handle, invalid_dataset, "node", "u_rated", &invalid_u_rated);
    CHECK_FALSE(PGM_validate_input_data(handle, invalid_dataset, options));
    CHECK(PGM_error_code(handle) == PGM_no_error);

    PGM_destroy_dataset_const(valid_dataset);
    PGM_destroy_dataset_const(invalid_dataset);
    PGM_destroy_options(options);
    PGM_destroy_handle(handle);
}

TEST_CASE("Validation JSON is returned through the C API") {
    PGM_Handle* handle = PGM_create_handle();
    REQUIRE(handle != nullptr);
    PGM_Options* options = PGM_create_options(handle);
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle, options, PGM_experimental_features_enabled);

    PGM_ID node_id = 3;
    double u_rated = 0.0;
    PGM_ConstDataset* dataset = PGM_create_dataset_const(handle, "input", 0, 1);
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle, dataset, "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle, dataset, "node", "id", &node_id);
    PGM_dataset_const_add_attribute_buffer(handle, dataset, "node", "u_rated", &u_rated);

    CHECK_FALSE(PGM_validate_input_data(handle, dataset, options));
    char const* data{};
    PGM_Idx size{};

    PGM_validation_report_get_json(handle, &data, &size);

    REQUIRE(data != nullptr);
    auto const report = nlohmann::json::parse(std::string{data, static_cast<std::size_t>(size)});
    CHECK(report["valid"] == false);
    CHECK(report["validation_type"] == "input");
    REQUIRE(report["results"].size() == 1);
    CHECK(report["results"][0]["scenario"].is_null());
    CHECK_FALSE(report["results"][0].contains("status"));
    REQUIRE(report["results"][0]["issues"].size() == 1);
    CHECK(report["results"][0]["issues"][0]["rule"] == "not_greater_than_zero");
    CHECK(report["results"][0]["issues"][0]["component"] == "node");
    CHECK(report["results"][0]["issues"][0]["field"] == "u_rated");
    CHECK(report["results"][0]["issues"][0]["ids"][0] == 3);

    data = "stale";
    size = 1;
    PGM_clear_validation_report(handle);
    PGM_validation_report_get_json(handle, &data, &size);
    CHECK(data == nullptr);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle)}.find("No validation report is available") !=
          std::string_view::npos);

    PGM_destroy_dataset_const(dataset);
    PGM_destroy_options(options);
    PGM_destroy_handle(handle);
}
