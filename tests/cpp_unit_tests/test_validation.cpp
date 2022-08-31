// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#include "doctest/doctest.h"
#include "nlohmann/json.hpp"
#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include "power_grid_model/container.hpp"
#include "power_grid_model/main_model.hpp"

namespace power_grid_model {

namespace meta_data {

using nlohmann::json;

// read json file
json read_json(std::filesystem::path const& json_file) {
    json j;
    std::ifstream f{json_file};
    f >> j;
    return j;
}

// memory buffer
struct BufferDeleter {
    void operator()(void* ptr) {
        std::free(ptr);
    }
};
using BufferPtr = std::unique_ptr<void, BufferDeleter>;
BufferPtr create_buffer(size_t size, size_t length) {
    return BufferPtr(std::malloc(size * length));
}
struct Buffer {
    BufferPtr ptr;
    IdxVector indptr;
    MutableDataPointer data_ptr;
};

void parse_single_object(void* ptr, json const& j, MetaData const& meta, Idx position) {
    meta.set_nan(ptr, position);
    for (auto const& it : j.items()) {
        // Allow and skip unknown attributes
        if (!meta.has_attr(it.key())) {
            continue;
        }
        DataAttribute attr = meta.find_attr(it.key());
        if (attr.numpy_type == "i1") {
            int8_t const value = it.value().get<int8_t>();
            meta.set_attr(ptr, &value, attr, position);
        }
        else if (attr.numpy_type == "i4") {
            int32_t const value = it.value().get<int32_t>();
            meta.set_attr(ptr, &value, attr, position);
        }
        else if (attr.numpy_type == "f8") {
            if (attr.dims.empty()) {
                // single double
                double const value = it.value().get<double>();
                meta.set_attr(ptr, &value, attr, position);
            }
            else {
                // double[3]
                std::array<double, 3> const value = it.value().get<std::array<double, 3>>();
                meta.set_attr(ptr, &value, attr, position);
            }
        }
    }
}

Buffer parse_single_type(json const& j, MetaData const& meta) {
    Buffer buffer;
    size_t const length = j.size();
    size_t const obj_size = meta.size;
    buffer.ptr = create_buffer(obj_size, length);
    for (Idx position = 0; position != (Idx)length; ++position) {
        parse_single_object(buffer.ptr.get(), j[position], meta, position);
    }
    buffer.indptr = {0, (Idx)length};
    buffer.data_ptr = MutableDataPointer{buffer.ptr.get(), buffer.indptr.data(), 1};
    return buffer;
}

std::map<std::string, Buffer> parse_single_dict(json const& j, std::string const& data_type) {
    PowerGridMetaData const& meta = meta_data().at(data_type);
    std::map<std::string, Buffer> buffer_map;
    for (auto const& it : j.items()) {
        // skip empty list
        if (it.value().size() == 0) {
            continue;
        }
        buffer_map[it.key()] = parse_single_type(it.value(), meta.at(it.key()));
    }
    return buffer_map;
}

template <bool is_const>
std::map<std::string, DataPointer<is_const>> generate_dataset(std::map<std::string, Buffer> const& buffer_map) {
    std::map<std::string, DataPointer<is_const>> dataset;
    for (auto const& [name, buffer] : buffer_map) {
        dataset[name] = buffer.data_ptr;
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
    PowerGridMetaData const& meta = meta_data().at(data_type);
    SingleData result;
    for (auto const& [name, buffer] : input.buffer_map) {
        MetaData const& component_meta = meta.at(name);
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
    PowerGridMetaData const& meta = meta_data().at(data_type);
    BatchData batch_data;
    for (auto const& j_single : j) {
        batch_data.individual_batch.push_back(convert_json_single(j_single, data_type));
    }
    Idx const n_batch = (Idx)batch_data.individual_batch.size();
    // summerize count of object per component
    std::map<std::string, Idx> obj_count;
    for (SingleData const& single_data : batch_data.individual_batch) {
        for (auto const& [name, buffer] : single_data.buffer_map) {
            obj_count[name] += buffer.indptr.back();
        }
    }
    // allocate and copy object into batch dataset
    for (auto const& [name, total_length] : obj_count) {
        MetaData const& component_meta = meta.at(name);
        // allocate
        Buffer batch_buffer;
        batch_buffer.ptr = create_buffer(component_meta.size, total_length);
        batch_buffer.indptr.resize(n_batch + 1, 0);
        batch_buffer.data_ptr = MutableDataPointer{batch_buffer.ptr.get(), batch_buffer.indptr.data(), n_batch};
        void* current_ptr = batch_buffer.ptr.get();
        // copy buffer
        for (Idx batch = 0; batch != n_batch; ++batch) {
            SingleData const& single_data = batch_data.individual_batch[batch];
            auto const found = single_data.buffer_map.find(name);
            if (found == single_data.buffer_map.cend()) {
                batch_buffer.indptr[batch + 1] = batch_buffer.indptr[batch];
                continue;
            }
            Buffer const& single_buffer = found->second;
            void const* const src_ptr = single_buffer.ptr.get();
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

// assert single result
void assert_result(ConstDataset const& result, ConstDataset const& reference_result, std::string const& data_type,
                   std::map<std::string, double> atol, double rtol) {
    PowerGridMetaData const& meta = meta_data().at(data_type);
    Idx const batch_size = result.cbegin()->second.batch_size();
    // loop all batch
    for (Idx batch = 0; batch != batch_size; ++batch) {
        // loop all component type name
        for (auto const& [type_name, reference_dataset] : reference_result) {
            MetaData const& component_meta = meta.at(type_name);
            Idx const length = reference_dataset.length_per_batch(batch);
            // offset batch
            void const* const result_ptr =
                reinterpret_cast<char const*>(result.at(type_name).raw_ptr()) + length * batch * component_meta.size;
            void const* const reference_result_ptr =
                reinterpret_cast<char const*>(reference_dataset.raw_ptr()) + length * batch * component_meta.size;
            // loop all attribute
            for (DataAttribute const& attr : component_meta.attributes) {
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

                // loop all object
                for (Idx obj = 0; obj != length; ++obj) {
                    // only check if reference result is not nan
                    if (component_meta.check_nan(reference_result_ptr, attr, obj)) {
                        continue;
                    }
                    bool const match =
                        component_meta.compare_attr(result_ptr, reference_result_ptr, dynamic_atol, rtol, attr, obj);
                    if (match) {
                        CHECK(match);
                    }
                    else {
                        std::string const case_str = "batch: #" + std::to_string(batch) + ", Component: " + type_name +
                                                     " #" + std::to_string(obj) + ", attribute: " + attr.name;
                        CHECK_MESSAGE(match, case_str);
                    }
                }
            }
        }
    }
}

// root path
#ifdef POWER_GRID_MODEL_VALIDATION_TEST_DATA_PATH
// use marco definition input
std::filesystem::path const data_path{POWER_GRID_MODEL_VALIDATION_TEST_DATA_PATH};
#else
// use relative path to this file
std::filesystem::path const data_path = std::filesystem::path{__FILE__}.parent_path().parent_path() / "data";
#endif

// method map
std::map<std::string, CalculationMethod> const calculation_method_mapping = {
    {"newton_raphson", CalculationMethod::newton_raphson},
    {"linear", CalculationMethod::linear},
    {"iterative_current", CalculationMethod::iterative_current},
    {"iterative_linear", CalculationMethod::iterative_linear},
};
using CalculationFunc = BatchParameter (MainModel::*)(double, Idx, CalculationMethod, Dataset const&,
                                                      ConstDataset const&, Idx);
std::map<std::pair<std::string, bool>, CalculationFunc> const calculation_type_mapping = {
    {{"power_flow", true}, &MainModel::calculate_power_flow<true>},
    {{"power_flow", false}, &MainModel::calculate_power_flow<false>},
    {{"state_estimation", true}, &MainModel::calculate_state_estimation<true>},
    {{"state_estimation", false}, &MainModel::calculate_state_estimation<false>},
};

// case parameters
struct CaseParam {
    std::filesystem::path case_dir;
    std::string case_name;
    std::string calculation_type;
    std::string calculation_method;
    bool sym{};
    bool is_batch{};
    double rtol{};
    BatchParameter batch_parameter{};
    std::map<std::string, double> atol;

    static std::string replace_backslash(std::string const& str) {
        std::string str_out{str};
        std::transform(str.cbegin(), str.cend(), str_out.begin(), [](char c) {
            return c == '\\' ? '/' : c;
        });
        return str_out;
    }
};

inline void add_cases(std::filesystem::path const& case_dir, std::string const& calculation_type, bool is_batch,
                      std::vector<CaseParam>& cases) {
    std::filesystem::path const param_file = case_dir / "params.json";
    json const j = read_json(param_file);
    // calculation method a string or array of strings
    std::vector<std::string> calculation_methods;
    if (j.at("calculation_method").type() == json::value_t::array) {
        j.at("calculation_method").get_to(calculation_methods);
    }
    else {
        calculation_methods.push_back(j.at("calculation_method").get<std::string>());
    }
    // loop sym and batch
    for (bool const sym : {true, false}) {
        std::string const output_prefix = sym ? "sym_output" : "asym_output";
        for (std::string const& calculation_method : calculation_methods) {
            std::string const batch_suffix = is_batch ? "_batch" : "";
            // add a case if output file exists
            std::filesystem::path const output_file = case_dir / (output_prefix + batch_suffix + ".json");
            if (std::filesystem::exists(output_file)) {
                CaseParam param{};
                param.case_dir = case_dir;
                param.case_name = CaseParam::replace_backslash(std::filesystem::relative(case_dir, data_path).string());
                param.calculation_type = calculation_type;
                param.calculation_method = calculation_method;
                param.sym = sym;
                param.is_batch = is_batch;
                j.at("rtol").get_to(param.rtol);
                json const j_atol = j.at("atol");
                if (j_atol.type() != json::value_t::object) {
                    param.atol = {{"default", j_atol.get<double>()}};
                }
                else {
                    j_atol.get_to(param.atol);
                }
                if (param.is_batch) {
                    j.at("independent").get_to(param.batch_parameter.independent);
                    j.at("cache_topology").get_to(param.batch_parameter.cache_topology);
                }
                param.case_name += sym ? "-sym" : "-asym";
                param.case_name += "-" + param.calculation_method;
                param.case_name += is_batch ? "-batch" : "";
                cases.push_back(param);
            }
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
    std::string const output_prefix = param.sym ? "sym_output" : "asym_output";
    // input
    validation_case.input = convert_json_single(read_json(param.case_dir / "input.json"), "input");
    // output and update
    if (!param.is_batch) {
        validation_case.output =
            convert_json_single(read_json(param.case_dir / (output_prefix + ".json")), output_prefix);
    }
    else {
        validation_case.update_batch = convert_json_batch(read_json(param.case_dir / "update_batch.json"), "update");
        validation_case.output_batch =
            convert_json_batch(read_json(param.case_dir / (output_prefix + "_batch.json")), output_prefix);
    }
    return validation_case;
}

inline std::vector<CaseParam> read_all_cases(bool is_batch) {
    std::vector<CaseParam> all_cases;
    // detect all test cases
    for (std::string calculation_type : {"power_flow", "state_estimation"}) {
        // loop all sub-directories
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(data_path / calculation_type)) {
            std::filesystem::path const case_dir = dir_entry.path();
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

inline std::vector<CaseParam> const& get_all_single_cases() {
    static std::vector<CaseParam> const all_cases = read_all_cases(false);
    return all_cases;
}

inline std::vector<CaseParam> const& get_all_batch_cases() {
    static std::vector<CaseParam> const all_cases = read_all_cases(true);
    return all_cases;
}

TEST_CASE("Check existence of validation data path") {
    REQUIRE(std::filesystem::exists(data_path));
    std::cout << "Validation test dataset: " << data_path << '\n';
}

void validate_single_case(CaseParam const& param) {
    std::cout << "Validation test: " << param.case_name << std::endl;
    ValidationCase const validation_case = create_validation_case(param);
    std::string const output_prefix = param.sym ? "sym_output" : "asym_output";
    SingleData result = create_result_dataset(validation_case.input, output_prefix);
    // create model and run
    MainModel model{50.0, validation_case.input.const_dataset, 0};
    CalculationFunc const func = calculation_type_mapping.at(std::make_pair(param.calculation_type, param.sym));
    (model.*func)(1e-8, 20, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);
    assert_result(result.const_dataset, validation_case.output.const_dataset, output_prefix, param.atol, param.rtol);
}

void validate_batch_case(CaseParam const& param) {
    std::cout << "Validation test: " << param.case_name << std::endl;
    ValidationCase const validation_case = create_validation_case(param);
    std::string const output_prefix = param.sym ? "sym_output" : "asym_output";
    SingleData result = create_result_dataset(validation_case.input, output_prefix);
    // create model
    MainModel model{50.0, validation_case.input.const_dataset, 0};
    Idx const n_batch = (Idx)validation_case.update_batch.individual_batch.size();
    CalculationFunc const func = calculation_type_mapping.at(std::make_pair(param.calculation_type, param.sym));

    // run in loops
    for (Idx batch = 0; batch != n_batch; ++batch) {
        MainModel model_copy{model};
        // update and run
        model_copy.update_component(validation_case.update_batch.individual_batch[batch].const_dataset);
        (model_copy.*func)(1e-8, 20, calculation_method_mapping.at(param.calculation_method), result.dataset, {}, -1);
        // check
        assert_result(result.const_dataset, validation_case.output_batch.individual_batch[batch].const_dataset,
                      output_prefix, param.atol, param.rtol);
    }

    // run in one-go, with different threading possibility
    SingleData batch_result = create_result_dataset(validation_case.input, output_prefix, n_batch);
    for (Idx threading : {-1, 0, 1, 2}) {
        BatchParameter batch_parameter =
            (model.*func)(1e-8, 20, calculation_method_mapping.at(param.calculation_method), batch_result.dataset,
                          validation_case.update_batch.const_dataset, threading);
        assert_result(batch_result.const_dataset, validation_case.output_batch.const_dataset, output_prefix, param.atol,
                      param.rtol);
        // check batch parameters
        CHECK(batch_parameter.independent == param.batch_parameter.independent);
        CHECK(batch_parameter.cache_topology == param.batch_parameter.cache_topology);
    }
}

TEST_CASE("Validation test single") {
    std::vector<CaseParam> const& all_cases = get_all_single_cases();
    for (CaseParam const& param : all_cases) {
        SUBCASE(param.case_name.c_str()) {
            try {
                validate_single_case(param);
            }
            catch (std::exception& e) {
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
            }
            catch (std::exception& e) {
                auto const msg = std::string("Unexpected exception with message: ") + e.what();
                FAIL_CHECK(msg);
            }
        }
    }
}

}  // namespace meta_data
}  // namespace power_grid_model