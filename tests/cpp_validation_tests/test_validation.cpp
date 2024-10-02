// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_c.h"
#include "power_grid_model_cpp.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <complex>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <regex>
#include <stdexcept>
#include <type_traits>

namespace power_grid_model::meta_data {

namespace {
inline bool is_nan(std::floating_point auto x) { return std::isnan(x); }
template <std::floating_point T> inline bool is_nan(std::complex<T> const& x) {
    return is_nan(x.real()) || is_nan(x.imag());
}
inline bool is_nan(int32_t x) { return x == std::numeric_limits<int32_t>::min(); }
inline bool is_nan(int8_t x) { return x == std::numeric_limits<int8_t>::min(); }
template <typename T, std::size_t N> inline bool is_nan(std::array<T, N> const& array) {
    for (auto const& element : array) {
        if (is_nan(element))
            return true;
    }
    return false;
}

template <class Functor, class... Args>
decltype(auto) pgm_type_func_selector(enum PGM_CType type, Functor&& f, Args&&... args) {
    switch (type) {
    case PGM_int32:
        return std::forward<Functor>(f).template operator()<int32_t>(std::forward<Args>(args)...);
    case PGM_int8:
        return std::forward<Functor>(f).template operator()<int8_t>(std::forward<Args>(args)...);
    case PGM_double:
        return std::forward<Functor>(f).template operator()<double>(std::forward<Args>(args)...);
    case PGM_double3:
        return std::forward<Functor>(f).template operator()<std::array<double, 3>>(std::forward<Args>(args)...);
    default:
        throw std::runtime_error("unknown data type");
    }
}

using nlohmann::json;

auto read_file(std::filesystem::path const& path) {
    std::ifstream const f{path};
    std::ostringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

auto read_json(std::filesystem::path const& path) {
    json j;
    std::ifstream f{path};
    f >> j;
    return j;
}

class UnsupportedValidationCase : public power_grid_model_cpp::PowerGridError {
  public:
    UnsupportedValidationCase(std::string const& calculation_type, bool sym)
        : power_grid_model_cpp::PowerGridError{[&]() {
              using namespace std::string_literals;
              auto const sym_str = sym ? "sym"s : "asym"s;
              return "Unsupported validation case: "s + sym_str + " "s + calculation_type;
          }()} {}
};

struct OwningMemory {
    std::vector<power_grid_model_cpp::Buffer> buffers{};
    std::vector<std::vector<Idx>> indptrs{};
};

struct OwningDataset {
    std::optional<power_grid_model_cpp::DatasetMutable> dataset;
    std::optional<power_grid_model_cpp::DatasetConst> const_dataset;
    // std::vector<power_grid_model_cpp::DatasetConst> batch_scenarios{};
    OwningMemory storage{};
};

auto create_owning_dataset(power_grid_model_cpp::DatasetWritable& writable_dataset) {
    auto const& info = writable_dataset.get_info();
    bool const is_batch = info.is_batch();
    Idx const batch_size = info.batch_size();
    auto const& dataset_name = info.name();
    OwningDataset owning_dataset{.dataset{power_grid_model_cpp::DatasetMutable{dataset_name, is_batch, batch_size}},
                                 .const_dataset = std::nullopt};

    for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
        auto const& component_name = info.component_name(component_idx);
        auto const& component_meta =
            power_grid_model_cpp::MetaData::get_component_by_name(dataset_name, component_name);
        Idx const component_elements_per_scenario = info.component_elements_per_scenario(component_idx);
        Idx const component_size = info.component_total_elements(component_idx);

        owning_dataset.storage.indptrs.emplace_back(
            info.component_elements_per_scenario(component_idx) < 0 ? batch_size + 1 : 0);
        bool empty_indptr = owning_dataset.storage.indptrs.at(component_idx).empty() ? true : false;
        Idx* indptr = empty_indptr ? nullptr : owning_dataset.storage.indptrs.at(component_idx).data();
        owning_dataset.storage.buffers.emplace_back(component_meta, component_size);
        writable_dataset.set_buffer(component_name, indptr, owning_dataset.storage.buffers.at(component_idx));
        owning_dataset.dataset.value().add_buffer(component_name, component_elements_per_scenario, component_size,
                                                  indptr, owning_dataset.storage.buffers.at(component_idx));
    }
    owning_dataset.const_dataset = writable_dataset;
    return owning_dataset;
}

OwningDataset create_result_dataset(OwningDataset const& input, std::string const& dataset_name, bool is_batch = false,
                                    Idx batch_size = 1) {
    OwningDataset owning_dataset{.dataset{power_grid_model_cpp::DatasetMutable{dataset_name, is_batch, batch_size}},
                                 .const_dataset = std::nullopt};
    auto const& info_input = input.const_dataset.value().get_info();

    for (Idx component_idx{}; component_idx != info_input.n_components(); ++component_idx) {
        auto const& component_name = info_input.component_name(component_idx);
        auto const& component_meta =
            power_grid_model_cpp::MetaData::get_component_by_name(dataset_name, component_name);
        Idx const component_elements_per_scenario = info_input.component_elements_per_scenario(component_idx);
        Idx const component_size = info_input.component_total_elements(component_idx);

        owning_dataset.storage.indptrs.emplace_back(
            info_input.component_elements_per_scenario(component_idx) < 0 ? batch_size + 1 : 0);
        bool empty_indptr = owning_dataset.storage.indptrs.at(component_idx).empty() ? true : false;
        Idx* indptr = empty_indptr ? nullptr : owning_dataset.storage.indptrs.at(component_idx).data();
        owning_dataset.storage.buffers.emplace_back(component_meta, component_size);
        owning_dataset.dataset.value().add_buffer(component_name, component_elements_per_scenario, component_size,
                                                  indptr, owning_dataset.storage.buffers.at(component_idx));
    }
    owning_dataset.const_dataset = owning_dataset.dataset.value();
    // construct_individual_scenarios(owning_dataset); // this may or may not be needed for batch stuff
    return owning_dataset;
}

// probably not needed, because we can get the individual scenario info by offsetting. Will test with batch validation.
/*auto construct_individual_scenarios(OwningDataset& owning_dataset) {
    for (Idx scenario_idx{}; scenario_idx < owning_dataset.dataset.batch_size(); ++scenario_idx) {
        owning_dataset.batch_scenarios.push_back(owning_dataset.const_dataset.get_individual_scenario(scenario_idx));
    }
}*/

OwningDataset load_dataset(std::filesystem::path const& path) {
// Issue in msgpack, reported in https://github.com/msgpack/msgpack-c/issues/1098
// May be a Clang Analyzer bug
#ifndef __clang_analyzer__ // TODO(mgovers): re-enable this when issue in msgpack is fixed
    power_grid_model_cpp::Deserializer deserializer{read_file(path), Idx{0}};
    auto& writable_dataset = deserializer.get_dataset();
    auto dataset = create_owning_dataset(writable_dataset);
    deserializer.parse_to_buffer();
    // construct_individual_scenarios(dataset);
    return dataset;
#else  // __clang_analyzer__ // issue in msgpack
    (void)path;
    // fallback for https://github.com/msgpack/msgpack-c/issues/1098
    return OwningDataset{.dataset = {false, 0, "", meta_data_gen::meta_data},
                         .const_dataset = {false, 0, "", meta_data_gen::meta_data}};
#endif // __clang_analyzer__ // issue in msgpack
}

template <typename T> std::string get_as_string(T const& attribute_value) {
    std::stringstream sstr;
    sstr << std::setprecision(16);
    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        sstr << "(" << attribute_value[0] << ", " << attribute_value[1] << ", " << attribute_value[2] << ")";
    } else if constexpr (std::is_same_v<std::decay_t<T>, int8_t>) {
        sstr << std::to_string(attribute_value);
    } else {
        sstr << attribute_value;
    }
    return sstr.str();
}

template <typename T>
bool check_angle_and_magnitude(T const& ref_angle, T const& angle, T const& ref_magnitude, T const& magnitude,
                               double atol, double rtol) {
    auto to_complex = [](double magnitude, double angle) { return std::polar(magnitude, angle); };
    auto is_within_tolerance = [&atol, &rtol](std::complex<double> element, std::complex<double> ref_element) {
        return std::abs(element - ref_element) < std::abs(ref_element) * rtol + atol;
    };

    if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
        std::array<std::complex<double>, 3> result, ref_result;
        std::ranges::transform(magnitude, angle, result.begin(), to_complex);
        std::ranges::transform(ref_magnitude, ref_angle, ref_result.begin(), to_complex);
        return std::ranges::equal(result, ref_result, is_within_tolerance);
    }
    if constexpr (std::is_same_v<std::decay_t<T>, double>) {
        std::complex<double> result = to_complex(magnitude, angle);
        std::complex<double> ref_result = to_complex(ref_magnitude, ref_angle);
        return is_within_tolerance(result, ref_result);
    } else {
        return ref_angle == angle && ref_magnitude == magnitude;
    }
}

template <typename T>
bool compare_value(T const& ref_attribute_value, T const& attribute_value, double atol, double rtol) {
    auto is_within_tolerance = [&atol, &rtol](std::complex<double> element, std::complex<double> ref_element) {
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

void assert_result(OwningDataset const& owning_result, OwningDataset const& owning_reference_result,
                   std::map<std::string, double, std::less<>> atol, double rtol) {
    using namespace std::string_literals;

    power_grid_model_cpp::DatasetConst const& result = owning_result.const_dataset.value();
    auto const& result_info = result.get_info();
    auto const& result_name = result_info.name();
    Idx const result_batch_size = result_info.batch_size();
    auto const& storage = owning_result.storage;

    power_grid_model_cpp::DatasetConst const& reference_result = owning_reference_result.const_dataset.value();
    auto const& reference_result_info = reference_result.get_info();
    auto const& reference_result_name = reference_result_info.name();
    auto const& reference_storage = owning_reference_result.storage;
    CHECK(storage.buffers.size() == reference_storage.buffers.size());

    // loop through all scenarios
    for (Idx scenario_idx{}; scenario_idx < result_batch_size; ++scenario_idx) {
        // loop through all components
        for (Idx component_idx{}; component_idx < reference_result_info.n_components(); ++component_idx) {
            auto const& component_name = reference_result_info.component_name(component_idx);
            auto const& component_meta =
                power_grid_model_cpp::MetaData::get_component_by_name(reference_result_name, component_name);

            auto& ref_buffer = reference_storage.buffers.at(component_idx);
            auto& buffer = storage.buffers.at(component_idx);
            Idx const elements_per_scenario = reference_result_info.component_elements_per_scenario(component_idx);
            CHECK(elements_per_scenario >= 0);
            // loop through all attributes
            for (Idx attribute_idx{}; attribute_idx < power_grid_model_cpp::MetaData::n_attributes(component_meta);
                 ++attribute_idx) {
                auto const& attribute_meta =
                    power_grid_model_cpp::MetaData::get_attribute_by_idx(component_meta, attribute_idx);
                auto attribute_type = power_grid_model_cpp::MetaData::attribute_ctype(attribute_meta);
                auto const& attribute_name = power_grid_model_cpp::MetaData::attribute_name(attribute_meta);
                // TODO skip u angle, need a way for common angle
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
                auto const& possible_attr_meta = is_angle ? power_grid_model_cpp::MetaData::get_attribute_by_name(
                                                                reference_result_name, component_name, magnitude_name)
                                                          : attribute_meta;
                // loop through all objects
                for (Idx obj{}; obj < elements_per_scenario; ++obj) {
                    auto check_results = [&is_angle, &scenario_idx, &obj, &component_name, &attribute_name,
                                          &dynamic_atol, rtol]<typename T>(
                                             T const& ref_attribute_value, T const& attribute_value,
                                             T const& ref_possible_attribute_value, T const& possible_attribute_value) {
                        bool const match = is_angle
                                               ? check_angle_and_magnitude<decltype(ref_attribute_value)>(
                                                     ref_attribute_value, attribute_value, ref_possible_attribute_value,
                                                     possible_attribute_value, dynamic_atol, rtol)
                                               : compare_value<decltype(ref_attribute_value)>(
                                                     ref_attribute_value, attribute_value, dynamic_atol, rtol);
                        if (match) {
                            CHECK(match);
                        } else {
                            std::stringstream case_sstr;
                            case_sstr << "dataset scenario: #" << scenario_idx << ", Component: " << component_name
                                      << " #" << obj << ", attribute: " << attribute_name << ": actual = "
                                      << get_as_string<decltype(attribute_value)>(attribute_value) + " vs. expected = "
                                      << get_as_string<decltype(ref_attribute_value)>(ref_attribute_value);
                            CHECK_MESSAGE(match, case_sstr.str());
                        }
                    };

                    auto const idx = (elements_per_scenario * scenario_idx) + obj;

                    auto check_individual_attribute = [&check_results, &buffer, &ref_buffer, &attribute_meta,
                                                       &possible_attr_meta, &is_angle, &idx]<typename T>() {
                        // get attribute values to check
                        T ref_attribute_value{};
                        T attribute_value{};
                        T ref_possible_attribute_value{};
                        T possible_attribute_value{};
                        auto get_values = [&ref_buffer, &buffer, &attribute_meta, &possible_attr_meta,
                                           &idx]<typename U>(U* ref_attribute_value, U* attribute_value,
                                                             U* ref_possible_attribute_value,
                                                             U* possible_attribute_value) {
                            ref_buffer.get_value(attribute_meta, ref_attribute_value, idx, 0);
                            buffer.get_value(attribute_meta, attribute_value, idx, 0);
                            ref_buffer.get_value(possible_attr_meta, ref_possible_attribute_value, idx, 0);
                            buffer.get_value(possible_attr_meta, possible_attribute_value, idx, 0);
                        };
                        if constexpr (std::is_same_v<std::decay_t<T>, std::array<double, 3>>) {
                            get_values(ref_attribute_value.data(), attribute_value.data(),
                                       ref_possible_attribute_value.data(), possible_attribute_value.data());
                        } else {
                            get_values(&ref_attribute_value, &attribute_value, &ref_possible_attribute_value,
                                       &possible_attribute_value);
                        }

                        // check attributes
                        if (is_nan(ref_attribute_value)) {
                            return;
                        }
                        // for angle attribute, also check the magnitude available
                        if (is_angle && is_nan(ref_possible_attribute_value)) {
                            return;
                        }
                        check_results(ref_attribute_value, attribute_value, ref_possible_attribute_value,
                                      possible_attribute_value);
                    };

                    pgm_type_func_selector(static_cast<PGM_CType>(attribute_type), check_individual_attribute);
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

// case parameters
struct CaseParam {
    std::filesystem::path case_dir;
    std::string case_name;
    std::string calculation_type;
    std::string calculation_method;
    std::string short_circuit_voltage_scaling;
    std::string tap_changing_strategy;
    double err_tol = 1e-8;
    Idx max_iter = 20;
    Idx threading = -1;
    bool sym{};
    bool is_batch{};
    double rtol{};
    bool fail{};
    std::map<std::string, double, std::less<>> atol;

    static std::string replace_backslash(std::string const& str) {
        std::string str_out{str};
        std::transform(str.cbegin(), str.cend(), str_out.begin(), [](char c) { return c == '\\' ? '/' : c; });
        return str_out;
    }
};

power_grid_model_cpp::Options get_options(CaseParam const& param, Idx threading = -1) {
    power_grid_model_cpp::Options options{};
    options.set_calculation_type(calculation_type_mapping.at(param.calculation_type));
    options.set_calculation_method(calculation_method_mapping.at(param.calculation_method));
    options.set_symmetric(param.sym ? 1 : 0);
    options.set_err_tol(1e-8);
    options.set_max_iter(20);
    options.set_threading(threading);
    options.set_short_circuit_voltage_scaling(sc_voltage_scaling_mapping.at(param.short_circuit_voltage_scaling));
    options.set_tap_changing_strategy(optimizer_strategy_mapping.at(param.tap_changing_strategy));
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

    param.fail = calculation_method_params.contains("fail");
    if (calculation_type == "short_circuit") {
        calculation_method_params.at("short_circuit_voltage_scaling").get_to(param.short_circuit_voltage_scaling);
    }

    param.tap_changing_strategy = calculation_method_params.value("tap_changing_strategy", "disabled");
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
constexpr bool should_skip_test(CaseParam const& param) { return param.fail; }

template <typename T>
    requires std::invocable<std::remove_cvref_t<T>>
void execute_test(CaseParam const& param, T&& func) {
    std::cout << "Validation test: " << param.case_name;

    if (should_skip_test(param)) {
        std::cout << " [skipped]" << std::endl;
    } else {
        std::cout << std::endl;
        func();
    }
}

void validate_single_case(CaseParam const& param) {
    execute_test(param, [&]() {
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const validation_case = create_validation_case(param, output_prefix);
        auto const result = create_result_dataset(validation_case.output.value(), output_prefix);

        // create and run model
        auto const& options = get_options(param);
        power_grid_model_cpp::Model model{50.0, validation_case.input.const_dataset.value()};
        model.calculate(options, result.dataset.value());

        // check results
        assert_result(result, validation_case.output.value(), param.atol, param.rtol);
    });
}

void validate_batch_case(CaseParam const& param) {
    execute_test(param, [&]() {
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const validation_case = create_validation_case(param, output_prefix);
        auto const& info = validation_case.update_batch.value().const_dataset.value().get_info();
        Idx batch_size = info.batch_size();
        auto const batch_result = create_result_dataset(validation_case.input, output_prefix, true, batch_size);

        // create model
        power_grid_model_cpp::Model model{50.0, validation_case.input.const_dataset.value()};

        // check results after whole update is finished
        for (Idx const threading : {-1, 0, 1, 2}) {
            CAPTURE(threading);
            // set options and run
            auto const& options = get_options(param, threading);
            model.calculate(options, batch_result.dataset.value());

            assert_result(batch_result, validation_case.output_batch.value(), param.atol, param.rtol);
        }

        /*// create model
        // TODO (mgovers): fix false positive of misc-const-correctness
        // NOLINTNEXTLINE(misc-const-correctness)
        power_grid_model_cpp::Model model{50.0, validation_case.input.const_dataset.value()};


        /////
        //MainModel model{50.0, validation_case.input.const_dataset, 0};
        auto const n_scenario = static_cast<Idx>(validation_case.update_batch.value().batch_scenarios.size());
        CalculationFunc const func = calculation_func(param);

        // first run for individual scenarios and check them all (do one update at the time)
        // run in loops
        for (Idx scenario = 0; scenario != n_scenario; ++scenario) {
            CAPTURE(scenario);

            MainModel model_copy{model};

            // update and run
            model_copy.update_component<permanent_update_t>(
                validation_case.update_batch.value().batch_scenarios[scenario]);
            ConstDataset const empty{false, 1, "update", meta_data_gen::meta_data};
            func(model_copy, calculation_method_mapping.at(param.calculation_method), result.dataset, empty, -1);

            // check
            assert_result(result.const_dataset, validation_case.output_batch.value().batch_scenarios[scenario],
                          param.atol, param.rtol);
        }*/
    });
}

} // namespace

TEST_CASE("Validation test single") {
    std::vector<CaseParam> const& all_cases = get_all_single_cases();
    for (CaseParam const& param : all_cases) {
        SUBCASE(param.case_name.c_str()) {
            try {
                validate_single_case(param);
            } catch (std::exception& e) {
                using namespace std::string_literals;

                auto const msg = "Unexpected exception with message: "s + e.what();
                FAIL_CHECK(msg);
            }
        }
    }
}

TEST_CASE("Validation test batch") {
    std::vector<CaseParam> const& all_cases = get_all_batch_cases();

    for (CaseParam const& param : all_cases) {
        SUBCASE(param.case_name.c_str()) {
            continue;
            try {
                validate_batch_case(param);
            } catch (std::exception& e) {
                using namespace std::string_literals;

                auto const msg = "Unexpected exception with message: "s + e.what();
                FAIL_CHECK(msg);
            }
        }
    }
}

} // namespace power_grid_model::meta_data
