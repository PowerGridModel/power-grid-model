// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_cpp.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model.hpp>

#include <Eigen/Dense>
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
#include <optional>
#include <regex>

namespace power_grid_model::meta_data {

namespace {

// is nan. Taken from three_phase_tensor.hpp
template <class Derived> inline bool is_nan(Eigen::ArrayBase<Derived> const& x) { return x.isNaN().all(); }
inline bool is_nan(std::floating_point auto x) { return std::isnan(x); }
template <std::floating_point T> inline bool is_nan(std::complex<T> const& x) {
    return is_nan(x.real()) || is_nan(x.imag());
}
inline bool is_nan(int32_t x) { return x == std::numeric_limits<int32_t>::min(); }
inline bool is_nan(int8_t x) { return x == std::numeric_limits<int8_t>::min(); }
template <class Enum>
    requires std::same_as<std::underlying_type_t<Enum>, int8_t>
inline bool is_nan(Enum x) {
    return static_cast<int8_t>(x) == std::numeric_limits<int8_t>::min();
}

template <class Functor, class... Args>
decltype(auto) pgm_type_func_selector(PGM_CType type, Functor&& f, Args&&... args) {
    switch (type) {
    case PGM_double:
        return std::forward<Functor>(f).template operator()<double>(std::forward<Args>(args)...);
    case PGM_double3:
        return std::forward<Functor>(f).template operator()<std::array<double, 3>>(std::forward<Args>(args)...);
    case PGM_int8:
        return std::forward<Functor>(f).template operator()<int8_t>(std::forward<Args>(args)...);
    case PGM_int32:
        return std::forward<Functor>(f).template operator()<int32_t>(std::forward<Args>(args)...);
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

struct OwningDataset {
    power_grid_model_cpp::DatasetMutable dataset;
    power_grid_model_cpp::DatasetConst const_dataset;
    std::vector<power_grid_model_cpp::Buffer> buffers{};
    std::vector<power_grid_model_cpp::DatasetConst> batch_scenarios{};
};

auto create_owning_dataset(power_grid_model_cpp::DatasetWritable& writable_dataset) {
    auto const& info = writable_dataset.get_info();
    bool const is_batch = info.is_batch();
    Idx const batch_size = info.batch_size();
    auto const& dataset_name = info.name();
    auto const& dataset_meta = power_grid_model_cpp::MetaData::get_dataset_by_name(dataset_name);
    std::vector<power_grid_model_cpp::Buffer> buffers;
    power_grid_model_cpp::DatasetMutable mutable_dataset{dataset_name, is_batch,
                                                         batch_size}; // Is this the true dataset type intended here?

    for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
        auto const& component_name = info.component_name(component_idx);
        auto const& component_meta = power_grid_model_cpp::MetaData::get_component_by_idx(dataset_meta, component_idx);
        Idx const component_elements_per_scenario = info.component_elements_per_scenario(component_idx);
        Idx const component_size = info.component_total_elements(component_idx);
        IdxVector indptr_vector{info.component_elements_per_scenario(component_idx) < 0 ? batch_size + 1 : 0};
        Idx* buffer_indptr = indptr_vector.empty() ? nullptr : indptr_vector.data();

        power_grid_model_cpp::Buffer buffer{component_meta, component_size};
        writable_dataset.set_buffer(component_name, buffer_indptr, buffer);
        mutable_dataset.add_buffer(component_name, component_elements_per_scenario, component_size, buffer_indptr,
                                   buffer);
        buffers.push_back(std::move(buffer));
    }
    return OwningDataset{
        .dataset = std::move(mutable_dataset),
        .const_dataset{writable_dataset},
        .buffers = std::move(buffers),
    };
}

auto create_owning_dataset(OwningDataset const& owning_dataset,
                           std::string const& result_dataset_name) { // can probably be combined with the other overload
    auto const& info = owning_dataset.dataset.get_info();
    bool const is_batch = info.is_batch();
    Idx const batch_size = info.batch_size();
    auto const& dataset_meta = power_grid_model_cpp::MetaData::get_dataset_by_name(result_dataset_name);
    std::vector<power_grid_model_cpp::Buffer> buffers;
    power_grid_model_cpp::DatasetMutable mutable_dataset{result_dataset_name, is_batch,
                                                         batch_size}; // Is this the true dataset type intended here?

    for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
        auto const& component_name = info.component_name(component_idx);
        auto const& component_meta = power_grid_model_cpp::MetaData::get_component_by_idx(dataset_meta, component_idx);
        Idx const component_size = info.component_total_elements(component_idx);
        Idx const component_elements_per_scenario = info.component_elements_per_scenario(component_idx);
        IdxVector indptr_vector{info.component_elements_per_scenario(component_idx) < 0 ? batch_size + 1 : 0};
        Idx* buffer_indptr = indptr_vector.empty() ? nullptr : indptr_vector.data();

        power_grid_model_cpp::Buffer buffer{component_meta, component_size};
        mutable_dataset.add_buffer(component_name, component_elements_per_scenario, component_size, buffer_indptr,
                                   buffer);
        buffers.push_back(std::move(buffer));
    }
    return OwningDataset{
        .dataset = std::move(mutable_dataset),
        .const_dataset{owning_dataset.dataset},
        .buffers = std::move(buffers),
    };
}

// probably not needed, because we can get the individual scenario info by offsetting. Will test with batch validation.
/*auto construct_individual_scenarios(OwningDataset& owning_dataset) {
    for (Idx scenario_idx{}; scenario_idx < owning_dataset.dataset.batch_size(); ++scenario_idx) {
        owning_dataset.batch_scenarios.push_back(owning_dataset.const_dataset.get_individual_scenario(scenario_idx));
    }
}*/

auto load_dataset(std::filesystem::path const& path) {
// Issue in msgpack, reported in https://github.com/msgpack/msgpack-c/issues/1098
// May be a Clang Analyzer bug
#ifndef __clang_analyzer__ // TODO(mgovers): re-enable this when issue in msgpack is fixed
    power_grid_model_cpp::Deserializer deserializer{read_file(path), 0};
    auto dataset = create_owning_dataset(deserializer.get_dataset());
    deserializer.parse_to_buffer();
    // construct_individual_scenarios(dataset);
    return dataset;
#else  // __clang_analyzer__ // issue in msgpack
    (void)path;
    return OwningDataset{}; // fallback for https://github.com/msgpack/msgpack-c/issues/1098
#endif // __clang_analyzer__ // issue in msgpack
}

// create single result set
OwningDataset create_result_dataset(OwningDataset const& input, std::string const& data_type, bool is_batch = false,
                                    Idx batch_size = 1) {
    auto owning_dataset =
        create_owning_dataset(input, data_type); // this overload can just become create_result_dataset
    // construct_individual_scenarios(owning_dataset);
    return owning_dataset;
}

template <typename T>
std::string get_as_string(RawDataConstPtr const& raw_data_ptr, MetaAttribute const& attr, Idx obj) {
    // ensure that we don't read outside owned memory
    REQUIRE(attr.ctype == ctype_v<T>);
    REQUIRE(attr.size == sizeof(T));

    T value{};
    attr.get_value(raw_data_ptr, reinterpret_cast<RawDataPtr>(&value), obj);

    std::stringstream sstr;
    sstr << std::setprecision(16);
    if constexpr (std::same_as<T, RealValue<asymmetric_t>>) {
        sstr << "(" << value(0) << ", " << value(1) << ", " << value(2) << ")";
    } else if constexpr (std::same_as<T, int8_t>) {
        sstr << std::to_string(value);
    } else {
        sstr << value;
    }
    return sstr.str();
}

std::string get_as_string(RawDataConstPtr const& raw_data_ptr, MetaAttribute const& attr, Idx obj) {
    using enum CType;
    using namespace std::string_literals;

    switch (attr.ctype) {
    case c_int32:
        return get_as_string<int32_t>(raw_data_ptr, attr, obj);
    case c_int8:
        return get_as_string<int8_t>(raw_data_ptr, attr, obj);
    case c_double:
        return get_as_string<double>(raw_data_ptr, attr, obj);
    case c_double3:
        return get_as_string<RealValue<asymmetric_t>>(raw_data_ptr, attr, obj);
    default:
        return "<unknown value type>"s;
    }
}

template <symmetry_tag sym>
bool check_angle_and_magnitude(RawDataConstPtr reference_result_ptr, RawDataConstPtr result_ptr,
                               MetaAttribute const& angle_attr, MetaAttribute const& mag_attr, double atol, double rtol,
                               Idx obj) {
    RealValue<sym> mag{};
    RealValue<sym> mag_ref{};
    RealValue<sym> angle{};
    RealValue<sym> angle_ref{};
    mag_attr.get_value(result_ptr, &mag, obj);
    mag_attr.get_value(reference_result_ptr, &mag_ref, obj);
    angle_attr.get_value(result_ptr, &angle, obj);
    angle_attr.get_value(reference_result_ptr, &angle_ref, obj);
    ComplexValue<sym> const result = mag * exp(1.0i * angle);
    ComplexValue<sym> const result_ref = mag_ref * exp(1.0i * angle_ref);
    if constexpr (is_symmetric_v<sym>) {
        return cabs(result - result_ref) < (cabs(result_ref) * rtol + atol);
    } else {
        return (cabs(result - result_ref) < (cabs(result_ref) * rtol + atol)).all();
    }
}

bool check_angle_and_magnitude(RawDataConstPtr reference_result_ptr, RawDataConstPtr result_ptr,
                               MetaAttribute const& angle_attr, MetaAttribute const& mag_attr, double atol, double rtol,
                               Idx obj) {
    if (angle_attr.ctype == CType::c_double) {
        assert(mag_attr.ctype == CType::c_double);
        return check_angle_and_magnitude<symmetric_t>(reference_result_ptr, result_ptr, angle_attr, mag_attr, atol,
                                                      rtol, obj);
    }
    assert(angle_attr.ctype == CType::c_double3);
    assert(mag_attr.ctype == CType::c_double3);
    return check_angle_and_magnitude<asymmetric_t>(reference_result_ptr, result_ptr, angle_attr, mag_attr, atol, rtol,
                                                   obj);
}

// assert single result
void assert_result(OwningDataset const& owning_result, OwningDataset const& owning_reference_result,
                   std::map<std::string, double, std::less<>> atol, double rtol) {
    using namespace std::string_literals;
    power_grid_model_cpp::DatasetConst const& result = owning_result.const_dataset;
    power_grid_model_cpp::DatasetConst const& reference_result = owning_reference_result.const_dataset;
    auto const& result_info = result.get_info();
    auto const& result_name = result_info.name();
    auto const& result_meta = power_grid_model_cpp::MetaData::get_dataset_by_name(result_name);
    Idx const result_batch_size = result_info.batch_size();

    auto const& reference_result_info = reference_result.get_info();
    auto const& reference_result_name = reference_result_info.name();
    auto const& reference_result_meta = power_grid_model_cpp::MetaData::get_dataset_by_name(reference_result_name);
    Idx const reference_result_batch_size = reference_result_info.batch_size();

    CHECK(result_name == reference_result_name);
    CHECK(result_meta == reference_result_meta);
    CHECK(result_batch_size == reference_result_batch_size);
    CHECK(owning_result.buffers.size() == owning_reference_result.buffers.size());
    // loop all scenario
    for (Idx scenario_idx = 0; scenario_idx < result_batch_size; ++scenario_idx) {
        // loop all component type name
        for (Idx component_idx{}; component_idx < reference_result_info.n_components(); ++component_idx) {
            auto const& component_meta =
                power_grid_model_cpp::MetaData::get_component_by_idx(reference_result_meta, component_idx);
            auto const& component_name = power_grid_model_cpp::MetaData::component_name(component_meta);
            auto const& ref_buffer = owning_reference_result.buffers.at(component_idx);
            auto const& buffer = owning_result.buffers.at(component_idx);
            // auto const& ref_buffer = reference_result.get_buffer(i); //why don't just use i to get both buffers here?
            // auto const& buffer = result.get_buffer(component_meta.name);
            Idx const elements_per_scenario = reference_result_info.component_elements_per_scenario(component_idx);
            assert(elements_per_scenario >= 0);
            // offset scenario
            // this can be done via buffer.get_value(), but at the attribute level
            RawDataConstPtr const result_ptr =
                reinterpret_cast<char const*>(buffer.data) + elements_per_scenario * scenario_idx * component_meta.size;
            RawDataConstPtr const reference_result_ptr = reinterpret_cast<char const*>(ref_buffer.data) +
                                                         elements_per_scenario * scenario_idx * component_meta.size;
            // loop all attribute
            for (Idx attribute_idx{}; attribute_idx < power_grid_model_cpp::MetaData::n_attributes(component_meta);
                 ++attribute_idx) {
                auto const& attribute_meta =
                    power_grid_model_cpp::MetaData::get_attribute_by_idx(component_meta, attribute_idx);
                auto const& attribute_type = power_grid_model_cpp::MetaData::attribute_ctype(attribute_meta);
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
                auto const& possible_attr_magnitude =
                    is_angle ? power_grid_model_cpp::MetaData::get_attribute_by_name(reference_result_name,
                                                                                     component_name, magnitude_name)
                             : attribute_meta;

                // loop all object
                for (Idx obj = 0; obj != elements_per_scenario;
                     ++obj) { // this loops over each element in one scenario after the buffer has been set correctly
                    auto lambda = []<typename T>() {
                        T variable{};
                        return variable;
                    };

                    auto const& ref_attribute_buffer = pgm_type_func_selector(attribute_type, lambda);

                    if (attr.check_nan(reference_result_ptr, obj)) {
                        continue;
                    }
                    // for angle attribute, also check the magnitude available
                    if (is_angle && possible_attr_magnitude.check_nan(reference_result_ptr, obj)) {
                        continue;
                    }
                    bool const match =
                        is_angle ? check_angle_and_magnitude(reference_result_ptr, result_ptr, attr,
                                                             possible_attr_magnitude, dynamic_atol, rtol, obj)
                                 : attr.compare_value(reference_result_ptr, result_ptr, dynamic_atol, rtol, obj);
                    if (match) {
                        CHECK(match);
                    } else {
                        std::stringstream case_sstr;
                        case_sstr << "dataset scenario: #" << scenario << ", Component: " << component_meta.name << " #"
                                  << obj << ", attribute: " << attr.name
                                  << ": actual = " << get_as_string(result_ptr, attr, obj) + " vs. expected = "
                                  << get_as_string(reference_result_ptr, attr, obj);
                        CHECK_MESSAGE(match, case_sstr.str());
                    }
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

// to be removed in the end
using CalculationFunc =
    std::function<BatchParameter(MainModel&, CalculationMethod, MutableDataset const&, ConstDataset const&, Idx)>;

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
    bool sym{};
    bool is_batch{};
    double rtol{};
    bool fail{};
    // to remove batch parameter in the end, most likely
    [[no_unique_address]] BatchParameter batch_parameter{};
    std::map<std::string, double, std::less<>> atol;

    static std::string replace_backslash(std::string const& str) {
        std::string str_out{str};
        std::transform(str.cbegin(), str.cend(), str_out.begin(), [](char c) { return c == '\\' ? '/' : c; });
        return str_out;
    }
};

// This is probably not needed since we can just use Model::Calculate after setting Options
/*CalculationFunc calculation_func(CaseParam const& param) {
    auto const get_options = [&param](CalculationMethod calculation_method, Idx threading) {
        return MainModel::Options{
            .calculation_type = calculation_type_mapping.at(param.calculation_type),
            .calculation_symmetry = param.sym ? CalculationSymmetry::symmetric : CalculationSymmetry::asymmetric,
            .calculation_method = calculation_method,
            .optimizer_type = param.tap_changing_strategy == "disabled" ? OptimizerType::no_optimization
                                                                        : OptimizerType::automatic_tap_adjustment,
            .optimizer_strategy = optimizer_strategy_mapping.at(param.tap_changing_strategy),
            .err_tol = 1e-8,
            .max_iter = 20,
            .threading = threading,
            .short_circuit_voltage_scaling = sc_voltage_scaling_mapping.at(param.short_circuit_voltage_scaling)};
    };

    return [get_options](MainModel& model, CalculationMethod calculation_method, MutableDataset const& dataset,
                         ConstDataset const& update_dataset, Idx threading) {
        auto options = get_options(calculation_method, threading);
        if (options.optimizer_type != OptimizerType::no_optimization) {
            REQUIRE(options.calculation_type == CalculationType::power_flow);
        }
        return model.calculate(options, dataset, update_dataset);
    };
}*/

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
    std::optional<OwningDataset> output{};
    std::optional<OwningDataset> update_batch{};
    std::optional<OwningDataset> output_batch{};
};

ValidationCase create_validation_case(CaseParam const& param, std::string const& output_type) {
    // input
    ValidationCase validation_case{.param = param, .input = load_dataset(param.case_dir / "input.json")};

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
        auto const output_prefix = get_output_type(param.calculation_type, param.sym); // checked
        auto const validation_case = create_validation_case(param, output_prefix);     // checked
        auto const result = create_result_dataset(validation_case.input, output_prefix);

        // Create options -> maybe make this into a function to re-use
        power_grid_model_cpp::Options options{};
        options.set_calculation_type(calculation_type_mapping.at(param.calculation_type));
        options.set_calculation_method(calculation_method_mapping.at(param.calculation_method));
        options.set_symmetric(param.sym ? 0 : 1);
        options.set_err_tol(1e-8);
        options.set_max_iter(20);
        options.set_threading(-1);
        options.set_short_circuit_voltage_scaling(sc_voltage_scaling_mapping.at(param.short_circuit_voltage_scaling));
        options.set_tap_changing_strategy(optimizer_strategy_mapping.at(param.tap_changing_strategy));

        // create model and run
        power_grid_model_cpp::Model model{50.0, validation_case.input.const_dataset};
        model.calculate(options, result.dataset);
        assert_result(result, validation_case.output.value(), param.atol, param.rtol);
        // assert_result(result.const_dataset, validation_case.output.value().const_dataset, param.atol, param.rtol);
    });
}

/*void validate_batch_case(CaseParam const& param) {
    execute_test(param, [&]() {
        auto const validation_case = create_validation_case(param);
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const result = create_result_dataset(validation_case.input, output_prefix);

        // create model
        // TODO (mgovers): fix false positive of misc-const-correctness
        // NOLINTNEXTLINE(misc-const-correctness)
        MainModel model{50.0, validation_case.input.const_dataset, 0};
        auto const n_scenario = static_cast<Idx>(validation_case.update_batch.value().batch_scenarios.size());
        CalculationFunc const func = calculation_func(param);

        // run in loops
        for (Idx scenario = 0; scenario != n_scenario; ++scenario) {
            CAPTURE(scenario);

            MainModel model_copy{model};

            // update and run
            model_copy.update_component<permanent_update_t>(
                validation_case.update_batch.value().batch_scenarios[scenario]);
            ConstDataset empty{false, 1, "update", meta_data_gen::meta_data};
            func(model_copy, calculation_method_mapping.at(param.calculation_method), result.dataset, empty, -1);

            // check
            assert_result(result.const_dataset, validation_case.output_batch.value().batch_scenarios[scenario],
                          param.atol, param.rtol);
        }

        // run in one-go, with different threading possibility
        auto const batch_result = create_result_dataset(validation_case.input, output_prefix, true, n_scenario);
        for (Idx const threading : {-1, 0, 1, 2}) {
            CAPTURE(threading);

            func(model, calculation_method_mapping.at(param.calculation_method), batch_result.dataset,
                 validation_case.update_batch.value().const_dataset, threading);

            assert_result(batch_result.const_dataset, validation_case.output_batch.value().const_dataset, param.atol,
                          param.rtol);
        }
    });
}*/

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

/*TEST_CASE("Validation test batch") {
    std::vector<CaseParam> const& all_cases = get_all_batch_cases();

    for (CaseParam const& param : all_cases) {
        SUBCASE(param.case_name.c_str()) {
            try {
                validate_batch_case(param);
            } catch (std::exception& e) {
                using namespace std::string_literals;

                auto const msg = "Unexpected exception with message: "s + e.what();
                FAIL_CHECK(msg);
            }
        }
    }
}*/

} // namespace power_grid_model::meta_data
