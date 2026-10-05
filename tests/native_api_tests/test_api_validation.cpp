#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model_c/dataset.h>
#include <power_grid_model_c/handle.h>
#include <power_grid_model_c/validation.h>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace {
using CHandle = std::unique_ptr<PGM_Handle, decltype(&PGM_destroy_handle)>;
using ConstDataset = std::unique_ptr<PGM_ConstDataset, decltype(&PGM_destroy_dataset_const)>;
using ValidationResult = std::unique_ptr<PGM_ValidationResult, decltype(&PGM_destroy_validation_result)>;

CHandle create_handle() { return {PGM_create_handle(), &PGM_destroy_handle}; }

ConstDataset create_dataset(PGM_Handle* handle, char const* type = "input", PGM_Idx is_batch = 0,
                            PGM_Idx batch_size = 1) {
    return {PGM_create_dataset_const(handle, type, is_batch, batch_size), &PGM_destroy_dataset_const};
}

void check_validation_failure(PGM_Handle* handle, PGM_ConstDataset const* dataset) {
    ValidationResult result{PGM_validate_input_data(handle, dataset, 1), &PGM_destroy_validation_result};
    CHECK(result == nullptr);
    CHECK(PGM_error_code(handle) == PGM_regular_error);
}
} // namespace

TEST_CASE("Node validation C API accepts valid and empty node data") {
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    auto absent_nodes = create_dataset(handle.get());
    REQUIRE(absent_nodes != nullptr);
    ValidationResult absent_result{PGM_validate_input_data(handle.get(), absent_nodes.get(), 1),
                                   &PGM_destroy_validation_result};
    REQUIRE(absent_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), absent_result.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    auto empty_nodes = create_dataset(handle.get());
    REQUIRE(empty_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), empty_nodes.get(), "node", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult empty_result{PGM_validate_input_data(handle.get(), empty_nodes.get(), 1),
                                  &PGM_destroy_validation_result};
    REQUIRE(empty_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), empty_result.get()) == 0);

    std::array<NodeInput, 1> const nodes{{{1, 100.0}}};
    auto valid_nodes = create_dataset(handle.get());
    REQUIRE(valid_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), valid_nodes.get(), "node", 1, 1, nullptr, nodes.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult valid_result{PGM_validate_input_data(handle.get(), valid_nodes.get(), 1),
                                  &PGM_destroy_validation_result};
    REQUIRE(valid_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), valid_result.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Node validation C API keeps findings alive and does not mutate input") {
    using power_grid_model::na_IntID;
    using power_grid_model::nan;
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    std::array<NodeInput, 5> nodes{{{na_IntID, nan},
                                    {4, std::numeric_limits<double>::infinity()},
                                    {4, -std::numeric_limits<double>::infinity()},
                                    {9, 0.0},
                                    {10, -1.0}}};
    auto const original_nodes = nodes;
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", static_cast<PGM_Idx>(nodes.size()),
                                 static_cast<PGM_Idx>(nodes.size()), nullptr, nodes.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), 1), &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    REQUIRE(PGM_validation_result_issue_count(handle.get(), result.get()) == 5);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    for (std::size_t index{}; index < nodes.size(); ++index) {
        CHECK(nodes[index].id == original_nodes[index].id);
        bool const unchanged_voltage = nodes[index].u_rated == original_nodes[index].u_rated ||
                                       (std::isnan(nodes[index].u_rated) && std::isnan(original_nodes[index].u_rated));
        CHECK(unchanged_voltage);
    }

    dataset.reset();
    PGM_ValidationIssue const infinity_issue = PGM_validation_result_get_issue(handle.get(), result.get(), 2);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    CHECK(infinity_issue.kind == PGM_validation_issue_infinity);
    REQUIRE(infinity_issue.component != nullptr);
    REQUIRE(infinity_issue.field != nullptr);
    CHECK(std::string_view{infinity_issue.component} == "node");
    CHECK(std::string_view{infinity_issue.field} == "u_rated");
    REQUIRE(infinity_issue.ids != nullptr);
    CHECK(infinity_issue.n_ids == 2);
    CHECK(infinity_issue.ids[0] == 4);
    CHECK(infinity_issue.ids[1] == 4);
}

TEST_CASE("Node validation C API supports columnar buffers") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    std::array<PGM_ID, 3> const ids{1, 2, 3};
    std::array<double, 3> const voltages{std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity(), 100.0};
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", 3, 3, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "id", ids.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "u_rated", voltages.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), 1), &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    REQUIRE(PGM_validation_result_issue_count(handle.get(), result.get()) == 2);
    PGM_ValidationIssue const infinity_issue = PGM_validation_result_get_issue(handle.get(), result.get(), 0);
    PGM_ValidationIssue const positive_issue = PGM_validation_result_get_issue(handle.get(), result.get(), 1);
    CHECK(infinity_issue.kind == PGM_validation_issue_infinity);
    CHECK(infinity_issue.n_ids == 2);
    CHECK(positive_issue.kind == PGM_validation_issue_not_greater_than_zero);
    CHECK(positive_issue.n_ids == 1);
    CHECK(positive_issue.ids[0] == 2);
}

TEST_CASE("Line validation C API selects symmetry and returns structured findings") {
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {2, 100.0}}};
    LineInput line{};
    line.id = 1;
    line.from_node = 1;
    line.to_node = 9;
    line.from_status = 2;
    line.to_status = 1;
    line.r1 = 0.1;
    line.x1 = 0.2;
    line.c1 = 0.3;
    line.tan1 = 0.4;
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", 2, 2, nullptr, nodes.data());
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "line", 1, 1, nullptr, &line);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult symmetric_result{PGM_validate_input_data(handle.get(), dataset.get(), 1),
                                      &PGM_destroy_validation_result};
    REQUIRE(symmetric_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), symmetric_result.get()) == 3);

    ValidationResult asymmetric_result{PGM_validate_input_data(handle.get(), dataset.get(), 0),
                                       &PGM_destroy_validation_result};
    REQUIRE(asymmetric_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), asymmetric_result.get()) == 7);

    line.id = 1;
    line.to_node = 9;
    line.r1 = 0.0;
    line.x1 = 0.0;
    line.i_n = -1.0;
    ValidationResult invalid_result{PGM_validate_input_data(handle.get(), dataset.get(), 1),
                                    &PGM_destroy_validation_result};
    REQUIRE(invalid_result != nullptr);
    REQUIRE(PGM_validation_result_issue_count(handle.get(), invalid_result.get()) == 5);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    dataset.reset();
    PGM_ValidationIssue reference_issue{};
    PGM_ValidationIssue boolean_issue{};
    PGM_ValidationIssue pair_issue{};
    PGM_ValidationIssue cross_unique_issue{};
    for (PGM_Idx issue_idx{}; issue_idx < PGM_validation_result_issue_count(handle.get(), invalid_result.get());
         ++issue_idx) {
        PGM_ValidationIssue const issue =
            PGM_validation_result_get_issue(handle.get(), invalid_result.get(), issue_idx);
        switch (issue.kind) {
        case PGM_validation_issue_invalid_id_reference:
            reference_issue = issue;
            break;
        case PGM_validation_issue_not_boolean:
            boolean_issue = issue;
            break;
        case PGM_validation_issue_two_values_zero:
            pair_issue = issue;
            break;
        case PGM_validation_issue_cross_component_not_unique:
            cross_unique_issue = issue;
            break;
        default:
            break;
        }
    }
    CHECK(reference_issue.reference_component != nullptr);
    CHECK(std::string_view{reference_issue.reference_component} == "node");
    CHECK(boolean_issue.kind == PGM_validation_issue_not_boolean);
    REQUIRE(pair_issue.n_fields == 2);
    CHECK(std::string_view{pair_issue.fields[0].field} == "r1");
    CHECK(std::string_view{pair_issue.fields[1].field} == "x1");
    REQUIRE(cross_unique_issue.n_fields == 2);
    CHECK(cross_unique_issue.n_objects == 2);
    CHECK(cross_unique_issue.objects[0].id == 1);
    CHECK(std::string_view{cross_unique_issue.objects[0].component} == "line");
    CHECK(std::string_view{cross_unique_issue.objects[1].component} == "node");
}

TEST_CASE("Line validation C API rejects invalid symmetry argument") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), 2), &PGM_destroy_validation_result};
    CHECK(result == nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
}

TEST_CASE("Line validation C API supports mixed row and columnar buffers") {
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {2, 100.0}}};
    std::array<PGM_ID, 1> const ids{3};
    std::array<PGM_ID, 1> const from_nodes{1};
    std::array<PGM_ID, 1> const to_nodes{2};
    std::array<power_grid_model::IntS, 1> const statuses{1};
    std::array<double, 1> const r1{0.1};
    std::array<double, 1> const x1{0.2};
    std::array<double, 1> const c1{0.3};
    std::array<double, 1> const tan1{0.4};
    std::array<double, 1> const r0{1.0};
    std::array<double, 1> const x0{0.0};
    std::array<double, 1> const c0{0.0};
    std::array<double, 1> const tan0{0.0};
    std::array<double, 1> const i_n{std::numeric_limits<double>::quiet_NaN()};
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", 2, 2, nullptr, nodes.data());
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "line", 1, 1, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "id", ids.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "from_node", from_nodes.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "to_node", to_nodes.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "from_status", statuses.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "to_status", statuses.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "r1", r1.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "x1", x1.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "c1", c1.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "tan1", tan1.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "r0", r0.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "x0", x0.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "c0", c0.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "tan0", tan0.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "line", "i_n", i_n.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), 1), &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), result.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Line validation reports references when nodes are absent") {
    using power_grid_model::LineInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    LineInput line{};
    line.id = 3;
    line.from_node = 1;
    line.to_node = 2;
    line.from_status = 1;
    line.to_status = 1;
    line.r1 = 0.1;
    line.x1 = 0.2;
    line.c1 = 0.3;
    line.tan1 = 0.4;
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "line", 1, 1, nullptr, &line);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), 1), &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    REQUIRE(PGM_validation_result_issue_count(handle.get(), result.get()) == 2);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Node validation C API rejects unsupported dataset shapes") {
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);

    auto update_dataset = create_dataset(handle.get(), "update");
    REQUIRE(update_dataset != nullptr);
    check_validation_failure(handle.get(), update_dataset.get());

    auto batch_dataset = create_dataset(handle.get(), "input", 1, 2);
    REQUIRE(batch_dataset != nullptr);
    check_validation_failure(handle.get(), batch_dataset.get());

    auto unsupported_dataset = create_dataset(handle.get());
    REQUIRE(unsupported_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), unsupported_dataset.get(), "link", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    check_validation_failure(handle.get(), unsupported_dataset.get());

    NodeInput const node{1, 100.0};
    auto mixed_dataset = create_dataset(handle.get());
    REQUIRE(mixed_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "node", 1, 1, nullptr, &node);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "link", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    check_validation_failure(handle.get(), mixed_dataset.get());

    check_validation_failure(handle.get(), nullptr);
}
