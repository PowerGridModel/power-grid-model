#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/component/load_gen.hpp>
#include <power_grid_model/component/source.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/node.hpp>
#include <power_grid_model/validation/rules.hpp>
#include <power_grid_model/validation/updates.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {
using power_grid_model::ID;
using power_grid_model::NodeInput;
using power_grid_model::validation::Issues;
using power_grid_model::validation::Rule;

struct TestRecord {
    ID id;
    double value;
};

struct CopyTrackedRecord {
    ID id;
    double value;
    inline static std::size_t copy_count{};

    CopyTrackedRecord(ID record_id, double record_value) : id{record_id}, value{record_value} {}
    CopyTrackedRecord(CopyTrackedRecord const& other) : id{other.id}, value{other.value} { ++copy_count; }
};

struct PairRecord {
    ID id;
    double first;
    double second;
};

struct ReferenceRecord {
    ID id;
    ID reference;
};

auto const test_id = [](TestRecord const& record) { return record.id; };
auto const test_value = [](TestRecord const& record) { return record.value; };
auto const copy_tracked_id = [](CopyTrackedRecord const& record) { return record.id; };
auto const pair_id = [](PairRecord const& record) { return record.id; };
auto const pair_first = [](PairRecord const& record) { return record.first; };
auto const pair_second = [](PairRecord const& record) { return record.second; };
auto const reference_id = [](ReferenceRecord const& record) { return record.id; };
auto const reference_value = [](ReferenceRecord const& record) { return record.reference; };
} // namespace

namespace power_grid_model::validation {

template <> struct InputValidator<Source> {
    static void validate(ConstDataset const& dataset, bool, ValidationContext const&, Issues& issues) {
        using Record = Source::InputType;
        auto const get_id = [](Record const& record) { return record.id; };
        auto const get_status = [](Record const& record) { return record.status; };
        dataset.for_each_component<meta_data::input_getter_s, Source>([&](auto const& rows) {
            rules::boolean<Record>(rows, "source", "status", get_status, get_id, issues);
        });
    }
};

} // namespace power_grid_model::validation

TEST_CASE("Node validation required fields") {
    using power_grid_model::na_IntID;
    using power_grid_model::nan;

    std::array<TestRecord, 3> const records{{{na_IntID, nan}, {2, 1.0}, {3, nan}}};
    Issues issues;

    power_grid_model::validation::rules::required<TestRecord>(records, "node", "id", test_id, test_id, issues);
    power_grid_model::validation::rules::required<TestRecord>(records, "node", "u_rated", test_value, test_id, issues);

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

    power_grid_model::validation::rules::finite<TestRecord>(records, "node", "u_rated", test_value, test_id, issues);
    power_grid_model::validation::rules::greater_than_zero<TestRecord>(records, "node", "u_rated", test_value, test_id,
                                                                       issues);

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

    power_grid_model::validation::rules::unique<TestRecord>(records, "node", "id", test_id, test_id, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues[0].rule == Rule::not_unique);
    std::vector<ID> const duplicate_ids{5, 5, 8, 8, 8};
    CHECK(issues[0].ids == duplicate_ids);
}

TEST_CASE("Node validation uniqueness does not copy row records") {
    std::array<CopyTrackedRecord, 3> const records{{{5, 1.0}, {5, 2.0}, {8, 3.0}}};
    CopyTrackedRecord::copy_count = 0;
    Issues issues;

    power_grid_model::validation::rules::unique<CopyTrackedRecord>(records, "node", "id", copy_tracked_id,
                                                                   copy_tracked_id, issues);

    CHECK(CopyTrackedRecord::copy_count == 0);
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].ids == std::vector<ID>{5, 5});
}

TEST_CASE("Line validation boolean and two-fields-zero rules") {
    using power_grid_model::IntS;
    using power_grid_model::na_IntS;
    using power_grid_model::nan;

    std::array<TestRecord, 4> const statuses{{{1, 0.0}, {2, 1.0}, {3, -2.0}, {4, static_cast<double>(na_IntS)}}};
    Issues issues;
    auto const get_status = [](TestRecord const& record) { return static_cast<IntS>(record.value); };
    power_grid_model::validation::rules::boolean<TestRecord>(statuses, "line", "from_status", get_status, test_id,
                                                             issues);

    std::array<PairRecord, 4> const impedances{{{1, 0.0, 0.0}, {2, 0.0, 1.0}, {3, 1.0, 0.0}, {4, nan, 0.0}}};
    power_grid_model::validation::rules::not_both_zero<PairRecord>(impedances, "line", "r1", "x1", pair_first,
                                                                   pair_second, pair_id, issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == Rule::not_boolean);
    CHECK(issues[0].ids == std::vector<ID>{3, 4});
    CHECK(issues[1].rule == Rule::two_values_zero);
    CHECK(issues[1].ids == std::vector<ID>{1});
    CHECK(issues[1].fields ==
          std::vector<power_grid_model::validation::FieldReference>{{"line", "r1"}, {"line", "x1"}});
}

TEST_CASE("Line validation node references and cross-component uniqueness") {
    std::array<ReferenceRecord, 3> const lines{{{10, 1}, {11, 2}, {12, 3}}};
    std::unordered_set<ID> const node_lookup{1, 2};
    Issues issues;
    power_grid_model::validation::rules::valid_id_reference<ReferenceRecord>(
        lines, "line", "from_node", "node", reference_value, reference_id, node_lookup, issues);

    std::unordered_map<std::string_view, std::vector<ID>> const ids_by_component{
        {"node", {1, 2, 3}}, {"line", {3, 4, 5}}};
    power_grid_model::validation::rules::cross_unique(ids_by_component, issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == Rule::invalid_id_reference);
    CHECK(issues[0].reference_component == "node");
    CHECK(issues[0].ids == std::vector<ID>{12});
    CHECK(issues[1].rule == Rule::cross_component_not_unique);
    CHECK(issues[1].fields ==
            std::vector<power_grid_model::validation::FieldReference>{{"line", "id"}, {"node", "id"}});
    CHECK(issues[1].objects == std::vector<power_grid_model::validation::ObjectReference>{{"line", 3}, {"node", 3}});
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
    power_grid_model::validation::validate_node(row_dataset, row_issues);

    REQUIRE(row_issues.size() == 5);
    CHECK(row_issues[0].rule == Rule::missing_value);
    CHECK(row_issues[0].field == "id");
        CHECK(row_issues[0].objects ==
            std::vector<power_grid_model::validation::ObjectReference>{{"node", na_IntID, 0}});
    CHECK(row_issues[1].rule == Rule::missing_value);
    CHECK(row_issues[1].field == "u_rated");
            CHECK(row_issues[1].objects ==
                std::vector<power_grid_model::validation::ObjectReference>{{"node", na_IntID, 0}});
    CHECK(row_issues[2].rule == Rule::infinity);
        CHECK(row_issues[2].objects == std::vector<power_grid_model::validation::ObjectReference>{{"node", 4, 1},
                                                                     {"node", 4, 2}});
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
    power_grid_model::validation::validate_node(columnar_dataset, columnar_issues);
    CHECK(columnar_issues == row_issues);

    ConstDataset absent_nodes{false, 1, "input", meta_data};
    Issues absent_issues;
    power_grid_model::validation::validate_node(absent_nodes, absent_issues);
    CHECK(absent_issues.empty());

    ConstDataset empty_nodes{false, 1, "input", meta_data};
    empty_nodes.add_buffer("node", 0, 0, nullptr, nullptr);
    Issues empty_issues;
    power_grid_model::validation::validate_node(empty_nodes, empty_issues);
    CHECK(empty_issues.empty());
}

TEST_CASE("Line validation applies symmetry-dependent required fields") {
    using power_grid_model::ConstDataset;
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {2, 100.0}}};
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

    ConstDataset dataset{false, 1, "input", meta_data};
    dataset.add_buffer("node", static_cast<power_grid_model::Idx>(nodes.size()),
                       static_cast<power_grid_model::Idx>(nodes.size()), nullptr, nodes.data());
    dataset.add_buffer("line", 1, 1, nullptr, &line);

    Issues symmetric_issues = power_grid_model::validation::validate_input_dataset(dataset, true);
    CHECK(symmetric_issues.empty());

    Issues asymmetric_issues = power_grid_model::validation::validate_input_dataset(dataset, false);
    REQUIRE(asymmetric_issues.size() == 4);
    CHECK(asymmetric_issues[0].rule == Rule::missing_value);
    CHECK(asymmetric_issues[0].field == "r0");
    CHECK(asymmetric_issues[1].field == "x0");
    CHECK(asymmetric_issues[2].field == "c0");
    CHECK(asymmetric_issues[3].field == "tan0");
}

TEST_CASE("Batch input validation applies node and line updates independently") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::BranchUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::LineInput;
    using power_grid_model::na_IntS;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::BatchIssues;

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {2, 100.0}}};
    LineInput line{};
    line.id = 4;
    line.from_node = 1;
    line.to_node = 2;
    line.from_status = 1;
    line.to_status = 1;
    line.r1 = 0.1;
    line.x1 = 0.2;
    line.c1 = 0.0;
    line.tan1 = 0.0;
    line.i_n = 100.0;

    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", static_cast<Idx>(nodes.size()), static_cast<Idx>(nodes.size()), nullptr, nodes.data());
    input.add_buffer("line", 1, 1, nullptr, &line);

    std::array<BaseUpdate, 2> const node_updates{{{1}, {2}}};
    std::array<BranchUpdate, 2> const line_updates{{{4, 2, na_IntS}, {4, na_IntS, 0}}};
    ConstDataset updates{true, 2, "update", meta_data};
    updates.add_buffer("node", 1, static_cast<Idx>(node_updates.size()), nullptr, node_updates.data());
    updates.add_buffer("line", 1, static_cast<Idx>(line_updates.size()), nullptr, line_updates.data());

    BatchIssues const issues = power_grid_model::validation::validate_batch_dataset(input, updates, true);

    REQUIRE(issues.size() == 2);
    REQUIRE(issues[0].size() == 1);
    CHECK(issues[0][0].rule == Rule::not_boolean);
    CHECK(issues[0][0].component == "line");
    CHECK(issues[0][0].field == "from_status");
    CHECK(issues[0][0].ids == std::vector<ID>{4});
    CHECK(issues[1].empty());
    CHECK(line.from_status == 1);
    CHECK(line.to_status == 1);
}

TEST_CASE("Batch input validation reports invalid update IDs and accepts empty batches") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::nan;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::BatchIssues;

    std::array<NodeInput, 1> const nodes{{{1, nan}}};
    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", 1, 1, nullptr, nodes.data());

    std::array<BaseUpdate, 1> const unknown_id{{{5}}};
    ConstDataset updates{true, 1, "update", meta_data};
    updates.add_buffer("node", 1, 1, nullptr, unknown_id.data());

    BatchIssues const issues = power_grid_model::validation::validate_batch_dataset(input, updates, true);
    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].size() == 1);
    CHECK(issues[0][0].rule == Rule::id_not_in_dataset);
    CHECK(issues[0][0].component == "node");
    CHECK(issues[0][0].ids == std::vector<ID>{5});

    ConstDataset empty_updates{true, 0, "update", meta_data};
    BatchIssues const empty_issues = power_grid_model::validation::validate_batch_dataset(input, empty_updates, true);
    CHECK(empty_issues.empty());
}

TEST_CASE("Batch input validation preserves invalid update ID order") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::BatchIssues;

    std::array<NodeInput, 1> const nodes{{{1, 100.0}}};
    std::array<BaseUpdate, 3> const node_updates{{{5}, {4}, {5}}};
    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", 1, 1, nullptr, nodes.data());
    ConstDataset updates{true, 1, "update", meta_data};
    updates.add_buffer("node", 3, 3, nullptr, node_updates.data());

    BatchIssues const issues = power_grid_model::validation::validate_batch_dataset(input, updates, true);

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].size() == 1);
    CHECK(issues[0][0].rule == Rule::id_not_in_dataset);
    CHECK(issues[0][0].ids == std::vector<ID>{5, 4, 5});
}

TEST_CASE("Batch input validation rejects ambiguous update IDs") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {1, 110.0}}};
    std::array<BaseUpdate, 1> const node_updates{{{1}}};
    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", static_cast<Idx>(nodes.size()), static_cast<Idx>(nodes.size()), nullptr, nodes.data());
    ConstDataset updates{true, 1, "update", meta_data};
    updates.add_buffer("node", 1, 1, nullptr, node_updates.data());

    auto const issues = power_grid_model::validation::validate_batch_dataset(input, updates, true);

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].size() == 1);
    CHECK(issues[0][0].rule == Rule::invalid_update_id);
    CHECK(issues[0][0].ids == std::vector<ID>{1});
    CHECK(issues[0][0].objects ==
          std::vector<power_grid_model::validation::ObjectReference>{{"node", 1, 0}});
}

TEST_CASE("Batch validation report separates identity and scenario outcomes") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::BranchUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::LineInput;
    using power_grid_model::NodeInput;
    using power_grid_model::na_IntS;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::ValidationStatus;
    using power_grid_model::validation::ValidationType;

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

    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", static_cast<Idx>(nodes.size()), static_cast<Idx>(nodes.size()), nullptr, nodes.data());
    input.add_buffer("line", 1, 1, nullptr, &line);

    std::array<BaseUpdate, 3> const node_updates{{{1}, {1}, {99}}};
    std::array<BranchUpdate, 3> const line_updates{{{3, 2, na_IntS}, {3, na_IntS, na_IntS}, {99, na_IntS, na_IntS}}};
    ConstDataset updates{true, 3, "update", meta_data};
    updates.add_buffer("node", 1, static_cast<Idx>(node_updates.size()), nullptr, node_updates.data());
    updates.add_buffer("line", 1, static_cast<Idx>(line_updates.size()), nullptr, line_updates.data());

    auto const report = power_grid_model::validation::validate_batch_report(input, updates, true);

    CHECK(report.type == ValidationType::batch);
    REQUIRE(report.results.size() == 4);
    CHECK_FALSE(report.results[0].scenario.has_value());
    CHECK(report.results[0].status == ValidationStatus::passed);
    CHECK(report.results[0].issues.empty());
    CHECK(report.results[1].scenario == 0);
    CHECK(report.results[1].status == ValidationStatus::failed);
    REQUIRE(report.results[1].issues.size() == 1);
    CHECK(report.results[1].issues[0].rule == Rule::not_boolean);
    CHECK(report.results[2].scenario == 1);
    CHECK(report.results[2].status == ValidationStatus::passed);
    CHECK(report.results[2].issues.empty());
    CHECK(report.results[3].scenario == 2);
    CHECK(report.results[3].status == ValidationStatus::blocked);
    REQUIRE(report.results[3].issues.size() == 2);
    CHECK(report.results[3].issues[0].rule == Rule::id_not_in_dataset);

    ConstDataset no_scenarios{true, 0, "update", meta_data};
    auto const empty_report = power_grid_model::validation::validate_batch_report(input, no_scenarios, true);
    CHECK(empty_report.results.empty());
}

TEST_CASE("Batch validation report blocks scenarios after input identity findings") {
    using power_grid_model::BaseUpdate;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::NodeInput;
    using power_grid_model::meta_data::meta_data_gen::meta_data;
    using power_grid_model::validation::ValidationStatus;

    std::array<NodeInput, 2> const nodes{{{1, 100.0}, {1, 110.0}}};
    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", static_cast<Idx>(nodes.size()), static_cast<Idx>(nodes.size()), nullptr, nodes.data());
    ConstDataset updates{true, 1, "update", meta_data};
    BaseUpdate const node_update{1};
    updates.add_buffer("node", 1, 1, nullptr, &node_update);

    auto const report = power_grid_model::validation::validate_batch_report(input, updates, true);

    REQUIRE(report.results.size() == 2);
    CHECK(report.results[0].status == ValidationStatus::failed);
    REQUIRE(report.results[0].issues.size() == 1);
    CHECK(report.results[0].issues[0].rule == Rule::not_unique);
    CHECK(report.results[1].status == ValidationStatus::blocked);
    REQUIRE(report.results[1].issues.size() == 1);
    CHECK(report.results[1].issues[0].rule == Rule::invalid_update_id);
    CHECK(report.results[1].issues[0].objects ==
          std::vector<power_grid_model::validation::ObjectReference>{{"node", 1, 0}});
}

TEST_CASE("Batch input validation applies metadata-driven source updates") {
    using power_grid_model::Source;
    using power_grid_model::SourceInput;
    using power_grid_model::SourceUpdate;
    using power_grid_model::nan;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    SourceInput input{};
    input.id = 1;
    input.node = 2;
    input.status = 1;
    input.u_ref = 1.0;
    input.u_ref_angle = 0.5;
    input.sk = 10.0;
    input.rx_ratio = 0.2;
    input.z01_ratio = 3.0;

    SourceUpdate update{};
    update.id = 1;
    update.status = 4;
    update.u_ref = 0.8;
    update.u_ref_angle = nan;

    std::vector<SourceInput> inputs{input};
    std::vector<SourceUpdate> updates{update};
    std::vector<std::size_t> const indices{0};
    power_grid_model::validation::batch_detail::apply_component_updates<Source>(inputs, updates, indices, meta_data);

    CHECK(inputs[0].status == 4);
    CHECK(inputs[0].u_ref == 0.8);
    CHECK(inputs[0].u_ref_angle == 0.5);
    CHECK(inputs[0].sk == 10.0);
    CHECK(inputs[0].rx_ratio == 0.2);
    CHECK(inputs[0].z01_ratio == 3.0);
    CHECK(input.status == 1);
    CHECK(input.u_ref == 1.0);
}

TEST_CASE("Batch input validation overlays asymmetric updates per phase") {
    using power_grid_model::AsymLoad;
    using power_grid_model::nan;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    using Input = AsymLoad::InputType;
    using Update = AsymLoad::UpdateType;
    Input input{};
    input.id = 7;
    input.p_specified(0) = 1.0;
    input.p_specified(1) = 2.0;
    input.p_specified(2) = 3.0;
    input.q_specified(0) = 4.0;
    input.q_specified(1) = 5.0;
    input.q_specified(2) = 6.0;

    Update update{};
    update.id = 7;
    update.p_specified(0) = 10.0;
    update.p_specified(1) = nan;
    update.p_specified(2) = 30.0;
    update.q_specified(0) = nan;
    update.q_specified(1) = 50.0;
    update.q_specified(2) = nan;

    std::vector<Input> inputs{input};
    std::vector<Update> updates{update};
    std::vector<std::size_t> const indices{0};
    power_grid_model::validation::batch_detail::apply_component_updates<AsymLoad>(inputs, updates, indices, meta_data);

    CHECK(inputs[0].p_specified(0) == 10.0);
    CHECK(inputs[0].p_specified(1) == 2.0);
    CHECK(inputs[0].p_specified(2) == 30.0);
    CHECK(inputs[0].q_specified(0) == 4.0);
    CHECK(inputs[0].q_specified(1) == 50.0);
    CHECK(inputs[0].q_specified(2) == 6.0);
    CHECK(input.p_specified(0) == 1.0);
    CHECK(input.q_specified(1) == 5.0);
}

TEST_CASE("Validation component list extends input and batch dispatch") {
    using power_grid_model::ComponentList;
    using power_grid_model::ConstDataset;
    using power_grid_model::Idx;
    using power_grid_model::Line;
    using power_grid_model::Node;
    using power_grid_model::NodeInput;
    using power_grid_model::Source;
    using power_grid_model::SourceInput;
    using power_grid_model::SourceUpdate;
    using power_grid_model::meta_data::meta_data_gen::meta_data;

    std::array<NodeInput, 1> const nodes{{{1, 100.0}}};
    SourceInput source{};
    source.id = 3;
    source.node = 1;
    source.status = 1;
    source.u_ref = 1.0;
    source.u_ref_angle = 0.0;
    source.sk = 10.0;
    source.rx_ratio = 0.2;
    source.z01_ratio = 3.0;

    ConstDataset input{false, 1, "input", meta_data};
    input.add_buffer("node", static_cast<Idx>(nodes.size()), static_cast<Idx>(nodes.size()), nullptr, nodes.data());
    input.add_buffer("source", 1, 1, nullptr, &source);
    using TestComponents = ComponentList<Node, Line, Source>;
    CHECK(power_grid_model::validation::validate_input_dataset(input, true, TestComponents{}).empty());

    SourceUpdate source_update{};
    source_update.id = 3;
    source_update.status = 4;
    ConstDataset updates{true, 1, "update", meta_data};
    updates.add_buffer("source", 1, 1, nullptr, &source_update);

    auto const scenario_issues =
        power_grid_model::validation::validate_batch_dataset(input, updates, true, TestComponents{});
    REQUIRE(scenario_issues.size() == 1);
    REQUIRE(scenario_issues[0].size() == 1);
    CHECK(scenario_issues[0][0].rule == Rule::not_boolean);
    CHECK(scenario_issues[0][0].component == "source");
    CHECK(scenario_issues[0][0].field == "status");
    CHECK(scenario_issues[0][0].ids == std::vector<ID>{3});
    CHECK(source.status == 1);
}
