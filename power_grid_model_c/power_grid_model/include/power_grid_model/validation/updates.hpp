#pragma once

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace power_grid_model::validation::batch_detail {

struct InputUpdateAttributePair {
    meta_data::MetaAttribute const* input;
    meta_data::MetaAttribute const* update;
};

template <class Component>
std::vector<InputUpdateAttributePair> component_update_attributes(meta_data::MetaData const& metadata) {
    auto const& input_component = metadata.get_dataset("input").get_component(Component::name);
    auto const& update_component = metadata.get_dataset("update").get_component(Component::name);

    std::vector<InputUpdateAttributePair> pairs;
    for (meta_data::MetaAttribute const& update_attribute : update_component.attributes) {
        std::string_view const field{update_attribute.name};
        if (field == "id") {
            continue;
        }

        power_grid_model::Idx const input_attribute_index = input_component.find_attribute(field);
        if (input_attribute_index < 0) {
            throw std::logic_error{"Update field '" + std::string{field} + "' has no corresponding input field."};
        }
        auto const& input_attribute = input_component.attributes[static_cast<std::size_t>(input_attribute_index)];
        if (input_attribute.ctype != update_attribute.ctype) {
            throw std::logic_error{"Input and update field types differ for '" + std::string{field} + "'."};
        }
        pairs.push_back({&input_attribute, &update_attribute});
    }
    return pairs;
}

template <class Component>
void apply_component_updates(std::vector<typename Component::InputType>& input,
                             std::vector<typename Component::UpdateType> const& updates,
                             std::vector<std::size_t> const& indices,
                             std::vector<InputUpdateAttributePair> const& attribute_pairs) {
    for (std::size_t update_index{}; update_index < updates.size(); ++update_index) {
        auto& input_record = input[indices[update_index]];
        auto const& update_record = updates[update_index];
        for (InputUpdateAttributePair const& pair : attribute_pairs) {
            auto const& input_attribute = *pair.input;
            auto const& update_attribute = *pair.update;
            auto const* update_value = reinterpret_cast<std::byte const*>(&update_record) + update_attribute.offset;
            if (update_attribute.ctype == power_grid_model::CType::c_double3) {
                auto const& update_phases =
                    *reinterpret_cast<power_grid_model::RealValue<power_grid_model::asymmetric_t> const*>(update_value);
                auto& input_phases =
                    input_attribute.template get_attribute<power_grid_model::RealValue<power_grid_model::asymmetric_t>>(
                        &input_record);
                for (power_grid_model::Idx phase{}; phase < 3; ++phase) {
                    if (!power_grid_model::is_nan(update_phases(phase))) {
                        input_phases(phase) = update_phases(phase);
                    }
                }
            } else if (!update_attribute.check_nan(&update_record, 0)) {
                input_attribute.set_value(&input_record, update_value, 0);
            }
        }
    }
}

} // namespace power_grid_model::validation::batch_detail
