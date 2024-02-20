// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <concepts>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace power_grid_model::meta_data {

namespace {

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

class UnsupportedValidationCase : public PowerGridError {
  public:
    UnsupportedValidationCase(std::string const& calculation_type, bool sym) {
        using namespace std::string_literals;

        auto const sym_str = sym ? "sym"s : "asym"s;
        append_msg("Unsupported validation case: "s + sym_str + " "s + calculation_type);
    };
};

// memory buffer
using BufferPtr = std::unique_ptr<void, std::add_pointer_t<void(RawDataConstPtr)>>; // custom deleter at runtime
struct Buffer {
    BufferPtr ptr{nullptr, [](void const*) {}};
    IdxVector indptr;
    MutableDataPointer data_ptr;
};

struct OwningDataset {
    Dataset dataset;
    ConstDataset const_dataset;
    std::map<std::string, Buffer> buffer_map;
    std::vector<ConstDataset> batch_scenarios;
};

template <bool is_const>
std::map<std::string, DataPointer<is_const>> generate_dataset(std::map<std::string, Buffer> const& buffer_map) {
    std::map<std::string, DataPointer<is_const>> dataset;
    for (auto const& [name, buffer] : buffer_map) {
        dataset[name] = static_cast<DataPointer<is_const>>(buffer.data_ptr);
    }
    return dataset;
}

auto create_owning_dataset(WritableDatasetHandler& info) {
    Idx const batch_size = info.batch_size();
    OwningDataset dataset;

    for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
        auto const& component_info = info.get_component_info(component_idx);
        auto const& component_meta = component_info.component;

        Buffer buffer{};
        buffer.ptr =
            BufferPtr{component_meta->create_buffer(component_info.total_elements), component_meta->destroy_buffer};
        buffer.indptr = IdxVector(component_info.elements_per_scenario < 0 ? batch_size + 1 : 0);
        buffer.data_ptr = MutableDataPointer{buffer.ptr.get(),
                                             component_info.elements_per_scenario < 0 ? buffer.indptr.data() : nullptr,
                                             batch_size, component_info.elements_per_scenario};

        info.set_buffer(component_info.component->name, buffer.indptr.data(), buffer.ptr.get());
        dataset.buffer_map[component_meta->name] = std::move(buffer);
    }
    dataset.const_dataset = info.export_dataset<true>();
    dataset.dataset = info.export_dataset<false>();

    return dataset;
}

auto construct_individual_scenarios(OwningDataset& dataset, WritableDatasetHandler const& info) {
    for (Idx scenario_idx{}; scenario_idx < info.batch_size(); ++scenario_idx) {
        dataset.batch_scenarios.push_back(info.export_dataset<true>(scenario_idx));
    }
}

auto load_dataset(std::filesystem::path const& path) {
// Issue in msgpack, reported in https://github.com/msgpack/msgpack-c/issues/1098
// May be a Clang Analyzer bug
#ifndef __clang_analyzer__ // TODO(mgovers): re-enable this when issue in msgpack is fixed
    auto deserializer = Deserializer{power_grid_model::meta_data::from_json, read_file(path)};
    auto& info = deserializer.get_dataset_info();
    auto dataset = create_owning_dataset(info);
    deserializer.parse();
    construct_individual_scenarios(dataset, info);
    return dataset;
#else  // __clang_analyzer__ // issue in msgpack
    (void)path;
    return OwningDataset{}; // fallback for https://github.com/msgpack/msgpack-c/issues/1098
#endif // __clang_analyzer__ // issue in msgpack
}

// create single result set
OwningDataset create_result_dataset(OwningDataset const& input, std::string const& data_type, bool is_batch = false,
                                    Idx batch_size = 1) {
    MetaDataset const& meta = meta_data.get_dataset(data_type);
    WritableDatasetHandler handler{is_batch, batch_size, meta.name};

    for (auto const& [name, data_ptr] : input.const_dataset) {
        assert(data_ptr.batch_size() == 1);
        Buffer const result_buffer;
        Idx const elements_per_scenario = data_ptr.elements_per_scenario(0);
        handler.add_component_info(name, elements_per_scenario, elements_per_scenario * batch_size);
    }
    return create_owning_dataset(handler);
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
    if constexpr (std::same_as<T, RealValue<false>>) {
        sstr << "(" << value(0) << ", " << value(1) << ", " << value(2) << ")";
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
        return get_as_string<RealValue<false>>(raw_data_ptr, attr, obj);
    default:
        return "<unknown value type>"s;
    }
}

template <bool sym>
bool assert_angle_and_magnitude(RawDataConstPtr reference_result_ptr, RawDataConstPtr result_ptr,
                                MetaAttribute const& angle_attr, MetaAttribute const& mag_attr, double atol,
                                double rtol, Idx obj) {
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
    if constexpr (sym) {
        return cabs(result - result_ref) < (cabs(result_ref) * rtol + atol);
    } else {
        return (cabs(result - result_ref) < (cabs(result_ref) * rtol + atol)).all();
    }
}

bool assert_angle_and_magnitude(RawDataConstPtr reference_result_ptr, RawDataConstPtr result_ptr,
                                MetaAttribute const& angle_attr, MetaAttribute const& mag_attr, double atol,
                                double rtol, Idx obj) {
    if (angle_attr.ctype == CType::c_double) {
        assert(mag_attr.ctype == CType::c_double);
        return assert_angle_and_magnitude<true>(reference_result_ptr, result_ptr, angle_attr, mag_attr, atol, rtol,
                                                obj);
    }
    assert(angle_attr.ctype == CType::c_double3);
    assert(mag_attr.ctype == CType::c_double3);
    return assert_angle_and_magnitude<false>(reference_result_ptr, result_ptr, angle_attr, mag_attr, atol, rtol, obj);
}

// assert single result
void assert_result(ConstDataset const& result, ConstDataset const& reference_result, std::string const& data_type,
                   std::map<std::string, double> atol, double rtol) {
    using namespace std::string_literals;
    MetaDataset const& meta = meta_data.get_dataset(data_type);
    Idx const batch_size = result.cbegin()->second.batch_size();
    // loop all scenario
    for (Idx scenario = 0; scenario != batch_size; ++scenario) {
        // loop all component type name
        for (auto const& [type_name, reference_dataset] : reference_result) {
            MetaComponent const& component_meta = meta.get_component(type_name);
            Idx const length = reference_dataset.elements_per_scenario(scenario);
            // offset scenario
            RawDataConstPtr const result_ptr =
                reinterpret_cast<char const*>(result.at(type_name).raw_ptr()) + length * scenario * component_meta.size;
            RawDataConstPtr const reference_result_ptr =
                reinterpret_cast<char const*>(reference_dataset.raw_ptr()) + length * scenario * component_meta.size;
            // loop all attribute
            for (MetaAttribute const& attr : component_meta.attributes) {
                // TODO skip u angle, need a way for common angle
                if (attr.name == "u_angle"s) {
                    continue;
                }
                // get absolute tolerance
                double dynamic_atol = atol.at("default");
                for (auto const& [reg, value] : atol) {
                    if (std::regex_match(attr.name, std::regex{reg})) {
                        dynamic_atol = value;
                        break;
                    }
                }
                // for other _angle attribute, we need to find the magnitue and compare together
                std::regex const angle_regex("(.*)(_angle)");
                std::smatch angle_match;
                std::string const attr_name = attr.name;
                bool const is_angle = std::regex_match(attr_name, angle_match, angle_regex);
                std::string const magnitude_name = angle_match[1];
                MetaAttribute const& possible_attr_magnitude =
                    is_angle ? component_meta.get_attribute(magnitude_name) : attr;

                // loop all object
                for (Idx obj = 0; obj != length; ++obj) {
                    // only check if reference result is not nan
                    if (attr.check_nan(reference_result_ptr, obj)) {
                        continue;
                    }
                    // for angle attribute, also check the magnitude available
                    if (is_angle && possible_attr_magnitude.check_nan(reference_result_ptr, obj)) {
                        continue;
                    }
                    bool const match =
                        is_angle ? assert_angle_and_magnitude(reference_result_ptr, result_ptr, attr,
                                                              possible_attr_magnitude, dynamic_atol, rtol, obj)
                                 : attr.compare_value(reference_result_ptr, result_ptr, dynamic_atol, rtol, obj);
                    if (match) {
                        CHECK(match);
                    } else {
                        std::stringstream case_sstr;
                        case_sstr << "dataset scenario: #" << scenario << ", Component: " << type_name << " #" << obj
                                  << ", attribute: " << attr.name
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
std::map<std::string, CalculationMethod> const calculation_method_mapping = {
    {"newton_raphson", CalculationMethod::newton_raphson},
    {"linear", CalculationMethod::linear},
    {"iterative_current", CalculationMethod::iterative_current},
    {"iterative_linear", CalculationMethod::iterative_linear},
    {"iec60909", CalculationMethod::iec60909},
};
std::map<std::string, ShortCircuitVoltageScaling> const sc_voltage_scaling_mapping = {
    {"minimum", ShortCircuitVoltageScaling::minimum}, {"maximum", ShortCircuitVoltageScaling::maximum}};
using CalculationFunc =
    std::function<BatchParameter(MainModel&, CalculationMethod, Dataset const&, ConstDataset const&, Idx)>;

// case parameters
struct CaseParam {
    std::filesystem::path case_dir;
    std::string case_name;
    std::string calculation_type;
    std::string calculation_method;
    std::string short_circuit_voltage_scaling;
    bool sym{};
    bool is_batch{};
    double rtol{};
    bool fail{};
    [[no_unique_address]] BatchParameter batch_parameter{};
    std::map<std::string, double> atol;

    static std::string replace_backslash(std::string const& str) {
        std::string str_out{str};
        std::transform(str.cbegin(), str.cend(), str_out.begin(), [](char c) { return c == '\\' ? '/' : c; });
        return str_out;
    }
};

CalculationFunc calculation_func(CaseParam const& param) {
    using namespace std::string_literals;
    std::string const calculation_type = param.calculation_type;
    bool const sym = param.sym;
    constexpr auto err_tol{1e-8};
    constexpr auto max_iter{20};
    std::string const voltage_scaling = param.short_circuit_voltage_scaling;

    if (calculation_type == "power_flow"s) {
        return [sym](MainModel& model, CalculationMethod calculation_method, Dataset const& dataset,
                     ConstDataset const& update_dataset, Idx threading) {
            if (sym) {
                return model.calculate_power_flow<true>(err_tol, max_iter, calculation_method, dataset, update_dataset,
                                                        threading);
            }
            return model.calculate_power_flow<false>(err_tol, max_iter, calculation_method, dataset, update_dataset,
                                                     threading);
        };
    }
    if (calculation_type == "state_estimation"s) {
        return [sym](MainModel& model, CalculationMethod calculation_method, Dataset const& dataset,
                     ConstDataset const& update_dataset, Idx threading) {
            if (sym) {
                return model.calculate_state_estimation<true>(err_tol, max_iter, calculation_method, dataset,
                                                              update_dataset, threading);
            }
            return model.calculate_state_estimation<false>(err_tol, max_iter, calculation_method, dataset,
                                                           update_dataset, threading);
        };
    }
    if (calculation_type == "short_circuit"s) {
        return [voltage_scaling](MainModel& model, CalculationMethod calculation_method, Dataset const& dataset,
                                 ConstDataset const& update_dataset, Idx threading) {
            return model.calculate_short_circuit(sc_voltage_scaling_mapping.at(voltage_scaling), calculation_method,
                                                 dataset, update_dataset, threading);
        };
    }
    throw UnsupportedValidationCase{calculation_type, sym};
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
    OwningDataset output;
    OwningDataset update_batch;
    OwningDataset output_batch;
};

ValidationCase create_validation_case(CaseParam const& param) {
    ValidationCase validation_case;
    validation_case.param = param;
    auto const output_type = get_output_type(param.calculation_type, param.sym);

    // input
    validation_case.input = load_dataset(param.case_dir / "input.json");

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
        auto const validation_case = create_validation_case(param);
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const result = create_result_dataset(validation_case.input, output_prefix);

        // create model and run
        MainModel model{50.0, validation_case.input.const_dataset, 0};
        CalculationFunc const func = calculation_func(param);

        func(model, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);
        assert_result(result.const_dataset, validation_case.output.const_dataset, output_prefix, param.atol,
                      param.rtol);
    });
}

void validate_batch_case(CaseParam const& param) {
    execute_test(param, [&]() {
        auto const validation_case = create_validation_case(param);
        auto const output_prefix = get_output_type(param.calculation_type, param.sym);
        auto const result = create_result_dataset(validation_case.input, output_prefix);

        // create model
        // TODO (mgovers): fix false positive of misc-const-correctness
        // NOLINTNEXTLINE(misc-const-correctness)
        MainModel model{50.0, validation_case.input.const_dataset, 0};
        Idx const n_scenario = static_cast<Idx>(validation_case.update_batch.batch_scenarios.size());
        CalculationFunc const func = calculation_func(param);

        // run in loops
        for (Idx scenario = 0; scenario != n_scenario; ++scenario) {
            CAPTURE(scenario);

            MainModel model_copy{model};

            // update and run
            model_copy.update_component<MainModel::permanent_update_t>(
                validation_case.update_batch.batch_scenarios[scenario]);
            func(model_copy, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);

            // check
            assert_result(result.const_dataset, validation_case.output_batch.batch_scenarios[scenario], output_prefix,
                          param.atol, param.rtol);
        }

        // run in one-go, with different threading possibility
        auto const batch_result = create_result_dataset(validation_case.input, output_prefix, true, n_scenario);
        for (Idx const threading : {-1, 0, 1, 2}) {
            CAPTURE(threading);

            func(model, calculation_method_mapping.at(param.calculation_method), batch_result.dataset,
                 validation_case.update_batch.const_dataset, threading);

            assert_result(batch_result.const_dataset, validation_case.output_batch.const_dataset, output_prefix,
                          param.atol, param.rtol);
        }
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
