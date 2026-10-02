#include "input.hpp"

#include "node.hpp"

#include <power_grid_model/common/exception.hpp>

#include <string>
#include <string_view>

namespace power_grid_model_c::validation {

Issues validate_input_dataset(power_grid_model::ConstDataset const& dataset) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw power_grid_model::InvalidArguments{"PGM_validate_input_data requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw power_grid_model::InvalidArguments{"PGM_validate_input_data does not support batch datasets.\n"};
    }

    bool has_node = false;
    for (power_grid_model::Idx component_idx{}; component_idx < dataset.n_components(); ++component_idx) {
        auto const component_name = dataset.get_component_info(component_idx).component->name;
        if (std::string_view{component_name} != "node") {
            throw power_grid_model::InvalidArguments{
                "PGM_validate_input_data supports only the node component; received '" + std::string{component_name} +
                "'.\n"};
        }
        has_node = true;
    }

    Issues issues;
    if (has_node) {
        validate_node(dataset, issues);
    }
    return issues;
}

} // namespace power_grid_model_c::validation
