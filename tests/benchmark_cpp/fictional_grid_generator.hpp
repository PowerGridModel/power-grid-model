// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>

#include <algorithm>
#include <random>

namespace power_grid_model::benchmark {

struct Option {
    Idx n_node_total_specified{}; // rough specification of total nodes
    Idx n_mv_feeder{};            // n mv feeder will be changed it is too small
    Idx n_node_per_mv_feeder{};
    Idx n_lv_feeder{};
    Idx n_connection_per_lv_feeder{};   // per connection: one node for connection joint, one node for actual house
    Idx n_parallel_hv_mv_transformer{}; // will be calculated
    Idx n_lv_grid{};                    // will be calculated
    double ratio_lv_grid{};             // ratio when lv grid will be generated
    bool has_mv_ring{};
    bool has_lv_ring{};
    bool has_tap_changer{};
    bool has_measurements{};
    bool has_fault{};
    bool has_tap_regulator{};
};

struct InputData {
    std::vector<NodeInput> node;
    std::vector<TransformerInput> transformer;
    std::vector<LineInput> line;
    std::vector<SourceInput> source;
    std::vector<SymLoadGenInput> sym_load;
    std::vector<AsymLoadGenInput> asym_load;
    std::vector<ShuntInput> shunt;
    std::vector<SymVoltageSensorInput> sym_voltage_sensor;
    std::vector<AsymVoltageSensorInput> asym_voltage_sensor;
    std::vector<SymPowerSensorInput> sym_power_sensor;
    std::vector<AsymPowerSensorInput> asym_power_sensor;
    std::vector<FaultInput> fault;
    std::vector<TransformerTapRegulatorInput> transformer_tap_regulator;

    ConstDataset get_dataset() const {
        ConstDataset dataset{false, 1, "input", meta_data::meta_data_gen::meta_data};
        dataset.add_buffer("node", node.size(), node.size(), nullptr, node.data());
        dataset.add_buffer("transformer", transformer.size(), transformer.size(), nullptr, transformer.data());
        dataset.add_buffer("line", line.size(), line.size(), nullptr, line.data());
        dataset.add_buffer("source", source.size(), source.size(), nullptr, source.data());
        dataset.add_buffer("sym_load", sym_load.size(), sym_load.size(), nullptr, sym_load.data());
        dataset.add_buffer("asym_load", asym_load.size(), asym_load.size(), nullptr, asym_load.data());
        dataset.add_buffer("shunt", shunt.size(), shunt.size(), nullptr, shunt.data());
        dataset.add_buffer("sym_voltage_sensor", sym_voltage_sensor.size(), sym_voltage_sensor.size(), nullptr,
                           sym_voltage_sensor.data());
        dataset.add_buffer("asym_voltage_sensor", asym_voltage_sensor.size(), asym_voltage_sensor.size(), nullptr,
                           asym_voltage_sensor.data());
        dataset.add_buffer("sym_power_sensor", sym_power_sensor.size(), sym_power_sensor.size(), nullptr,
                           sym_power_sensor.data());
        dataset.add_buffer("asym_power_sensor", asym_power_sensor.size(), asym_power_sensor.size(), nullptr,
                           asym_power_sensor.data());
        dataset.add_buffer("fault", fault.size(), fault.size(), nullptr, fault.data());
        dataset.add_buffer("transformer_tap_regulator", transformer_tap_regulator.size(),
                           transformer_tap_regulator.size(), nullptr, transformer_tap_regulator.data());

        return dataset;
    }
};

template <symmetry_tag sym> struct OutputData {
    std::vector<NodeOutput<sym>> node;
    std::vector<BranchOutput<sym>> transformer;
    std::vector<BranchOutput<sym>> line;
    std::vector<ApplianceOutput<sym>> source;
    std::vector<ApplianceOutput<sym>> sym_load;
    std::vector<ApplianceOutput<sym>> asym_load;
    std::vector<ApplianceOutput<sym>> shunt;
    Idx batch_size{1};

    MutableDataset get_dataset() {
        std::string const dataset_name = is_symmetric_v<sym> ? "sym_output" : "asym_output";
        MutableDataset dataset{true, batch_size, dataset_name, meta_data::meta_data_gen::meta_data};
        dataset.add_buffer("node", node.size() / batch_size, node.size(), nullptr, node.data());
        dataset.add_buffer("transformer", transformer.size() / batch_size, transformer.size(), nullptr,
                           transformer.data());
        dataset.add_buffer("line", line.size() / batch_size, line.size(), nullptr, line.data());
        dataset.add_buffer("source", source.size() / batch_size, source.size(), nullptr, source.data());
        dataset.add_buffer("sym_load", sym_load.size() / batch_size, sym_load.size(), nullptr, sym_load.data());
        dataset.add_buffer("asym_load", asym_load.size() / batch_size, asym_load.size(), nullptr, asym_load.data());
        dataset.add_buffer("shunt", shunt.size() / batch_size, shunt.size(), nullptr, shunt.data());
        return dataset;
    }
};

struct ShortCircuitOutputData {
    std::vector<NodeShortCircuitOutput> node;
    std::vector<BranchShortCircuitOutput> transformer;
    std::vector<BranchShortCircuitOutput> line;
    std::vector<ApplianceShortCircuitOutput> source;
    std::vector<ApplianceShortCircuitOutput> sym_load;
    std::vector<ApplianceShortCircuitOutput> asym_load;
    std::vector<ApplianceShortCircuitOutput> shunt;
    Idx batch_size{1};

    MutableDataset get_dataset() {
        std::string const dataset_name = "sc_output";
        MutableDataset dataset{true, batch_size, dataset_name, meta_data::meta_data_gen::meta_data};
        dataset.add_buffer("node", node.size() / batch_size, node.size(), nullptr, node.data());
        dataset.add_buffer("transformer", transformer.size() / batch_size, transformer.size(), nullptr,
                           transformer.data());
        dataset.add_buffer("line", line.size() / batch_size, line.size(), nullptr, line.data());
        dataset.add_buffer("source", source.size() / batch_size, source.size(), nullptr, source.data());
        dataset.add_buffer("sym_load", sym_load.size() / batch_size, sym_load.size(), nullptr, sym_load.data());
        dataset.add_buffer("asym_load", asym_load.size() / batch_size, asym_load.size(), nullptr, asym_load.data());
        dataset.add_buffer("shunt", shunt.size() / batch_size, shunt.size(), nullptr, shunt.data());
        return dataset;
    }
};

struct BatchData {
    std::vector<SymLoadGenUpdate> sym_load;
    std::vector<AsymLoadGenUpdate> asym_load;
    std::vector<SymPowerSensorUpdate> sym_power_sensor;
    std::vector<AsymPowerSensorUpdate> asym_power_sensor;

    Idx batch_size{0};

    ConstDataset get_dataset() const {
        ConstDataset dataset{true, batch_size, "update", meta_data::meta_data_gen::meta_data};
        if (batch_size == 0) {
            return dataset;
        }
        dataset.add_buffer("sym_load", sym_load.size() / batch_size, sym_load.size(), nullptr, sym_load.data());
        dataset.add_buffer("asym_load", asym_load.size() / batch_size, asym_load.size(), nullptr, asym_load.data());
        dataset.add_buffer("sym_power_sensor", sym_power_sensor.size() / batch_size, sym_power_sensor.size(), nullptr,
                           sym_power_sensor.data());
        dataset.add_buffer("asym_power_sensor", asym_power_sensor.size() / batch_size, asym_power_sensor.size(),
                           nullptr, asym_power_sensor.data());
        return dataset;
    }
};

// Deliberately use default seed for reproducability
// NOLINTNEXTLINE(cert-msc32-c, cert-msc51-cpp)
class FictionalGridGenerator {
  public:
    void generate_grid(Option const& option) { generate_grid(option, std::random_device{}()); }

    void generate_grid(Option const& option, std::random_device::result_type seed) {
        // initialization
        input_ = InputData{};
        mv_ring_.clear();
        lv_ring_.clear();
        option_ = option;
        gen_ = std::mt19937_64{seed};
        id_gen_ = 0;
        // process option to calculate n_lv_grid
        Idx total_mv_connection = option_.n_mv_feeder * option_.n_node_per_mv_feeder + 2;
        Idx const node_per_lv_grid = option_.n_lv_feeder * option_.n_connection_per_lv_feeder * 2 + 1;
        if (total_mv_connection > option_.n_node_total_specified) {
            option_.n_lv_grid = 0;
            option_.n_mv_feeder = (option_.n_node_total_specified - 2) / option_.n_node_per_mv_feeder;
            total_mv_connection = option_.n_mv_feeder * option_.n_node_per_mv_feeder;
        } else {
            option_.n_lv_grid = (option_.n_node_total_specified - total_mv_connection) / node_per_lv_grid;
        }
        if (option_.n_lv_grid > total_mv_connection) {
            option_.n_mv_feeder = option_.n_lv_grid / option_.n_node_per_mv_feeder + 1;
        }
        total_mv_connection = option_.n_mv_feeder * option_.n_node_per_mv_feeder;
        option_.ratio_lv_grid = total_mv_connection > 0
                                    ? static_cast<double>(option_.n_lv_grid) / static_cast<double>(total_mv_connection)
                                    : 1.0;
        // each mv feeder 10 MVA, each transformer 60 MVA, scaled up by 10%
        option_.n_parallel_hv_mv_transformer =
            static_cast<Idx>(static_cast<double>(option.n_mv_feeder) * 10.0 * 1.1 / 60.0) + 1;
        // start generating grid
        generate_mv_grid();

        if (option.has_measurements) {
            generate_sensors();
        }
        if (option.has_tap_changer) {
            generate_tap_changer();
        }
    }

    InputData const& input_data() const { return input_; }

    template <typename OutputDataType> OutputDataType generate_output_data(Idx batch_size = 1) const {
        batch_size = std::max(batch_size, Idx{1});
        OutputDataType output{};
        output.batch_size = batch_size;
        output.node.resize(input_.node.size() * batch_size);
        output.transformer.resize(input_.transformer.size() * batch_size);
        output.line.resize(input_.line.size() * batch_size);
        output.source.resize(input_.source.size() * batch_size);
        output.sym_load.resize(input_.sym_load.size() * batch_size);
        output.asym_load.resize(input_.asym_load.size() * batch_size);
        output.shunt.resize(input_.shunt.size() * batch_size);
        return output;
    }

    BatchData generate_batch_input(Idx batch_size) { return generate_batch_input(batch_size, std::random_device{}()); }

    BatchData generate_batch_input(Idx batch_size, std::random_device::result_type seed) {
        batch_size = std::max(batch_size, Idx{0});
        gen_ = std::mt19937_64{seed};
        BatchData batch_data{};
        batch_data.batch_size = batch_size;
        generate_load_series(input_.sym_load, batch_data.sym_load, batch_size);
        generate_load_series(input_.asym_load, batch_data.asym_load, batch_size);
        generate_power_sensor_series(input_.sym_power_sensor, batch_data.sym_power_sensor, batch_size);
        generate_power_sensor_series(input_.asym_power_sensor, batch_data.asym_power_sensor, batch_size);
        return batch_data;
    }

  private:
    Option option_{};
    std::mt19937_64 gen_;
    ID id_gen_;
    InputData input_;
    std::vector<ID> mv_ring_;
    std::vector<ID> lv_ring_;

    void generate_mv_grid() {
        // source node
        ID const id_source_node = id_gen_++;
        NodeInput const source_node{.id = id_source_node, .u_rated = 150.0e3};
        input_.node.push_back(source_node);
        SourceInput const source{.id = id_gen_++,
                                 .node = id_source_node,
                                 .status = 1,
                                 .u_ref = 1.05,
                                 .u_ref_angle = nan,
                                 .sk = 2000e6,
                                 .rx_ratio = nan,
                                 .z01_ratio = nan};
        input_.source.push_back(source);

        // transformer and mv busbar
        ID const id_mv_busbar = id_gen_++;
        NodeInput const mv_busbar{.id = id_mv_busbar, .u_rated = 10.5e3};
        input_.node.push_back(mv_busbar);
        for (Idx i = 0; i != option_.n_parallel_hv_mv_transformer; ++i) {
            // transformer, 150/10.5kV, 60MVA, uk=20.3%
            TransformerInput const transformer{.id = id_gen_++,
                                               .from_node = id_source_node,
                                               .to_node = id_mv_busbar,
                                               .from_status = 1,
                                               .to_status = 1,
                                               .u1 = 150.0e3,
                                               .u2 = 10.5e3,
                                               .sn = 60.0e6,
                                               .uk = 0.203,
                                               .pk = 200e3,
                                               .i0 = 0.01,
                                               .p0 = 40e3,
                                               .winding_from = WindingType::wye_n,
                                               .winding_to = WindingType::delta,
                                               .clock = 5,
                                               .tap_side = BranchSide::from,
                                               .tap_pos = 0,
                                               .tap_min = -10,
                                               .tap_max = 10,
                                               .tap_nom = 0,
                                               .tap_size = 2.5e3,
                                               .uk_min = nan,
                                               .uk_max = nan,
                                               .pk_min = nan,
                                               .pk_max = nan,
                                               .r_grounding_from = nan,
                                               .x_grounding_from = nan,
                                               .r_grounding_to = nan,
                                               .x_grounding_to = nan};
            input_.transformer.push_back(transformer);
            // shunt, Z0 = 0 + j7 ohm
            ShuntInput const shunt{
                .id = id_gen_++, .node = id_mv_busbar, .status = 1, .g1 = 0.0, .b1 = 0.0, .g0 = 0.0, .b0 = -1.0 / 7.0};
            input_.shunt.push_back(shunt);
        }

        // template input
        NodeInput const mv_node{.id = 0, .u_rated = 10.5e3};
        SymLoadGenInput const mv_sym_load{0, 0, 1, LoadGenType::const_i, 0.8e6, 0.6e6};
        // cable 3 * 630Al XLPE 10kV, per km
        LineInput const mv_line{.id = 0,
                                .from_node = 0,
                                .to_node = 0,
                                .from_status = 1,
                                .to_status = 1,
                                .r1 = 0.063,
                                .x1 = 0.103,
                                .c1 = 0.4e-6,
                                .tan1 = 0.0004,
                                .r0 = 0.275,
                                .x0 = 0.101,
                                .c0 = 0.66e-6,
                                .tan0 = 0.0,
                                .i_n = 1e3};

        // random generator
        std::uniform_int_distribution<Idx> load_type_gen{0, 2};
        // scaling factor: (from 0.8 to 1.2) * 10.0 / n_node_per_feeder
        // this will result in total length of the cable for about 10.0 km
        //    and total load for about 10 MVA
        std::uniform_real_distribution<double> scaling_gen{
            0.8 * 10.0 / static_cast<double>(option_.n_node_per_mv_feeder),
            1.2 * 10.0 / static_cast<double>(option_.n_node_per_mv_feeder)};
        std::bernoulli_distribution lv_gen{option_.ratio_lv_grid};

        // loop all feeder
        for (Idx i = 0; i < option_.n_mv_feeder; i++) {
            ID prev_node_id = id_mv_busbar;
            // loop all mv connection
            for (Idx j = 0; j < option_.n_node_per_mv_feeder; ++j) {
                // node
                ID const current_node_id = id_gen_++;
                NodeInput node = mv_node;
                node.id = current_node_id;
                input_.node.push_back(node);
                // line
                LineInput line = mv_line;
                line.id = id_gen_++;
                line.from_node = prev_node_id;
                line.to_node = current_node_id;
                scale_cable(line, scaling_gen(gen_));
                input_.line.push_back(line);
                // generate lv grid
                if (lv_gen(gen_)) {
                    generate_lv_grid(current_node_id, 10.0 / static_cast<double>(option_.n_node_per_mv_feeder));
                } else {
                    // generate mv sym load
                    SymLoadGenInput sym_load = mv_sym_load;
                    sym_load.id = id_gen_++;
                    sym_load.node = current_node_id;
                    sym_load.type = static_cast<LoadGenType>(load_type_gen(gen_));
                    double const sym_scale = scaling_gen(gen_);
                    sym_load.p_specified *= sym_scale;
                    sym_load.q_specified *= sym_scale;
                    input_.sym_load.push_back(sym_load);
                }

                // push to ring node
                if (j == option_.n_node_per_mv_feeder - 1) {
                    mv_ring_.push_back(current_node_id);
                }
                // iterate previous node
                prev_node_id = current_node_id;
            }
        }

        // add loop if needed, and there are more than one feeder, and there are ring nodes
        if (mv_ring_.size() > 1 && option_.has_mv_ring) {
            mv_ring_.push_back(mv_ring_.front());
            // loop all far end nodes
            for (auto it = mv_ring_.cbegin(); it != mv_ring_.cend() - 1; ++it) {
                // line
                LineInput line = mv_line;
                line.id = id_gen_++;
                line.from_node = *it;
                line.to_node = *(it + 1);
                // scale
                scale_cable(line, scaling_gen(gen_));
                input_.line.push_back(line);
            }
        }
    }

    void generate_lv_grid(ID mv_node, double mv_base_load) {
        ID const id_lv_busbar = id_gen_++;
        NodeInput const lv_busbar{.id = id_lv_busbar, .u_rated = 400.0};
        input_.node.push_back(lv_busbar);
        // transformer, 1500 kVA or mv base load, uk=6%, pk=8.8kW
        TransformerInput const transformer{.id = id_gen_++,
                                           .from_node = mv_node,
                                           .to_node = id_lv_busbar,
                                           .from_status = 1,
                                           .to_status = 1,
                                           .u1 = 10.5e3,
                                           .u2 = 420.0,
                                           .sn = std::max(1500e3, mv_base_load * 1.2),
                                           .uk = 0.06,
                                           .pk = 8.8e3,
                                           .i0 = 0.01,
                                           .p0 = 1e3,
                                           .winding_from = WindingType::delta,
                                           .winding_to = WindingType::wye_n,
                                           .clock = 11,
                                           .tap_side = BranchSide::from,
                                           .tap_pos = 3,
                                           .tap_min = 5,
                                           .tap_max = 1,
                                           .tap_nom = 3,
                                           .tap_size = 250.0,
                                           .uk_min = nan,
                                           .uk_max = nan,
                                           .pk_min = nan,
                                           .pk_max = nan,
                                           .r_grounding_from = nan,
                                           .x_grounding_from = nan,
                                           .r_grounding_to = nan,
                                           .x_grounding_to = nan};
        input_.transformer.push_back(transformer);

        // template
        NodeInput const lv_node{.id = 0, .u_rated = 400.0};
        AsymLoadGenInput const lv_asym_load{
            0, 0, 1, LoadGenType::const_i, RealValue<asymmetric_t>{0.0}, RealValue<asymmetric_t>{0.0}};
        // 4*150 Al, per km
        LineInput const lv_main_line{.id = 0,
                                     .from_node = 0,
                                     .to_node = 0,
                                     .from_status = 1,
                                     .to_status = 1,
                                     .r1 = 0.206,
                                     .x1 = 0.079,
                                     .c1 = 0.72e-6,
                                     .tan1 = 0.0004,
                                     .r0 = 0.94,
                                     .x0 = 0.387,
                                     .c0 = 0.36e-6,
                                     .tan0 = 0.0,
                                     .i_n = 300.0};
        // 4*16 Cu, per km
        LineInput const lv_connection_line{.id = 0,
                                           .from_node = 0,
                                           .to_node = 0,
                                           .from_status = 1,
                                           .to_status = 1,
                                           .r1 = 1.15,
                                           .x1 = 0.096,
                                           .c1 = 0.43e-6,
                                           .tan1 = 0.0004,
                                           .r0 = 4.6,
                                           .x0 = 0.408,
                                           .c0 = 0.258e-6,
                                           .tan0 = 0.0,
                                           .i_n = 80.0};

        // generator
        std::uniform_int_distribution<Idx> load_type_gen{0, 2};
        std::uniform_int_distribution<Idx> load_phase_gen{0, 2};
        // mv_base_load in total, divided by all users, scale down by 20%
        double const base_load =
            mv_base_load / static_cast<double>(option_.n_lv_feeder * option_.n_connection_per_lv_feeder) / 1.2;
        std::uniform_real_distribution<double> load_scaling_gen{0.8 * base_load, 1.2 * base_load};
        // main cable length generation
        // total length 0.2 km +/- 20%
        std::uniform_real_distribution<double> main_cable_gen{
            0.8 * 0.2 / static_cast<double>(option_.n_connection_per_lv_feeder),
            1.2 * 0.2 / static_cast<double>(option_.n_connection_per_lv_feeder)};
        // connection cable length generation
        // length 5 m - 20 m
        std::uniform_real_distribution<double> connection_cable_gen{5e-3, 20e-3};

        // loop feeders
        for (Idx i = 0; i < option_.n_lv_feeder; ++i) {
            ID prev_main_node_id = id_lv_busbar;
            // loop all LV connection
            for (Idx j = 0; j < option_.n_connection_per_lv_feeder; ++j) {
                // main node
                ID const current_main_node_id = id_gen_++;
                NodeInput main_node = lv_node;
                main_node.id = current_main_node_id;
                input_.node.push_back(main_node);
                // connection node
                ID const connection_node_id = id_gen_++;
                NodeInput connection_node = lv_node;
                connection_node.id = connection_node_id;
                input_.node.push_back(connection_node);
                // main line
                LineInput main_line = lv_main_line;
                main_line.id = id_gen_++;
                main_line.from_node = prev_main_node_id;
                main_line.to_node = current_main_node_id;
                scale_cable(main_line, main_cable_gen(gen_));
                input_.line.push_back(main_line);
                // connection line
                LineInput connection_line = lv_connection_line;
                connection_line.id = id_gen_++;
                connection_line.from_node = current_main_node_id;
                connection_line.to_node = connection_node_id;
                scale_cable(connection_line, connection_cable_gen(gen_));
                input_.line.push_back(connection_line);
                // asym load
                AsymLoadGenInput asym_load = lv_asym_load;
                asym_load.id = id_gen_++;
                asym_load.node = connection_node_id;
                asym_load.type = static_cast<LoadGenType>(load_type_gen(gen_));
                Idx const phase = load_phase_gen(gen_);
                double const apparent_power = load_scaling_gen(gen_);
                asym_load.p_specified(phase) = apparent_power * 0.8;
                asym_load.q_specified(phase) = apparent_power * 0.6;
                input_.asym_load.push_back(asym_load);

                // push to ring node
                if (j == option_.n_connection_per_lv_feeder - 1) {
                    lv_ring_.push_back(current_main_node_id);
                }
                // iterate previous node
                prev_main_node_id = current_main_node_id;
            }
        }

        // add loop if needed, and there are more than one feeder, and there are ring nodes
        if (lv_ring_.size() > 1 && option_.has_lv_ring) {
            lv_ring_.push_back(lv_ring_.front());
            // loop all far end nodes
            for (auto it = lv_ring_.cbegin(); it != lv_ring_.cend() - 1; ++it) {
                // line
                LineInput line = lv_main_line;
                line.id = id_gen_++;
                line.from_node = *it;
                line.to_node = *(it + 1);
                // scale
                scale_cable(line, main_cable_gen(gen_));
                input_.line.push_back(line);
            }
        }
    }

    void generate_sensors() {
        constexpr double voltage_tol = 0.1;
        constexpr double power_tol = 0.1;

        // voltage sensors
        std::ranges::transform(input_.source, std::back_inserter(input_.sym_voltage_sensor),
                               [this](SourceInput const& source) {
                                   auto const& source_node = input_.node[source.node];

                                   auto const base_source_voltage = source.u_ref * source_node.u_rated;
                                   auto const voltage_sigma = voltage_tol * base_source_voltage;

                                   return SymVoltageSensorInput{.id = id_gen_++,
                                                                .measured_object = source.node,
                                                                .u_sigma = voltage_sigma,
                                                                .u_measured = base_source_voltage,
                                                                .u_angle_measured = nan};
                               });
        std::ranges::transform(
            input_.source, std::back_inserter(input_.asym_voltage_sensor), [this](SourceInput const& source) {
                auto const& source_node = input_.node[source.node];

                auto const base_source_voltage = source.u_ref * source_node.u_rated;
                auto const voltage_sigma = voltage_tol * base_source_voltage;

                return AsymVoltageSensorInput{.id = id_gen_++,
                                              .measured_object = source.node,
                                              .u_sigma = voltage_sigma,
                                              .u_measured = RealValue<asymmetric_t>{base_source_voltage},
                                              .u_angle_measured = RealValue<asymmetric_t>{nan}};
            });

        // appliance power sensors
        std::ranges::transform(input_.shunt, std::back_inserter(input_.sym_power_sensor),
                               [this](ShuntInput const& shunt) {
                                   auto const& node = input_.node[shunt.node];
                                   double const base_voltage2 = node.u_rated * node.u_rated;
                                   double const base_p = base_voltage2 * shunt.g1;
                                   double const base_q = base_voltage2 * shunt.b1;
                                   return SymPowerSensorInput{.id = id_gen_++,
                                                              .measured_object = shunt.id,
                                                              .measured_terminal_type = MeasuredTerminalType::shunt,
                                                              .power_sigma = nan,
                                                              .p_measured = base_p,
                                                              .q_measured = base_q,
                                                              .p_sigma = power_tol * cabs(base_p),
                                                              .q_sigma = power_tol * cabs(base_q)};
                               });
        std::ranges::transform(input_.sym_load, std::back_inserter(input_.sym_power_sensor),
                               [this](SymLoadGenInput const& load) {
                                   return SymPowerSensorInput{.id = id_gen_++,
                                                              .measured_object = load.id,
                                                              .measured_terminal_type = MeasuredTerminalType::load,
                                                              .power_sigma = nan,
                                                              .p_measured = load.p_specified,
                                                              .q_measured = load.q_specified,
                                                              .p_sigma = power_tol * cabs(load.p_specified),
                                                              .q_sigma = power_tol * cabs(load.q_specified)};
                               });
        std::ranges::transform(
            input_.asym_load, std::back_inserter(input_.asym_power_sensor), [this](AsymLoadGenInput const& load) {
                return AsymPowerSensorInput{
                    .id = id_gen_++,
                    .measured_object = load.id,
                    .measured_terminal_type = MeasuredTerminalType::load,
                    .power_sigma = nan,
                    .p_measured = load.p_specified,
                    .q_measured = load.q_specified,
                    .p_sigma = {power_tol * cabs(load.p_specified(0)), power_tol * cabs(load.p_specified(1)),
                                power_tol * cabs(load.p_specified(2))},
                    .q_sigma = {power_tol * cabs(load.q_specified(0)), power_tol * cabs(load.q_specified(1)),
                                power_tol * cabs(load.q_specified(2))}};
            });

        // branch power sensors
        std::ranges::transform(input_.line, std::back_inserter(input_.sym_power_sensor), [this](LineInput const& line) {
            return SymPowerSensorInput{.id = id_gen_++,
                                       .measured_object = line.id,
                                       .measured_terminal_type = MeasuredTerminalType::branch_from,
                                       .power_sigma = 1e6,
                                       .p_measured = 0.0,
                                       .q_measured = 0.0};
        });
    }

    void generate_fault() {
        input_.fault.emplace_back(FaultInput{
            .id = id_gen_++, .status = 1, .fault_type = FaultType::three_phase, .fault_object = input_.node.back().id});
    }

    void generate_tap_changer() {
        constexpr auto voltage_scaling = 1.1;
        constexpr auto voltage_band = 0.05;

        if (input_.transformer.empty()) {
            return;
        }
        auto const& transformer = input_.transformer.front();
        auto const& u_rated = input_.node[transformer.to_node].u_rated;

        input_.transformer_tap_regulator.emplace_back(TransformerTapRegulatorInput{
            .id = id_gen_++,
            .regulated_object = transformer.id,
            .status = 1,
            .control_side = ControlSide::to,
            .u_set = voltage_scaling * u_rated,
            .u_band = transformer.tap_size + voltage_band * u_rated,
        });
    }

    static void scale_cable(LineInput& line, double cable_ratio) {
        line.r1 *= cable_ratio;
        line.x1 *= cable_ratio;
        line.c1 *= cable_ratio;
        line.r0 *= cable_ratio;
        line.x0 *= cable_ratio;
        line.c0 *= cable_ratio;
    }

    template <class T, class U>
    void generate_load_series(std::vector<T> const& input, std::vector<U>& load_series, Idx batch_size) {
        std::uniform_real_distribution<double> load_scaling_gen{0.0, 1.0};
        load_series.resize(input.size() * batch_size);
        auto const n_object = std::ssize(input);
        for (Idx const batch : IdxRange{batch_size}) {
            for (Idx const object : IdxRange{n_object}) {
                T const& input_obj = input[object];
                U& update_obj = load_series[batch * n_object + object];
                update_obj.id = input_obj.id;
                update_obj.status = na_IntS;
                if constexpr (is_symmetric_v<typename T::sym>) {
                    update_obj.p_specified = input_obj.p_specified * load_scaling_gen(gen_);
                    update_obj.q_specified = input_obj.q_specified * load_scaling_gen(gen_);
                } else {
                    update_obj.p_specified = {input_obj.p_specified(0) * load_scaling_gen(gen_),
                                              input_obj.p_specified(1) * load_scaling_gen(gen_),
                                              input_obj.p_specified(2) * load_scaling_gen(gen_)};
                    update_obj.q_specified = {input_obj.q_specified(0) * load_scaling_gen(gen_),
                                              input_obj.q_specified(1) * load_scaling_gen(gen_),
                                              input_obj.q_specified(2) * load_scaling_gen(gen_)};
                }
            }
        }
    }

    template <class T, class U>
    void generate_voltage_sensor_series(std::vector<T> const& input, std::vector<U>& sensor_series, Idx batch_size) {
        std::uniform_real_distribution<double> voltage_offset_scaling_gen{0.0, 1.0};

        sensor_series.resize(input.size() * batch_size);
        auto const n_object = std::ssize(input);
        for (Idx batch : IdxRange{batch_size}) {
            for (Idx object : IdxRange{n_object}) {
                T const& input_obj = input[object];
                U& update_obj = sensor_series[batch * n_object + object];
                update_obj.id = input_obj.id;
                if constexpr (is_symmetric_v<typename T::sym>) {
                    update_obj.u_measured =
                        input_obj.u_measured * (1 + input_obj.u_sigma * voltage_offset_scaling_gen(gen_));
                } else {
                    update_obj.u_measured =
                        input_obj.u_measured *
                        (1.0 + input_obj.u_sigma * RealValue<asymmetric_t>{voltage_offset_scaling_gen(gen_),
                                                                           voltage_offset_scaling_gen(gen_),
                                                                           voltage_offset_scaling_gen(gen_)});
                }
            }
        }
    }

    template <class T, class U>
    void generate_power_sensor_series(std::vector<T> const& input, std::vector<U>& sensor_series, Idx batch_size) {
        std::uniform_real_distribution<double> load_scaling_gen{0.0, 1.0};

        sensor_series.resize(input.size() * batch_size);
        auto const n_object = std::ssize(input);
        for (Idx const object : IdxRange{n_object}) {
            T const& input_obj = input[object];

            for (Idx const batch : IdxRange{batch_size}) {
                U& update_obj = sensor_series[batch * n_object + object];
                update_obj.id = input_obj.id;
                if constexpr (is_symmetric_v<typename T::sym>) {
                    update_obj.p_measured = input_obj.p_measured * load_scaling_gen(gen_);
                    update_obj.q_measured = input_obj.q_measured * load_scaling_gen(gen_);
                } else {
                    update_obj.p_measured = {input_obj.p_measured(0) * load_scaling_gen(gen_),
                                             input_obj.p_measured(1) * load_scaling_gen(gen_),
                                             input_obj.p_measured(2) * load_scaling_gen(gen_)};
                    update_obj.q_measured = {input_obj.q_measured(0) * load_scaling_gen(gen_),
                                             input_obj.q_measured(1) * load_scaling_gen(gen_),
                                             input_obj.q_measured(2) * load_scaling_gen(gen_)};
                }
            }
        }
    }
};

} // namespace power_grid_model::benchmark
