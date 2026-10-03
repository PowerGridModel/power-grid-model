#pragma once

#include "issues.hpp"
#include "node.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string>
#include <string_view>

namespace power_grid_model::validation {

inline Issues validate_input_dataset(ConstDataset const& dataset) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw InvalidArguments{"Input validation requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw InvalidArguments{"Input validation does not support batch datasets.\n"};
    }

    bool has_node = false;
    for (Idx component_idx{}; component_idx < dataset.n_components(); ++component_idx) {
        auto const component_name = dataset.get_component_info(component_idx).component->name;
        if (std::string_view{component_name} != "node") {
            throw InvalidArguments{"Input validation supports only the node component; received '" +
                                   std::string{component_name} + "'.\n"};
        }
        has_node = true;
    }

    Issues issues;
    if (has_node) {
        validate_node(dataset, issues);
    }
    return issues;
}

} // namespace power_grid_model::validation
