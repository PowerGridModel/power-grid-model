// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "core_utils.hpp"
#include "state.hpp"

#include "../all_components.hpp"
#include "../common/iterator_facade.hpp"
#include "../container.hpp"

#include <map>

namespace power_grid_model::main_core::update {

namespace detail {
template <component_c Component, std::forward_iterator ForwardIterator, typename Func>
    requires std::invocable<std::remove_cvref_t<Func>, typename Component::UpdateType, Idx2D const&>
inline void iterate_component_sequence(Func func, ForwardIterator begin, ForwardIterator end,
                                       std::span<Idx2D const> sequence_idx) {
    assert(std::distance(begin, end) >= static_cast<ptrdiff_t>(sequence_idx.size()));

    Idx seq = 0;

    // loop to to update component
    for (auto it = begin; it != end; ++it, ++seq) {
        // get component directly using sequence id
        func(*it, sequence_idx[seq]);
    }
}

template <typename T> bool check_id_na(T const& obj) {
    if constexpr (requires { obj.id; }) {
        return is_nan(obj.id);
    } else if constexpr (requires { obj.get().id; }) {
        return is_nan(obj.get().id);
    } else {
        throw UnreachableHit{"check_component_independence", "Only components with id are supported"};
    }
}

} // namespace detail

namespace independence {
struct UpdateCompProperties {
    bool has_any_elements{false};                    // whether the component has any elements in the update data
    bool ids_all_na{false};                          // whether all ids are all NA
    bool ids_part_na{false};                         // whether some ids are NA but some are not
    bool dense{false};                               // whether the component is dense
    bool uniform{false};                             // whether the component is uniform
    bool is_columnar{false};                         // whether the component is columnar
    bool update_ids_match{false};                    // whether the ids match
    Idx elements_ps_in_update{utils::invalid_index}; // count of elements for this component per scenario in update
    Idx elements_in_base{utils::invalid_index};      // count of elements for this component per scenario in input

    constexpr bool no_id() const { return !has_any_elements || ids_all_na; }
    constexpr bool qualify_for_optional_id() const {
        return update_ids_match && ids_all_na && uniform && elements_ps_in_update == elements_in_base;
    }
    constexpr bool provided_ids_valid() const {
        return is_empty_component() || (update_ids_match && !(ids_all_na || ids_part_na));
    }
    constexpr bool is_empty_component() const { return !has_any_elements; }
    constexpr bool is_independent() const { return qualify_for_optional_id() || provided_ids_valid(); }
    constexpr Idx get_n_elements() const {
        assert(uniform || elements_ps_in_update == utils::invalid_index);

        return qualify_for_optional_id() ? elements_ps_in_update : na_Idx;
    }
};

template <typename CompType> void process_buffer_span(auto const& all_spans, UpdateCompProperties& properties) {
    properties.ids_all_na = std::ranges::all_of(all_spans, [](auto const& vec) {
        return std::ranges::all_of(vec, [](auto const& item) { return detail::check_id_na(item); });
    });
    properties.ids_part_na = std::ranges::any_of(all_spans,
                                                 [](auto const& vec) {
                                                     return std::ranges::any_of(vec, [](auto const& item) {
                                                         return detail::check_id_na(item);
                                                     });
                                                 }) &&
                             !properties.ids_all_na;

    if (all_spans.empty()) {
        properties.update_ids_match = true;
        return;
    }
    // Remember the begin iterator of the first scenario, then loop over the remaining scenarios and
    // check the ids
    auto const first_span = all_spans.front();
    // check the subsequent scenarios
    // only return true if ids of all scenarios match the ids of the first batch

    properties.update_ids_match =
        std::ranges::all_of(all_spans.cbegin() + 1, all_spans.cend(), [&first_span](auto const& current_span) {
            return std::ranges::equal(current_span, first_span,
                                      [](typename CompType::UpdateType const& obj,
                                         typename CompType::UpdateType const& first) { return obj.id == first.id; });
        });
}

template <class CompType>
UpdateCompProperties check_component_independence(ConstDataset const& update_data, Idx n_component) {
    UpdateCompProperties properties;
    auto const component_idx = update_data.find_component(CompType::name, false);
    properties.is_columnar = update_data.is_columnar(CompType::name);
    properties.dense = update_data.is_dense(CompType::name);
    properties.uniform = update_data.is_uniform(CompType::name);
    properties.has_any_elements =
        component_idx != utils::invalid_index && update_data.get_component_info(component_idx).total_elements > 0;
    properties.elements_ps_in_update =
        properties.uniform ? update_data.uniform_elements_per_scenario(CompType::name) : utils::invalid_index;
    properties.elements_in_base = n_component;

    if (properties.is_columnar) {
        process_buffer_span<CompType>(
            update_data.template get_columnar_buffer_span_all_scenarios<meta_data::update_getter_s, CompType>(),
            properties);
    } else {
        process_buffer_span<CompType>(
            update_data.template get_buffer_span_all_scenarios<meta_data::update_getter_s, CompType>(), properties);
    }

    return properties;
}

template <class... ComponentTypes>
using UpdateIndependence = std::array<UpdateCompProperties, utils::n_types<ComponentTypes...>>;

inline void validate_update_data_independence(UpdateCompProperties const& comp, std::string const& comp_name) {
    if (comp.is_empty_component()) {
        return; // empty dataset is still supported
    }
    auto const elements_ps = comp.get_n_elements();
    assert(comp.uniform || elements_ps < 0);

    if (elements_ps >= 0 && comp.elements_in_base < elements_ps) {
        throw DatasetError("Update data has more elements per scenario than input data for component " + comp_name +
                           "!");
    }
    if (comp.ids_part_na) {
        throw DatasetError("IDs contain both numbers and NANs for component " + comp_name + " in update data!");
    }
    if (comp.ids_all_na && comp.elements_in_base != elements_ps) {
        throw DatasetError("Update data without IDs for component " + comp_name +
                           " has a different number of elements per scenario then input data!");
    }
}

template <class... ComponentTypes, class ComponentContainer>
UpdateIndependence<ComponentTypes...> check_update_independence(MainModelState<ComponentContainer> const& state,
                                                                ConstDataset const& update_data) {
    return utils::run_functor_with_all_types_return_array<ComponentTypes...>(
        [&state, &update_data]<typename CompType>() {
            auto const n_component = state.components.template size<CompType>();
            return check_component_independence<CompType>(update_data, n_component);
        });
}

} // namespace independence

namespace detail {

template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void get_component_sequence_impl(MainModelState<ComponentContainer> const& state, ForwardIterator begin,
                                        ForwardIterator end, OutputIterator destination, Idx n_comp_elements) {
    using UpdateType = typename Component::UpdateType;

    if (n_comp_elements < 0) {
        std::ranges::transform(begin, end, destination, [&state](UpdateType const& update) {
            return get_component_idx_by_id<Component>(state, update.id);
        });
    } else {
        assert(std::distance(begin, end) <= n_comp_elements);
        std::ranges::transform(
            begin, end, destination,
            [group = get_component_group_idx<Component>(state), index = 0](auto const& /*update*/) mutable {
                return Idx2D{group, index++}; // NOSONAR
            });
    }
}

template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline std::vector<Idx2D> get_component_sequence_by_iter(MainModelState<ComponentContainer> const& state,
                                                         ForwardIterator begin, ForwardIterator end,
                                                         Idx n_comp_elements = na_Idx) {
    std::vector<Idx2D> result;
    result.reserve(std::distance(begin, end));
    get_component_sequence_impl<Component>(state, begin, end, std::back_inserter(result), n_comp_elements);
    return result;
}

// get sequence idx map of a certain batch scenario
template <typename CompType, class ComponentContainer>
std::vector<Idx2D> get_component_sequence(MainModelState<ComponentContainer> const& state,
                                          ConstDataset const& update_data, Idx scenario_idx,
                                          independence::UpdateCompProperties const& comp_independence = {}) {
    auto const get_sequence = [&state, n_comp_elements = comp_independence.get_n_elements()](auto const& span) {
        return get_component_sequence_by_iter<CompType>(state, std::begin(span), std::end(span), n_comp_elements);
    };
    if (update_data.is_columnar(CompType::name)) {
        auto const buffer_span =
            update_data.get_columnar_buffer_span<meta_data::update_getter_s, CompType>(scenario_idx);
        return get_sequence(buffer_span);
    }
    auto const buffer_span = update_data.get_buffer_span<meta_data::update_getter_s, CompType>(scenario_idx);
    return get_sequence(buffer_span);
}
} // namespace detail

template <class... ComponentTypes, class ComponentContainer>
utils::SequenceIdx<ComponentTypes...>
get_all_sequence_idx_map(MainModelState<ComponentContainer> const& state, ConstDataset const& update_data,
                         Idx scenario_idx, utils::ComponentFlags<ComponentTypes...> const& components_to_store,
                         independence::UpdateIndependence<ComponentTypes...> const& independence, bool cached) {
    return utils::run_functor_with_all_types_return_array<ComponentTypes...>(
        [&state, &update_data, scenario_idx, &components_to_store, &independence, cached]<typename CompType>() {
            auto const component_properties =
                std::get<utils::index_of_component<CompType, ComponentTypes...>>(independence);
            // The sequence for the independent components is cached (true). For the remaining components, the sequence
            // cannot be cached (false), so the independence flags are inverted to not return an empty sequence when
            // this is the case.

            if (bool const component_independence = cached != component_properties.is_independent();
                !component_independence ||
                !std::get<utils::index_of_component<CompType, ComponentTypes...>>(components_to_store)) {
                return std::vector<Idx2D>{};
            }
            independence::validate_update_data_independence(component_properties, CompType::name);
            return detail::get_component_sequence<CompType>(state, update_data, scenario_idx, component_properties);
        });
}

// template to update components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin,
                                     ForwardIterator end, OutputIterator changed_it,
                                     std::span<Idx2D const> sequence_idx) {
    using UpdateType = typename Component::UpdateType;

    UpdateChange state_changed;

    detail::iterate_component_sequence<Component>(
        [&state_changed, &changed_it, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto& comp = get_component<Component>(state, sequence_single);
            assert(state.components.get_id_by_idx(sequence_single) == comp.id());
            auto const comp_changed = comp.update(update_data);
            state_changed = state_changed || comp_changed;

            if (comp_changed.param || comp_changed.topo) {
                *changed_it++ = sequence_single;
            }
        },
        std::move(begin), std::move(end), sequence_idx);

    return state_changed;
}
template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin,
                                     ForwardIterator end, OutputIterator changed_it) {
    return update_component<Component>(state, begin, end, changed_it,
                                       detail::get_component_sequence_by_iter<Component>(state, begin, end));
}

// template to get the inverse update for components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<typename Component::UpdateType> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void update_inverse(MainModelState<ComponentContainer> const& state, ForwardIterator begin, ForwardIterator end,
                           OutputIterator destination, std::span<Idx2D const> sequence_idx) {
    using UpdateType = typename Component::UpdateType;

    detail::iterate_component_sequence<Component>(
        [&destination, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto const& comp = get_component<Component>(state, sequence_single);
            *destination++ = comp.inverse(update_data);
        },
        begin, end, sequence_idx);
}
template <component_c Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<typename Component::UpdateType> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void update_inverse(MainModelState<ComponentContainer> const& state, ForwardIterator begin, ForwardIterator end,
                           OutputIterator destination) {
    return update_inverse<Component>(state, begin, end, destination,
                                     detail::get_component_sequence_by_iter<Component>(state, begin, end));
}

} // namespace power_grid_model::main_core::update
