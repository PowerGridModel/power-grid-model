// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <string>

namespace power_grid_model_cpp {
namespace {
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
    TypeCombo<row_t, columnar_t, dense_t, mixed_optional_id_t>, TypeCombo<row_t, columnar_t, sparse_t, optional_id_t>,
    TypeCombo<row_t, row_t, dense_t, invalid_id_t>, TypeCombo<row_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, columnar_t, sparse_t, invalid_id_t>,
    TypeCombo<columnar_t, row_t, dense_t, invalid_id_t>, TypeCombo<columnar_t, row_t, sparse_t, invalid_id_t>,
    TypeCombo<row_t, columnar_t, dense_t, invalid_id_t>, TypeCombo<row_t, columnar_t, sparse_t, invalid_id_t>) {

    using namespace std::string_literals;
    using input_type = typename T::input_type;
    using update_type = typename T::update_type;
    using sparsity_type = typename T::sparsity_type;
    using id_check_type = typename T::id_check_type;

    DatasetConst input_dataset{"input", 0, 1};
    DatasetConst update_dataset{"update", 1, 2};

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
    DatasetMutable batch_output_dataset{"sym_output", 1, 2};
    batch_output_dataset.add_buffer("node", 1, 2, nullptr, batch_node_output);

    Options const batch_options{};
    Model model{50.0, input_dataset};

    SUBCASE("Permanent update") {
        if constexpr (std::is_same_v<id_check_type, invalid_id_t>) {
            CHECK_THROWS_AS(model.update(update_dataset), PowerGridError);
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

} // namespace power_grid_model_cpp
