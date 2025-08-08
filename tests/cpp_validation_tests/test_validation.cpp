// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include <power_grid_model_cpp.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <complex>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <regex>
#include <stdexcept>
#include <type_traits>

namespace power_grid_model_cpp {
namespace {
class UnsupportedValidationCase : public PowerGridError {
  public:
    UnsupportedValidationCase(std::string const& calculation_type, bool sym)
        : PowerGridError{std::format("Unsupported validation case: {} {}", sym ? "sym" : "asym", calculation_type)} {}
};

using nlohmann::json;

auto read_json(std::filesystem::path const& path) {
    json j;
    std::ifstream f{path};
    f >> j;
    return j;
}

OwningDataset create_result_dataset(OwningDataset const& input, std::string const& dataset_name, bool is_batch = false,
                                    Idx batch_size = 1) {
    DatasetInfo const& input_info = input.dataset.get_info();

    OwningDataset result{.dataset = DatasetMutable{dataset_name, is_batch, batch_size}, .storage{}};

    for (Idx component_idx{}; component_idx != input_info.n_components(); ++component_idx) {
        auto const& component_name = input_info.component_name(component_idx);
        auto const& component_meta = MetaData::get_component_by_name(dataset_name, component_name);
        Idx const component_elements_per_scenario = input_info.component_elements_per_scenario(component_idx);
        Idx const component_size = input_info.component_total_elements(component_idx);

        auto& current_indptr = result.storage.indptrs.emplace_back(
            input_info.component_elements_per_scenario(component_idx) < 0 ? batch_size + 1 : 0);
        Idx const* const indptr = current_indptr.empty() ? nullptr : current_indptr.data();
        auto& current_buffer = result.storage.buffers.emplace_back(component_meta, component_size);
        result.dataset.add_buffer(component_name, component_elements_per_scenario, component_size, indptr,
                                  current_buffer);
    }
    return result;
}

OwningDataset load_dataset(std::filesystem::path const& path) {
// Issue in msgpack, reported in https://github.com/msgpack/msgpack-c/issues/1098
// May be a Clang Analyzer bug
#ifndef __clang_analyzer__ // TODO(mgovers): re-enable this when issue in msgpack is fixed
    auto read_file = [](std::filesystem::path const& read_file_path) {
        std::ifstream const f{read_file_path};
        std::ostringstream buffer;
        buffer << f.rdbuf();
        return buffer.str();
    };

    Deserializer deserializer{read_file(path), PGM_json};
    auto& writable_dataset = deserializer.get_dataset();
    auto dataset = create_owning_dataset(writable_dataset);
    deserializer.parse_to_buffer();
    return dataset;
#else  // __clang_analyzer__ // issue in msgpack
    (void)path;
    // fallback for https://github.com/msgpack/msgpack-c/issues/1098
    return OwningDataset{.dataset{"Empty dataset", false, Idx{1}}};
#endif // __clang_analyzer__ // issue in msgpack
}

template <typename T> std::string get_as_string(T const& attribute_value) {
    std::stringstream sstr;
    sstr << std::setprecision(16);
    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        sstr << "(" << attribute_value[0] << ", " << attribute_value[1] << ", " << attribute_value[2] << ")";
    } else if constexpr (std::is_same_v<std::decay_t<T>, IntS>) {
        sstr << std::to_string(attribute_value);
    } else {
        sstr << attribute_value;
    }
    return sstr.str();
}

class Subcase {
    class RaisesFailed : public PowerGridError {
      public:
        RaisesFailed(std::string_view message) : PowerGridError{std::format("Failed raises check: {}", message)} {}
    };

  public:
    Subcase(std::optional<std::string> raises, std::optional<std::string> xfail)
        : raises_{std::move(raises)}, xfail_raises_{std::move(xfail)} {}

    void check_message(bool statement, std::string_view message) {
        if (statement || xfail_raises_ != "AssertionError") {
            CHECK_MESSAGE(statement, message);
        } else {
            // delay the check to the end
            has_failing_assertion = true;
            CHECK_MESSAGE(!statement, std::format("xfailed assertion: {}", message));
        }
    }

    // This is the entrypoint for executing a case.
    // It was opted to go for a more Pythonic way to implement this, rather than a C++-style way.
    // This was done to keep the similarity between the Python and C++ validation tests.
    // The structure of this function is similar to how pytest.raises and pytest.mark.xfail work.
    template <typename T>
        requires std::invocable<std::remove_cvref_t<T>, Subcase&>
    void execute_case(T&& statement) noexcept {
        CHECK_NOTHROW(maybe_mark_xfail(maybe_with_raises(std::forward<T>(statement)))(*this));
    }

  private:
    std::optional<std::string> raises_{};
    std::optional<std::string> xfail_raises_{};
    bool has_failing_assertion{};

    template <typename T>
        requires std::invocable<std::remove_cvref_t<T>, Subcase&>
    auto maybe_with_raises(T&& statement) noexcept {
        return [this, statement_ = std::forward<T>(statement)](Subcase& subcase) {
            if (!raises_) {
                return statement_(subcase);
            }
            try {
                statement_(subcase);
                throw RaisesFailed{std::format(
                    "Test case marked as raises with message '{}' but no exception was thrown", raises_.value())};
            } catch (std::exception const& e) {
                if (match_exception(e, raises_.value())) {
                    // correct exception raised => pass
                    subcase.has_failing_assertion = false; // assertions may fail when an exception is raised
                } else {
                    throw;
                }
            }
        };
    }

    template <typename T>
        requires std::invocable<std::remove_cvref_t<T>, Subcase&>
    auto maybe_mark_xfail(T&& statement) noexcept {
        return [this, statement_ = std::forward<T>(statement)](Subcase& subcase) {
            if (!xfail_raises_) {
                return statement_(subcase);
            }
            try {
                statement_(subcase);
                bool const xfailed = subcase.has_failing_assertion;
                CHECK_MESSAGE(xfailed, "XPASS");
            } catch (std::exception const& e) {
                subcase.check_message(match_exception(e, xfail_raises_.value()),
                                      std::format("Test case marked as xfail with message '{}' but got exception: {}",
                                                  xfail_raises_.value(), e.what()));
            }
        };
    }

    static bool match_exception(std::exception const& e, std::string_view message) {
        // error mapping; similar to src/power_grid_model/_core/error_handling.py
        std::map<std::string, std::regex, std::less<>> const error_mapping = {
            {"MissingCaseForEnumError", std::regex{" is not implemented for (.+) #(-?\\d+)!\n"}},
            {"InvalidArguments", std::regex{" is not implemented for "}}, // multiple different flavors
            {"ConflictVoltage",
             std::regex{"Conflicting voltage for line (-?\\d+)\n voltage at from node (-?\\d+) is (.*)\n "
                        "voltage at to node (-?\\d+) is (.*)\n"}},
            {"InvalidBranch",
             std::regex{"Branch (-?\\d+) has the same from- and to-node (-?\\d+),\n This is not allowed!\n"}},
            {"InvalidBranch3",
             std::regex{"Branch3 (-?\\d+) is connected to the same node at least twice. Node 1\\/2\\/3: "
                        "(-?\\d+)\\/(-?\\d+)\\/(-?\\d+),\n This is not allowed!\n"}},
            {"InvalidTransformerClock", std::regex{"Invalid clock for transformer (-?\\d+), clock (-?\\d+)\n"}},
            {"SparseMatrixError", std::regex{"Sparse matrix error"}}, // multiple different flavors
            {"NotObservableError", std::regex{"Not enough measurements available for state estimation.\n"}},
            {"IterationDiverge", std::regex{"Iteration failed to converge"}}, // potentially multiple different flavors
            {"MaxIterationReached", std::regex{"Maximum number of iterations reached"}},
            {"ConflictID", std::regex{"Conflicting id detected: (-?\\d+)\n"}},
            {"IDNotFound", std::regex{"The id cannot be found: (-?\\d+)\n"}},
            {"InvalidMeasuredObject", std::regex{"(\\w+) measurement is not supported for object of type (\\w+)"}},
            {"InvalidRegulatedObject",
             std::regex{"(\\w+) regulator is not supported for object "}}, // potentially multiple different flavors
            {"AutomaticTapCalculationError",
             std::regex{
                 "Automatic tap changing regulator with tap_side at LV side is not supported. Found at id (-?\\d+)\n"}},
            {"AutomaticTapInputError", std::regex{"Automatic tap changer has invalid configuration"}},
            {"IDWrongType", std::regex{"Wrong type for object with id (-?\\d+)\n"}},
            {"ConflictingAngleMeasurementType", std::regex{"Conflicting angle measurement type"}},
            {"InvalidCalculationMethod", std::regex{"The calculation method is invalid for this calculation!"}},
            {"InvalidShortCircuitPhaseOrType", std::regex{"short circuit type"}}, // multiple different flavors
            {"TapStrategySearchIncompatible", std::regex{"Search method is incompatible with optimization strategy: "}},
            {"PowerGridDatasetError", std::regex{"Dataset error: "}}, // multiple different flavors
            {"PowerGridUnreachableHit",
             std::regex{"Unreachable code hit when executing "}}, // multiple different flavors
            {"PowerGridNotImplementedError",
             std::regex{"The functionality is either not supported or not yet implemented!"}},
            {"Failed", std::regex{"Failed raises check: (.*)"}} // for xfail cases
        };

        if (auto const it = error_mapping.find(message); it != error_mapping.end()) {
            return regex_search(e.what(), it->second);
        }
        return false;
    }
};

template <typename T>
bool check_angle_and_magnitude(T const& ref_angle, T const& angle, T const& ref_magnitude, T const& magnitude,
                               double atol, double rtol) {
    auto to_complex = [](double r, double theta) { return std::polar(r, theta); };
    auto is_within_tolerance = [atol, rtol](std::complex<double> element, std::complex<double> ref_element) {
        return std::abs(element - ref_element) < std::abs(ref_element) * rtol + atol;
    };

    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        std::array<std::complex<double>, 3> result;
        std::array<std::complex<double>, 3> ref_result;
        std::ranges::transform(magnitude, angle, result.begin(), to_complex);
        std::ranges::transform(ref_magnitude, ref_angle, ref_result.begin(), to_complex);
        return std::ranges::equal(result, ref_result, is_within_tolerance);
    }
    if constexpr (std::is_same_v<std::decay_t<T>, double>) {
        std::complex<double> const result = to_complex(magnitude, angle);
        std::complex<double> const ref_result = to_complex(ref_magnitude, ref_angle);
        return is_within_tolerance(result, ref_result);
    } else {
        return ref_angle == angle && ref_magnitude == magnitude;
    }
}

template <typename T>
bool compare_value(T const& ref_attribute_value, T const& attribute_value, double atol, double rtol) {
    auto is_within_tolerance = [atol, rtol](std::complex<double> element, std::complex<double> ref_element) {
        return std::abs(element - ref_element) < std::abs(ref_element) * rtol + atol;
    };

    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        return std::ranges::equal(attribute_value, ref_attribute_value, is_within_tolerance);
    } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
        return is_within_tolerance(attribute_value, ref_attribute_value);
    } else {
        return ref_attribute_value == attribute_value;
    }
}

template <typename T>
void check_results(T const& ref_attribute_value, T const& attribute_value, T const& ref_possible_attribute_value,
                   T const& possible_attribute_value, bool const& is_angle, Idx scenario_idx, Idx obj,
                   std::string const& component_name, std::string const& attribute_name, double const& dynamic_atol,
                   double const& rtol, Subcase& subcase) {
    bool const match =
        is_angle
            ? check_angle_and_magnitude<decltype(ref_attribute_value)>(ref_attribute_value, attribute_value,
                                                                       ref_possible_attribute_value,
                                                                       possible_attribute_value, dynamic_atol, rtol)
            : compare_value<decltype(ref_attribute_value)>(ref_attribute_value, attribute_value, dynamic_atol, rtol);
    if (!match) {
        auto const case_str = std::format(
            "Dataset scenario: #{} Component: {} #{} attribute: {}: actual = {} vs. expected = {}", scenario_idx,
            component_name, obj, attribute_name, get_as_string<decltype(attribute_value)>(attribute_value),
            get_as_string<decltype(ref_attribute_value)>(ref_attribute_value));
        subcase.check_message(match, case_str);
    }
}

template <typename T>
bool skip_check(T const& ref_attribute_value, bool const is_angle, T const& ref_possible_attribute_value) {
    // check attributes. For angle attribute, also check magnitude available
    return is_nan(ref_attribute_value) || (is_angle && is_nan(ref_possible_attribute_value));
}

template <typename T>
void check_individual_attribute(Buffer const& buffer, Buffer const& ref_buffer,
                                MetaAttribute const* const attribute_meta,
                                MetaAttribute const* const possible_attr_meta, bool const& is_angle,
                                Idx elements_per_scenario, Idx scenario_idx, Idx obj, std::string const& component_name,
                                std::string const& attribute_name, double const& dynamic_atol, double const& rtol,
                                Subcase& subcase) {
    Idx idx = (elements_per_scenario * scenario_idx) + obj;
    // get attribute values to check
    T ref_attribute_value{};
    T attribute_value{};
    T ref_possible_attribute_value{};
    T possible_attribute_value{};
    auto get_values = [&ref_buffer, &buffer, &attribute_meta, &possible_attr_meta,
                       idx]<typename U>(U* ref_value, U* value, U* ref_possible_value, U* possible_value) {
        ref_buffer.get_value(attribute_meta, ref_value, idx, 0);
        buffer.get_value(attribute_meta, value, idx, 0);
        ref_buffer.get_value(possible_attr_meta, ref_possible_value, idx, 0);
        buffer.get_value(possible_attr_meta, possible_value, idx, 0);
    };
    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        get_values(ref_attribute_value.data(), attribute_value.data(), ref_possible_attribute_value.data(),
                   possible_attribute_value.data());
    } else {
        get_values(&ref_attribute_value, &attribute_value, &ref_possible_attribute_value, &possible_attribute_value);
    }

    if (!skip_check(ref_attribute_value, is_angle, ref_possible_attribute_value)) {
        check_results(ref_attribute_value, attribute_value, ref_possible_attribute_value, possible_attribute_value,
                      is_angle, scenario_idx, obj, component_name, attribute_name, dynamic_atol, rtol, subcase);
    }
}

void assert_result(OwningDataset const& owning_result, OwningDataset const& owning_reference_result,
                   std::map<std::string, double, std::less<>> atol, double rtol, Subcase& subcase) {
    using namespace std::string_literals;

    DatasetConst const result{owning_result.dataset};
    auto const& result_info = result.get_info();
    auto const& result_name = result_info.name();
    Idx const result_batch_size = result_info.batch_size();
    auto const& storage = owning_result.storage;

    DatasetConst const& reference_result = owning_reference_result.dataset;
    auto const& reference_result_info = reference_result.get_info();
    auto const& reference_result_name = reference_result_info.name();
    auto const& reference_storage = owning_reference_result.storage;
    subcase.check_message(storage.buffers.size() == reference_storage.buffers.size(),
                          std::format("Buffer size mismatch: actual {}, expected {}", storage.buffers.size(),
                                      reference_storage.buffers.size()));

    // loop through all scenarios
    for (Idx scenario_idx{}; scenario_idx < result_batch_size; ++scenario_idx) {
        // loop through all components
        for (Idx component_idx{}; component_idx < reference_result_info.n_components(); ++component_idx) {
            auto const& component_name = reference_result_info.component_name(component_idx);
            auto const* const component_meta = MetaData::get_component_by_name(reference_result_name, component_name);

            auto const& ref_buffer = reference_storage.buffers.at(component_idx);
            auto const& buffer = storage.buffers.at(component_idx);
            Idx const elements_per_scenario = reference_result_info.component_elements_per_scenario(component_idx);
            subcase.check_message(elements_per_scenario >= 0,
                                  std::format("elements_per_scenario < 0: actual {}", elements_per_scenario));
            // loop through all attributes
            for (Idx attribute_idx{}; attribute_idx < MetaData::n_attributes(component_meta); ++attribute_idx) {
                auto const* const attribute_meta = MetaData::get_attribute_by_idx(component_meta, attribute_idx);
                auto attribute_type = MetaData::attribute_ctype(attribute_meta);
                auto const& attribute_name = MetaData::attribute_name(attribute_meta);
                // TODO need a way for common angle: u angle skipped for now
                if (attribute_name == "u_angle"s) {
                    continue;
                }
                // get absolute tolerance
                double dynamic_atol = atol.at("default");
                for (auto const& [reg, value] : atol) {
                    if (std::regex_match(attribute_name, std::regex{reg})) {
                        dynamic_atol = value;
                        break;
                    }
                }
                // for other _angle attribute, we need to find the magnitue and compare together
                std::regex const angle_regex("(.*)(_angle)");
                std::smatch angle_match;
                bool const is_angle = std::regex_match(attribute_name, angle_match, angle_regex);
                std::string const magnitude_name = angle_match[1];
                auto const& possible_attr_meta =
                    is_angle ? MetaData::get_attribute_by_name(reference_result_name, component_name, magnitude_name)
                             : attribute_meta;
                // loop through all objects
                for (Idx obj{}; obj < elements_per_scenario; ++obj) {
                    auto callable_wrapper = [&buffer, &ref_buffer, &attribute_meta, &possible_attr_meta, is_angle,
                                             elements_per_scenario, scenario_idx, obj, &component_name, &attribute_name,
                                             dynamic_atol, rtol, &subcase]<typename T>() {
                        check_individual_attribute<T>(buffer, ref_buffer, attribute_meta, possible_attr_meta, is_angle,
                                                      elements_per_scenario, scenario_idx, obj, component_name,
                                                      attribute_name, dynamic_atol, rtol, subcase);
                    };

                    pgm_type_func_selector(attribute_type, callable_wrapper);
                }
            }
        }
    }
}

// root path
#ifdef POWER_GRID_MODEL_VALIDATION_TEST_DATA_DIR
// use marco definition input
std::filesystem::path const data_dir{POWER_GRID_MODEL_VALIDATION_TEST_DATA_DIR};
#else
// use relative path to this file
std::filesystem::path const data_dir = std::filesystem::path{__FILE__}.parent_path().parent_path() / "data";
#endif

// method map
std::map<std::string, PGM_CalculationType, std::less<>> const calculation_type_mapping = {
    {"power_flow", PGM_power_flow}, {"state_estimation", PGM_state_estimation}, {"short_circuit", PGM_short_circuit}};
std::map<std::string, PGM_CalculationMethod, std::less<>> const calculation_method_mapping = {
    {"newton_raphson", PGM_newton_raphson},       {"linear", PGM_linear},
    {"iterative_current", PGM_iterative_current}, {"iterative_linear", PGM_iterative_linear},
    {"linear_current", PGM_linear_current},       {"iec60909", PGM_iec60909}};
std::map<std::string, PGM_ShortCircuitVoltageScaling, std::less<>> const sc_voltage_scaling_mapping = {
    {"", PGM_short_circuit_voltage_scaling_maximum}, // not provided returns default value
    {"minimum", PGM_short_circuit_voltage_scaling_minimum},
    {"maximum", PGM_short_circuit_voltage_scaling_maximum}};
std::map<std::string, PGM_TapChangingStrategy, std::less<>> const optimizer_strategy_mapping = {
    {"disabled", PGM_tap_changing_strategy_disabled},
    {"any_valid_tap", PGM_tap_changing_strategy_any_valid_tap},
    {"min_voltage_tap", PGM_tap_changing_strategy_min_voltage_tap},
    {"max_voltage_tap", PGM_tap_changing_strategy_max_voltage_tap},
    {"fast_any_tap", PGM_tap_changing_strategy_fast_any_tap}};
std::map<std::string, PGM_ExperimentalFeatures, std::less<>> const experimental_features_mapping = {
    {"disabled", PGM_experimental_features_disabled}, {"enabled", PGM_experimental_features_enabled}};

// case parameters
struct CaseParam {
    std::filesystem::path case_dir;
    std::string case_name;
    std::string calculation_type;
    std::string calculation_method;
    std::string short_circuit_voltage_scaling;
    std::string tap_changing_strategy;
    std::string experimental_features;
    double err_tol = 1e-8;
    Idx max_iter = 20;
    bool sym{};
    bool is_batch{};
    double rtol{};
    std::optional<std::string> raises{};
    std::optional<std::string> xfail{};
    std::map<std::string, double, std::less<>> atol;

    static std::string replace_backslash(std::string const& str) {
        std::string str_out{str};
        std::ranges::transform(str, str_out.begin(), [](char c) { return c == '\\' ? '/' : c; });
        return str_out;
    }
};

Options get_options(CaseParam const& param, Idx threading = -1) {
    Options options{};
    options.set_calculation_type(calculation_type_mapping.at(param.calculation_type));
    options.set_calculation_method(calculation_method_mapping.at(param.calculation_method));
    options.set_symmetric(param.sym ? PGM_symmetric : PGM_asymmetric);
    options.set_err_tol(param.err_tol);
    options.set_max_iter(param.max_iter);
    options.set_threading(threading);
    options.set_short_circuit_voltage_scaling(sc_voltage_scaling_mapping.at(param.short_circuit_voltage_scaling));
    options.set_tap_changing_strategy(optimizer_strategy_mapping.at(param.tap_changing_strategy));
    options.set_experimental_features(experimental_features_mapping.at(param.experimental_features));
    return options;
}

std::string get_output_type(std::string const& calculation_type, bool sym) {
    using namespace std::string_literals;

    if (calculation_type == "short_circuit"s) {
        if (sym) {
            throw UnsupportedValidationCase{calculation_type, sym};
        }
        return "sc_output"s;
    }
    if (sym) {
        return "sym_output"s;
    }
    return "asym_output"s;
}

std::optional<CaseParam> construct_case(std::filesystem::path const& case_dir, json const& j,
                                        std::string const& calculation_type, bool is_batch,
                                        std::string const& calculation_method, bool sym) {
    using namespace std::string_literals;

    auto const batch_suffix = is_batch ? "_batch"s : ""s;

    // add a case if output file exists
    std::filesystem::path const output_file =
        case_dir / (get_output_type(calculation_type, sym) + batch_suffix + ".json"s);
    if (!std::filesystem::exists(output_file)) {
        return std::nullopt;
    }

    CaseParam param{};
    param.case_dir = case_dir;
    param.case_name = CaseParam::replace_backslash(std::filesystem::relative(case_dir, data_dir).string());
    param.calculation_type = calculation_type;
    param.calculation_method = calculation_method;
    param.sym = sym;
    param.is_batch = is_batch;
    j.at("rtol").get_to(param.rtol);
    json const& j_atol = j.at("atol");
    if (j_atol.type() != json::value_t::object) {
        param.atol = {{"default", j_atol.get<double>()}};
    } else {
        j_atol.get_to(param.atol);
    }

    json calculation_method_params;
    calculation_method_params.update(j, true);
    if (j.contains("extra_params")) {
        if (json const& extra_params = j.at("extra_params"); extra_params.contains(calculation_method)) {
            calculation_method_params.update(extra_params.at(calculation_method), true);
        }
    }

    if (calculation_method_params.contains("raises")) {
        if (json const& raises = calculation_method_params.at("raises"); raises.contains("raises")) {
            param.raises = raises.at("raises").get<std::string>();
        }
    }
    if (calculation_method_params.contains("xfail")) {
        if (json const& xfail = calculation_method_params.at("xfail"); xfail.contains("raises")) {
            param.xfail = xfail.at("raises").get<std::string>();
        }
    }
    if (calculation_type == "short_circuit") {
        calculation_method_params.at("short_circuit_voltage_scaling").get_to(param.short_circuit_voltage_scaling);
    }

    param.tap_changing_strategy = calculation_method_params.value("tap_changing_strategy", "disabled");
    param.experimental_features = calculation_method_params.value("experimental_features", "disabled");
    param.case_name += sym ? "-sym"s : "-asym"s;
    param.case_name += "-"s + param.calculation_method;
    param.case_name += is_batch ? "_batch"s : ""s;

    return param;
}

void add_cases(std::filesystem::path const& case_dir, std::string const& calculation_type, bool is_batch,
               std::vector<CaseParam>& cases) {
    using namespace std::string_literals;

    std::filesystem::path const param_file = case_dir / "params.json";
    json const j = read_json(param_file);
    // calculation method a string or array of strings
    std::vector<std::string> calculation_methods;
    if (j.at("calculation_method").type() == json::value_t::array) {
        j.at("calculation_method").get_to(calculation_methods);
    } else {
        calculation_methods.push_back(j.at("calculation_method").get<std::string>());
    }
    // loop sym and batch
    for (bool const sym : {true, false}) {
        for (auto const& calculation_method : calculation_methods) {
            if (calculation_method == "iec60909"s && sym) {
                continue; // only asym short circuit calculations are supported
            }

            CHECK_NOTHROW([&]() {
                if (auto test_case = construct_case(case_dir, j, calculation_type, is_batch, calculation_method, sym)) {
                    cases.push_back(*std::move(test_case));
                }
            }());
        }
    }
}

// test case with parameter
struct ValidationCase {
    CaseParam param;
    OwningDataset input;
    std::optional<OwningDataset> output;
    std::optional<OwningDataset> update_batch;
    std::optional<OwningDataset> output_batch;
};

ValidationCase create_validation_case(CaseParam const& param, std::string const& output_type) {
    // input
    ValidationCase validation_case{.param = param,
                                   .input = load_dataset(param.case_dir / "input.json"),
                                   .output = std::nullopt,
                                   .update_batch = std::nullopt,
                                   .output_batch = std::nullopt};

    // output and update
    if (!param.is_batch) {
        validation_case.output = load_dataset(param.case_dir / (output_type + ".json"));
    } else {
        validation_case.update_batch = load_dataset(param.case_dir / "update_batch.json");
        validation_case.output_batch = load_dataset(param.case_dir / (output_type + "_batch.json"));
    }
    return validation_case;
}

std::vector<CaseParam> read_all_cases(bool is_batch) {
    std::vector<CaseParam> all_cases;
    // detect all test cases
    for (std::string const calculation_type : {"power_flow", "state_estimation", "short_circuit"}) {
        // loop all sub-directories
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(data_dir / calculation_type)) {
            std::filesystem::path const& case_dir = dir_entry.path();
            if (!std::filesystem::exists(case_dir / "params.json")) {
                continue;
            }

            // try to add cases
            add_cases(case_dir, calculation_type, is_batch, all_cases);
        }
    }
    std::cout << "Total test cases: " << all_cases.size() << '\n';
    return all_cases;
}

std::vector<CaseParam> const& get_all_single_cases() {
    static std::vector<CaseParam> const all_cases = read_all_cases(false);
    return all_cases;
}

std::vector<CaseParam> const& get_all_batch_cases() {
    static std::vector<CaseParam> const all_cases = read_all_cases(true);
    return all_cases;
}

} // namespace

TEST_CASE("Check existence of validation data path") {
    REQUIRE(std::filesystem::exists(data_dir));
    std::cout << "Validation test dataset: " << data_dir << '\n';
}

namespace {
constexpr bool should_skip_test(CaseParam const& /*param*/) { return false; }

template <typename T>
    requires std::invocable<std::remove_cvref_t<T>, Subcase&>
void execute_test(CaseParam const& param, T&& func) noexcept {
    Subcase subcase{param.raises, param.xfail};
    bool const skip = should_skip_test(param);
    std::cout << std::format("Validation test: {}{}{}\n", param.case_name, skip ? " [skipped]" : "",
                             param.xfail ? " [xfail]" : "");
    if (skip) {
        return;
    }

    subcase.execute_case(std::forward<T>(func));
}

void validate_single_case(CaseParam const& param) {
    execute_test(param, [&param](Subcase& subcase) {
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const validation_case = create_validation_case(param, output_prefix);
        auto const result = create_result_dataset(validation_case.output.value(), output_prefix);

        // create and run model
        auto const& options = get_options(param);
        Model model{50.0, validation_case.input.dataset};
        model.calculate(options, result.dataset);

        // check results
        assert_result(result, validation_case.output.value(), param.atol, param.rtol, subcase);
    });
}

void validate_batch_case(CaseParam const& param) {
    execute_test(param, [&](Subcase& subcase) {
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const validation_case = create_validation_case(param, output_prefix);
        auto const& info = validation_case.update_batch.value().dataset.get_info();
        Idx const batch_size = info.batch_size();
        auto const batch_result =
            create_result_dataset(validation_case.output_batch.value(), output_prefix, true, batch_size);

        // create model
        Model model{50.0, validation_case.input.dataset};

        // check results after whole update is finished
        for (Idx const threading : {-1, 0, 1, 2}) {
            CAPTURE(threading);
            // set options and run
            auto const& options = get_options(param, threading);
            model.calculate(options, batch_result.dataset, validation_case.update_batch.value().dataset);

            // check results
            assert_result(batch_result, validation_case.output_batch.value(), param.atol, param.rtol, subcase);
        }
    });
}

} // namespace

TEST_CASE("Validation test single - power flow") {
    std::vector<CaseParam> const& all_cases = get_all_single_cases();
    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "power_flow") {
            SUBCASE(param.case_name.c_str()) { validate_single_case(param); }
        }
    }
}

TEST_CASE("Validation test single - state estimation") {
    std::vector<CaseParam> const& all_cases = get_all_single_cases();
    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "state_estimation") {
            SUBCASE(param.case_name.c_str()) { validate_single_case(param); }
        }
    }
}

TEST_CASE("Validation test single - short circuit") {
    std::vector<CaseParam> const& all_cases = get_all_single_cases();
    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "short_circuit") {
            SUBCASE(param.case_name.c_str()) { validate_single_case(param); }
        }
    }
}

TEST_CASE("Validation test batch - power flow") {
    std::vector<CaseParam> const& all_cases = get_all_batch_cases();

    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "power_flow") {
            SUBCASE(param.case_name.c_str()) { validate_batch_case(param); }
        }
    }
}

TEST_CASE("Validation test batch - state estimation") {
    std::vector<CaseParam> const& all_cases = get_all_batch_cases();

    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "state_estimation") {
            SUBCASE(param.case_name.c_str()) { validate_batch_case(param); }
        }
    }
}

TEST_CASE("Validation test batch - short circuit") {
    std::vector<CaseParam> const& all_cases = get_all_batch_cases();

    for (CaseParam const& param : all_cases) {
        if (param.calculation_type == "short_circuit") {
            SUBCASE(param.case_name.c_str()) { validate_batch_case(param); }
        }
    }
}

} // namespace power_grid_model_cpp
