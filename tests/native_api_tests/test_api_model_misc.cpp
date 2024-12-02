// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_cpp/meta_data.hpp>
#include <power_grid_model_cpp/model.hpp>

#include <doctest/doctest.h>

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
                                          reinterpret_cast<void*>(sym_output_from_batch.data()));
        output_data_from_updated_single.add_buffer(comp_type, elements_per_scenario, total_elements, nullptr,
                                                   reinterpret_cast<void*>(sym_output_from_updated_single.data()));

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
        sym_output.add_attribute_buffer("node", "u_pu", reinterpret_cast<void*>(sym_node_output_u_pu.data()));

        sym_output.add_buffer("line", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("line", "i_from", reinterpret_cast<void*>(sym_line_output_i_from.data()));

        sym_output.add_buffer("source", 2, 2, nullptr, nullptr);
        sym_output.add_attribute_buffer("source", "i", reinterpret_cast<void*>(sym_source_output_i.data()));

        sym_output.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("sym_load", "i", reinterpret_cast<void*>(sym_sym_load_output_i.data()));

        sym_output.add_buffer("asym_load", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("asym_load", "i", reinterpret_cast<void*>(sym_asym_load_output_i.data()));

        sym_output.add_buffer("shunt", 1, 1, nullptr, nullptr);
        sym_output.add_attribute_buffer("shunt", "i", reinterpret_cast<void*>(sym_shunt_output_i.data()));

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
        asym_output.add_attribute_buffer("node", "u_pu", reinterpret_cast<void*>(asym_node_output_u_pu.data()));

        asym_output.add_buffer("line", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("line", "i_from", reinterpret_cast<void*>(asym_line_output_i_from.data()));

        asym_output.add_buffer("source", 2, 2, nullptr, nullptr);
        asym_output.add_attribute_buffer("source", "i", reinterpret_cast<void*>(asym_source_output_i.data()));

        asym_output.add_buffer("sym_load", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("sym_load", "i", reinterpret_cast<void*>(asym_sym_load_output_i.data()));

        asym_output.add_buffer("asym_load", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("asym_load", "i", reinterpret_cast<void*>(asym_asym_load_output_i.data()));

        asym_output.add_buffer("shunt", 1, 1, nullptr, nullptr);
        asym_output.add_attribute_buffer("shunt", "i", reinterpret_cast<void*>(asym_shunt_output_i.data()));

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
