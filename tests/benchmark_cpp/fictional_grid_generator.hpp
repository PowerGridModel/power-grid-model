// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/main_model.hpp>

#include <algorithm>
#include <random>

namespace power_grid_model::benchmark {

struct Option {
    Idx n_node_total_specified; // rough specification of total nodes
    Idx n_mv_feeder;            // n mv feeder will be changed it is too small
    Idx n_node_per_mv_feeder;
    Idx n_lv_feeder;
    Idx n_connection_per_lv_feeder;   // per connection: one node for connection joint, one node for actual house
    Idx n_parallel_hv_mv_transformer; // will be calculated
    Idx n_lv_grid;                    // will be calculated
    double ratio_lv_grid;             // ratio when lv grid will be generated
    bool has_mv_ring;
    bool has_lv_ring;
};

struct InputData {
    std::vector<NodeInput> node;
    std::vector<TransformerInput> transformer;
    std::vector<LineInput> line;
    std::vector<SourceInput> source;
    std::vector<SymLoadGenInput> sym_load;
    std::vector<AsymLoadGenInput> asym_load;
    std::vector<ShuntInput> shunt;

    ConstDataset get_dataset() const {
        ConstDataset dataset;
        dataset.try_emplace("node", node.data(), static_cast<Idx>(node.size()));
        dataset.try_emplace("transformer", transformer.data(), static_cast<Idx>(transformer.size()));
        dataset.try_emplace("line", line.data(), static_cast<Idx>(line.size()));
        dataset.try_emplace("source", source.data(), static_cast<Idx>(source.size()));
        dataset.try_emplace("sym_load", sym_load.data(), static_cast<Idx>(sym_load.size()));
        dataset.try_emplace("asym_load", asym_load.data(), static_cast<Idx>(asym_load.size()));
        dataset.try_emplace("shunt", shunt.data(), static_cast<Idx>(shunt.size()));
        return dataset;
    }
};

template <bool sym> struct OutputData {
    std::vector<NodeOutput<sym>> node;
    std::vector<BranchOutput<sym>> transformer;
    std::vector<BranchOutput<sym>> line;
    std::vector<ApplianceOutput<sym>> source;
    std::vector<ApplianceOutput<sym>> sym_load;
    std::vector<ApplianceOutput<sym>> asym_load;
    std::vector<ApplianceOutput<sym>> shunt;
    Idx batch_size{1};

    Dataset get_dataset() {
        Dataset dataset;
        dataset.try_emplace("node", node.data(), batch_size, static_cast<Idx>(node.size()) / batch_size);
        dataset.try_emplace("transformer", transformer.data(), batch_size,
                            static_cast<Idx>(transformer.size()) / batch_size);
        dataset.try_emplace("line", line.data(), batch_size, static_cast<Idx>(line.size()) / batch_size);
        dataset.try_emplace("source", source.data(), batch_size, static_cast<Idx>(source.size()) / batch_size);
        dataset.try_emplace("sym_load", sym_load.data(), batch_size, static_cast<Idx>(sym_load.size()) / batch_size);
        dataset.try_emplace("asym_load", asym_load.data(), batch_size, static_cast<Idx>(asym_load.size()) / batch_size);
        dataset.try_emplace("shunt", shunt.data(), batch_size, static_cast<Idx>(shunt.size()) / batch_size);
        return dataset;
    }
};

struct BatchData {
    std::vector<SymLoadGenUpdate> sym_load;
    std::vector<AsymLoadGenUpdate> asym_load;
    Idx batch_size{0};

    ConstDataset get_dataset() const {
        ConstDataset dataset;
        if (batch_size == 0) {
            return dataset;
        }
        dataset.try_emplace("sym_load", sym_load.data(), batch_size, static_cast<Idx>(sym_load.size()) / batch_size);
        dataset.try_emplace("asym_load", asym_load.data(), batch_size, static_cast<Idx>(asym_load.size()) / batch_size);
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
        option_.ratio_lv_grid = static_cast<double>(option_.n_lv_grid) / static_cast<double>(total_mv_connection);
        // each mv feeder 10 MVA, each transformer 60 MVA, scaled up by 10%
        option_.n_parallel_hv_mv_transformer =
            static_cast<Idx>(static_cast<double>(option.n_mv_feeder) * 10.0 * 1.1 / 60.0) + 1;
        // start generating grid
        generate_mv_grid();
    }

    InputData const& input_data() const { return input_; }

    template <bool sym> OutputData<sym> generate_output_data(Idx batch_size = 1) const {
        batch_size = std::max(batch_size, Idx{1});
        OutputData<sym> output{};
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
        NodeInput const source_node{{id_source_node}, 150.0e3};
        input_.node.push_back(source_node);
        SourceInput const source{{{id_gen_++}, id_source_node, 1}, 1.05, nan, 2000e6, nan, nan};
        input_.source.push_back(source);

        // transformer and mv busbar
        ID const id_mv_busbar = id_gen_++;
        NodeInput const mv_busbar{{id_mv_busbar}, 10.5e3};
        input_.node.push_back(mv_busbar);
        for (Idx i = 0; i != option_.n_parallel_hv_mv_transformer; ++i) {
            // transformer, 150/10.5kV, 60MVA, uk=20.3%
            TransformerInput const transformer{{{id_gen_++}, id_source_node, id_mv_busbar, 1, 1},
                                               150.0e3,
                                               10.5e3,
                                               60.0e6,
                                               0.203,
                                               200e3,
                                               0.01,
                                               40e3,
                                               WindingType::wye_n,
                                               WindingType::delta,
                                               5,
                                               BranchSide::from,
                                               0,
                                               -10,
                                               10,
                                               0,
                                               2.5e3,
                                               nan,
                                               nan,
                                               nan,
                                               nan,
                                               nan,
                                               nan,
                                               nan,
                                               nan};
            input_.transformer.push_back(transformer);
            // shunt, Z0 = 0 + j7 ohm
            ShuntInput const shunt{{{id_gen_++}, id_mv_busbar, 1}, 0.0, 0.0, 0.0, -1.0 / 7.0};
            input_.shunt.push_back(shunt);
        }

        // template input
        NodeInput const mv_node{{0}, 10.5e3};
        SymLoadGenInput const mv_sym_load{{{{0}, 0, 1}, LoadGenType::const_i}, 0.8e6, 0.6e6};
        // cable 3 * 630Al XLPE 10kV, per km
        LineInput const mv_line{{{0}, 0, 0, 1, 1}, 0.063, 0.103, 0.4e-6, 0.0004, 0.275, 0.101, 0.66e-6, 0.0, 1e3};

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
        NodeInput const lv_busbar{{id_lv_busbar}, 400.0};
        input_.node.push_back(lv_busbar);
        // transformer, 1500 kVA or mv base load, uk=6%, pk=8.8kW
        TransformerInput const transformer{{{id_gen_++}, mv_node, id_lv_busbar, 1, 1},
                                           10.5e3,
                                           420.0,
                                           std::max(1500e3, mv_base_load * 1.2),
                                           0.06,
                                           8.8e3,
                                           0.01,
                                           1e3,
                                           WindingType::delta,
                                           WindingType::wye_n,
                                           11,
                                           BranchSide::from,
                                           3,
                                           5,
                                           1,
                                           3,
                                           250.0,
                                           nan,
                                           nan,
                                           nan,
                                           nan,
                                           nan,
                                           nan,
                                           nan,
                                           nan};
        input_.transformer.push_back(transformer);

        // template
        NodeInput const lv_node{{0}, 400.0};
        AsymLoadGenInput const lv_asym_load{
            {{{0}, 0, 1}, LoadGenType::const_i}, RealValue<false>{0.0}, RealValue<false>{0.0}};
        // 4*150 Al, per km
        LineInput const lv_main_line{{{0}, 0, 0, 1, 1}, 0.206, 0.079, 0.72e-6, 0.0004, 0.94, 0.387,
                                     0.36e-6,           0.0,   300.0};
        // 4*16 Cu, per km
        LineInput const lv_connection_line{{{0}, 0, 0, 1, 1}, 1.15, 0.096, 0.43e-6, 0.0004, 4.6, 0.408,
                                           0.258e-6,          0.0,  80.0};

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
        auto const n_object = static_cast<ptrdiff_t>(input.size());
        for (ptrdiff_t batch = 0; batch < batch_size; ++batch) {
            for (ptrdiff_t object = 0; object < n_object; ++object) {
                T const& input_obj = input[object];
                U& update_obj = load_series[batch * n_object + object];
                update_obj.id = input_obj.id;
                update_obj.status = na_IntS;
                update_obj.p_specified *= load_scaling_gen(gen_);
                update_obj.q_specified *= load_scaling_gen(gen_);
            }
        }
    }
};

} // namespace power_grid_model::benchmark
