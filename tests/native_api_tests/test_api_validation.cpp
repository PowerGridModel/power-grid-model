#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/update.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model_c/dataset.h>
#include <power_grid_model_c/handle.h>
#include <power_grid_model_c/options.h>
#include <power_grid_model_c/validation.h>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <limits>
#include <memory>
#include <string>

namespace {
using CHandle = std::unique_ptr<PGM_Handle, decltype(&PGM_destroy_handle)>;
using ConstDataset = std::unique_ptr<PGM_ConstDataset, decltype(&PGM_destroy_dataset_const)>;
using COptions = std::unique_ptr<PGM_Options, decltype(&PGM_destroy_options)>;
using ValidationResult = std::unique_ptr<PGM_ValidationResult, decltype(&PGM_destroy_validation_result)>;

CHandle create_handle() { return {PGM_create_handle(), &PGM_destroy_handle}; }
COptions create_options(PGM_Handle* handle) { return {PGM_create_options(handle), &PGM_destroy_options}; }

ConstDataset create_dataset(PGM_Handle* handle, char const* type = "input", PGM_Idx is_batch = 0,
                            PGM_Idx batch_size = 1) {
    return {PGM_create_dataset_const(handle, type, is_batch, batch_size), &PGM_destroy_dataset_const};
}

void check_validation_failure(PGM_Handle* handle, PGM_ConstDataset const* dataset) {
    auto options = create_options(handle);
    ValidationResult result{PGM_validate_input_data(handle, dataset, options.get()), &PGM_destroy_validation_result};
    CHECK(result == nullptr);
    CHECK(PGM_error_code(handle) == PGM_regular_error);
}

std::string get_validation_report(PGM_Handle* handle, PGM_ValidationResult* result) {
    char const* data{};
    PGM_Idx size{};
    PGM_validation_result_get_json(handle, result, &data, &size);
    if (data == nullptr || size < 0) {
        return {};
    }
    return {data, static_cast<std::size_t>(size)};
}

void check_validation_success(PGM_Handle* handle, PGM_ConstDataset const* dataset) {
    auto options = create_options(handle);
    ValidationResult result{PGM_validate_input_data(handle, dataset, options.get()), &PGM_destroy_validation_result};
    CHECK(result != nullptr);
    CHECK(PGM_error_code(handle) == PGM_no_error);
    if (result != nullptr) {
        auto const report = nlohmann::json::parse(get_validation_report(handle, result.get()));
        CHECK(report.at("valid") == true);
    }
}
} // namespace

TEST_CASE("Node validation C API accepts valid and empty node data") {
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

    auto absent_nodes = create_dataset(handle.get());
    REQUIRE(absent_nodes != nullptr);
    ValidationResult absent_result{PGM_validate_input_data(handle.get(), absent_nodes.get(), options.get()),
                                   &PGM_destroy_validation_result};
    REQUIRE(absent_result != nullptr);
    auto const absent_report = nlohmann::json::parse(get_validation_report(handle.get(), absent_result.get()));
    CHECK(absent_report.at("schema_version") == 3);
    CHECK(absent_report.at("validation_type") == "input");
    CHECK(absent_report.at("valid") == true);
    CHECK(absent_report.at("results").empty());
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    auto empty_nodes = create_dataset(handle.get());
    REQUIRE(empty_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), empty_nodes.get(), "node", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult empty_result{PGM_validate_input_data(handle.get(), empty_nodes.get(), options.get()),
                                  &PGM_destroy_validation_result};
    REQUIRE(empty_result != nullptr);
    auto const empty_report = nlohmann::json::parse(get_validation_report(handle.get(), empty_result.get()));
    CHECK(empty_report.at("valid") == true);
    CHECK(empty_report.at("results").empty());

    std::array<NodeInput, 1> const nodes{{{1, 100.0}}};
    auto valid_nodes = create_dataset(handle.get());
    REQUIRE(valid_nodes != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), valid_nodes.get(), "node", 1, 1, nullptr, nodes.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult valid_result{PGM_validate_input_data(handle.get(), valid_nodes.get(), options.get()),
                                  &PGM_destroy_validation_result};
    REQUIRE(valid_result != nullptr);
    auto const valid_report = nlohmann::json::parse(get_validation_report(handle.get(), valid_result.get()));
    CHECK(valid_report.at("valid") == true);
    CHECK(valid_report.at("results").empty());
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Node validation C API keeps findings alive and does not mutate input") {
    using power_grid_model::na_IntID;
    using power_grid_model::nan;
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

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

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    char const* report_data{};
    PGM_Idx report_size{};
    PGM_validation_result_get_json(handle.get(), result.get(), &report_data, &report_size);
    REQUIRE(report_data != nullptr);
    REQUIRE(report_size > 0);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    std::string const report_copy{report_data, static_cast<std::size_t>(report_size)};
    auto const report = nlohmann::json::parse(report_copy);
    CHECK(report.at("schema_version") == 3);
    CHECK(report.at("validation_type") == "input");
    CHECK(report.at("valid") == false);
    auto const& issues = report.at("results").at(0).at("issues");
    REQUIRE(issues.size() == 9);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    for (std::size_t index{}; index < nodes.size(); ++index) {
        CHECK(nodes[index].id == original_nodes[index].id);
        bool const unchanged_voltage = nodes[index].u_rated == original_nodes[index].u_rated ||
                                       (std::isnan(nodes[index].u_rated) && std::isnan(original_nodes[index].u_rated));
        CHECK(unchanged_voltage);
    }

    char const* repeated_report_data{};
    PGM_Idx repeated_report_size{};
    PGM_validation_result_get_json(handle.get(), result.get(), &repeated_report_data, &repeated_report_size);
    CHECK(repeated_report_data == report_data);
    CHECK(repeated_report_size == report_size);
    CHECK(std::string{repeated_report_data, static_cast<std::size_t>(repeated_report_size)} == report_copy);

    dataset.reset();
    char const* report_after_dataset_destroy{};
    PGM_Idx report_size_after_dataset_destroy{};
    PGM_validation_result_get_json(handle.get(), result.get(), &report_after_dataset_destroy,
                                   &report_size_after_dataset_destroy);
    CHECK(report_after_dataset_destroy == report_data);
    CHECK(report_size_after_dataset_destroy == report_size);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
    std::size_t infinity_issue_count{};
    for (auto const& issue : issues) {
        CHECK_FALSE(issue.contains("fields"));
        CHECK_FALSE(issue.contains("objects"));
        CHECK(issue.contains("row"));
        if (issue.at("kind") == "infinity") {
            ++infinity_issue_count;
            CHECK(issue.at("component") == "node");
            CHECK(issue.at("field") == "u_rated");
            CHECK(issue.at("id") == 4);
            CHECK(issue.at("row") >= 1);
            CHECK(issue.at("row") <= 2);
            CHECK(issue.at("details").is_object());
        }
    }
    CHECK(infinity_issue_count == 2);
}

TEST_CASE("Node validation C API supports columnar buffers") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

    std::array<PGM_ID, 3> const ids{1, 2, 3};
    std::array<double, 3> const voltages{std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity(), 100.0};
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), dataset.get(), "node", 3, 3, nullptr, nullptr);
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "id", ids.data());
    PGM_dataset_const_add_attribute_buffer(handle.get(), dataset.get(), "node", "u_rated", voltages.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    auto const report = nlohmann::json::parse(get_validation_report(handle.get(), result.get()));
    CHECK(report.at("valid") == false);
    auto const& issues = report.at("results").at(0).at("issues");
    REQUIRE(issues.size() == 3);
    CHECK(issues.at(0).at("kind") == "infinity");
    CHECK(issues.at(0).at("id") == 1);
    CHECK(issues.at(0).at("row") == 0);
    CHECK(issues.at(1).at("kind") == "infinity");
    CHECK(issues.at(1).at("id") == 2);
    CHECK(issues.at(1).at("row") == 1);
    CHECK(issues.at(2).at("kind") == "not_greater_than");
    CHECK(issues.at(2).at("details").at("ref_value") == 0);
    CHECK(issues.at(2).at("id") == 2);
}

TEST_CASE("Node validation C API resets JSON outputs on errors") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);
    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);

    char const* data = "stale";
    PGM_Idx size = 99;
    PGM_validation_result_get_json(handle.get(), nullptr, &data, &size);
    CHECK(data == nullptr);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);

    size = 99;
    PGM_validation_result_get_json(handle.get(), result.get(), nullptr, &size);
    CHECK(size == 0);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);

    data = "stale";
    PGM_validation_result_get_json(handle.get(), result.get(), &data, nullptr);
    CHECK(data == nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
}

TEST_CASE("Line validation C API selects symmetry and returns structured findings") {
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto options = create_options(handle.get());

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

    ValidationResult symmetric_result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                                      &PGM_destroy_validation_result};
    REQUIRE(symmetric_result != nullptr);
    auto const symmetric_report = nlohmann::json::parse(get_validation_report(handle.get(), symmetric_result.get()));
    auto const symmetric_issue_count = symmetric_report.at("results").at(0).at("issues").size();
    CHECK(symmetric_issue_count > 0);

    PGM_set_symmetric(handle.get(), options.get(), 0);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult asymmetric_result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                                       &PGM_destroy_validation_result};
    REQUIRE(asymmetric_result != nullptr);
    auto const asymmetric_report = nlohmann::json::parse(get_validation_report(handle.get(), asymmetric_result.get()));
    auto const asymmetric_issue_count = asymmetric_report.at("results").at(0).at("issues").size();
    CHECK(asymmetric_issue_count > 0);
    CHECK(symmetric_issue_count != asymmetric_issue_count);

    line.id = 1;
    line.to_node = 9;
    line.r1 = 0.0;
    line.x1 = 0.0;
    line.i_n = -1.0;
    PGM_set_symmetric(handle.get(), options.get(), 1);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult invalid_result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                                    &PGM_destroy_validation_result};
    REQUIRE(invalid_result != nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);

    dataset.reset();
    auto const invalid_report = nlohmann::json::parse(get_validation_report(handle.get(), invalid_result.get()));
    auto const& issues = invalid_report.at("results").at(0).at("issues");
    CHECK_FALSE(issues.empty());
    auto const find_issue = [&issues](char const* kind) -> nlohmann::json const* {
        for (auto const& issue : issues) {
            if (issue.at("kind") == kind) {
                return &issue;
            }
        }
        return nullptr;
    };

    auto const* reference_issue = find_issue("invalid_id_reference");
    REQUIRE(reference_issue != nullptr);
    CHECK(reference_issue->at("details").at("reference").at("component") == "node");
    CHECK(reference_issue->at("component") == "line");
    CHECK(reference_issue->at("field") == "to_node");
    CHECK(reference_issue->at("id") == 1);
    CHECK(reference_issue->at("row") == 0);

    auto const* boolean_issue = find_issue("not_boolean");
    REQUIRE(boolean_issue != nullptr);

    auto const* pair_issue = find_issue("two_values_zero");
    REQUIRE(pair_issue != nullptr);
    bool has_r1{};
    bool has_x1{};
    for (auto const& issue : issues) {
        if (issue.at("kind") == "two_values_zero") {
            has_r1 = has_r1 || issue.at("field") == "r1";
            has_x1 = has_x1 || issue.at("field") == "x1";
        }
    }
    CHECK(has_r1);
    CHECK(has_x1);

    auto const* positive_issue = find_issue("not_greater_than");
    REQUIRE(positive_issue != nullptr);
    CHECK(positive_issue->at("component") == "line");
    CHECK(positive_issue->at("field") == "i_n");
    CHECK(positive_issue->at("details").at("ref_value") == 0);

    std::size_t cross_unique_issue_count{};
    bool has_line_duplicate{};
    bool has_node_duplicate{};
    for (auto const& issue : issues) {
        if (issue.at("kind") != "cross_component_not_unique") {
            continue;
        }
        ++cross_unique_issue_count;
        CHECK(issue.at("field") == "id");
        CHECK(issue.at("id") == 1);
        CHECK(issue.at("row") == 0);
        has_line_duplicate = has_line_duplicate || issue.at("component") == "line";
        has_node_duplicate = has_node_duplicate || issue.at("component") == "node";
    }
    CHECK(cross_unique_issue_count == 2);
    CHECK(has_line_duplicate);
    CHECK(has_node_duplicate);
}

TEST_CASE("Line validation C API rejects invalid symmetry argument") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto options = create_options(handle.get());
    auto dataset = create_dataset(handle.get());
    REQUIRE(dataset != nullptr);

    PGM_set_symmetric(handle.get(), options.get(), 2);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    CHECK(result == nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);

    ValidationResult null_options_result{PGM_validate_input_data(handle.get(), dataset.get(), nullptr),
                                         &PGM_destroy_validation_result};
    CHECK(null_options_result == nullptr);
    CHECK(PGM_error_code(handle.get()) == PGM_regular_error);
}

TEST_CASE("Line validation C API supports mixed row and columnar buffers") {
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

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

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    auto const report = nlohmann::json::parse(get_validation_report(handle.get(), result.get()));
    CHECK(report.at("valid") == true);
    CHECK(report.at("results").empty());
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Line validation reports references when nodes are absent") {
    using power_grid_model::LineInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

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

    ValidationResult result{PGM_validate_input_data(handle.get(), dataset.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    auto const report = nlohmann::json::parse(get_validation_report(handle.get(), result.get()));
    CHECK(report.at("results").at(0).at("issues").size() == 2);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Node validation C API rejects invalid shapes and ignores unsupported components") {
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
    check_validation_success(handle.get(), unsupported_dataset.get());

    NodeInput const node{1, 100.0};
    auto mixed_dataset = create_dataset(handle.get());
    REQUIRE(mixed_dataset != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "node", 1, 1, nullptr, &node);
    PGM_dataset_const_add_buffer(handle.get(), mixed_dataset.get(), "link", 0, 0, nullptr, nullptr);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);
    check_validation_success(handle.get(), mixed_dataset.get());

    check_validation_failure(handle.get(), nullptr);
}

TEST_CASE("Batch validation C API reports scenario-grouped node and line findings") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::BranchUpdate;
    using power_grid_model::LineInput;
    using power_grid_model::na_IntS;
    using power_grid_model::NodeInput;

    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {2, 100.0}}};
    LineInput line{};
    line.id = 3;
    line.from_node = 1;
    line.to_node = 2;
    line.from_status = 1;
    line.to_status = 1;
    line.r1 = 0.1;
    line.x1 = 0.2;
    line.c1 = 0.0;
    line.tan1 = 0.0;
    line.i_n = 100.0;

    auto input = create_dataset(handle.get());
    REQUIRE(input != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), input.get(), "node", 2, 2, nullptr, nodes.data());
    PGM_dataset_const_add_buffer(handle.get(), input.get(), "line", 1, 1, nullptr, &line);
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    std::array<BaseUpdate, 3> const node_updates{{{1}, {1}, {99}}};
    std::array<BranchUpdate, 3> const line_updates{{{3, 2, na_IntS}, {3, na_IntS, na_IntS}, {99, na_IntS, na_IntS}}};
    auto updates = create_dataset(handle.get(), "update", 1, 3);
    REQUIRE(updates != nullptr);
    PGM_dataset_const_add_buffer(handle.get(), updates.get(), "node", 1, 3, nullptr, node_updates.data());
    PGM_dataset_const_add_buffer(handle.get(), updates.get(), "line", 1, 3, nullptr, line_updates.data());
    REQUIRE(PGM_error_code(handle.get()) == PGM_no_error);

    ValidationResult result{PGM_validate_batch_data(handle.get(), input.get(), updates.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    auto const report = nlohmann::json::parse(get_validation_report(handle.get(), result.get()));
    CHECK(report.at("schema_version") == 3);
    CHECK(report.at("validation_type") == "batch");
    CHECK(report.at("valid") == false);
    auto const& results = report.at("results");
    REQUIRE(results.size() == 2);
    CHECK(results.at(0).at("scenario") == 0);
    CHECK_FALSE(results.at(0).contains("status"));
    REQUIRE(results.at(0).at("issues").size() == 1);
    CHECK(results.at(0).at("issues").at(0).at("kind") == "not_boolean");
    CHECK(results.at(0).at("issues").at(0).at("component") == "line");
    CHECK(results.at(0).at("issues").at(0).at("field") == "from_status");
    CHECK(results.at(0).at("issues").at(0).at("id") == 3);
    CHECK(results.at(0).at("issues").at(0).at("row") == 0);
    CHECK(results.at(1).at("scenario") == 2);
    CHECK_FALSE(results.at(1).contains("status"));
    REQUIRE(results.at(1).at("issues").size() == 2);
    for (auto const& issue : results.at(1).at("issues")) {
        CHECK(issue.at("kind") == "id_not_in_dataset");
        CHECK(issue.at("id") == 99);
        CHECK(issue.at("row") == 0);
        CHECK(issue.at("details").at("reference").at("dataset") == "input");
    }
    CHECK(line.from_status == 1);
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}

TEST_CASE("Batch validation C API returns an empty valid zero-scenario report") {
    auto const handle = create_handle();
    REQUIRE(handle != nullptr);
    auto const options = create_options(handle.get());
    auto const input = create_dataset(handle.get());
    auto const updates = create_dataset(handle.get(), "update", 1, 0);
    REQUIRE(input != nullptr);
    REQUIRE(updates != nullptr);

    ValidationResult result{PGM_validate_batch_data(handle.get(), input.get(), updates.get(), options.get()),
                            &PGM_destroy_validation_result};
    REQUIRE(result != nullptr);
    auto const report = nlohmann::json::parse(get_validation_report(handle.get(), result.get()));
    CHECK(report.at("schema_version") == 3);
    CHECK(report.at("validation_type") == "batch");
    CHECK(report.at("valid") == true);
    CHECK(report.at("results").empty());
    CHECK(PGM_error_code(handle.get()) == PGM_no_error);
}
