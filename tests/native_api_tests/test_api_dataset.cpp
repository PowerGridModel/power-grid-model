// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "load_dataset.hpp"

#include <power_grid_model_c/basics.h>
#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/model.hpp>
#include <power_grid_model_cpp/options.hpp>

#include <functional>
#include <initializer_list>
#include <set>
#include <string>
#include <string_view>

#include <doctest/doctest.h>

namespace power_grid_model_cpp {
namespace {
using power_grid_model_cpp_test::load_dataset;
constexpr auto const json_data = R"({
        "version": "1.0",
        "type": "input",
        "is_batch": false,
        "attributes": {},
        "data": {
            "node": [
                {"id": 1, "u_rated": 100},
                {"id": 2, "u_rated": 100}
            ],
            "source": [{"id": 3, "node": 1, "status": 1, "u_ref": 1.0, "sk": 1000}],
            "transformer": [
                {"id": 4, "from_node": 1, "to_node": 2, "from_status": 1, "to_status": 1, "u1": 100, "u2": 100, "sn": 1000, "uk": 0.01, "pk": 0.0, "i0": 0.0, "p0": 0.0, "winding_from": 0, "winding_to": 1, "clock": 0, "tap_side": 0, "tap_size": 5, "tap_min": -10, "tap_max": 10}
            ],
            "sym_power_sensor": [
                 {"id": 5, "measured_object": 4, "measured_terminal_type": 0, "p_measured": 0.0, "q_measured": 0.0, "power_sigma": 1.0}
            ],
            "sym_voltage_sensor": [
                 {"id": 6, "measured_object": 1, "measured_terminal_type": 9, "u_measured": 1.0, "u_sigma": 1.0}
            ],
            "asym_current_sensor": [
                 {"id": 7, "measured_object": 4, "measured_terminal_type": 1, "angle_measurement_type": 0, "i_measured": [0.0, 0.0, 0.0], "i_angle_measurement": [0.0, 0.0, 0.0], "i_sigma": 1.0, "i_angle_sigma": 1.0}
            ],
            "sym_gen": [
                {"id": 8, "node": 2, "status": 1, "type": 0, "p_specified": 20.0, "q_specified": 10.0}
            ],
            "voltage_regulator": [
                {"id": 9, "regulated_object": 8, "status": 1, "u_ref": 1.0}
            ]
        }
    })"; // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
} // namespace
TEST_CASE("Test get_output_type") {
    using namespace std::string_literals;
    SUBCASE("Power flow") {
        CHECK(get_output_type(PGM_power_flow, true) ==
              "sym_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
        CHECK(get_output_type(PGM_power_flow, false) ==
              "asym_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }
    SUBCASE("State estimation") {
        CHECK(get_output_type(PGM_state_estimation, true) ==
              "sym_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
        CHECK(get_output_type(PGM_state_estimation, false) ==
              "asym_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }
    SUBCASE("Short circuit") {
        CHECK(get_output_type(PGM_short_circuit, true) ==
              "sc_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
        CHECK(get_output_type(PGM_short_circuit, false) ==
              "sc_output"s); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }
}

TEST_CASE("Test get_irrelevant_components") {
    using namespace std::string_literals;
    SUBCASE("Power flow") {
        auto const component_list = std::set<std::string, std::less<>>{
            "sym_voltage_sensor"s,
            "sym_current_sensor"s,
            "sym_power_sensor"s,
            "asym_voltage_sensor"s,
            "asym_current_sensor"s,
            "asym_power_sensor"s,
            "fault"s}; // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
        CHECK(component_list == get_irrelevant_components(PGM_power_flow));
    }
    SUBCASE("State estimation") {
        auto const component_list = std::set<std::string, std::less<>>{
            "fault"s, "transformer_tap_regulator"s,
            "voltage_regulator"s}; // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
        CHECK(component_list == get_irrelevant_components(PGM_state_estimation));
    }

    SUBCASE("Short circuit") {

        auto const component_list = std::set<std::string, std::less<>>{
            "sym_voltage_sensor"s,        "sym_current_sensor"s,  "sym_power_sensor"s,
            "asym_voltage_sensor"s,       "asym_current_sensor"s, "asym_power_sensor"s,
            "transformer_tap_regulator"s, "voltage_regulator"s}; // NOLINT(misc-include-cleaner)
                                                                 // https://github.com/llvm/llvm-project/issues/98122
        CHECK(component_list == get_irrelevant_components(PGM_short_circuit));
    }
}

TEST_CASE("OwningDataset - filter irrelevant components") {
    using namespace std::string_literals;
    auto const input_dataset = load_dataset(json_data);
    auto options = Options{};
    Model model{50.0, input_dataset.dataset};

    auto check_irrelevant_components = [](DatasetInfo const& info,
                                          std::initializer_list<std::string_view> irrelevant_components) {
        auto const expected_component_set = std::set<std::string_view>{irrelevant_components};
        for (auto idx = 0; idx < info.n_components(); ++idx) {
            auto const component_name = info.component_name(idx);
            CHECK(!expected_component_set.contains(component_name));
        }
    };

    SUBCASE("Power flow filters out faults and sensors") {
        auto const output_dataset = OwningDataset{input_dataset, PGM_power_flow, true};
        options.set_calculation_type(PGM_power_flow);
        options.set_calculation_method(PGM_newton_raphson);

        CHECK_NOTHROW(model.calculate(options, output_dataset.dataset));
        auto const& info = output_dataset.dataset.get_info();
        check_irrelevant_components(
            info,
            {"fault"s, "sym_power_sensor"s, "sym_voltage_sensor"s,
             "asym_current_sensor"s}); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }

    SUBCASE("State estimation filters out faults") {
        auto const output_dataset = OwningDataset{input_dataset, PGM_state_estimation, true};
        options.set_calculation_type(PGM_state_estimation);
        options.set_calculation_method(PGM_newton_raphson);

        CHECK_NOTHROW(model.calculate(options, output_dataset.dataset));
        auto const& info = output_dataset.dataset.get_info();
        check_irrelevant_components(
            info,
            {"fault"s,
             "voltage_regulator"s}); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }

    SUBCASE("Short circuit filters out sensors") {
        auto const output_dataset = OwningDataset{input_dataset, PGM_short_circuit, true};
        options.set_calculation_type(PGM_short_circuit);
        options.set_calculation_method(PGM_iec60909);

        CHECK_NOTHROW(model.calculate(options, output_dataset.dataset));
        auto const& info = output_dataset.dataset.get_info();
        check_irrelevant_components(
            info,
            {"sym_power_sensor"s, "sym_voltage_sensor"s, "asym_current_sensor"s,
             "voltage_regulator"s}); // NOLINT(misc-include-cleaner) https://github.com/llvm/llvm-project/issues/98122
    }
}
} // namespace power_grid_model_cpp
