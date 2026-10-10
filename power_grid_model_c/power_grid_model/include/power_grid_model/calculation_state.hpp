// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <array>
#include <optional>
#include <vector>

namespace power_grid_model {

struct ModelStateRequest {
    bool y_bus{};
    bool jacobian{};
};

enum class ModelStateBusKind : IntS { input_node, synthetic_branch3 };

struct ModelStateGroupMapping {
    Idx group{};
    Idx n_bus{};
    bool is_symmetric{};

    // Reverse ragged mapping: bus i owns user nodes in [bus_user_indptr[i], bus_user_indptr[i + 1]).
    // Synthetic buses have no user-node entries and are identified by bus_kind and origin_branch3_id.
    IdxVector bus_user_indptr;
    IdxVector bus_user_sequence;
    std::vector<ID> bus_user_id;
    std::vector<ModelStateBusKind> bus_kind;
    std::vector<ID> origin_branch3_id;
};

struct ModelStateYBus {
    IdxVector row_indptr;
    IdxVector col_indices;
    std::vector<double> admittance_real;
    std::vector<double> admittance_imag;
};

struct ModelStateJacobian {
    IdxVector row_indptr_lu;
    IdxVector col_indices_lu;
    std::array<std::vector<double>, 4> blocks; // H, N, M, L; flattened row-major per sparse entry.
    Idx iteration{};
};

struct ModelStateGroup {
    ModelStateGroupMapping mapping;
    std::optional<ModelStateYBus> y_bus;
    std::optional<ModelStateJacobian> jacobian;
    std::vector<ModelStateJacobian> jacobian_history;
};

struct ModelStateOutput {
    bool y_bus_requested{};
    bool jacobian_requested{};
    // Forward mapping indexed by input node sequence. Disconnected nodes use {-1, -1}.
    IdxVector input_node_group;
    IdxVector input_node_bus;
    std::vector<ID> input_node_id;
    std::vector<ModelStateGroup> groups;
};

} // namespace power_grid_model