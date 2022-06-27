// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <random>

#include "power_grid_model/main_model.hpp"
#include "power_grid_model/timer.hpp"

namespace power_grid_model::benchmark {

struct Option {
    Idx n_node_total_specified;  // rough specification of total nodes
    Idx n_mv_feeder;  // n mv feeder will be changed it is too small
    Idx n_node_per_mv_feeder;
    Idx n_lv_feeder;
    Idx n_connection_per_lv_feeder;  // per connection: one node for connection joint, one node for actual house
    Idx n_parallel_mv_lv_transformer;
    Idx n_lv_grid;  // will be calculated
    double ratio_lv_grid;  // ratio when lv grid will be generated
};

struct InputData {
    std::vector<NodeInput> node;
    std::vector<SourceInput> source;
    std::vector<SymLoadGenInput> sym_load;
    std::vector<AsymLoadGenInput> asym_load;
    std::vector<TransformerInput> transformer;
    std::vector<LineInput> line;
};

template <bool sym>
struct OutputData {
    std::vector<NodeOutput<sym>> node;
    std::vector<ApplianceOutput<sym>> source;
    std::vector<ApplianceOutput<sym>> sym_load;
    std::vector<ApplianceOutput<sym>> asym_load;
    std::vector<BranchOutput<sym>> transformer;
    std::vector<BranchOutput<sym>> line;
};

class FictionalGridGenerator {
   public:
    FictionalGridGenerator() {
    }

    void generate_grid(Option const& option) {
        generate_grid(option, std::random_device{}());
    }

    void generate_grid(Option const& option, std::random_device::result_type seed) {
        // initialization
        option_ = option;
        gen_ = std::mt19937_64{seed};
        id_gen_ = 0;
        // process option to calculate n_lv_grid
        Idx const total_mv_connection = option_.n_mv_feeder * option_.n_node_per_mv_feeder;
        Idx const node_per_lv_grid = option_.n_lv_feeder * option_.n_connection_per_lv_feeder * 2 + 1;
        option_.n_lv_grid = (option_.n_node_total_specified - total_mv_connection) / node_per_lv_grid;
        if (option_.n_lv_grid > total_mv_connection) {
            option_.n_mv_feeder = option_.n_lv_grid / option_.n_node_per_mv_feeder + 1;
        }
        option_.ratio_lv_grid =
            (double)option_.n_lv_grid / (double)(option_.n_mv_feeder * option_.n_node_per_mv_feeder);
        // start generating grid
        generate_mv_grid();
    }

    void generate_mv_grid() {

    }

    void generate_lv_grid(Idx mv_node) {
    }

   private:
    Option option_{};
    std::mt19937_64 gen_;
    Idx id_gen_;
    InputData input_;
    OutputData<true> sym_output_;
    OutputData<false> asym_output_;
};

}  // namespace power_grid_model::benchmark