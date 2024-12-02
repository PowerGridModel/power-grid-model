// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_cpp/meta_data.hpp>
#include <power_grid_model_cpp/model.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <limits>
#include <numbers>
#include <span>
#include <vector>

namespace power_grid_model_cpp {
namespace {
using std::numbers::pi;
using std::numbers::sqrt3;

constexpr Idx default_option{-1};
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr double deg_120 = 2.0 / 3.0 * pi;
constexpr double deg_240 = 4.0 / 3.0 * pi;

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

Options get_default_options(PGM_SymmetryType calculation_symmetry, PGM_CalculationMethod calculation_method,
                            Idx threading = default_option) {
    Options opt;
    opt.set_calculation_type(PGM_power_flow);
    opt.set_symmetric(calculation_symmetry);
    opt.set_calculation_method(calculation_method);
    if (threading != default_option) {
        opt.set_threading(threading);
    }
    return opt;
}

// struct regular_update {
//     using update_type = permanent_update_t;
// };

// struct cached_update {
//     using update_type = cached_update_t;
// };

namespace test {
constexpr double z_bus_2 = 1.0 / (0.015 + 0.5e6 / 10e3 / 10e3 * 2);
constexpr double z_total = z_bus_2 + 10.0;
constexpr double u1 = 1.05 * z_bus_2 / (z_bus_2 + 10.0);
constexpr double i = 1.05 * 10e3 / z_total / sqrt3;
constexpr double i_shunt = 0.015 / 0.025 * i;
constexpr double i_load = 0.005 / 0.025 * i;
} // namespace test

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
        DatasetConst result{"input", 0, 1};

        result.add_buffer("node", node_id.size(), node_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("node", "id", node_id.data());
        result.add_attribute_buffer("node", "u_rated", node_u_rated.data());

        result.add_buffer("line", line_id.size(), line_id.size(), nullptr, nullptr);
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

        result.add_buffer("link", link_id.size(), link_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("link", "id", link_id.data());
        result.add_attribute_buffer("link", "from_node", link_from_node.data());
        result.add_attribute_buffer("link", "to_node", link_to_node.data());
        result.add_attribute_buffer("link", "from_status", link_from_status.data());
        result.add_attribute_buffer("link", "to_status", link_to_status.data());

        result.add_buffer("source", source_id.size(), source_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("source", "id", source_id.data());
        result.add_attribute_buffer("source", "node", source_node.data());
        result.add_attribute_buffer("source", "status", source_status.data());
        result.add_attribute_buffer("source", "u_ref", source_u_ref.data());
        result.add_attribute_buffer("source", "u_ref_angle", source_u_ref_angle.data());
        result.add_attribute_buffer("source", "sk", source_sk.data());

        result.add_buffer("sym_load", sym_load_id.size(), sym_load_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("sym_load", "id", sym_load_id.data());
        result.add_attribute_buffer("sym_load", "node", sym_load_node.data());
        result.add_attribute_buffer("sym_load", "status", sym_load_status.data());
        result.add_attribute_buffer("sym_load", "type", sym_load_type.data());
        result.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
        result.add_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());

        result.add_buffer("asym_load", asym_load_id.size(), asym_load_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("asym_load", "id", asym_load_id.data());
        result.add_attribute_buffer("asym_load", "node", asym_load_node.data());
        result.add_attribute_buffer("asym_load", "status", asym_load_status.data());
        result.add_attribute_buffer("asym_load", "type", asym_load_type.data());
        result.add_attribute_buffer("asym_load", "p_specified", asym_load_p_specified.data());
        result.add_attribute_buffer("asym_load", "q_specified", asym_load_q_specified.data());

        result.add_buffer("shunt", shunt_id.size(), shunt_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("shunt", "id", shunt_id.data());
        result.add_attribute_buffer("shunt", "node", shunt_node.data());
        result.add_attribute_buffer("shunt", "status", shunt_status.data());
        result.add_attribute_buffer("shunt", "g1", shunt_g1.data());
        result.add_attribute_buffer("shunt", "b1", shunt_b1.data());
        result.add_attribute_buffer("shunt", "g0", shunt_g0.data());
        result.add_attribute_buffer("shunt", "b0", shunt_b0.data());

        return result;
    }

    // // output vector
    // std::vector<NodeOutput<symmetric_t>> sym_node = std::vector<NodeOutput<symmetric_t>>(3);
    // std::vector<BranchOutput<symmetric_t>> sym_branch = std::vector<BranchOutput<symmetric_t>>(2);
    // std::vector<ApplianceOutput<symmetric_t>> sym_appliance = std::vector<ApplianceOutput<symmetric_t>>(5);
    // std::vector<NodeOutput<asymmetric_t>> asym_node = std::vector<NodeOutput<asymmetric_t>>(3);
    // std::vector<BranchOutput<asymmetric_t>> asym_branch = std::vector<BranchOutput<asymmetric_t>>(2);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_appliance = std::vector<ApplianceOutput<asymmetric_t>>(5);

    // // individual symmetric
    // std::vector<BranchOutput<symmetric_t>> sym_line = std::vector<BranchOutput<symmetric_t>>(1);
    // std::vector<BranchOutput<symmetric_t>> sym_link = std::vector<BranchOutput<symmetric_t>>(1);
    // std::vector<ApplianceOutput<symmetric_t>> sym_load_sym = std::vector<ApplianceOutput<symmetric_t>>(1);
    // std::vector<ApplianceOutput<symmetric_t>> sym_load_asym = std::vector<ApplianceOutput<symmetric_t>>(1);
    // std::vector<ApplianceOutput<symmetric_t>> sym_source = std::vector<ApplianceOutput<symmetric_t>>(2);
    // std::vector<ApplianceOutput<symmetric_t>> sym_shunt = std::vector<ApplianceOutput<symmetric_t>>(1);

    // // individual asymmetric
    // std::vector<BranchOutput<asymmetric_t>> asym_line = std::vector<BranchOutput<asymmetric_t>>(1);
    // std::vector<BranchOutput<asymmetric_t>> asym_link = std::vector<BranchOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_load_sym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_load_asym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_source = std::vector<ApplianceOutput<asymmetric_t>>(2);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_shunt = std::vector<ApplianceOutput<asymmetric_t>>(1);

    // // update vector
    // std::vector<SymLoadGenUpdate> sym_load_update{{7, 1, 1.0e6, nan}};
    // std::vector<AsymLoadGenUpdate> asym_load_update{{8, 0, RealValue<asymmetric_t>{nan},
    // RealValue<asymmetric_t>{nan}}}; std::vector<ShuntUpdate> shunt_update{{9, 0, nan, 0.02, nan, 0.02}};
    // std::vector<ShuntUpdate> shunt_update_2{{6, 0, nan, 0.01, nan, 0.01}}; // used for test case alternate compute
    // mode std::vector<SourceUpdate> source_update{{10, 1, test::u1, nan}}; std::vector<BranchUpdate> link_update{{5,
    // 1, 0}};

    // // batch update vector
    // std::vector<SymLoadGenUpdate> batch_sym_load_update{{7, 1, 1.0e6, nan}, {7}, {7}, {7}, {7}};
    // std::vector<AsymLoadGenUpdate> batch_asym_load_update{
    //     {8, 0, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}, {8}, {8}, {8}, {8}};
    // std::vector<ShuntUpdate> batch_shunt_update{{9, 0, nan, 0.02, nan, 0.02}, {9}, {9}, {9}, {9}};
    // std::vector<SourceUpdate> batch_source_update{{10, 1, test::u1, nan}, {10}, {10}, {10}, {10}};
    // std::vector<BranchUpdate> batch_link_update{{5, 1, 0}, {5}, {5}, {5}, {5}};
};

// auto default_model(State const& state) -> Model { return Model{50.0, state.get_input_dataset()}; }
} // namespace

TEST_CASE("API model - all updates") {
    using namespace std::string_literals;

    State state;
    auto const input_dataset = state.get_input_dataset();
    auto const& input_info = input_dataset.get_info();
    auto model = Model{50.0, input_dataset};

    // update vector
    std::vector<ID> sym_load_update_id{7};
    std::vector<IntS> sym_load_update_status{1};
    std::vector<double> sym_load_update_p_specified{2.5e6};

    std::vector<ID> asym_load_update_id{8};
    std::vector<IntS> asym_load_update_status{0};

    std::vector<ID> shunt_update_id{9};
    std::vector<IntS> shunt_update_status{0};
    std::vector<double> shunt_update_b1{0.02};
    std::vector<double> shunt_update_b0{0.02};

    // used for test case alternate compute mode
    std::vector<ID> shunt_update_2_id{6};
    std::vector<IntS> source_update_2_status{0};
    std::vector<double> shunt_update_2_b1{0.01};
    std::vector<double> shunt_update_2_b0{0.01};

    std::vector<ID> source_update_id{10};
    std::vector<IntS> source_update_status{1};
    std::vector<double> source_update_u_ref{test::u1};

    std::vector<ID> link_update_id{5};
    std::vector<IntS> link_update_from_status{1};
    std::vector<IntS> link_update_to_status{0};

    DatasetConst update_data{"update", 1, 1};
    update_data.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("sym_load", "id", sym_load_update_id.data());
    update_data.add_attribute_buffer("sym_load", "status", sym_load_update_status.data());
    update_data.add_attribute_buffer("sym_load", "p_specified", sym_load_update_p_specified.data());

    update_data.add_buffer("asym_load", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("asym_load", "id", asym_load_update_id.data());
    update_data.add_attribute_buffer("asym_load", "status", asym_load_update_status.data());

    update_data.add_buffer("shunt", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("shunt", "id", shunt_update_id.data());
    update_data.add_attribute_buffer("shunt", "status", shunt_update_status.data());
    update_data.add_attribute_buffer("shunt", "b1", shunt_update_b1.data());
    update_data.add_attribute_buffer("shunt", "b0", shunt_update_b0.data());

    update_data.add_buffer("source", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("source", "id", source_update_id.data());
    update_data.add_attribute_buffer("source", "status", source_update_status.data());
    update_data.add_attribute_buffer("source", "u_ref", source_update_u_ref.data());

    update_data.add_buffer("link", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("link", "id", link_update_id.data());
    update_data.add_attribute_buffer("link", "from_status", link_update_from_status.data());
    update_data.add_attribute_buffer("link", "to_status", link_update_to_status.data());

    auto const output_dataset_type = "sym_output"s;
    for (Idx comp_type_idx = 0; comp_type_idx < input_info.n_components(); ++comp_type_idx) {
        CAPTURE(comp_type_idx);

        auto const comp_type = input_info.component_name(comp_type_idx);
        CAPTURE(comp_type);

        auto const comp_meta = MetaData::get_component_by_name(output_dataset_type, comp_type);
        auto const total_elements = input_info.component_total_elements(comp_type_idx);
        auto const elements_per_scenario = input_info.component_elements_per_scenario(comp_type_idx);
        auto const n_bytes = total_elements * MetaData::component_size(comp_meta);

        std::vector<char> sym_output_from_batch(n_bytes);
        std::vector<char> sym_output_from_updated_single(n_bytes);

        DatasetMutable output_data_from_batch{output_dataset_type, 1, 1};
        DatasetMutable output_data_from_updated_single{output_dataset_type, false, 1};

        output_data_from_batch.add_buffer(comp_type, elements_per_scenario, total_elements, nullptr,
                                          sym_output_from_batch.data());
        output_data_from_updated_single.add_buffer(comp_type, elements_per_scenario, total_elements, nullptr,
                                                   sym_output_from_updated_single.data());

        auto opt = get_default_options(PGM_symmetric, PGM_linear);
        model.calculate(opt, output_data_from_batch, update_data);
        model.update(update_data);
        model.calculate(opt, output_data_from_updated_single);

        CHECK(sym_output_from_batch == sym_output_from_updated_single);
    }
}

TEST_CASE("API model - updates w/ alternating compute mode") {
    State state;
    auto const input_dataset = state.get_input_dataset();
    auto const& input_info = input_dataset.get_info();
    auto model = Model{50.0, input_dataset};

    auto const check_sym = [&] {
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
    auto const check_asym = [&] {
        std::vector<double> asym_node_output_u_pu(3 * 3);
        std::vector<double> asym_line_output_i_from(1 * 3);
        std::vector<double> asym_source_output_i(2 * 3);
        std::vector<double> asym_sym_load_output_i(1 * 3);
        std::vector<double> asym_asym_load_output_i(1 * 3);
        std::vector<double> asym_shunt_output_i(1 * 3);

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

    // update vector
    std::vector<ID> sym_load_update_id{7};
    std::vector<IntS> sym_load_update_status{1};
    std::vector<double> sym_load_update_p_specified{2.5e6};

    std::vector<ID> asym_load_update_id{8};
    std::vector<IntS> asym_load_update_status{0};

    std::vector<ID> shunt_update_id{9};
    std::vector<IntS> shunt_update_status{0};
    std::vector<double> shunt_update_b1{0.02};
    std::vector<double> shunt_update_b0{0.02};

    DatasetConst update_data{"update", 1, 1};
    update_data.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("sym_load", "id", sym_load_update_id.data());
    update_data.add_attribute_buffer("sym_load", "status", sym_load_update_status.data());
    update_data.add_attribute_buffer("sym_load", "p_specified", sym_load_update_p_specified.data());

    update_data.add_buffer("asym_load", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("asym_load", "id", asym_load_update_id.data());
    update_data.add_attribute_buffer("asym_load", "status", asym_load_update_status.data());

    update_data.add_buffer("shunt", 1, 1, nullptr, nullptr);
    update_data.add_attribute_buffer("shunt", "id", shunt_update_id.data());
    update_data.add_attribute_buffer("shunt", "status", shunt_update_status.data());
    update_data.add_attribute_buffer("shunt", "b1", shunt_update_b1.data());
    update_data.add_attribute_buffer("shunt", "b0", shunt_update_b0.data());

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
auto get_incomplete_state() -> State {
    State result;

    // std::ranges::fill(result.source_status, 1);  // TODO(mgovers): remove
    std::ranges::fill(result.source_u_ref, nan);
    std::ranges::fill(result.source_u_ref_angle, nan);
    std::ranges::fill(result.sym_load_p_specified, nan);
    std::ranges::fill(result.asym_load_p_specified, nan);

    return result;
}
} // namespace

TEST_CASE("API model - incomplete input") {
    State complete_state;
    State incomplete_state = get_incomplete_state();

    auto test_model = Model{50.0, incomplete_state.get_input_dataset()};

    for (auto symmetry : {PGM_symmetric, PGM_asymmetric}) {
        CAPTURE(symmetry);

        auto const calculation_symmetry = symmetry == PGM_symmetric ? "Symmetric" : "Asymmetric";
        auto const output_type = symmetry == PGM_symmetric ? "sym_output" : "asym_output";

        SUBCASE(calculation_symmetry) {

            auto n_bytes = complete_state.node_id.size() *
                           MetaData::component_size(MetaData::get_component_by_name(output_type, "node"));

            std::vector<char> test_sym_node(n_bytes);
            DatasetMutable test_result_data{output_type, true, 1};
            test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr,
                                        test_sym_node.data());

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
                DatasetConst incomplete_update_data{"update", true, 1};
                incomplete_update_data.add_buffer("source", incomplete_state.source_id.size(),
                                                  incomplete_state.source_id.size(), nullptr, nullptr);
                incomplete_update_data.add_attribute_buffer("source", "id", incomplete_state.source_id.data());
                incomplete_update_data.add_attribute_buffer("source", "u_ref", incomplete_state.source_u_ref.data());
                incomplete_update_data.add_attribute_buffer("source", "u_ref_angle",
                                                            incomplete_state.source_u_ref_angle.data());

                incomplete_update_data.add_buffer("sym_load", incomplete_state.sym_load_id.size(),
                                                  incomplete_state.sym_load_id.size(), nullptr, nullptr);
                incomplete_update_data.add_attribute_buffer("sym_load", "id", incomplete_state.sym_load_id.data());
                incomplete_update_data.add_attribute_buffer("sym_load", "p_specified",
                                                            incomplete_state.sym_load_p_specified.data());

                incomplete_update_data.add_buffer("asym_load", incomplete_state.asym_load_id.size(),
                                                  incomplete_state.asym_load_id.size(), nullptr, nullptr);
                incomplete_update_data.add_attribute_buffer("asym_load", "id", incomplete_state.asym_load_id.data());
                incomplete_update_data.add_attribute_buffer("asym_load", "p_specified",
                                                            incomplete_state.asym_load_p_specified.data());

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
                DatasetConst complete_update_data{"update", true, 1};
                complete_update_data.add_buffer("source", complete_state.source_id.size(),
                                                complete_state.source_id.size(), nullptr, nullptr);
                complete_update_data.add_attribute_buffer("source", "id", complete_state.source_id.data());
                complete_update_data.add_attribute_buffer("source", "u_ref", complete_state.source_u_ref.data());
                complete_update_data.add_attribute_buffer("source", "u_ref_angle",
                                                          complete_state.source_u_ref_angle.data());

                complete_update_data.add_buffer("sym_load", complete_state.sym_load_id.size(),
                                                complete_state.sym_load_id.size(), nullptr, nullptr);
                complete_update_data.add_attribute_buffer("sym_load", "id", complete_state.sym_load_id.data());
                complete_update_data.add_attribute_buffer("sym_load", "p_specified",
                                                          complete_state.sym_load_p_specified.data());

                complete_update_data.add_buffer("asym_load", complete_state.asym_load_id.size(),
                                                complete_state.asym_load_id.size(), nullptr, nullptr);
                complete_update_data.add_attribute_buffer("asym_load", "id", complete_state.asym_load_id.data());
                complete_update_data.add_attribute_buffer("asym_load", "p_specified",
                                                          complete_state.asym_load_p_specified.data());

                auto ref_model = Model{50.0, complete_state.get_input_dataset()};
                std::vector<char> ref_sym_node(n_bytes);
                DatasetMutable ref_result_data{output_type, true, 1};
                ref_result_data.add_buffer("node", ref_sym_node.size(), ref_sym_node.size(), nullptr,
                                           ref_sym_node.data());

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

                CHECK(test_sym_node == ref_sym_node);
            }
        }
    }
}

// TEST_CASE("Test main model - Incomplete followed by complete") { // TODO(mgovers): This tests the reset of 2
// consecutive
//                                                                  // batch scenarios and definitely needs to
//                                                                  be tested
//     using CalculationMethod::linear;

//     State const state;
//     auto model = default_model(state);
//     auto test_model = incomplete_input_model(state);

//     constexpr Idx batch_size = 2;

//     std::vector<SourceUpdate> mixed_source_update{
//         {6, 1, nan, nan}, {10, 1, nan, nan}, {6, 1, 1.05, nan}, {10, 1, 1.05, 0}};
//     std::vector<SymLoadGenUpdate> mixed_sym_load_update{{7, 1, nan, 1.0}, {7, 1, 0.5e6, nan}};
//     std::vector<AsymLoadGenUpdate> mixed_asym_load_update{
//         {8, 1, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{1.0}},
//         {8, 1, RealValue<asymmetric_t>{0.5e6 / 3.0}, RealValue<asymmetric_t>{nan}}};

//     auto const source_indptr = IdxVector{0, 0, static_cast<Idx>(mixed_source_update.size())};

//     REQUIRE(source_indptr.size() == batch_size + 1);

//     ConstDataset mixed_update_data{true, batch_size, "update", meta_data::meta_data_gen::meta_data};
//     mixed_update_data.add_buffer("source", 2, 4, nullptr, mixed_source_update.data());
//     mixed_update_data.add_buffer("sym_load", 1, 2, nullptr, mixed_sym_load_update.data());
//     mixed_update_data.add_buffer("asym_load", 1, 2, nullptr, mixed_asym_load_update.data());

//     ConstDataset second_scenario_update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     second_scenario_update_data.add_buffer("source", 2, 2, nullptr, mixed_source_update.data() + 2);
//     second_scenario_update_data.add_buffer("sym_load", 1, 1, nullptr, mixed_sym_load_update.data() + 1);
//     second_scenario_update_data.add_buffer("asym_load", 1, 1, nullptr, mixed_asym_load_update.data() + 1);

//     SUBCASE("Symmetrical") {
//         MutableDataset test_result_data{true, batch_size, "sym_output", meta_data::meta_data_gen::meta_data};
//         MutableDataset ref_result_data{false, 1, "sym_output", meta_data::meta_data_gen::meta_data};

//         std::vector<NodeOutput<symmetric_t>> test_sym_node(batch_size * state.sym_node.size(),
//                                                            {na_IntID, na_IntS, nan, nan, nan, nan, nan});
//         std::vector<NodeOutput<symmetric_t>> ref_sym_node(state.sym_node.size(),
//                                                           {na_IntID, na_IntS, nan, nan, nan, nan, nan});
//         test_result_data.add_buffer("node", state.sym_node.size(), test_sym_node.size(), nullptr,
//         test_sym_node.data()); ref_result_data.add_buffer("node", ref_sym_node.size(), ref_sym_node.size(),
//         nullptr, ref_sym_node.data());

//         CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                               .calculation_symmetry = symmetric,
//                                               .calculation_method = linear,
//                                               .err_tol = 1e-8,
//                                               .max_iter = 1},
//                                              test_result_data, mixed_update_data),
//                         BatchCalculationError);
//         model.calculate({.calculation_type = power_flow,
//                               .calculation_symmetry = symmetric,
//                               .calculation_method = linear,
//                               .err_tol = 1e-8,
//                               .max_iter = 1},
//                              ref_result_data, second_scenario_update_data);

//         CHECK(is_nan(test_sym_node[0].u_pu));
//         CHECK(is_nan(test_sym_node[1].u_pu));
//         CHECK(is_nan(test_sym_node[2].u_pu));
//         CHECK(test_sym_node[state.sym_node.size() + 0].u_pu == doctest::Approx(ref_sym_node[0].u_pu));
//         CHECK(test_sym_node[state.sym_node.size() + 1].u_pu == doctest::Approx(ref_sym_node[1].u_pu));
//         CHECK(test_sym_node[state.sym_node.size() + 2].u_pu == doctest::Approx(ref_sym_node[2].u_pu));
//     }

//     SUBCASE("Asymmetrical") {
//         MutableDataset test_result_data{true, batch_size, "asym_output",
//         meta_data::meta_data_gen::meta_data}; MutableDataset ref_result_data{false, 1, "asym_output",
//         meta_data::meta_data_gen::meta_data};

//         std::vector<NodeOutput<asymmetric_t>> test_asym_node(
//             batch_size * state.sym_node.size(),
//             {na_IntID, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan},
//              RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}});
//         std::vector<NodeOutput<asymmetric_t>> ref_asym_node(
//             state.sym_node.size(),
//             {na_IntID, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan},
//              RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}});
//         test_result_data.add_buffer("node", state.sym_node.size(), test_asym_node.size(), nullptr,
//                                     test_asym_node.data());
//         ref_result_data.add_buffer("node", ref_asym_node.size(), ref_asym_node.size(), nullptr,
//         ref_asym_node.data());

//         CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                               .calculation_symmetry = asymmetric,
//                                               .calculation_method = linear,
//                                               .err_tol = 1e-8,
//                                               .max_iter = 1},
//                                              test_result_data, mixed_update_data),
//                         BatchCalculationError);
//         model.calculate({.calculation_type = power_flow,
//                               .calculation_symmetry = asymmetric,
//                               .calculation_method = linear,
//                               .err_tol = 1e-8,
//                               .max_iter = 1},
//                              ref_result_data, second_scenario_update_data);

//         for (auto component_idx : {0, 1, 2}) {
//             CAPTURE(component_idx);

//             CHECK(is_nan(test_asym_node[component_idx].u_pu));

//             for (auto phase_idx : {0, 1, 2}) {
//                 CAPTURE(phase_idx);

//                 CHECK(test_asym_node[state.asym_node.size() + component_idx].u_pu(phase_idx) ==
//                       doctest::Approx(ref_asym_node[component_idx].u_pu(phase_idx)));
//             }
//         }
//     }

} // namespace power_grid_model_cpp
