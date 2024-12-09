// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_cpp/meta_data.hpp>
#include <power_grid_model_cpp/model.hpp>
#include <power_grid_model_cpp/serialization.hpp>
#include <power_grid_model_cpp/utils.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <numbers>
#include <span>
#include <string>
#include <vector>

namespace {
using namespace std::string_literals;

// Types for template parameters
struct row_t {};
struct columnar_t {};
struct sparse_t {};
struct dense_t {};
struct with_id_t {};
struct optional_id_t {};
struct mixed_optional_id_t {};
struct invalid_id_t {};

template <typename first, typename second, typename third, typename fourth> struct TypeCombo {
    using input_type = first;
    using update_type = second;
    using sparsity_type = third;
    using id_check_type = fourth;
};
} // namespace

TYPE_TO_STRING_AS("row_t, row_t, dense_t, with_id_t", TypeCombo<row_t, row_t, dense_t, with_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, sparse_t, with_id_t", TypeCombo<row_t, row_t, sparse_t, with_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, dense_t, with_id_t", TypeCombo<columnar_t, columnar_t, dense_t, with_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, sparse_t, with_id_t",
                  TypeCombo<columnar_t, columnar_t, sparse_t, with_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, dense_t, with_id_t", TypeCombo<columnar_t, row_t, dense_t, with_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, sparse_t, with_id_t", TypeCombo<columnar_t, row_t, sparse_t, with_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, dense_t, with_id_t", TypeCombo<row_t, columnar_t, dense_t, with_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, sparse_t, with_id_t", TypeCombo<row_t, columnar_t, sparse_t, with_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, dense_t, optional_id_t", TypeCombo<row_t, row_t, dense_t, optional_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, sparse_t, optional_id_t", TypeCombo<row_t, row_t, sparse_t, optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, dense_t, optional_id_t",
                  TypeCombo<columnar_t, columnar_t, dense_t, optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, sparse_t, optional_id_t",
                  TypeCombo<columnar_t, columnar_t, sparse_t, optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, dense_t, optional_id_t", TypeCombo<columnar_t, row_t, dense_t, optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, sparse_t, optional_id_t", TypeCombo<columnar_t, row_t, sparse_t, optional_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, dense_t, optional_id_t", TypeCombo<row_t, columnar_t, dense_t, optional_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, sparse_t, optional_id_t", TypeCombo<row_t, columnar_t, sparse_t, optional_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, dense_t, invalid_id_t", TypeCombo<row_t, row_t, dense_t, invalid_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, dense_t, mixed_optional_id_t", TypeCombo<row_t, row_t, dense_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, sparse_t, mixed_optional_id_t",
                  TypeCombo<row_t, row_t, sparse_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, dense_t, mixed_optional_id_t",
                  TypeCombo<columnar_t, columnar_t, dense_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, sparse_t, mixed_optional_id_t",
                  TypeCombo<columnar_t, columnar_t, sparse_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, dense_t, mixed_optional_id_t",
                  TypeCombo<columnar_t, row_t, dense_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, sparse_t, mixed_optional_id_t",
                  TypeCombo<columnar_t, row_t, sparse_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, dense_t, mixed_optional_id_t",
                  TypeCombo<row_t, columnar_t, dense_t, mixed_optional_id_t>);
TYPE_TO_STRING_AS("row_t, row_t, sparse_t, invalid_id_t", TypeCombo<row_t, row_t, sparse_t, invalid_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, dense_t, invalid_id_t",
                  TypeCombo<columnar_t, columnar_t, dense_t, invalid_id_t>);
TYPE_TO_STRING_AS("columnar_t, columnar_t, sparse_t, invalid_id_t",
                  TypeCombo<columnar_t, columnar_t, sparse_t, invalid_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, dense_t, invalid_id_t", TypeCombo<columnar_t, row_t, dense_t, invalid_id_t>);
TYPE_TO_STRING_AS("columnar_t, row_t, sparse_t, invalid_id_t", TypeCombo<columnar_t, row_t, sparse_t, invalid_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, dense_t, invalid_id_t", TypeCombo<row_t, columnar_t, dense_t, invalid_id_t>);
TYPE_TO_STRING_AS("row_t, columnar_t, sparse_t, invalid_id_t", TypeCombo<row_t, columnar_t, sparse_t, invalid_id_t>);

namespace power_grid_model_cpp {

/*

source_1 -- node_0 --load_2

source and node inputs are row based.
load input is either row based or columnar.
load update is row based / columnar and dense / sparse.

invalid_id_t tests are for testing the error handling of the model when the id is not found in the update dataset.
optional_id_t tests are for testing the model when the id is not added to the update dataset.

*/
TEST_CASE_TEMPLATE(
    "API update id tests", T, TypeCombo<row_t, row_t, dense_t, with_id_t>, TypeCombo<row_t, row_t, sparse_t, with_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, with_id_t>, TypeCombo<columnar_t, columnar_t, sparse_t, with_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, with_id_t>, TypeCombo<columnar_t, row_t, sparse_t, with_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, with_id_t>, TypeCombo<row_t, columnar_t, sparse_t, with_id_t>,
    TypeCombo<row_t, row_t, dense_t, optional_id_t>, TypeCombo<row_t, row_t, sparse_t, optional_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, optional_id_t>,
    TypeCombo<columnar_t, columnar_t, sparse_t, optional_id_t>, TypeCombo<columnar_t, row_t, dense_t, optional_id_t>,
    TypeCombo<columnar_t, row_t, sparse_t, optional_id_t>, TypeCombo<row_t, columnar_t, dense_t, optional_id_t>,
    TypeCombo<row_t, columnar_t, sparse_t, optional_id_t>, TypeCombo<row_t, row_t, dense_t, invalid_id_t>,
    TypeCombo<row_t, row_t, dense_t, mixed_optional_id_t>, TypeCombo<row_t, row_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, columnar_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, mixed_optional_id_t>,
    TypeCombo<columnar_t, row_t, sparse_t, mixed_optional_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, mixed_optional_id_t>, TypeCombo<row_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, columnar_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<row_t, columnar_t, sparse_t, invalid_id_t>) {

    using namespace std::string_literals;
    using input_type = typename T::input_type;
    using update_type = typename T::update_type;
    using sparsity_type = typename T::sparsity_type;
    using id_check_type = typename T::id_check_type;

    DatasetConst input_dataset{"input", false, 1};
    DatasetConst update_dataset{"update", true, 2};

    std::vector<ID> const node_id{0};
    std::vector<double> const node_u_rated{100.0};
    Buffer node_buffer{PGM_def_input_node, 1};
    node_buffer.set_nan();
    node_buffer.set_value(PGM_def_input_node_id, node_id.data(), -1);
    node_buffer.set_value(PGM_def_input_node_u_rated, node_u_rated.data(), -1);
    input_dataset.add_buffer("node", 1, 1, nullptr, node_buffer);

    std::vector<ID> const source_id{1};
    std::vector<ID> const source_node{0};
    std::vector<int8_t> const source_status{1};
    std::vector<double> const source_u_ref{1.0};
    std::vector<double> const source_sk{1000.0};
    std::vector<double> const source_rx_ratio{0.0};
    Buffer source_buffer{PGM_def_input_source, 1};
    source_buffer.set_nan();
    source_buffer.set_value(PGM_def_input_source_id, source_id.data(), -1);
    source_buffer.set_value(PGM_def_input_source_node, source_node.data(), -1);
    source_buffer.set_value(PGM_def_input_source_status, source_status.data(), -1);
    source_buffer.set_value(PGM_def_input_source_u_ref, source_u_ref.data(), -1);
    source_buffer.set_value(PGM_def_input_source_sk, source_sk.data(), -1);
    source_buffer.set_value(PGM_def_input_source_rx_ratio, source_rx_ratio.data(), -1);
    input_dataset.add_buffer("source", 1, 1, nullptr, source_buffer);

    std::vector<ID> const sym_load_id{2};
    std::vector<ID> const sym_load_node{0};
    std::vector<int8_t> const sym_load_status{1};
    std::vector<int8_t> const sym_load_type{2};
    std::vector<double> const sym_load_p_specified{0.0};
    std::vector<double> const sym_load_q_specified{500.0};
    Buffer sym_load_buffer{PGM_def_input_sym_load, 1};
    sym_load_buffer.set_nan();

    if constexpr (std::is_same_v<input_type, row_t>) {
        sym_load_buffer.set_value(PGM_def_input_sym_load_id, sym_load_id.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_node, sym_load_node.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_status, sym_load_status.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_type, sym_load_type.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_p_specified, sym_load_p_specified.data(), -1);
        sym_load_buffer.set_value(PGM_def_input_sym_load_q_specified, sym_load_q_specified.data(), -1);
        input_dataset.add_buffer("sym_load", 1, 1, nullptr, sym_load_buffer);
    } else {
        input_dataset.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        input_dataset.add_attribute_buffer("sym_load", "id", sym_load_id.data());
        input_dataset.add_attribute_buffer("sym_load", "node", sym_load_node.data());
        input_dataset.add_attribute_buffer("sym_load", "status", sym_load_status.data());
        input_dataset.add_attribute_buffer("sym_load", "type", sym_load_type.data());
        input_dataset.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
        input_dataset.add_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());
    }

    std::vector<Idx> const sym_load_indptr{0, 1, 2};
    std::vector<double> const load_updates_q_specified = {100.0, 300.0};
    auto load_updates_id = [] {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            return std::vector<ID>{99, 2};
        }
        return std::vector<ID>{2, 2};
    }();

    auto source_indptr = [] {
        if constexpr (std::is_same_v<id_check_type, mixed_optional_id_t>) {
            return std::vector<Idx>{0, 1, 1};
        }
        return std::vector<Idx>{0, 0, 0};
    }();
    std::vector<ID> const source_updates_id = {1};

    Buffer source_update_buffer{PGM_def_update_sym_load, 1};
    source_update_buffer.set_nan();
    source_update_buffer.set_value(PGM_def_update_source_id, source_updates_id.data(), -1);

    Buffer sym_load_update_buffer{PGM_def_update_sym_load, 2};
    sym_load_update_buffer.set_nan();
    if constexpr (!std::is_same_v<id_check_type, optional_id_t>) {
        sym_load_update_buffer.set_value(PGM_def_update_sym_load_id, load_updates_id.data(), -1);
    }
    sym_load_update_buffer.set_value(PGM_def_update_sym_load_q_specified, load_updates_q_specified.data(), -1);

    if constexpr (std::is_same_v<update_type, row_t>) {
        if constexpr (std::is_same_v<sparsity_type, dense_t>) {
            update_dataset.add_buffer("sym_load", 1, 2, nullptr, sym_load_update_buffer);
        } else {
            update_dataset.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), sym_load_update_buffer);
        }
        // source is always sparse. the sparsity tag affects the sym_load
        update_dataset.add_buffer("source", -1, source_indptr.back(), source_indptr.data(), source_update_buffer);
    } else {
        if constexpr (std::is_same_v<sparsity_type, dense_t>) {
            update_dataset.add_buffer("sym_load", 1, 2, nullptr, nullptr);
        } else {
            update_dataset.add_buffer("sym_load", -1, 2, sym_load_indptr.data(), nullptr);
        }
        // source is always sparse. the sparsity tag affects the sym_load
        update_dataset.add_buffer("source", -1, source_indptr.back(), source_indptr.data(), nullptr);

        if constexpr (std::is_same_v<id_check_type, mixed_optional_id_t>) {
            update_dataset.add_attribute_buffer("source", "id", source_updates_id.data());
        } else if constexpr (!std::is_same_v<id_check_type, optional_id_t>) {
            update_dataset.add_attribute_buffer("sym_load", "id", load_updates_id.data());
        }
        update_dataset.add_attribute_buffer("sym_load", "q_specified", load_updates_q_specified.data());
    }

    // output dataset
    Buffer batch_node_output{PGM_def_sym_output_node, 2};
    batch_node_output.set_nan();
    DatasetMutable batch_output_dataset{"sym_output", true, 2};
    batch_output_dataset.add_buffer("node", 1, 2, nullptr, batch_node_output);

    Options const batch_options{};
    Model model{50.0, input_dataset};

    SUBCASE("Permanent update") {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            CHECK_THROWS_WITH_AS(model.update(update_dataset), doctest::Contains("The id cannot be found"),
                                 PowerGridError);
        } else {
            CHECK_NOTHROW(model.update(update_dataset));
        }
    }
    SUBCASE("Batch update") {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            CHECK_THROWS_AS(model.calculate(batch_options, batch_output_dataset, update_dataset), PowerGridBatchError);
        } else {
            CHECK_NOTHROW(model.calculate(batch_options, batch_output_dataset, update_dataset));
        }
    }
}

namespace {
using namespace std::string_literals;
using std::numbers::sqrt3;

enum class CalculationSymmetry : Idx { symmetric = PGM_symmetric, asymmetric = PGM_asymmetric };
enum class LoadGenType : IntS {
    const_pq = 0, // constant power
    const_y = 1,  // constant element_admittance (impedance)
    const_i = 2,  // constant current
};
enum class MeasuredTerminalType : IntS {
    branch_from = 0,
    branch_to = 1,
    source = 2,
    shunt = 3,
    load = 4,
    generator = 5,
    branch3_1 = 6,
    branch3_2 = 7,
    branch3_3 = 8,
    node = 9
};

Options get_default_options(PGM_SymmetryType calculation_symmetry, PGM_CalculationMethod calculation_method) {
    Options opt;
    opt.set_calculation_type(PGM_power_flow);
    opt.set_symmetric(calculation_symmetry);
    opt.set_calculation_method(calculation_method);
    return opt;
}

OwningDatasetConst load_dataset(std::string const& json_string) {
    Deserializer deserializer{json_string, PGM_json};
    auto& writable_dataset = deserializer.get_dataset();
    auto owning_dataset = create_owning_dataset(writable_dataset);
    deserializer.parse_to_buffer();
    return owning_dataset;
}

namespace test {
constexpr double z_bus_2 = 1.0 / (0.015 + 0.5e6 / 10e3 / 10e3 * 2);
constexpr double z_total = z_bus_2 + 10.0;
constexpr double u1 = 1.05 * z_bus_2 / (z_bus_2 + 10.0);
constexpr double i = 1.05 * 10e3 / z_total / sqrt3;
constexpr double i_shunt = 0.015 / 0.025 * i;
constexpr double i_load = 0.005 / 0.025 * i;
} // namespace test

auto const state_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1, "u_rated": 10000},
      {"id": 2, "u_rated": 10000},
      {"id": 3, "u_rated": 10000}
    ],
    "line": [
      {"id": 4, "from_node": 1, "to_node": 2, "from_status": 1, "to_status": 1, "r1": 10, "x1": 0, "c1": 0, "tan1": 0, "r0": 10, "x0": 0, "c0": 0, "tan0": 0, "i_n": 1000}
    ],
    "link": [
      {"id": 5, "from_node": 2, "to_node": 3, "from_status": 1, "to_status": 1}
    ],
    "source": [
      {"id": 6, "node": 1, "status": 1, "u_ref": 1.05, "sk": 1000000000000},
      {"id": 10, "node": 3, "status": 0, "u_ref": 1.05, "u_ref_angle": 0, "sk": 1000000000000}
    ],
    "sym_load": [
      {"id": 7, "node": 3, "status": 1, "type": 1, "p_specified": 500000, "q_specified": 0}
    ],
    "asym_load": [
      {"id": 8, "node": 3, "status": 1, "type": 1, "p_specified": [166666.6666666667, 166666.6666666667, 166666.6666666667], "q_specified": [0, 0, 0]}
    ],
    "shunt": [
      {"id": 9, "node": 3, "status": 1, "g1": 0.015, "b1": 0, "g0": 0.015, "b0": 0}
    ]
  }
})json"s;

auto const update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "sym_load": [
        {"id": 7, "status": 1, "p_specified": 2500000}
      ],
      "asym_load": [
        {"id": 8, "status": 0}
      ],
      "shunt": [
        {"id": 9, "status": 0, "b1": 0.02, "b0": 0.02}
      ],
      "source": [
        {"id": 10, "status": 1, "u_ref": 0.84}
      ],
      "link": [
        {"id": 5, "from_status": 1, "to_status": 0}
      ]
    }
  ]
})json"s;

auto const update_vector_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "sym_load": [
        {"id": 7, "status": 1, "p_specified": 2500000}
      ],
      "asym_load": [
        {"id": 8, "status": 0}
      ],
      "shunt": [
        {"id": 9, "status": 0, "b1": 0.02, "b0": 0.02}
      ]
    }
  ]
})json"s;

struct State {
    std::vector<ID> node_id{1, 2, 3};
    std::vector<double> node_u_rated{10e3, 10e3, 10e3};

    std::vector<ID> line_id{4};
    std::vector<ID> line_from_node{1};
    std::vector<ID> line_to_node{2};
    std::vector<IntS> line_from_status{1};
    std::vector<IntS> line_to_status{1};
    std::vector<double> line_r1{10.0};
    std::vector<double> line_x1{0.0};
    std::vector<double> line_c1{0.0};
    std::vector<double> line_tan1{0.0};
    std::vector<double> line_r0{10.0};
    std::vector<double> line_x0{0.0};
    std::vector<double> line_c0{0.0};
    std::vector<double> line_tan0{0.0};
    std::vector<double> line_i_n{1e3};

    std::vector<ID> link_id{5};
    std::vector<ID> link_from_node{2};
    std::vector<ID> link_to_node{3};
    std::vector<IntS> link_from_status{1};
    std::vector<IntS> link_to_status{1};

    std::vector<ID> source_id{6, 10};
    std::vector<ID> source_node{1, 3};
    std::vector<IntS> source_status{1, 0};
    std::vector<double> source_u_ref{1.05, 1.05};
    std::vector<double> source_u_ref_angle{nan, 0.0};
    std::vector<double> source_sk{1e12, 1e12};

    std::vector<ID> sym_load_id{7};
    std::vector<ID> sym_load_node{3};
    std::vector<IntS> sym_load_status{1};
    std::vector<LoadGenType> sym_load_type{LoadGenType::const_y};
    std::vector<double> sym_load_p_specified{0.5e6};
    std::vector<double> sym_load_q_specified{0.0};

    std::vector<ID> asym_load_id{8};
    std::vector<ID> asym_load_node{3};
    std::vector<IntS> asym_load_status{1};
    std::vector<LoadGenType> asym_load_type{LoadGenType::const_y};
    std::vector<double> asym_load_p_specified{0.5e6 / 3.0, 0.5e6 / 3.0, 0.5e6 / 3.0};
    std::vector<double> asym_load_q_specified{0.0, 0.0, 0.0};

    std::vector<ID> shunt_id{9};
    std::vector<ID> shunt_node{3};
    std::vector<IntS> shunt_status{1};
    std::vector<double> shunt_g1{0.015};
    std::vector<double> shunt_b1{0.0};
    std::vector<double> shunt_g0{0.015};
    std::vector<double> shunt_b0{0.0};

    auto get_input_dataset() const {
        DatasetConst result{"input", false, 1};

        result.add_buffer("node", std::ssize(node_id), std::ssize(node_id), nullptr, nullptr);
        result.add_attribute_buffer("node", "id", node_id.data());
        result.add_attribute_buffer("node", "u_rated", node_u_rated.data());

        result.add_buffer("line", std::ssize(line_id), std::ssize(line_id), nullptr, nullptr);
        result.add_attribute_buffer("line", "id", line_id.data());
        result.add_attribute_buffer("line", "from_node", line_from_node.data());
        result.add_attribute_buffer("line", "to_node", line_to_node.data());
        result.add_attribute_buffer("line", "from_status", line_from_status.data());
        result.add_attribute_buffer("line", "to_status", line_to_status.data());
        result.add_attribute_buffer("line", "r1", line_r1.data());
        result.add_attribute_buffer("line", "x1", line_x1.data());
        result.add_attribute_buffer("line", "c1", line_c1.data());
        result.add_attribute_buffer("line", "tan1", line_tan1.data());
        result.add_attribute_buffer("line", "r0", line_r0.data());
        result.add_attribute_buffer("line", "x0", line_x0.data());
        result.add_attribute_buffer("line", "c0", line_c0.data());
        result.add_attribute_buffer("line", "tan0", line_tan0.data());
        result.add_attribute_buffer("line", "i_n", line_i_n.data());

        result.add_buffer("link", std::ssize(link_id), std::ssize(link_id), nullptr, nullptr);
        result.add_attribute_buffer("link", "id", link_id.data());
        result.add_attribute_buffer("link", "from_node", link_from_node.data());
        result.add_attribute_buffer("link", "to_node", link_to_node.data());
        result.add_attribute_buffer("link", "from_status", link_from_status.data());
        result.add_attribute_buffer("link", "to_status", link_to_status.data());

        result.add_buffer("source", std::ssize(source_id), std::ssize(source_id), nullptr, nullptr);
        result.add_attribute_buffer("source", "id", source_id.data());
        result.add_attribute_buffer("source", "node", source_node.data());
        result.add_attribute_buffer("source", "status", source_status.data());
        result.add_attribute_buffer("source", "u_ref", source_u_ref.data());
        result.add_attribute_buffer("source", "u_ref_angle", source_u_ref_angle.data());
        result.add_attribute_buffer("source", "sk", source_sk.data());

        result.add_buffer("sym_load", std::ssize(sym_load_id), std::ssize(sym_load_id), nullptr, nullptr);
        result.add_attribute_buffer("sym_load", "id", sym_load_id.data());
        result.add_attribute_buffer("sym_load", "node", sym_load_node.data());
        result.add_attribute_buffer("sym_load", "status", sym_load_status.data());
        result.add_attribute_buffer("sym_load", "type", sym_load_type.data());
        result.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
        result.add_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());

        result.add_buffer("asym_load", std::ssize(asym_load_id), std::ssize(asym_load_id), nullptr, nullptr);
        result.add_attribute_buffer("asym_load", "id", asym_load_id.data());
        result.add_attribute_buffer("asym_load", "node", asym_load_node.data());
        result.add_attribute_buffer("asym_load", "status", asym_load_status.data());
        result.add_attribute_buffer("asym_load", "type", asym_load_type.data());
        result.add_attribute_buffer("asym_load", "p_specified", asym_load_p_specified.data());
        result.add_attribute_buffer("asym_load", "q_specified", asym_load_q_specified.data());

        result.add_buffer("shunt", std::ssize(shunt_id), std::ssize(shunt_id), nullptr, nullptr);
        result.add_attribute_buffer("shunt", "id", shunt_id.data());
        result.add_attribute_buffer("shunt", "node", shunt_node.data());
        result.add_attribute_buffer("shunt", "status", shunt_status.data());
        result.add_attribute_buffer("shunt", "g1", shunt_g1.data());
        result.add_attribute_buffer("shunt", "b1", shunt_b1.data());
        result.add_attribute_buffer("shunt", "g0", shunt_g0.data());
        result.add_attribute_buffer("shunt", "b0", shunt_b0.data());

        return result;
    }
};
} // namespace

TEST_CASE("API model - all updates") {
    // TODO(mgovers): remove
    // State const state;
    // auto const input_dataset = state.get_input_dataset();
    // Serializer serializer{state.get_input_dataset(), PGM_json};
    // auto const str = serializer.get_to_zero_terminated_string(0, 2);

    auto const owning_input_dataset = load_dataset(state_json);
    auto const& input_dataset = owning_input_dataset.dataset;

    auto const& input_info = input_dataset.get_info();
    auto model = Model{50.0, input_dataset};

    // // update vector
    // std::vector<ID> sym_load_update_id{7};
    // std::vector<IntS> sym_load_update_status{1};
    // std::vector<double> sym_load_update_p_specified{2.5e6};

    // std::vector<ID> asym_load_update_id{8};
    // std::vector<IntS> asym_load_update_status{0};

    // std::vector<ID> shunt_update_id{9};
    // std::vector<IntS> shunt_update_status{0};
    // std::vector<double> shunt_update_b1{0.02};
    // std::vector<double> shunt_update_b0{0.02};

    // // used for test case alternate compute mode
    // std::vector<ID> const shunt_update_2_id{6};
    // std::vector<IntS> const source_update_2_status{0};
    // std::vector<double> const shunt_update_2_b1{0.01};
    // std::vector<double> const shunt_update_2_b0{0.01};

    // std::vector<ID> source_update_id{10};
    // std::vector<IntS> source_update_status{1};
    // std::vector<double> source_update_u_ref{test::u1};

    // std::vector<ID> link_update_id{5};
    // std::vector<IntS> link_update_from_status{1};
    // std::vector<IntS> link_update_to_status{0};

    // DatasetConst update_data{"update", true, 1};
    // update_data.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("sym_load", "id", sym_load_update_id.data());
    // update_data.add_attribute_buffer("sym_load", "status", sym_load_update_status.data());
    // update_data.add_attribute_buffer("sym_load", "p_specified", sym_load_update_p_specified.data());

    // update_data.add_buffer("asym_load", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("asym_load", "id", asym_load_update_id.data());
    // update_data.add_attribute_buffer("asym_load", "status", asym_load_update_status.data());

    // update_data.add_buffer("shunt", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("shunt", "id", shunt_update_id.data());
    // update_data.add_attribute_buffer("shunt", "status", shunt_update_status.data());
    // update_data.add_attribute_buffer("shunt", "b1", shunt_update_b1.data());
    // update_data.add_attribute_buffer("shunt", "b0", shunt_update_b0.data());

    // update_data.add_buffer("source", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("source", "id", source_update_id.data());
    // update_data.add_attribute_buffer("source", "status", source_update_status.data());
    // update_data.add_attribute_buffer("source", "u_ref", source_update_u_ref.data());

    // update_data.add_buffer("link", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("link", "id", link_update_id.data());
    // update_data.add_attribute_buffer("link", "from_status", link_update_from_status.data());
    // update_data.add_attribute_buffer("link", "to_status", link_update_to_status.data());

    // TODO(mgovers): remove
    // Serializer serializer{update_data, PGM_json};
    // auto const str = serializer.get_to_zero_terminated_string(0, 2);

    auto const owning_update_dataset = load_dataset(update_json);
    auto const& update_data = owning_update_dataset.dataset;

    auto const output_dataset_type = "sym_output"s;
    for (Idx comp_type_idx = 0; comp_type_idx < input_info.n_components(); ++comp_type_idx) {
        CAPTURE(comp_type_idx);

        auto const comp_type = input_info.component_name(comp_type_idx);
        CAPTURE(comp_type);

        auto const* comp_meta = MetaData::get_component_by_name(output_dataset_type, comp_type);
        auto const total_elements = input_info.component_total_elements(comp_type_idx);
        auto const elements_per_scenario = input_info.component_elements_per_scenario(comp_type_idx);

        for (Idx attribute_idx = 0; attribute_idx < MetaData::n_attributes(comp_meta); ++attribute_idx) {
            CAPTURE(attribute_idx);
            auto const* attr_meta = MetaData::get_attribute_by_idx(comp_meta, attribute_idx);
            auto const attribute_name = MetaData::attribute_name(attr_meta);
            CAPTURE(attribute_name);

            pgm_type_func_selector(attr_meta, [&model, &update_data, &output_dataset_type, &comp_type, &attribute_name,
                                               elements_per_scenario, total_elements]<typename T>() {
                std::vector<T> sym_output_from_batch(total_elements);
                std::vector<T> sym_output_from_updated_single(total_elements);

                DatasetMutable output_data_from_batch{output_dataset_type, true, 1};
                DatasetMutable output_data_from_updated_single{output_dataset_type, false, 1};

                output_data_from_batch.add_buffer(comp_type, elements_per_scenario, total_elements, nullptr, nullptr);
                output_data_from_updated_single.add_buffer(comp_type, elements_per_scenario, total_elements, nullptr,
                                                           nullptr);

                output_data_from_batch.add_attribute_buffer(comp_type, attribute_name, sym_output_from_batch.data());
                output_data_from_updated_single.add_attribute_buffer(comp_type, attribute_name,
                                                                     sym_output_from_updated_single.data());

                auto opt = get_default_options(PGM_symmetric, PGM_linear);
                model.calculate(opt, output_data_from_batch, update_data);
                model.update(update_data);
                model.calculate(opt, output_data_from_updated_single);

                for (Idx i = 0; i < total_elements; ++i) {
                    CAPTURE(i);
                    CHECK(sym_output_from_batch[i] == sym_output_from_updated_single[i]);
                }
            });
        }
    }
}

TEST_CASE("API model - updates w/ alternating compute mode") {
    auto const owning_input_dataset = load_dataset(state_json);
    auto const& input_dataset = owning_input_dataset.dataset;

    auto model = Model{50.0, input_dataset};

    auto const check_sym = [&model] {
        std::vector<double> sym_node_output_u_pu(3);
        std::vector<double> sym_line_output_i_from(1);
        std::vector<double> sym_source_output_i(2);
        std::vector<double> sym_sym_load_output_i(1);
        std::vector<double> sym_asym_load_output_i(1);
        std::vector<double> sym_shunt_output_i(1);

        DatasetMutable sym_output{"sym_output", false, 1};
        sym_output.add_buffer("node", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("node", "u_pu", sym_node_output_u_pu.data());

        sym_output.add_buffer("line", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("line", "i_from", sym_line_output_i_from.data());

        sym_output.add_buffer("source", 2, 2, nullptr, nullptr);
        sym_output.add_attribute_buffer("source", "i", sym_source_output_i.data());

        sym_output.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("sym_load", "i", sym_sym_load_output_i.data());

        sym_output.add_buffer("asym_load", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("asym_load", "i", sym_asym_load_output_i.data());

        sym_output.add_buffer("shunt", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("shunt", "i", sym_shunt_output_i.data());

        model.calculate(get_default_options(PGM_symmetric, PGM_linear), sym_output);

        CHECK(sym_node_output_u_pu[0] == doctest::Approx(1.05));
        CHECK(sym_node_output_u_pu[1] == doctest::Approx(test::u1));
        CHECK(sym_node_output_u_pu[2] == doctest::Approx(test::u1));
        CHECK(sym_line_output_i_from[0] == doctest::Approx(test::i));
        CHECK(sym_source_output_i[0] == doctest::Approx(test::i));
        CHECK(sym_source_output_i[1] == doctest::Approx(0.0));
        CHECK(sym_sym_load_output_i[0] == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(sym_asym_load_output_i[0] == doctest::Approx(0.0));
        CHECK(sym_shunt_output_i[0] == doctest::Approx(0.0));
    };
    auto const check_asym = [&model] {
        std::vector<double> asym_node_output_u_pu(9);
        std::vector<double> asym_line_output_i_from(3);
        std::vector<double> asym_source_output_i(6);
        std::vector<double> asym_sym_load_output_i(3);
        std::vector<double> asym_asym_load_output_i(3);
        std::vector<double> asym_shunt_output_i(3);

        DatasetMutable asym_output{"asym_output", false, 1};
        asym_output.add_buffer("node", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("node", "u_pu", asym_node_output_u_pu.data());

        asym_output.add_buffer("line", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("line", "i_from", asym_line_output_i_from.data());

        asym_output.add_buffer("source", 2, 2, nullptr, nullptr);
        asym_output.add_attribute_buffer("source", "i", asym_source_output_i.data());

        asym_output.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("sym_load", "i", asym_sym_load_output_i.data());

        asym_output.add_buffer("asym_load", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("asym_load", "i", asym_asym_load_output_i.data());

        asym_output.add_buffer("shunt", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("shunt", "i", asym_shunt_output_i.data());

        model.calculate(get_default_options(PGM_asymmetric, PGM_linear), asym_output);

        CHECK(asym_node_output_u_pu[0 * 3 + 0] == doctest::Approx(1.05));
        CHECK(asym_node_output_u_pu[1 * 3 + 1] == doctest::Approx(test::u1));
        CHECK(asym_node_output_u_pu[2 * 2 + 1] == doctest::Approx(test::u1));
        CHECK(asym_line_output_i_from[0] == doctest::Approx(test::i));
        CHECK(asym_source_output_i[0 * 3 + 1] == doctest::Approx(test::i));
        CHECK(asym_source_output_i[1 * 3 + 2] == doctest::Approx(0.0));
        CHECK(asym_sym_load_output_i[0] == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(asym_asym_load_output_i[1] == doctest::Approx(0.0));
        CHECK(asym_shunt_output_i[2] == doctest::Approx(0.0));
    };

    // // update vector
    // std::vector<ID> sym_load_update_id{7};
    // std::vector<IntS> sym_load_update_status{1};
    // std::vector<double> sym_load_update_p_specified{2.5e6};

    // std::vector<ID> asym_load_update_id{8};
    // std::vector<IntS> asym_load_update_status{0};

    // std::vector<ID> shunt_update_id{9};
    // std::vector<IntS> shunt_update_status{0};
    // std::vector<double> shunt_update_b1{0.02};
    // std::vector<double> shunt_update_b0{0.02};

    // DatasetConst update_data{"update", true, 1};
    // update_data.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("sym_load", "id", sym_load_update_id.data());
    // update_data.add_attribute_buffer("sym_load", "status", sym_load_update_status.data());
    // update_data.add_attribute_buffer("sym_load", "p_specified", sym_load_update_p_specified.data());

    // update_data.add_buffer("asym_load", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("asym_load", "id", asym_load_update_id.data());
    // update_data.add_attribute_buffer("asym_load", "status", asym_load_update_status.data());

    // update_data.add_buffer("shunt", 1, 1, nullptr, nullptr);
    // update_data.add_attribute_buffer("shunt", "id", shunt_update_id.data());
    // update_data.add_attribute_buffer("shunt", "status", shunt_update_status.data());
    // update_data.add_attribute_buffer("shunt", "b1", shunt_update_b1.data());
    // update_data.add_attribute_buffer("shunt", "b0", shunt_update_b0.data());

    // // TODO(mgovers): remove
    // Serializer serializer{update_data, PGM_json};
    // auto const str = serializer.get_to_zero_terminated_string(0, 2);

    auto const owning_update_dataset = load_dataset(update_vector_json);
    auto const& update_data = owning_update_dataset.dataset;

    // This will lead to no topo change but param change
    model.update(update_data);

    check_sym();
    check_asym();

    SUBCASE("No new update") {
        // Math state may be fully cached
    }
    SUBCASE("No new parameter change") {
        // Math state may be fully cached
        model.update(update_data);
    }

    check_asym();
    check_sym();
}

namespace {
auto const incomplete_state_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 1, "u_rated": 10000},
      {"id": 2, "u_rated": 10000},
      {"id": 3, "u_rated": 10000}
    ],
    "line": [
      {"id": 4, "from_node": 1, "to_node": 2, "from_status": 1, "to_status": 1, "r1": 10, "x1": 0, "c1": 0, "tan1": 0, "r0": 10, "x0": 0, "c0": 0, "tan0": 0, "i_n": 1000}
    ],
    "link": [
      {"id": 5, "from_node": 2, "to_node": 3, "from_status": 1, "to_status": 1}
    ],
    "source": [
      {"id": 6, "node": 1, "status": 1, "sk": 1000000000000},
      {"id": 10, "node": 3, "status": 0, "sk": 1000000000000}
    ],
    "sym_load": [
      {"id": 7, "node": 3, "status": 1, "type": 1, "q_specified": 0}
    ],
    "asym_load": [
      {"id": 8, "node": 3, "status": 1, "type": 1, "q_specified": [0, 0, 0]}
    ],
    "shunt": [
      {"id": 9, "node": 3, "status": 1, "g1": 0.015, "b1": 0, "g0": 0.015, "b0": 0}
    ]
  }
})json"s;

auto const incomplete_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"id": 6},
        {"id": 10}
      ],
      "sym_load": [
        {"id": 7}
      ],
      "asym_load": [
        {"id": 8}
      ]
    }
  ]
})json"s;

auto const complete_update_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"id": 6, "u_ref": 1.05},
        {"id": 10, "u_ref": 1.05, "u_ref_angle": 0}
      ],
      "sym_load": [
        {"id": 7, "p_specified": 500000}
      ],
      "asym_load": [
        {"id": 8, "p_specified": [166666.6666666667, 166666.6666666667, 166666.6666666667]}
      ]
    }
  ]
})json"s;

auto get_incomplete_state() -> State {
    State result;

    std::ranges::fill(result.source_u_ref, nan);
    std::ranges::fill(result.source_u_ref_angle, nan);
    std::ranges::fill(result.sym_load_p_specified, nan);
    std::ranges::fill(result.asym_load_p_specified, nan);

    return result;
}
} // namespace

TEST_CASE("API model - incomplete input") {
    State const complete_state; // TODO(mgovers): also migrate
    // State const incomplete_state = get_incomplete_state();

    auto const owning_input_dataset = load_dataset(incomplete_state_json);
    auto const& incomplete_input_data = owning_input_dataset.dataset;

    // // TODO(mgovers): remove
    // Serializer serializer{update_data, PGM_json};
    // auto const str = serializer.get_to_zero_terminated_string(0, 2);

    // auto const owning_update_dataset = load_dataset(update_vector_json);
    // auto const& update_data = owning_update_dataset.dataset;

    auto test_model = Model{50.0, incomplete_input_data};

    for (auto symmetry : {PGM_symmetric, PGM_asymmetric}) {
        CAPTURE(symmetry);

        auto const calculation_symmetry = symmetry == PGM_symmetric ? "Symmetric"s : "Asymmetric"s;
        auto const output_type = symmetry == PGM_symmetric ? "sym_output"s : "asym_output"s;

        SUBCASE(calculation_symmetry.c_str()) {
            auto const* node_output_meta = MetaData::get_component_by_name(output_type, "node");

            Buffer test_node_output(node_output_meta, std::ssize(complete_state.node_id));
            DatasetMutable test_result_data{output_type, true, 1};
            test_result_data.add_buffer("node", test_node_output.size(), test_node_output.size(), nullptr,
                                        test_node_output);

            SUBCASE("Target dataset") {
                CHECK_THROWS_WITH_AS(test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data),
                                     doctest::Contains("Sparse matrix error, possibly singular matrix!"),
                                     PowerGridRegularError);
            }
            SUBCASE("Empty single scenario update dataset") {
                DatasetConst const empty_update_data{"update", true, 1};
                SUBCASE("Single update") {
                    test_model.update(empty_update_data);
                    CHECK_THROWS_WITH_AS(
                        test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data),
                        doctest::Contains("Sparse matrix error, possibly singular matrix!"), PowerGridRegularError);
                }
                SUBCASE("Batch") {
                    CHECK_THROWS_WITH_AS(test_model.calculate(get_default_options(symmetry, PGM_linear),
                                                              test_result_data, empty_update_data),
                                         doctest::Contains("Sparse matrix error, possibly singular matrix!"),
                                         PowerGridRegularError);
                }
            }
            SUBCASE("Incomplete update dataset") {
                // DatasetConst incomplete_update_data{"update", true, 1};
                // incomplete_update_data.add_buffer("source", std::ssize(incomplete_state.source_id),
                //                                   std::ssize(incomplete_state.source_id), nullptr, nullptr);
                // incomplete_update_data.add_attribute_buffer("source", "id", incomplete_state.source_id.data());
                // incomplete_update_data.add_attribute_buffer("source", "u_ref", incomplete_state.source_u_ref.data());
                // incomplete_update_data.add_attribute_buffer("source", "u_ref_angle",
                //                                             incomplete_state.source_u_ref_angle.data());

                // incomplete_update_data.add_buffer("sym_load", std::ssize(incomplete_state.sym_load_id),
                //                                   std::ssize(incomplete_state.sym_load_id), nullptr, nullptr);
                // incomplete_update_data.add_attribute_buffer("sym_load", "id", incomplete_state.sym_load_id.data());
                // incomplete_update_data.add_attribute_buffer("sym_load", "p_specified",
                //                                             incomplete_state.sym_load_p_specified.data());

                // incomplete_update_data.add_buffer("asym_load", std::ssize(incomplete_state.asym_load_id),
                //                                   std::ssize(incomplete_state.asym_load_id), nullptr, nullptr);
                // incomplete_update_data.add_attribute_buffer("asym_load", "id", incomplete_state.asym_load_id.data());
                // incomplete_update_data.add_attribute_buffer("asym_load", "p_specified",
                //                                             incomplete_state.asym_load_p_specified.data());
                // // TODO(mgovers): remove
                // Serializer serializer{incomplete_update_data, PGM_json};
                // auto const str = serializer.get_to_zero_terminated_string(0, 2);

                auto const owning_update_dataset = load_dataset(incomplete_update_json);
                auto const& incomplete_update_data = owning_update_dataset.dataset;

                SUBCASE("Single update") {
                    CHECK_NOTHROW(test_model.update(incomplete_update_data));
                    CHECK_THROWS_WITH_AS(
                        test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data),
                        doctest::Contains("Sparse matrix error, possibly singular matrix!"), PowerGridRegularError);
                }
                SUBCASE("Batch") {
                    CHECK_THROWS_AS(test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data,
                                                         incomplete_update_data),
                                    PowerGridBatchError);
                }
            }
            SUBCASE("Complete update dataset") {
                // DatasetConst complete_update_data{"update", true, 1};
                // complete_update_data.add_buffer("source", std::ssize(complete_state.source_id),
                //                                 std::ssize(complete_state.source_id), nullptr, nullptr);
                // complete_update_data.add_attribute_buffer("source", "id", complete_state.source_id.data());
                // complete_update_data.add_attribute_buffer("source", "u_ref", complete_state.source_u_ref.data());
                // complete_update_data.add_attribute_buffer("source", "u_ref_angle",
                //                                           complete_state.source_u_ref_angle.data());

                // complete_update_data.add_buffer("sym_load", std::ssize(complete_state.sym_load_id),
                //                                 std::ssize(complete_state.sym_load_id), nullptr, nullptr);
                // complete_update_data.add_attribute_buffer("sym_load", "id", complete_state.sym_load_id.data());
                // complete_update_data.add_attribute_buffer("sym_load", "p_specified",
                //                                           complete_state.sym_load_p_specified.data());

                // complete_update_data.add_buffer("asym_load", std::ssize(complete_state.asym_load_id),
                //                                 std::ssize(complete_state.asym_load_id), nullptr, nullptr);
                // complete_update_data.add_attribute_buffer("asym_load", "id", complete_state.asym_load_id.data());
                // complete_update_data.add_attribute_buffer("asym_load", "p_specified",
                //                                           complete_state.asym_load_p_specified.data());
                // // // TODO(mgovers): remove
                // Serializer serializer{complete_update_data, PGM_json};
                // auto const str = serializer.get_to_zero_terminated_string(0, 2);

                auto const owning_update_dataset = load_dataset(complete_update_json);
                auto const& complete_update_data = owning_update_dataset.dataset;

                auto ref_model = Model{50.0, complete_state.get_input_dataset()};
                Buffer ref_node_output(node_output_meta, std::ssize(complete_state.node_id));
                DatasetMutable ref_result_data{output_type, true, 1};
                ref_result_data.add_buffer("node", ref_node_output.size(), ref_node_output.size(), nullptr,
                                           ref_node_output);

                CHECK_NOTHROW(ref_model.calculate(get_default_options(symmetry, PGM_linear), ref_result_data));

                SUBCASE("Single calculation") {
                    test_model.update(complete_update_data);
                    CHECK_NOTHROW(test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data,
                                                       complete_update_data));
                }
                SUBCASE("Batch") {
                    CHECK_NOTHROW(test_model.calculate(get_default_options(symmetry, PGM_linear), test_result_data,
                                                       complete_update_data));
                }

                for (Idx node_idx = 0; node_idx < std::ssize(complete_state.node_id); ++node_idx) {
                    CAPTURE(node_idx);

                    for (Idx attr_idx = 0; attr_idx < MetaData::n_attributes(node_output_meta); ++attr_idx) {
                        auto const* attr_meta = MetaData::get_attribute_by_idx(node_output_meta, attr_idx);
                        auto const attr_name = MetaData::attribute_name(attr_meta);
                        CAPTURE(attr_name);

                        pgm_type_func_selector(attr_meta,
                                               [&test_node_output, &ref_node_output, attr_meta, node_idx]<typename T> {
                                                   T test_value{nan_value<T>()};
                                                   T ref_value{nan_value<T>()};
                                                   test_node_output.get_value(attr_meta, &test_value, node_idx, 0);
                                                   ref_node_output.get_value(attr_meta, &ref_value, node_idx, 0);

                                                   if constexpr (std::is_floating_point_v<T>) {
                                                       CHECK(test_value == doctest::Approx(ref_value));
                                                   } else {
                                                       CHECK(test_value == ref_value);
                                                   }
                                               });
                    }
                }
            }
        }
    }
}

TEST_CASE("API model - Incomplete scenario update followed by complete") {
    State const complete_state;
    State const incomplete_state = get_incomplete_state();

    auto ref_model = Model{50.0, complete_state.get_input_dataset()};
    auto test_model = Model{50.0, incomplete_state.get_input_dataset()};

    constexpr Idx batch_size = 2;
    auto const n_nodes = static_cast<Idx>(complete_state.node_id.size());

    std::vector<ID> mixed_source_update_id{6, 10, 6, 10};
    std::vector<IntS> mixed_source_update_status{1, 1, 1, 1};
    std::vector<double> mixed_source_update_u_ref{nan, nan, 1.05, 1.05};
    std::vector<double> mixed_source_update_u_ref_angle{nan, nan, nan, 0};

    std::vector<ID> mixed_sym_load_update_id{7, 7};
    std::vector<ID> mixed_sym_load_update_status{1, 1};
    std::vector<double> mixed_sym_load_update_p_specified{nan, 0.5e6};
    std::vector<double> mixed_sym_load_update_q_specified{1.0, nan};

    std::vector<ID> mixed_asym_load_update_id{8, 8};
    std::vector<ID> mixed_asym_load_update_status{1, 1};
    std::vector<double> mixed_asym_load_update_p_specified{nan, nan, nan, 0.5e6 / 3.0, 0.5e6 / 3.0, 0.5e6 / 3.0};
    std::vector<double> mixed_asym_load_update_q_specified{1.0, 1.0, 1.0, nan, nan, nan};

    std::vector<Idx> const source_indptr{0, 0, static_cast<Idx>(mixed_source_update_id.size())};

    REQUIRE(source_indptr.size() == batch_size + 1);

    DatasetConst mixed_update_data{"update", true, batch_size};

    mixed_update_data.add_buffer("source", 2, 4, nullptr, nullptr);
    mixed_update_data.add_attribute_buffer("source", "id", mixed_source_update_id.data());
    mixed_update_data.add_attribute_buffer("source", "status", mixed_source_update_status.data());
    mixed_update_data.add_attribute_buffer("source", "u_ref", mixed_source_update_u_ref.data());
    mixed_update_data.add_attribute_buffer("source", "u_ref_angle", mixed_source_update_u_ref_angle.data());

    mixed_update_data.add_buffer("sym_load", 1, 2, nullptr, nullptr);
    mixed_update_data.add_attribute_buffer("sym_load", "id", mixed_sym_load_update_id.data());
    mixed_update_data.add_attribute_buffer("sym_load", "status", mixed_sym_load_update_status.data());
    mixed_update_data.add_attribute_buffer("sym_load", "p_specified", mixed_sym_load_update_p_specified.data());
    mixed_update_data.add_attribute_buffer("sym_load", "q_specified", mixed_sym_load_update_q_specified.data());

    mixed_update_data.add_buffer("asym_load", 1, 2, nullptr, nullptr);
    mixed_update_data.add_attribute_buffer("asym_load", "id", mixed_asym_load_update_id.data());
    mixed_update_data.add_attribute_buffer("asym_load", "status", mixed_asym_load_update_status.data());
    mixed_update_data.add_attribute_buffer("asym_load", "p_specified", mixed_asym_load_update_p_specified.data());
    mixed_update_data.add_attribute_buffer("asym_load", "q_specified", mixed_asym_load_update_q_specified.data());

    DatasetConst second_scenario_update_data{"update", true, 1};

    second_scenario_update_data.add_buffer("source", 2, 2, nullptr, nullptr);
    second_scenario_update_data.add_attribute_buffer("source", "id", mixed_source_update_id.data() + 2);
    second_scenario_update_data.add_attribute_buffer("source", "status", mixed_source_update_status.data() + 2);
    second_scenario_update_data.add_attribute_buffer("source", "u_ref", mixed_source_update_u_ref.data() + 2);
    second_scenario_update_data.add_attribute_buffer("source", "u_ref_angle",
                                                     mixed_source_update_u_ref_angle.data() + 2);

    second_scenario_update_data.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    second_scenario_update_data.add_attribute_buffer("sym_load", "id", mixed_sym_load_update_id.data() + 1);
    second_scenario_update_data.add_attribute_buffer("sym_load", "status", mixed_sym_load_update_status.data() + 1);
    second_scenario_update_data.add_attribute_buffer("sym_load", "p_specified",
                                                     mixed_sym_load_update_p_specified.data() + 1);
    second_scenario_update_data.add_attribute_buffer("sym_load", "q_specified",
                                                     mixed_sym_load_update_q_specified.data() + 1);

    second_scenario_update_data.add_buffer("asym_load", 1, 1, nullptr, nullptr);
    second_scenario_update_data.add_attribute_buffer("asym_load", "id", mixed_asym_load_update_id.data() + 1);
    second_scenario_update_data.add_attribute_buffer("asym_load", "status", mixed_asym_load_update_status.data() + 1);
    second_scenario_update_data.add_attribute_buffer("asym_load", "p_specified",
                                                     mixed_asym_load_update_p_specified.data() + 1);
    second_scenario_update_data.add_attribute_buffer("asym_load", "q_specified",
                                                     mixed_asym_load_update_q_specified.data() + 1);

    for (auto symmetry : {PGM_symmetric, PGM_asymmetric}) {
        CAPTURE(symmetry);

        auto const calculation_symmetry = symmetry == PGM_symmetric ? "Symmetric"s : "Asymmetric"s;
        auto const output_type = symmetry == PGM_symmetric ? "sym_output"s : "asym_output"s;
        auto const n_phases = symmetry == PGM_symmetric ? 1 : 3;

        SUBCASE(calculation_symmetry.c_str()) {
            DatasetMutable test_result_data{output_type, true, batch_size};
            DatasetMutable ref_result_data{output_type, true, 1};

            std::vector<double> test_node_output_u_pu(batch_size * n_nodes * n_phases, nan);
            std::vector<double> ref_node_output_u_pu(n_nodes * n_phases, nan);

            test_result_data.add_buffer("node", n_nodes, batch_size * n_nodes, nullptr, nullptr);
            test_result_data.add_attribute_buffer("node", "u_pu", test_node_output_u_pu.data());

            ref_result_data.add_buffer("node", n_nodes, n_nodes, nullptr, nullptr);
            ref_result_data.add_attribute_buffer("node", "u_pu", ref_node_output_u_pu.data());

            CHECK_THROWS_AS(test_model.calculate(get_default_options(PGM_symmetric, PGM_linear), test_result_data,
                                                 mixed_update_data),
                            PowerGridBatchError);

            ref_model.calculate(get_default_options(PGM_symmetric, PGM_linear), ref_result_data,
                                second_scenario_update_data);

            for (auto node_idx = 0; node_idx < n_nodes; ++node_idx) {
                CAPTURE(node_idx);

                for (auto phase_idx = 0; phase_idx < n_phases; ++phase_idx) {
                    CAPTURE(phase_idx);

                    CHECK(is_nan(test_node_output_u_pu[node_idx * n_phases + phase_idx]));
                    CHECK(test_node_output_u_pu[(n_nodes + node_idx) * n_phases + phase_idx] ==
                          doctest::Approx(ref_node_output_u_pu[node_idx * n_phases + phase_idx]));
                }
            }
        }
    }
}

} // namespace power_grid_model_cpp
