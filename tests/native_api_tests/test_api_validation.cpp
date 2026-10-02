#include "validation/node.hpp"
#include "validation/rules.hpp"

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
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
using power_grid_model::ID;
using power_grid_model::NodeInput;
using power_grid_model_c::validation::Issue;
using power_grid_model_c::validation::Issues;
using power_grid_model_c::validation::Rule;

struct TestRecord {
    ID id;
    double value;
};

auto const test_id = [](TestRecord const& record) { return record.id; };
auto const test_value = [](TestRecord const& record) { return record.value; };

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

TEST_CASE("Node validation required fields") {
    using power_grid_model::na_IntID;
    using power_grid_model::nan;

    std::array<TestRecord, 3> const records{{{na_IntID, nan}, {2, 1.0}, {3, nan}}};
    Issues issues;

    power_grid_model_c::validation::rules::required<TestRecord>(records, "node", "id", test_id, test_id, issues);
    power_grid_model_c::validation::rules::required<TestRecord>(records, "node", "u_rated", test_value, test_id,
                                                                issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == Rule::missing_value);
    CHECK(issues[0].field == "id");
    std::vector<ID> const missing_id{na_IntID};
    CHECK(issues[0].ids == missing_id);
    CHECK(issues[1].field == "u_rated");
    std::vector<ID> const missing_voltage{na_IntID, 3};
    CHECK(issues[1].ids == missing_voltage);
}

TEST_CASE("Node validation finite and greater-than-zero rules") {
    using power_grid_model::nan;

    std::array<TestRecord, 5> const records{{{1, std::numeric_limits<double>::infinity()},
                                             {2, -std::numeric_limits<double>::infinity()},
                                             {3, 0.0},
                                             {4, -1.0},
                                             {5, nan}}};
    Issues issues;

    power_grid_model_c::validation::rules::finite<TestRecord>(records, "node", "u_rated", test_value, test_id, issues);
    power_grid_model_c::validation::rules::greater_than_zero<TestRecord>(records, "node", "u_rated", test_value,
                                                                         test_id, issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == Rule::infinity);
    std::vector<ID> const infinite_ids{1, 2};
    CHECK(issues[0].ids == infinite_ids);
    CHECK(issues[1].rule == Rule::not_greater_than_zero);
    std::vector<ID> const nonpositive_ids{2, 3, 4};
    CHECK(issues[1].ids == nonpositive_ids);
}

TEST_CASE("Node validation uniqueness rule preserves duplicate multiplicity") {
    std::array<TestRecord, 5> const records{{{5, 1.0}, {5, 2.0}, {8, 3.0}, {8, 4.0}, {8, 5.0}}};
    Issues issues;

    power_grid_model_c::validation::rules::unique<TestRecord>(records, "node", "id", test_id, test_id, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].rule == Rule::not_unique);
    std::vector<ID> const duplicate_ids{5, 5, 8, 8, 8};
    CHECK(issues[0].ids == duplicate_ids);
}

TEST_CASE("Node validation composition supports row and columnar data") {
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::na_IntID;
    using power_grid_model::nan;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    std::array<NodeInput, 5> const records{{{na_IntID, nan},
                                            {4, std::numeric_limits<double>::infinity()},
                                            {4, -std::numeric_limits<double>::infinity()},
                                            {9, 0.0},
                                            {10, -1.0}}};
    ConstDataset row_dataset{false, 1, "input", meta_data};
    row_dataset.add_buffer("node", static_cast<Idx>(records.size()), static_cast<Idx>(records.size()), nullptr,
                           records.data());

    Issues row_issues;
    power_grid_model_c::validation::validate_node(row_dataset, row_issues);

    REQUIRE(row_issues.size() == 5);
    CHECK(row_issues[0].rule == Rule::missing_value);
    CHECK(row_issues[0].field == "id");
    CHECK(row_issues[1].rule == Rule::missing_value);
    CHECK(row_issues[1].field == "u_rated");
    CHECK(row_issues[2].rule == Rule::infinity);
    CHECK(row_issues[3].rule == Rule::not_unique);
    CHECK(row_issues[4].rule == Rule::not_greater_than_zero);
    CHECK(row_issues[2].ids == std::vector<ID>{4, 4});
    CHECK(row_issues[3].ids == std::vector<ID>{4, 4});
    CHECK(row_issues[4].ids == std::vector<ID>{4, 9, 10});

    std::array<ID, 5> const ids{na_IntID, 4, 4, 9, 10};
    std::array<double, 5> const voltages{nan, std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity(), 0.0, -1.0};
    ConstDataset columnar_dataset{false, 1, "input", meta_data};
    columnar_dataset.add_buffer("node", static_cast<Idx>(ids.size()), static_cast<Idx>(ids.size()), nullptr, nullptr);
    columnar_dataset.add_attribute_buffer("node", "id", ids.data());
    columnar_dataset.add_attribute_buffer("node", "u_rated", voltages.data());

    Issues columnar_issues;
    power_grid_model_c::validation::validate_node(columnar_dataset, columnar_issues);
    CHECK(columnar_issues == row_issues);

    ConstDataset absent_nodes{false, 1, "input", meta_data};
    Issues absent_issues;
    power_grid_model_c::validation::validate_node(absent_nodes, absent_issues);
    CHECK(absent_issues.empty());

    ConstDataset empty_nodes{false, 1, "input", meta_data};
    empty_nodes.add_buffer("node", 0, 0, nullptr, nullptr);
    Issues empty_issues;
    power_grid_model_c::validation::validate_node(empty_nodes, empty_issues);
    CHECK(empty_issues.empty());
}

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
