// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <concepts>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace power_grid_model::meta_data {

namespace {

using nlohmann::json;

// read json file
json read_json(std::filesystem::path const& json_file) {
    json j;
    std::ifstream f{json_file};
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
struct BufferDeleter {
    void operator()(RawDataPtr ptr) { std::free(ptr); }
};
using BufferPtr = std::unique_ptr<void, BufferDeleter>;
BufferPtr create_buffer(size_t size, size_t length) { return BufferPtr(std::malloc(size * length)); }
struct Buffer {
    BufferPtr ptr;
    IdxVector indptr;
    MutableDataPointer data_ptr;
};

void parse_single_object(RawDataPtr ptr, json const& j, MetaComponent const& meta, Idx position) {
    for (auto const& it : j.items()) {
        // Allow and skip unknown attributes
        if (meta.has_attribute(it.key()) == 0) {
            continue;
        }
        MetaAttribute const& attr = meta.get_attribute(it.key());
        using enum CType;
        switch (attr.ctype) {
        case c_int8: {
            int8_t const value = it.value().get<int8_t>();
            attr.set_value(ptr, &value, position);
            break;
        }
        case c_int32: {
            int32_t const value = it.value().get<int32_t>();
            attr.set_value(ptr, &value, position);
            break;
        }
        case c_double: {
            double const value = it.value().get<double>();
            attr.set_value(ptr, &value, position);
            break;
        }
        case c_double3: {
            std::array<double, 3> const value = it.value().get<std::array<double, 3>>();
            attr.set_value(ptr, &value, position);
            break;
        }
        default:
            throw MissingCaseForEnumError("CType for attribute", attr.ctype);
        }
    }
}

Buffer parse_single_type(json const& j, MetaComponent const& meta) {
    Buffer buffer;
    size_t const length = j.size();
    size_t const obj_size = meta.size;
    buffer.ptr = create_buffer(obj_size, length);
    meta.set_nan(buffer.ptr.get(), 0, static_cast<Idx>(length));
    for (Idx position = 0; position != static_cast<Idx>(length); ++position) {
        parse_single_object(buffer.ptr.get(), j[position], meta, position);
    }
    buffer.indptr = {0, static_cast<Idx>(length)};
    buffer.data_ptr = MutableDataPointer{buffer.ptr.get(), buffer.indptr.data(), 1};
    return buffer;
}

std::map<std::string, Buffer> parse_single_dict(json const& j, std::string const& data_type) {
    MetaDataset const& meta = meta_data().get_dataset(data_type);
    std::map<std::string, Buffer> buffer_map;
    for (auto const& it : j.items()) {
        // skip empty list
        if (it.value().empty()) {
            continue;
        }
        buffer_map[it.key()] = parse_single_type(it.value(), meta.get_component(it.key()));
    }
    return buffer_map;
}

template <bool is_const>
std::map<std::string, DataPointer<is_const>> generate_dataset(std::map<std::string, Buffer> const& buffer_map) {
    std::map<std::string, DataPointer<is_const>> dataset;
    for (auto const& [name, buffer] : buffer_map) {
        dataset[name] = static_cast<DataPointer<is_const>>(buffer.data_ptr);
    }
    return dataset;
}

struct SingleData {
    Dataset dataset;
    ConstDataset const_dataset;
    std::map<std::string, Buffer> buffer_map;
};

// parse single json data
SingleData convert_json_single(json const& j, std::string const& data_type) {
    SingleData single_data;
    single_data.buffer_map = parse_single_dict(j, data_type);
    single_data.const_dataset = generate_dataset<true>(single_data.buffer_map);
    single_data.dataset = generate_dataset<false>(single_data.buffer_map);

    return single_data;
}

// create single result set
SingleData create_result_dataset(SingleData const& input, std::string const& data_type, Idx n_batch = 1) {
    MetaDataset const& meta = meta_data().get_dataset(data_type);
    SingleData result;
    for (auto const& [name, buffer] : input.buffer_map) {
        MetaComponent const& component_meta = meta.get_component(name);
        Buffer result_buffer;
        Idx const length = buffer.indptr.back();
        result_buffer.ptr = create_buffer(component_meta.size, length * n_batch);
        result_buffer.indptr.resize(n_batch + 1, 0);
        result_buffer.data_ptr = MutableDataPointer{result_buffer.ptr.get(), result_buffer.indptr.data(), n_batch};
        // set indptr
        for (Idx batch = 0; batch != n_batch; ++batch) {
            result_buffer.indptr[batch + 1] = (batch + 1) * length;
        }
        result.buffer_map[name] = std::move(result_buffer);
    }
    result.dataset = generate_dataset<false>(result.buffer_map);
    result.const_dataset = generate_dataset<true>(result.buffer_map);
    return result;
}

struct BatchData {
    Dataset dataset;
    ConstDataset const_dataset;
    std::map<std::string, Buffer> buffer_map;
    std::deque<SingleData> individual_batch;
};

// parse batch json data
BatchData convert_json_batch(json const& j, std::string const& data_type) {
    MetaDataset const& meta = meta_data().get_dataset(data_type);
    BatchData batch_data;
    for (auto const& j_single : j) {
        batch_data.individual_batch.push_back(convert_json_single(j_single, data_type));
    }
    Idx const n_batch = static_cast<Idx>(batch_data.individual_batch.size());
    // summerize count of object per component
    std::map<std::string, Idx> obj_count;
    for (SingleData const& single_data : batch_data.individual_batch) {
        for (auto const& [name, buffer] : single_data.buffer_map) {
            obj_count[name] += buffer.indptr.back();
        }
    }
    // allocate and copy object into batch dataset
    for (auto const& [name, total_length] : obj_count) {
        MetaComponent const& component_meta = meta.get_component(name);
        // allocate
        Buffer batch_buffer;
        batch_buffer.ptr = create_buffer(component_meta.size, total_length);
        batch_buffer.indptr.resize(n_batch + 1, 0);
        batch_buffer.data_ptr = MutableDataPointer{batch_buffer.ptr.get(), batch_buffer.indptr.data(), n_batch};
        RawDataPtr current_ptr = batch_buffer.ptr.get();
        // copy buffer
        for (Idx batch = 0; batch != n_batch; ++batch) {
            SingleData const& single_data = batch_data.individual_batch[batch];
            auto const found = single_data.buffer_map.find(name);
            if (found == single_data.buffer_map.cend()) {
                batch_buffer.indptr[batch + 1] = batch_buffer.indptr[batch];
                continue;
            }
            Buffer const& single_buffer = found->second;
            RawDataConstPtr const src_ptr = single_buffer.ptr.get();
            Idx const length = single_buffer.indptr.back();
            // copy memory, assign indptr
            std::memcpy(current_ptr, src_ptr, length * component_meta.size);
            batch_buffer.indptr[batch + 1] = batch_buffer.indptr[batch] + length;
            // shift current ptr
            current_ptr = (char*)current_ptr + length * component_meta.size;
        }
        // move into buffer map
        batch_data.buffer_map[name] = std::move(batch_buffer);
    }
    // create dataset
    batch_data.const_dataset = generate_dataset<true>(batch_data.buffer_map);
    batch_data.dataset = generate_dataset<false>(batch_data.buffer_map);
    return batch_data;
}

template <typename T>
std::string get_as_string(RawDataConstPtr const& raw_data_ptr, MetaAttribute const& attr, Idx obj) {
    // ensure that we don't read outside owned memory
    REQUIRE(attr.ctype == ctype_v<T>);
    REQUIRE(attr.size == sizeof(T));

    T value{};
    attr.get_value(raw_data_ptr, reinterpret_cast<RawDataPtr>(&value), obj);

    if constexpr (std::same_as<T, RealValue<false>>) {
        return "(" + std::to_string(value(0)) + ", " + std::to_string(value(1)) + ", " + std::to_string(value(2)) + ")";
    } else {
        return std::to_string(value);
    }
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
    MetaDataset const& meta = meta_data().get_dataset(data_type);
    Idx const batch_size = result.cbegin()->second.batch_size();
    // loop all batch
    for (Idx batch = 0; batch != batch_size; ++batch) {
        // loop all component type name
        for (auto const& [type_name, reference_dataset] : reference_result) {
            MetaComponent const& component_meta = meta.get_component(type_name);
            Idx const length = reference_dataset.elements_per_scenario(batch);
            // offset batch
            RawDataConstPtr const result_ptr =
                reinterpret_cast<char const*>(result.at(type_name).raw_ptr()) + length * batch * component_meta.size;
            RawDataConstPtr const reference_result_ptr =
                reinterpret_cast<char const*>(reference_dataset.raw_ptr()) + length * batch * component_meta.size;
            // loop all attribute
            for (MetaAttribute const& attr : component_meta.attributes) {
                // TODO skip u angle, need a way for common angle
                if (attr.name == "u_angle") {
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
                bool const is_angle = std::regex_match(attr.name, angle_match, angle_regex);
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
                        std::string const case_str =
                            "batch: #" + std::to_string(batch) + ", Component: " + type_name + " #" +
                            std::to_string(obj) + ", attribute: " + attr.name +
                            ": actual = " + get_as_string(result_ptr, attr, obj) +
                            " vs. expected = " + get_as_string(reference_result_ptr, attr, obj);
                        CHECK_MESSAGE(match, case_str);
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
using CalculationFunc =
    std::function<BatchParameter(MainModel&, CalculationMethod, Dataset const&, ConstDataset const&, Idx)>;

CalculationFunc calculation_func(std::string const& calculation_type, bool const sym) {
    using namespace std::string_literals;

    constexpr auto err_tol{1e-8};
    constexpr auto max_iter{20};
    constexpr auto c_factor{1.1};

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
        return [](MainModel& model, CalculationMethod calculation_method, Dataset const& dataset,
                  ConstDataset const& update_dataset, Idx threading) {
            return model.calculate_short_circuit(c_factor, calculation_method, dataset, update_dataset, threading);
        };
    }
    throw UnsupportedValidationCase{calculation_type, sym};
}

// case parameters
struct CaseParam {
    std::filesystem::path case_dir;
    std::string case_name;
    std::string calculation_type;
    std::string calculation_method;
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
    if (j.contains("fail")) {
        j.at("fail").get_to(param.fail);
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
    SingleData input;
    SingleData output;
    BatchData update_batch;
    BatchData output_batch;
};

ValidationCase create_validation_case(CaseParam const& param) {
    ValidationCase validation_case;
    validation_case.param = param;
    auto const output_type = get_output_type(param.calculation_type, param.sym);

    // input
    validation_case.input = convert_json_single(read_json(param.case_dir / "input.json"), "input");
    // output and update
    if (!param.is_batch) {
        validation_case.output = convert_json_single(read_json(param.case_dir / (output_type + ".json")), output_type);
    } else {
        validation_case.update_batch = convert_json_batch(read_json(param.case_dir / "update_batch.json"), "update");
        validation_case.output_batch =
            convert_json_batch(read_json(param.case_dir / (output_type + "_batch.json")), output_type);
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
        ValidationCase const validation_case = create_validation_case(param);
        std::string const output_prefix = get_output_type(param.calculation_type, param.sym);
        SingleData const result = create_result_dataset(validation_case.input, output_prefix);

        // create model and run
        MainModel model{50.0, validation_case.input.const_dataset, 0};
        CalculationFunc const func = calculation_func(param.calculation_type, param.sym);

        func(model, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);
        assert_result(result.const_dataset, validation_case.output.const_dataset, output_prefix, param.atol,
                      param.rtol);
    });
}

void validate_batch_case(CaseParam const& param) {
    execute_test(param, [&]() {
        ValidationCase const validation_case = create_validation_case(param);
        std::string const output_prefix = get_output_type(param.calculation_type, param.sym);
        SingleData const result = create_result_dataset(validation_case.input, output_prefix);

        // create model
        // TODO (mgovers): fix false positive of misc-const-correctness
        // NOLINTNEXTLINE(misc-const-correctness)
        MainModel model{50.0, validation_case.input.const_dataset, 0};
        Idx const n_batch = static_cast<Idx>(validation_case.update_batch.individual_batch.size());
        CalculationFunc const func = calculation_func(param.calculation_type, param.sym);

        // run in loops
        for (Idx batch = 0; batch != n_batch; ++batch) {
            MainModel model_copy{model};
            // update and run
            model_copy.update_component<MainModel::permanent_update_t>(
                validation_case.update_batch.individual_batch[batch].const_dataset);
            func(model_copy, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);

            // check
            assert_result(result.const_dataset, validation_case.output_batch.individual_batch[batch].const_dataset,
                          output_prefix, param.atol, param.rtol);
        }

        // run in one-go, with different threading possibility
        SingleData const batch_result = create_result_dataset(validation_case.input, output_prefix, n_batch);
        for (Idx const threading : {-1, 0, 1, 2}) {
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
                auto const msg = std::string("Unexpected exception with message: ") + e.what();
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
                auto const msg = std::string("Unexpected exception with message: ") + e.what();
                FAIL_CHECK(msg);
            }
        }
    }
}

} // namespace power_grid_model::meta_data
