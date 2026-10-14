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
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

using CInputValidationFunction = PGM_Idx (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_input_data), CInputValidationFunction>);
using CBatchValidationFunction = PGM_Idx (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_ConstDataset const*,
                                             PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_batch_data), CBatchValidationFunction>);
using CValidationReportGetJsonFunction = void (*)(PGM_Handle*, char const**, PGM_Idx*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validation_report_get_json), CValidationReportGetJsonFunction>);

using OwnedHandle = std::unique_ptr<PGM_Handle, decltype(&PGM_destroy_handle)>;
using OwnedOptions = std::unique_ptr<PGM_Options, decltype(&PGM_destroy_options)>;
using OwnedConstDataset = std::unique_ptr<PGM_ConstDataset, decltype(&PGM_destroy_dataset_const)>;

TEST_CASE("Data validation requires experimental features") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_disabled);

    SUBCASE("input validation") {
        CHECK(PGM_validate_input_data(handle.get(), nullptr, options.get()) == 0);
        CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
        CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental features") !=
              std::string_view::npos);
    }

    SUBCASE("batch validation") {
        CHECK(PGM_validate_batch_data(handle.get(), nullptr, nullptr, options.get()) == 0);
        CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
        CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental features") !=
              std::string_view::npos);
    }
}

TEST_CASE("Input validation functions return report validity") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);

    PGM_ID valid_id = 1;
    double valid_u_rated = 110.0;
    auto const valid_dataset =
        OwnedConstDataset{PGM_create_dataset_const(handle.get(), "input", 0, 1), &PGM_destroy_dataset_const};
    REQUIRE(valid_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), valid_dataset.get(), "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), valid_dataset.get(), "node", "id", &valid_id);
    PGM_dataset_const_add_attribute_buffer(handle.get(), valid_dataset.get(), "node", "u_rated", &valid_u_rated);
    CHECK(PGM_validate_input_data(handle.get(), valid_dataset.get(), options.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    PGM_ID invalid_id = 2;
    double invalid_u_rated = 0.0;
    auto const invalid_dataset =
        OwnedConstDataset{PGM_create_dataset_const(handle.get(), "input", 0, 1), &PGM_destroy_dataset_const};
    REQUIRE(invalid_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), invalid_dataset.get(), "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), invalid_dataset.get(), "node", "id", &invalid_id);
    PGM_dataset_const_add_attribute_buffer(handle.get(), invalid_dataset.get(), "node", "u_rated", &invalid_u_rated);
    CHECK(PGM_validate_input_data(handle.get(), invalid_dataset.get(), options.get()) == 1);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Validation JSON is returned through the C API") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);

    PGM_ID node_id = 3;
    double u_rated = 0.0;
    auto const dataset =
        OwnedConstDataset{PGM_create_dataset_const(handle.get(), "input", 0, 1), &PGM_destroy_dataset_const};
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "id", &node_id);
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "u_rated", &u_rated);

    CHECK(PGM_validate_input_data(handle.get(), dataset.get(), options.get()) == 1);
    char const* data{};
    PGM_Idx size{};

    PGM_validation_report_get_json(handle.get(), &data, &size);

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

    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_disabled);
    CHECK(PGM_validate_input_data(handle.get(), dataset.get(), options.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental features") != std::string_view::npos);

    data = "stale";
    size = 1;
    PGM_validation_report_get_json(handle.get(), &data, &size);
    CHECK(data == nullptr);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle.get())}.find("No validation report is available") !=
          std::string_view::npos);

    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);
    CHECK(PGM_validate_input_data(handle.get(), dataset.get(), options.get()) == 1);
    data = "stale";
    size = 1;
    PGM_clear_validation_report(handle.get());
    PGM_validation_report_get_json(handle.get(), &data, &size);
    CHECK(data == nullptr);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle.get())}.find("No validation report is available") !=
          std::string_view::npos);
}
