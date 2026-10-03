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
    ValidationResult result{PGM_validate_input_data(handle, dataset), &PGM_destroy_validation_result};
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
    ValidationResult absent_result{PGM_validate_input_data(handle.get(), absent_nodes.get()),
                                   &PGM_destroy_validation_result};
    REQUIRE(absent_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), absent_result.get()) == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    auto empty_nodes = create_dataset(handle.get());
    REQUIRE(empty_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), empty_nodes.get(), "node", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult empty_result{PGM_validate_input_data(handle.get(), empty_nodes.get()),
                                  &PGM_destroy_validation_result};
    REQUIRE(empty_result != nullptr);
    CHECK(PGM_validation_result_issue_count(handle.get(), empty_result.get()) == 0);

    std::array<NodeInput, 1> const nodes{{{1, 100.0}}};
    auto valid_nodes = create_dataset(handle.get());
    REQUIRE(valid_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), valid_nodes.get(), "node", 1, 1, nullptr, nodes.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult valid_result{PGM_validate_input_data(handle.get(), valid_nodes.get()),
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

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get()), &PGM_destroy_validation_result};
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

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get()), &PGM_destroy_validation_result};
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
    PGM_dataset_const_add_buffer(handle.get(), unsupported_dataset.get(), "line", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    check_validation_failure(handle.get(), unsupported_dataset.get());

    NodeInput const node{1, 100.0};
    auto mixed_dataset = create_dataset(handle.get());
    REQUIRE(mixed_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "node", 1, 1, nullptr, &node);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "line", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    check_validation_failure(handle.get(), mixed_dataset.get());

    check_validation_failure(handle.get(), nullptr);
}
