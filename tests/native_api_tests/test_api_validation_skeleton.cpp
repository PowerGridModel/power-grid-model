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

using CValidationFunction = PGM_Idx (*)(PGM_Handle*, PGM_ConstDataset const*, PGM_ConstDataset const*,
                                        PGM_Options const*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validate_data), CValidationFunction>);
using CValidationReportGetJsonFunction = void (*)(PGM_Handle*, char const**, PGM_Idx*) noexcept;
static_assert(std::is_same_v<decltype(&PGM_validation_report_get_json), CValidationReportGetJsonFunction>);

namespace {
using OwnedHandle = std::unique_ptr<PGM_Handle, decltype(&PGM_destroy_handle)>;
using OwnedOptions = std::unique_ptr<PGM_Options, decltype(&PGM_destroy_options)>;
using OwnedConstDataset = std::unique_ptr<PGM_ConstDataset, decltype(&PGM_destroy_dataset_const)>;

OwnedConstDataset create_single_node_input_dataset(PGM_Handle* handle, PGM_ID const* node_id, double const* u_rated) {
    auto dataset = OwnedConstDataset{PGM_create_dataset_const(handle, "input", 0, 1), &PGM_destroy_dataset_const};

    PGM_dataset_const_add_buffer(handle, dataset.get(), "node", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "node", "id", node_id);
    PGM_dataset_const_add_attribute_buffer(handle, dataset.get(), "node", "u_rated", u_rated);
    REQUIRE(dataset != nullptr);
    return dataset;
}

void check_validation_report_unavailable(PGM_Handle* handle) {
    char const* data = "stale";
    PGM_Idx size = 1;
    PGM_validation_report_get_json(handle, &data, &size);
    CHECK(data == nullptr);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle)}.find("No validation report is available") !=
          std::string_view::npos);
}
} // namespace

TEST_CASE("Data validation requires experimental features") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_disabled);

    CHECK(PGM_validate_data(handle.get(), nullptr, nullptr, options.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
    CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental features") != std::string_view::npos);
}

TEST_CASE("Input validation returns report validity") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);

    PGM_ID valid_id = 1;
    double valid_u_rated = 110.0;
    auto const valid_dataset = create_single_node_input_dataset(handle.get(), &valid_id, &valid_u_rated);

    PGM_ID invalid_id = 2;
    double invalid_u_rated = 0.0;
    auto const invalid_dataset = create_single_node_input_dataset(handle.get(), &invalid_id, &invalid_u_rated);

    SUBCASE("valid input returns zero") {
        CHECK(PGM_validate_data(handle.get(), valid_dataset.get(), nullptr, options.get()) == 0);
        CHECK(PGM_error_code(handle.get()) == PGM_no_error);
        char const* data{};
        PGM_Idx size{};
        PGM_validation_report_get_json(handle.get(), &data, &size);

        REQUIRE(data != nullptr);
        auto const report = nlohmann::json::parse(std::string{data, static_cast<std::size_t>(size)});
        CHECK(report["validation_type"] == "input");
    }

    SUBCASE("invalid input returns one") {
        CHECK(PGM_validate_data(handle.get(), invalid_dataset.get(), nullptr, options.get()) == 1);
        CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    }
}

TEST_CASE("Validation JSON is returned through the C API") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);

    PGM_ID node_id = 3;
    double u_rated = 0.0;
    auto const dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);

    SUBCASE("validation findings are serialized") {
        CHECK(PGM_validate_data(handle.get(), dataset.get(), nullptr, options.get()) == 1);
        char const* data{};
        PGM_Idx size{};

        PGM_validation_report_get_json(handle.get(), &data, &size);

        REQUIRE(data != nullptr);
        auto const report = nlohmann::json::parse(std::string{data, static_cast<std::size_t>(size)});
        CHECK(report == nlohmann::json::parse(R"json(
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

    SUBCASE("API errors clear an existing validation report") {
        CHECK(PGM_validate_data(handle.get(), dataset.get(), nullptr, options.get()) == 1);
        PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_disabled);
        CHECK(PGM_validate_data(handle.get(), dataset.get(), nullptr, options.get()) == 0);
        CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
        CHECK(std::string_view{PGM_error_message(handle.get())}.find("experimental features") !=
              std::string_view::npos);

        check_validation_report_unavailable(handle.get());
    }

    SUBCASE("clearing the validation report makes it unavailable") {
        CHECK(PGM_validate_data(handle.get(), dataset.get(), nullptr, options.get()) == 1);
        PGM_clear_validation_report(handle.get());

        check_validation_report_unavailable(handle.get());
    }
}

TEST_CASE("Batch validation is selected when an update dataset is supplied") {
    auto const handle = OwnedHandle{PGM_create_handle(), &PGM_destroy_handle};
    REQUIRE(handle != nullptr);
    auto const options = OwnedOptions{PGM_create_options(handle.get()), &PGM_destroy_options};
    REQUIRE(options != nullptr);
    PGM_set_experimental_features(handle.get(), options.get(), PGM_experimental_features_enabled);

    PGM_ID node_id = 4;
    double u_rated = 110.0;
    auto const input_dataset = create_single_node_input_dataset(handle.get(), &node_id, &u_rated);
    auto const update_dataset =
        OwnedConstDataset{PGM_create_dataset_const(handle.get(), "update", 1, 1), &PGM_destroy_dataset_const};
    REQUIRE(update_dataset != nullptr);

    CHECK(PGM_validate_data(handle.get(), input_dataset.get(), update_dataset.get(), options.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    char const* data{};
    PGM_Idx size{};
    PGM_validation_report_get_json(handle.get(), &data, &size);

    REQUIRE(data != nullptr);
    auto const report = nlohmann::json::parse(std::string{data, static_cast<std::size_t>(size)});
    CHECK(report["validation_type"] == "batch");
}
