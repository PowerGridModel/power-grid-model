// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp/model.hpp>

#include <doctest/doctest.h>

#include <limits>
#include <numbers>
#include <vector>

namespace power_grid_model_cpp {
namespace {
using std::numbers::pi;
using std::numbers::sqrt3;

constexpr Idx default_option{-1};
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr double deg_120 = 2.0 / 3.0 * pi;
constexpr double deg_240 = 4.0 / 3.0 * pi;

enum class CalculationSymmetry : Idx { symmetric = 0, asymmetric = 1 };
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

enum class FaultType : IntS {
    // three_phase = 0,
    single_phase_to_ground = 1,
    // two_phase = 2,
    // two_phase_to_ground = 3,
    // nan = -128
};

enum class FaultPhase : IntS {
    // abc = 0,
    a = 1,
    // b = 2,
    // c = 3,
    // ab = 4,
    // ac = 5,
    // bc = 6,
    // default_value = -1,
    // nan = na_IntS
};

Options get_default_options(CalculationSymmetry calculation_symmetry, PGM_CalculationMethod calculation_method,
                            Idx threading = default_option) {
    Options opt;
    opt.set_calculation_type(PGM_power_flow);
    opt.set_symmetric(static_cast<Idx>(calculation_symmetry));
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

    std::vector<ID> sym_power_sensor_id{11, 13, 14, 15, 16, 17, 28};
    std::vector<ID> sym_power_sensor_measured_object{4, 6, 6, 9, 7, 8, 3};
    std::vector<MeasuredTerminalType> sym_power_sensor_measured_terminal_type{
        MeasuredTerminalType::branch_from, MeasuredTerminalType::source, MeasuredTerminalType::source,
        MeasuredTerminalType::shunt,       MeasuredTerminalType::load,   MeasuredTerminalType::load,
        MeasuredTerminalType::node};
    std::vector<double> sym_power_sensor_power_sigma{0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};
    std::vector<double> sym_power_sensor_p_measured{1.1e6, 1.3e6, 1.4e6, 1.5e6, 1.6e6, 1.7e6, 3.0e6};
    std::vector<double> sym_power_sensor_q_measured{1.1e3, 1.3e3, 1.4e3, 1.5e3, 1.6e3, 1.7e3, 3.0e3};

    std::vector<ID> asym_power_sensor_id{18, 20, 21, 22, 23, 24, 29};
    std::vector<ID> asym_power_sensor_measured_object{4, 6, 6, 9, 7, 8, 3};
    std::vector<MeasuredTerminalType> asym_power_sensor_measured_terminal_type{
        MeasuredTerminalType::branch_from, MeasuredTerminalType::source, MeasuredTerminalType::source,
        MeasuredTerminalType::shunt,       MeasuredTerminalType::load,   MeasuredTerminalType::load,
        MeasuredTerminalType::node};
    std::vector<double> asym_power_sensor_power_sigma{0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};
    std::vector<double> asym_power_sensor_p_measured{
        2.11e6, 2.12e6, 2.13e6, // 18
        2.31e6, 2.32e6, 2.33e6, // 20
        2.41e6, 2.42e6, 2.43e6, // 21
        2.51e6, 2.52e6, 2.53e6, // 22
        2.61e6, 2.62e6, 2.63e6, // 23
        2.71e6, 2.72e6, 2.73e6, // 24
        5.01e6, 5.02e6, 5.03e6  // 28
    };
    std::vector<double> asym_power_sensor_q_measured{
        2.11e3, 2.12e3, 2.13e3, // 18
        2.31e3, 2.32e3, 2.33e3, // 20
        2.41e3, 2.42e3, 2.43e3, // 21
        2.51e3, 2.52e3, 2.53e3, // 22
        2.61e3, 2.62e3, 2.63e3, // 23
        2.71e3, 2.72e3, 2.73e3, // 24
        5.01e3, 5.02e3, 5.03e3, // 29
    };

    std::vector<ID> sym_voltage_sensor_id{25, 26};
    std::vector<ID> sym_voltage_sensor_measured_object{1, 2};
    std::vector<double> sym_voltage_sensor_u_sigma{105.0, 105.0};
    std::vector<double> sym_voltage_sensor_u_measured{10.1e3, 10.2e3};
    std::vector<double> sym_voltage_sensor_u_angle_measured{0.1, 0.2};

    std::vector<ID> asym_voltage_sensor_id{27};
    std::vector<ID> asym_voltage_sensor_measured_object{3};
    std::vector<double> asym_voltage_sensor_u_sigma{105.0};
    std::vector<double> asym_voltage_sensor_u_measured{10.31e3 / sqrt3, 10.32e3 / sqrt3, 10.33e3 / sqrt3};
    std::vector<double> asym_voltage_sensor_u_angle_measured{0.0, -deg_120, -deg_240};

    std::vector<ID> fault_id{30};
    std::vector<IntS> fault_status{1};
    std::vector<FaultType> fault_type{FaultType::single_phase_to_ground};
    std::vector<FaultPhase> fault_phase{FaultPhase::a};
    std::vector<ID> fault_object{3};
    std::vector<double> fault_r_f{0.1};
    std::vector<double> fault_x_f{0.1};

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

        result.add_buffer("sym_power_sensor", sym_power_sensor_id.size(), sym_power_sensor_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("sym_power_sensor", "id", sym_power_sensor_id.data());
        result.add_attribute_buffer("sym_power_sensor", "measured_object", sym_power_sensor_measured_object.data());
        result.add_attribute_buffer("sym_power_sensor", "measured_terminal_type",
                                    sym_power_sensor_measured_terminal_type.data());
        result.add_attribute_buffer("sym_power_sensor", "power_sigma", sym_power_sensor_power_sigma.data());
        result.add_attribute_buffer("sym_power_sensor", "p_measured", sym_power_sensor_p_measured.data());
        result.add_attribute_buffer("sym_power_sensor", "q_measured", sym_power_sensor_q_measured.data());

        result.add_buffer("asym_power_sensor", asym_power_sensor_id.size(), asym_power_sensor_id.size(), nullptr,
                          nullptr);
        result.add_attribute_buffer("asym_power_sensor", "id", asym_power_sensor_id.data());
        result.add_attribute_buffer("asym_power_sensor", "measured_object", asym_power_sensor_measured_object.data());
        result.add_attribute_buffer("asym_power_sensor", "measured_terminal_type",
                                    asym_power_sensor_measured_terminal_type.data());
        result.add_attribute_buffer("asym_power_sensor", "power_sigma", asym_power_sensor_power_sigma.data());
        result.add_attribute_buffer("asym_power_sensor", "p_measured", asym_power_sensor_p_measured.data());
        result.add_attribute_buffer("asym_power_sensor", "q_measured", asym_power_sensor_q_measured.data());

        result.add_buffer("sym_voltage_sensor", sym_voltage_sensor_id.size(), sym_voltage_sensor_id.size(), nullptr,
                          nullptr);
        result.add_attribute_buffer("sym_voltage_sensor", "id", sym_voltage_sensor_id.data());
        result.add_attribute_buffer("sym_voltage_sensor", "measured_object", sym_voltage_sensor_measured_object.data());
        result.add_attribute_buffer("sym_voltage_sensor", "u_sigma", sym_voltage_sensor_u_sigma.data());
        result.add_attribute_buffer("sym_voltage_sensor", "u_measured", sym_voltage_sensor_u_measured.data());
        result.add_attribute_buffer("sym_voltage_sensor", "u_angle_measured",
                                    sym_voltage_sensor_u_angle_measured.data());

        result.add_buffer("asym_voltage_sensor", asym_voltage_sensor_id.size(), asym_voltage_sensor_id.size(), nullptr,
                          nullptr);
        result.add_attribute_buffer("asym_voltage_sensor", "id", asym_voltage_sensor_id.data());
        result.add_attribute_buffer("asym_voltage_sensor", "measured_object",
                                    asym_voltage_sensor_measured_object.data());
        result.add_attribute_buffer("asym_voltage_sensor", "u_sigma", asym_voltage_sensor_u_sigma.data());
        result.add_attribute_buffer("asym_voltage_sensor", "u_measured", asym_voltage_sensor_u_measured.data());
        result.add_attribute_buffer("asym_voltage_sensor", "u_angle_measured",
                                    asym_voltage_sensor_u_angle_measured.data());

        result.add_buffer("fault", fault_id.size(), fault_id.size(), nullptr, nullptr);
        result.add_attribute_buffer("fault", "id", fault_id.data());
        result.add_attribute_buffer("fault", "status", fault_status.data());
        result.add_attribute_buffer("fault", "fault_type", fault_type.data());
        result.add_attribute_buffer("fault", "fault_phase", fault_phase.data());
        result.add_attribute_buffer("fault", "fault_object", fault_object.data());
        result.add_attribute_buffer("fault", "r_f", fault_r_f.data());
        result.add_attribute_buffer("fault", "x_f", fault_x_f.data());

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
    // std::vector<VoltageSensorOutput<symmetric_t>> sym_voltage_sensor_sensor =
    // std::vector<VoltageSensorOutput<symmetric_t>>(2); std::vector<VoltageSensorOutput<symmetric_t>>
    // asym_voltage_sensor_sensor_sym_output =
    //     std::vector<VoltageSensorOutput<symmetric_t>>(1);
    // std::vector<PowerSensorOutput<symmetric_t>> sym_power_sensor = std::vector<PowerSensorOutput<symmetric_t>>(7);
    // std::vector<PowerSensorOutput<symmetric_t>> asym_power_sensor_sym_output =
    //     std::vector<PowerSensorOutput<symmetric_t>>(7);

    // // individual asymmetric
    // std::vector<BranchOutput<asymmetric_t>> asym_line = std::vector<BranchOutput<asymmetric_t>>(1);
    // std::vector<BranchOutput<asymmetric_t>> asym_link = std::vector<BranchOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_load_sym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_load_asym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_source = std::vector<ApplianceOutput<asymmetric_t>>(2);
    // std::vector<ApplianceOutput<asymmetric_t>> asym_shunt = std::vector<ApplianceOutput<asymmetric_t>>(1);
    // std::vector<VoltageSensorOutput<asymmetric_t>> asym_voltage_sensor_sensor =
    //     std::vector<VoltageSensorOutput<asymmetric_t>>(1);
    // std::vector<VoltageSensorOutput<asymmetric_t>> sym_voltage_sensor_sensor_asym_output =
    //     std::vector<VoltageSensorOutput<asymmetric_t>>(2);
    // std::vector<PowerSensorOutput<asymmetric_t>> asym_power_sensor = std::vector<PowerSensorOutput<asymmetric_t>>(7);
    // std::vector<PowerSensorOutput<asymmetric_t>> sym_power_sensor_asym_output =
    //     std::vector<PowerSensorOutput<asymmetric_t>>(7);

    // // update vector
    // std::vector<SymLoadGenUpdate> sym_load_update{{7, 1, 1.0e6, nan}};
    // std::vector<AsymLoadGenUpdate> asym_load_update{{8, 0, RealValue<asymmetric_t>{nan},
    // RealValue<asymmetric_t>{nan}}}; std::vector<ShuntUpdate> shunt_update{{9, 0, nan, 0.02, nan, 0.02}};
    // std::vector<ShuntUpdate> shunt_update_2{{6, 0, nan, 0.01, nan, 0.01}}; // used for test case alternate compute
    // mode std::vector<SourceUpdate> source_update{{10, 1, test::u1, nan}}; std::vector<BranchUpdate> link_update{{5,
    // 1, 0}}; std::vector<FaultUpdate> fault_update{{30, 1, FaultType::three_phase, FaultPhase::abc, 1, nan, nan}};

    // // batch update vector
    // std::vector<SymLoadGenUpdate> batch_sym_load_update{{7, 1, 1.0e6, nan}, {7}, {7}, {7}, {7}};
    // std::vector<AsymLoadGenUpdate> batch_asym_load_update{
    //     {8, 0, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}, {8}, {8}, {8}, {8}};
    // std::vector<ShuntUpdate> batch_shunt_update{{9, 0, nan, 0.02, nan, 0.02}, {9}, {9}, {9}, {9}};
    // std::vector<SourceUpdate> batch_source_update{{10, 1, test::u1, nan}, {10}, {10}, {10}, {10}};
    // std::vector<BranchUpdate> batch_link_update{{5, 1, 0}, {5}, {5}, {5}, {5}};
    // std::vector<FaultUpdate> batch_fault_update{
    //     {30, 1, FaultType::three_phase, FaultPhase::abc, 1, nan, nan}, {30}, {30}, {30}, {30}};
};

auto default_model(State const& state) -> Model { return Model{50.0, state.get_input_dataset()}; }
} // namespace

TEST_CASE("API Model - misc") {
    State state;

    SUBCASE("Test get indexer") {
        std::vector<ID> const node_id{1, 2, 3};
        std::vector<double> const node_u_rated{10.0e3, 10.0e3, 10.0e3};

        DatasetConst input_dataset{"input", 0, 1};
        input_dataset.add_buffer("node", node_id.size(), node_id.size(), nullptr, nullptr);
        input_dataset.add_attribute_buffer("node", "id", node_id.data());
        input_dataset.add_attribute_buffer("node", "u_rated", node_u_rated.data());

        auto model = Model{50.0, input_dataset};

        std::vector<ID> const ids_to_index{2, 1, 3, 2};
        std::vector<Idx> const expected_indexer{1, 0, 2, 1};
        std::vector<Idx> indexer(ids_to_index.size());
        model.get_indexer("node", ids_to_index.size(), ids_to_index.data(), indexer.data());
        CHECK(indexer == expected_indexer);
    }

    SUBCASE("Test duplicated id") {
        std::vector<ID> node_id{1, 1, 3};
        DatasetConst input_dataset{"input", 0, 1};

        input_dataset.add_buffer("node", node_id.size(), node_id.size(), nullptr, nullptr);
        input_dataset.add_attribute_buffer("node", "id", node_id.data());

        auto construct_model = [&] { Model{50.0, input_dataset}; };
        CHECK_THROWS_WITH_AS(construct_model(), "Conflicting id detected: 1\n", PowerGridRegularError);
    }

    SUBCASE("Test non-existing id") {
        std::vector<ID> const node_id{1, 2, 3};
        std::vector<double> const node_u_rated{10.0e3, 10.0e3, 10.0e3};

        std::vector<ID> link_id{5};
        std::vector<ID> link_from_node{99};
        std::vector<ID> link_to_node{3};
        std::vector<IntS> link_from_status{1};
        std::vector<IntS> link_to_status{1};

        DatasetConst input_dataset{"input", 0, 1};

        input_dataset.add_buffer("node", node_id.size(), node_id.size(), nullptr, nullptr);
        input_dataset.add_attribute_buffer("node", "id", node_id.data());
        input_dataset.add_attribute_buffer("node", "u_rated", node_u_rated.data());

        input_dataset.add_buffer("link", link_id.size(), link_id.size(), nullptr, nullptr);
        input_dataset.add_attribute_buffer("link", "id", link_id.data());
        input_dataset.add_attribute_buffer("link", "from_node", link_from_node.data());
        input_dataset.add_attribute_buffer("link", "to_node", link_to_node.data());
        input_dataset.add_attribute_buffer("link", "from_status", link_from_status.data());
        input_dataset.add_attribute_buffer("link", "to_status", link_to_status.data());

        auto construct_model = [&] { Model{50.0, input_dataset}; };
        CHECK_THROWS_WITH_AS(construct_model(), "The id cannot be found: 99\n", PowerGridRegularError);
    }

    // SUBCASE("Test id for wrong type") { // TODO(mgovers): needed; captured in Python test but not all flavors; maybe
    //                                     // move to test_main_core_input.cpp
    //     MainModel model2{50.0, meta_data::meta_data_gen::meta_data};

    //     state.link_input[0].from_node = 4;
    //     model2.add_component<Node>(state.node_input); // 1 2 3
    //     model2.add_component<Line>(state.line_input); // 4
    //     CHECK_THROWS_AS(model2.add_component<Link>(state.link_input), IDWrongType);

    //     // Fix link input, retry
    //     state.link_input[0].from_node = 2;
    //     model2.add_component<Link>(state.link_input); // 5

    //     model2.add_component<Source>(state.source_input);      // 6 10
    //     model2.add_component<SymLoad>(state.sym_load_input);   // 7
    //     model2.add_component<AsymLoad>(state.asym_load_input); // 8
    //     model2.add_component<Shunt>(state.shunt_input);        // 9

    //     // voltage sensor with a measured id which is not a node (link)
    //     state.sym_voltage_sensor_sensor_input[0].measured_object = 5;
    //     CHECK_THROWS_AS(model2.add_component<SymVoltageSensor>(state.sym_voltage_sensor_sensor_input), IDWrongType);

    //     // Test for all MeasuredTerminalType instances
    //     using enum MeasuredTerminalType;
    //     std::vector<MeasuredTerminalType> const mt_types{branch_from, branch_to, generator, load, shunt, source};

    //     // power sensor with terminal branch, with a measured id which is not a branch (node)
    //     for (auto const& mt_type : mt_types) {
    //         state.sym_power_sensor_input[0].measured_object = 1;
    //         state.sym_power_sensor_input[0].measured_terminal_type = mt_type;
    //         CHECK_THROWS_AS(model2.add_component<SymPowerSensor>(state.sym_power_sensor_input), IDWrongType);
    //     }
    // }
}

// TEST_CASE_TEMPLATE("Test main model - unknown id", settings, regular_update,
//                    cached_update) { // TODO(mgovers): we need this test
//     State const state;
//     auto model = default_model(state);

//     std::vector<SourceUpdate> const source_update2{SourceUpdate{100, true, nan, nan}};
//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("source", source_update2.size(), source_update2.size(), nullptr, source_update2.data());
//     CHECK_THROWS_AS((model.update_components<typename settings::update_type>(update_data)), IDNotFound);
// }

// TEST_CASE_TEMPLATE(
//     "Test main model - update only load", settings, regular_update,
//     cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation
//     tests State state; auto model = default_model(state);

//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
//                            state.sym_load_update.data());
//     update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
//                            state.asym_load_update.data());
//     model.update_components<typename settings::update_type>(update_data);

//     SUBCASE("Symmetrical") {
//         auto const solver_output =
//             model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric,
//             CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.sym_node);
//         model.output_result<Branch>(solver_output, state.sym_branch);
//         model.output_result<Appliance>(solver_output, state.sym_appliance);
//         CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2));
//         CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[4].i == doctest::Approx(test::i_shunt));
//     }
//     SUBCASE("Asymmetrical") {
//         auto const solver_output = model.calculate<power_flow_t, asymmetric_t>(
//             get_default_options(asymmetric, CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.asym_node);
//         model.output_result<Branch>(solver_output, state.asym_branch);
//         model.output_result<Appliance>(solver_output, state.asym_appliance);
//         CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
//         CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2));
//         CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[4].i(2) == doctest::Approx(test::i_shunt));
//     }
// }

// TEST_CASE_TEMPLATE(
//     "Test main model - update load and shunt param", settings, regular_update,
//     cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation
//     tests State state; auto model = default_model(state);

//     state.sym_load_update[0].p_specified = 2.5e6;
//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
//                            state.sym_load_update.data());
//     update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
//                            state.asym_load_update.data());
//     update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
//                            state.shunt_update.data());
//     model.update_components<typename settings::update_type>(update_data);

//     SUBCASE("Symmetrical") {
//         auto const solver_output =
//             model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric,
//             CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.sym_node);
//         model.output_result<Branch>(solver_output, state.sym_branch);
//         model.output_result<Appliance>(solver_output, state.sym_appliance);
//         CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2 + test::i_shunt));
//         CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
//     }
//     SUBCASE("Asymmetrical") {
//         auto const solver_output = model.calculate<power_flow_t, asymmetric_t>(
//             get_default_options(asymmetric, CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.asym_node);
//         model.output_result<Branch>(solver_output, state.asym_branch);
//         model.output_result<Appliance>(solver_output, state.asym_appliance);
//         CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
//         CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2 + test::i_shunt));
//         CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
//     }
// }

// TEST_CASE_TEMPLATE(
//     "Test main model - all updates", settings, regular_update,
//     cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation
//     tests State state; auto model = default_model(state);

//     state.sym_load_update[0].p_specified = 2.5e6;
//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
//                            state.sym_load_update.data());
//     update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
//                            state.asym_load_update.data());
//     update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
//                            state.shunt_update.data());
//     update_data.add_buffer("source", state.source_update.size(), state.source_update.size(), nullptr,
//                            state.source_update.data());
//     update_data.add_buffer("link", state.link_update.size(), state.link_update.size(), nullptr,
//                            state.link_update.data());
//     update_data.add_buffer("fault", state.fault_update.size(), state.fault_update.size(), nullptr,
//                            state.fault_update.data());

//     model.update_components<typename settings::update_type>(update_data);

//     SUBCASE("Symmetrical") {
//         auto const solver_output =
//             model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric,
//             CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.sym_node);
//         model.output_result<Branch>(solver_output, state.sym_branch);
//         model.output_result<Appliance>(solver_output, state.sym_appliance);
//         CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_branch[0].i_from == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.sym_appliance[0].i == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.sym_appliance[1].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[2].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
//     }
//     SUBCASE("Asymmetrical") {
//         auto const solver_output = model.calculate<power_flow_t, asymmetric_t>(
//             get_default_options(asymmetric, CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.asym_node);
//         model.output_result<Branch>(solver_output, state.asym_branch);
//         model.output_result<Appliance>(solver_output, state.asym_appliance);
//         CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
//         CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.asym_appliance[0].i(1) == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.asym_appliance[1].i(2) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
//     }
// }

// TEST_CASE_TEMPLATE("Test main model - single permanent update from batch", settings, regular_update,
//                    cached_update) { // TODO(mgovers): we should whitebox-test this instead
//     State state;
//     auto model = default_model(state);

//     state.batch_sym_load_update[0].p_specified = 2.5e6;
//     ConstDataset update_data{true, 5, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", 1, state.batch_sym_load_update.size(), nullptr,
//                            state.batch_sym_load_update.data());
//     update_data.add_buffer("asym_load", 1, state.batch_asym_load_update.size(), nullptr,
//                            state.batch_asym_load_update.data());
//     update_data.add_buffer("shunt", 1, state.batch_shunt_update.size(), nullptr, state.batch_shunt_update.data());
//     update_data.add_buffer("source", 1, state.batch_source_update.size(), nullptr, state.batch_source_update.data());
//     update_data.add_buffer("link", 1, state.batch_link_update.size(), nullptr, state.batch_link_update.data());
//     update_data.add_buffer("fault", 1, state.batch_fault_update.size(), nullptr, state.batch_fault_update.data());

//     model.update_components<typename settings::update_type>(update_data);

//     SUBCASE("Symmetrical") {
//         auto const solver_output =
//             model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric,
//             CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.sym_node);
//         model.output_result<Branch>(solver_output, state.sym_branch);
//         model.output_result<Appliance>(solver_output, state.sym_appliance);
//         CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_branch[0].i_from == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.sym_appliance[0].i == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.sym_appliance[1].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[2].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
//         CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
//     }
//     SUBCASE("Asymmetrical") {
//         auto const solver_output = model.calculate<power_flow_t, asymmetric_t>(
//             get_default_options(asymmetric, CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.asym_node);
//         model.output_result<Branch>(solver_output, state.asym_branch);
//         model.output_result<Appliance>(solver_output, state.asym_appliance);
//         CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
//         CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.asym_appliance[0].i(1) == doctest::Approx(0.0).epsilon(1e-6));
//         CHECK(state.asym_appliance[1].i(2) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
//     }
// }

// TEST_CASE_TEMPLATE("Test main model - restore components", settings, regular_update,
//                    cached_update) { // TODO(mgovers): either whitebox (as a sub-part of batch impl) or otherwise drop
//                                     // entirely (tested by batch update)
//     State state;
//     auto model = default_model(state);

//     auto const solver_output_orig =
//         model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));

//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
//                            state.sym_load_update.data());
//     update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
//                            state.asym_load_update.data());

//     model.update_components<typename settings::update_type>(update_data);
//     model.restore_components(update_data);

//     SUBCASE("Symmetrical") {
//         auto const solver_output_result =
//             model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric,
//             CalculationMethod::linear));
//         model.output_result<Node>(solver_output_result, state.sym_node);
//         model.output_result<Branch>(solver_output_result, state.sym_branch);
//         model.output_result<Appliance>(solver_output_result, state.sym_appliance);

//         CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
//         CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
//         if constexpr (settings::update_type::value) {
//             CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load));
//             CHECK(state.sym_appliance[3].i == doctest::Approx(test::i_load));
//         } else {
//             CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2));
//             CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
//         }
//         CHECK(state.sym_appliance[4].i == doctest::Approx(test::i_shunt));
//     }
//     SUBCASE("Asymmetrical") {
//         auto const solver_output = model.calculate<power_flow_t, asymmetric_t>(
//             get_default_options(asymmetric, CalculationMethod::linear));
//         model.output_result<Node>(solver_output, state.asym_node);
//         model.output_result<Branch>(solver_output, state.asym_branch);
//         model.output_result<Appliance>(solver_output, state.asym_appliance);

//         CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
//         CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
//         CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
//         if constexpr (settings::update_type::value) {
//             CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load));
//             CHECK(state.asym_appliance[3].i(1) == doctest::Approx(test::i_load));
//         } else {
//             CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2));
//             CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         }
//         CHECK(state.asym_appliance[4].i(2) == doctest::Approx(test::i_shunt));
//     }
// }

// TEST_CASE_TEMPLATE("Test main model - updates w/ alternating compute mode", settings, regular_update,
//                    cached_update) { // TODO(mgovers): move to api tests; not possible with current validation
//                    framework
//     constexpr auto check_sym = [](MainModel const& model_, auto const& math_output_) {
//         State state_;
//         model_.output_result<Node>(math_output_, state_.sym_node);
//         model_.output_result<Branch>(math_output_, state_.sym_branch);
//         model_.output_result<Appliance>(math_output_, state_.sym_appliance);

//         CHECK(state_.sym_node[0].u_pu == doctest::Approx(1.05));
//         CHECK(state_.sym_node[1].u_pu == doctest::Approx(test::u1));
//         CHECK(state_.sym_node[2].u_pu == doctest::Approx(test::u1));
//         CHECK(state_.sym_branch[0].i_from == doctest::Approx(test::i));
//         CHECK(state_.sym_appliance[0].i == doctest::Approx(test::i));
//         CHECK(state_.sym_appliance[1].i == doctest::Approx(0.0));
//         CHECK(state_.sym_appliance[2].i == doctest::Approx(test::i_load * 2 + test::i_shunt));
//         CHECK(state_.sym_appliance[3].i == doctest::Approx(0.0));
//         CHECK(state_.sym_appliance[4].i == doctest::Approx(0.0));
//     };
//     constexpr auto check_asym = [](MainModel const& model_, auto const& math_output_) {
//         State state_;
//         model_.output_result<Node>(math_output_, state_.asym_node);
//         model_.output_result<Branch>(math_output_, state_.asym_branch);
//         model_.output_result<Appliance>(math_output_, state_.asym_appliance);
//         CHECK(state_.asym_node[0].u_pu(0) == doctest::Approx(1.05));
//         CHECK(state_.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
//         CHECK(state_.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
//         CHECK(state_.asym_branch[0].i_from(0) == doctest::Approx(test::i));
//         CHECK(state_.asym_appliance[0].i(1) == doctest::Approx(test::i));
//         CHECK(state_.asym_appliance[1].i(2) == doctest::Approx(0.0));
//         CHECK(state_.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2 + test::i_shunt));
//         CHECK(state_.asym_appliance[3].i(1) == doctest::Approx(0.0));
//         CHECK(state_.asym_appliance[4].i(2) == doctest::Approx(0.0));
//     };

//     State state;
//     auto model = default_model(state);

//     state.sym_load_update[0].p_specified = 2.5e6;

//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
//                            state.sym_load_update.data());
//     update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
//                            state.asym_load_update.data());
//     update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
//                            state.shunt_update.data());

//     // This will lead to no topo change but param change
//     model.update_components<typename settings::update_type>(update_data);

//     auto const math_output_sym_1 =
//         model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
//     check_sym(model, math_output_sym_1);

//     auto const math_output_asym_1 =
//         model.calculate<power_flow_t, asymmetric_t>(get_default_options(asymmetric, CalculationMethod::linear));
//     check_asym(model, math_output_asym_1);

//     SUBCASE("No new update") {
//         // Math state may be fully cached
//     }
//     if constexpr (std::same_as<typename settings::update_type, regular_update>) {
//         SUBCASE("No new parameter change") {
//             // Math state may be fully cached due to no change
//             model.update_components<typename settings::update_type>(update_data);
//         }
//     }
//     SUBCASE("With parameter change") {
//         // Restore to original state and re-apply same update: causes param change for cached update
//         model.restore_components(update_data);
//         model.update_components<typename settings::update_type>(update_data);
//     }

//     auto const math_output_asym_2 =
//         model.calculate<power_flow_t, asymmetric_t>(get_default_options(asymmetric, CalculationMethod::linear));
//     check_asym(model, math_output_asym_2);

//     auto const math_output_sym_2 =
//         model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
//     check_sym(model, math_output_sym_2);

//     model.restore_components(update_data);
// }

// namespace {
// auto incomplete_input_model(State const& state) -> MainModel {
//     MainModel model{50.0, meta_data::meta_data_gen::meta_data};

//     std::vector<SourceInput> const incomplete_source_input{{6, 1, 1, nan, nan, 1e12, nan, nan},
//                                                            {10, 3, 1, nan, nan, 1e12, nan, nan}};
//     std::vector<SymLoadGenInput> const incomplete_sym_load_input{{7, 3, 1, LoadGenType::const_y, nan, 0.0}};
//     std::vector<AsymLoadGenInput> const incomplete_asym_load_input{
//         {8, 3, 1, LoadGenType::const_y, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{0.0}}};

//     model.add_component<Node>(state.node_input);
//     model.add_component<Line>(state.line_input);
//     model.add_component<Link>(state.link_input);
//     model.add_component<Source>(incomplete_source_input);
//     model.add_component<SymLoad>(incomplete_sym_load_input);
//     model.add_component<AsymLoad>(incomplete_asym_load_input);
//     model.add_component<Shunt>(state.shunt_input);
//     model.set_construction_complete();

//     return model;
// }
// } // namespace

// TEST_CASE("Test main model - incomplete input") {
//     using CalculationMethod::iterative_current;
//     using CalculationMethod::linear;
//     using CalculationMethod::linear_current;
//     using CalculationMethod::newton_raphson;

//     State const state;
//     auto model = default_model(state);
//     auto test_model = incomplete_input_model(state);

//     std::vector<SourceUpdate> complete_source_update{{6, 1, 1.05, nan}, {10, 1, 1.05, 0}};
//     std::vector<SymLoadGenUpdate> complete_sym_load_update{{7, 1, 0.5e6, nan}};
//     std::vector<AsymLoadGenUpdate> complete_asym_load_update{
//         {8, 1, RealValue<asymmetric_t>{0.5e6 / 3.0}, RealValue<asymmetric_t>{nan}}};

//     ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     update_data.add_buffer("source", complete_source_update.size(), complete_source_update.size(), nullptr,
//                            complete_source_update.data());
//     update_data.add_buffer("sym_load", complete_sym_load_update.size(), complete_sym_load_update.size(), nullptr,
//                            complete_sym_load_update.data());
//     update_data.add_buffer("asym_load", complete_asym_load_update.size(), complete_asym_load_update.size(), nullptr,
//                            complete_asym_load_update.data());

//     std::vector<SourceUpdate> incomplete_source_update{{6, na_IntS, nan, nan}, {10, na_IntS, nan, nan}};
//     std::vector<SymLoadGenUpdate> incomplete_sym_load_update{{7, na_IntS, nan, nan}};
//     std::vector<AsymLoadGenUpdate> incomplete_asym_load_update{
//         {8, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}};

//     ConstDataset incomplete_update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
//     incomplete_update_data.add_buffer("source", incomplete_source_update.size(), incomplete_source_update.size(),
//                                       nullptr, incomplete_source_update.data());
//     incomplete_update_data.add_buffer("sym_load", incomplete_sym_load_update.size(),
//     incomplete_sym_load_update.size(),
//                                       nullptr, incomplete_sym_load_update.data());
//     incomplete_update_data.add_buffer("asym_load", incomplete_asym_load_update.size(),
//                                       incomplete_asym_load_update.size(), nullptr,
//                                       incomplete_asym_load_update.data());

//     MainModel const ref_model{model};

//     SUBCASE("Asymmetrical - Complete") { // TODO(mgovers): no validation case for asym exists
//         MutableDataset test_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
//         MutableDataset ref_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};

//         std::vector<NodeOutput<asymmetric_t>> test_asym_node(state.asym_node.size());
//         std::vector<NodeOutput<asymmetric_t>> ref_asym_node(state.asym_node.size());
//         test_result_data.add_buffer("node", test_asym_node.size(), test_asym_node.size(), nullptr,
//                                     test_asym_node.data());
//         ref_result_data.add_buffer("node", ref_asym_node.size(), ref_asym_node.size(), nullptr,
//         ref_asym_node.data());

//         SUBCASE("Test linear calculation") {
//             test_model.calculate(get_default_options(asymmetric, linear), test_result_data, update_data);
//             model.calculate(get_default_options(asymmetric, linear), ref_result_data, update_data);
//         }

//         SUBCASE("Test linear current calculation") {
//             test_model.calculate(get_default_options(asymmetric, linear_current), test_result_data, update_data);
//             model.calculate(get_default_options(asymmetric, linear_current), ref_result_data, update_data);
//         }

//         SUBCASE("Test iterative current calculation") {
//             test_model.calculate(get_default_options(asymmetric, iterative_current), test_result_data, update_data);
//             model.calculate(get_default_options(asymmetric, iterative_current), ref_result_data, update_data);
//         }

//         SUBCASE("Test iterative Newton-Rhapson calculation") {
//             test_model.calculate(get_default_options(asymmetric, newton_raphson), test_result_data, update_data);
//             model.calculate(get_default_options(asymmetric, newton_raphson), ref_result_data, update_data);
//         }

//         for (auto component_idx : {0, 1, 2}) {
//             CAPTURE(component_idx);

//             for (auto phase_idx : {0, 1, 2}) {
//                 CAPTURE(phase_idx);

//                 CHECK(test_asym_node[component_idx].u_pu(phase_idx) ==
//                       doctest::Approx(ref_asym_node[component_idx].u_pu(phase_idx)));
//             }
//         }
//     }

//     SUBCASE("Symmetrical - Incomplete") { // TODO(mgovers): not tested elsewhere; maybe test in API model?
//         MutableDataset test_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
//         MutableDataset const ref_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};

//         std::vector<NodeOutput<symmetric_t>> test_sym_node(state.sym_node.size());
//         test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr,
//         test_sym_node.data());

//         SUBCASE("Target dataset") {
//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = symmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data),
//                             SparseMatrixError);
//         }
//         SUBCASE("Empty update dataset") {
//             ConstDataset const update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};

//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = symmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data, update_data),
//                             SparseMatrixError);
//         }
//         SUBCASE("Update dataset") {
//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = symmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data, incomplete_update_data),
//                             BatchCalculationError);
//         }
//     }

//     SUBCASE("Asymmetrical - Incomplete") { // TODO(mgovers): not tested elsewhere; maybe test in API model?
//         MutableDataset test_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
//         MutableDataset const ref_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};

//         std::vector<NodeOutput<asymmetric_t>> test_sym_node(state.sym_node.size());
//         test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr,
//         test_sym_node.data());

//         SUBCASE("Target dataset") {
//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = asymmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data),
//                             SparseMatrixError);
//         }
//         SUBCASE("Empty update dataset") {
//             ConstDataset const update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};

//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = asymmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data, update_data),
//                             SparseMatrixError);
//         }
//         SUBCASE("Update dataset") {
//             CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
//                                                   .calculation_symmetry = asymmetric,
//                                                   .calculation_method = linear,
//                                                   .err_tol = 1e-8,
//                                                   .max_iter = 1},
//                                                  test_result_data, incomplete_update_data),
//                             BatchCalculationError);
//         }
//     }
// }

// TEST_CASE("Test main model - Incomplete followed by complete") { // TODO(mgovers): This tests the reset of 2
// consecutive
//                                                                  // batch scenarios and definitely needs to be tested
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
//         test_sym_node.data()); ref_result_data.add_buffer("node", ref_sym_node.size(), ref_sym_node.size(), nullptr,
//         ref_sym_node.data());

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
//         MutableDataset test_result_data{true, batch_size, "asym_output", meta_data::meta_data_gen::meta_data};
//         MutableDataset ref_result_data{false, 1, "asym_output", meta_data::meta_data_gen::meta_data};

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
// }

} // namespace power_grid_model_cpp
