// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// main model class

// main include
#include "container.hpp"
#include "main_model_fwd.hpp"

// common
#include "common/common.hpp"
#include "common/exception.hpp"

// component include
#include "all_components.hpp"
#include "auxiliary/dataset.hpp"
#include "auxiliary/input.hpp"

// main model implementation
#include "main_core/input.hpp"
#include "main_core/update.hpp"

// stl library
#include <memory>
#include <span>

namespace power_grid_model {

// solver output type to output type getter meta function

// main model implementation template
template <class T, class U> class MainModelImpl;

template <class... ExtraRetrievableType, class... ComponentType>
class MainModelImpl<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {
  private:
    // internal type traits
    // container class
    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    using MainModelState = main_core::MainModelState<ComponentContainer>;

    template <class CT>
    static constexpr size_t index_of_component = container_impl::get_cls_pos_v<CT, ComponentType...>;

    // trait on type list
    // struct of entry
    // name of the component, and the index in the list
    struct ComponentEntry {
        char const* name;
        size_t index;
    };

    static constexpr size_t n_types = sizeof...(ComponentType);

    using SequenceIdx = std::array<std::vector<Idx2D>, n_types>;

    using OwnedUpdateDataset = std::tuple<std::vector<typename ComponentType::UpdateType>...>;

    static constexpr Idx ignore_output{-1};

  protected:
    // run functors with all component types
    template <class Functor> static constexpr void run_functor_with_all_types_return_void(Functor functor) {
        (functor.template operator()<ComponentType>(), ...);
    }
    template <class Functor> static constexpr auto run_functor_with_all_types_return_array(Functor functor) {
        return std::array { functor.template operator()<ComponentType>()... };
    }

  public:
    using Options = MainModelOptions;

    // constructor with data
    explicit MainModelImpl(double system_frequency, ConstDataset const& input_data, Idx pos = 0)
        : system_frequency_{system_frequency}, meta_data_{&input_data.meta_data()} {
        assert(input_data.get_description().dataset->name == std::string_view("input"));
        add_components(input_data, pos);
        set_construction_complete();
    }

    // constructor with only frequency
    explicit MainModelImpl(double system_frequency, meta_data::MetaData const& meta_data)
        : system_frequency_{system_frequency}, meta_data_{&meta_data} {}

    // get number
    template <class CompType> Idx component_count() const {
        assert(construction_complete_);
        return state_.components.template size<CompType>();
    }

    // helper function to add vectors of components
    template <class CompType> void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(components.begin(), components.end());
    }
    template <class CompType> void add_component(std::span<typename CompType::InputType const> components) {
        add_component<CompType>(components.begin(), components.end());
    }
    template <class CompType>
    void add_component(ConstDataset::RangeObject<typename CompType::InputType const> components) {
        add_component<CompType>(components.begin(), components.end());
    }

    // template to construct components
    // using forward interators
    // different selection based on component type
    template <std::derived_from<Base> CompType, forward_iterator_like<typename CompType::InputType> ForwardIterator>
    void add_component(ForwardIterator begin, ForwardIterator end) {
        assert(!construction_complete_);
        main_core::add_component<CompType>(state_, begin, end, system_frequency_);
    }

    void add_components(ConstDataset const& input_data, Idx pos = 0) {
        auto const add_func = [this, pos, &input_data]<typename CT>() {
            if (input_data.is_columnar(CT::name)) {
                this->add_component<CT>(input_data.get_columnar_buffer_span<meta_data::input_getter_s, CT>(pos));
            } else {
                this->add_component<CT>(input_data.get_buffer_span<meta_data::input_getter_s, CT>(pos));
            }
        };
        run_functor_with_all_types_return_void(add_func);
    }

    // template to update components
    // using forward interators
    // different selection based on component type
    // if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
    template <class CompType, cache_type_c CacheType,
              forward_iterator_like<typename CompType::UpdateType> ForwardIterator>
    void update_component(ForwardIterator begin, ForwardIterator end, std::vector<Idx2D> const& sequence_idx) {
        constexpr auto comp_index = index_of_component<CompType>;

        assert(construction_complete_);
        assert(static_cast<ptrdiff_t>(sequence_idx.size()) == std::distance(begin, end));

        if constexpr (CacheType::value) {
            main_core::update_inverse<CompType>(
                state_, begin, end, std::back_inserter(std::get<comp_index>(cached_inverse_update_)), sequence_idx);
        }

        UpdateChange const changed = main_core::update_component<CompType>(
            state_, begin, end, std::back_inserter(std::get<comp_index>(parameter_changed_components_)), sequence_idx);

        // update, get changed variable
        //update_state(changed);
        if constexpr (CacheType::value) {
            cached_state_changes_ = cached_state_changes_ || changed;
        }
    }

    // helper function to update vectors of components
    template <class CompType, cache_type_c CacheType>
    void update_component(std::vector<typename CompType::UpdateType> const& components,
                          std::vector<Idx2D> const& sequence_idx) {
        if (!components.empty()) {
            update_component<CompType, CacheType>(components.begin(), components.end(), sequence_idx);
        }
    }
    template <class CompType, cache_type_c CacheType>
    void update_component(std::span<typename CompType::UpdateType const> components,
                          std::vector<Idx2D> const& sequence_idx) {
        if (!components.empty()) {
            update_component<CompType, CacheType>(components.begin(), components.end(), sequence_idx);
        }
    }
    template <class CompType, cache_type_c CacheType>
    void update_component(ConstDataset::RangeObject<typename CompType::UpdateType const> components,
                          std::vector<Idx2D> const& sequence_idx) {
        if (!components.empty()) {
            update_component<CompType, CacheType>(components.begin(), components.end(), sequence_idx);
        }
    }

    // update all components
    template <cache_type_c CacheType>
    void update_component(ConstDataset const& update_data, Idx pos, SequenceIdx const& sequence_idx_map) {
        assert(construction_complete_);
        assert(update_data.get_description().dataset->name == std::string_view("update"));
        auto const update_func = [this, pos, &update_data, &sequence_idx_map]<typename CT>() {
            if (update_data.is_columnar(CT::name)) {
                this->update_component<CT, CacheType>(
                    update_data.get_columnar_buffer_span<meta_data::update_getter_s, CT>(pos),
                    sequence_idx_map[index_of_component<CT>]);
            } else {
                this->update_component<CT, CacheType>(update_data.get_buffer_span<meta_data::update_getter_s, CT>(pos),
                                                      sequence_idx_map[index_of_component<CT>]);
            }
        };
        run_functor_with_all_types_return_void(update_func);
    }

    // update all components
    template <cache_type_c CacheType> void update_component(ConstDataset const& update_data, Idx pos = 0) {
        update_component<CacheType>(update_data, pos, get_sequence_idx_map(update_data));
    }

    // set complete construction
    // initialize internal arrays
    void set_construction_complete() {
        assert(!construction_complete_);
#ifndef NDEBUG
        // set construction_complete for debug assertions
        construction_complete_ = true;
#endif // !NDEBUG
        state_.components.set_construction_complete();
        //construct_topology();
    }

    /*
    the the sequence indexer given an input array of ID's for a given component type
    */
    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        auto const get_index_func = [&state = this->state_, component_type, id_begin, size,
                                     indexer_begin]<typename CT>() {
            if (component_type == CT::name) {
                std::transform(id_begin, id_begin + size, indexer_begin,
                               [&state](ID id) { return main_core::get_component_idx_by_id<CT>(state, id).pos; });
            }
        };
        run_functor_with_all_types_return_void(get_index_func);
    }

    // get sequence idx map of a certain batch scenario
    SequenceIdx get_sequence_idx_map(ConstDataset const& update_data, Idx scenario_idx) const {
        auto const process_buffer_span = [](auto const& buffer_span, auto const& get_sequence) {
            auto const it_begin = buffer_span.begin();
            auto const it_end = buffer_span.end();
            return get_sequence(it_begin, it_end);
        };

        auto const get_seq_idx_func = [&state = this->state_, &update_data, scenario_idx,
                                       &process_buffer_span]<typename CT>() -> std::vector<Idx2D> {
            auto const get_sequence = [&state](auto const& it_begin, auto const& it_end) {
                return main_core::get_component_sequence<CT>(state, it_begin, it_end);
            };

            if (update_data.is_columnar(CT::name)) {
                auto const buffer_span =
                    update_data.get_columnar_buffer_span<meta_data::update_getter_s, CT>(scenario_idx);
                return process_buffer_span(buffer_span, get_sequence);
            }
            auto const buffer_span = update_data.get_buffer_span<meta_data::update_getter_s, CT>(scenario_idx);
            return process_buffer_span(buffer_span, get_sequence);
        };

        return run_functor_with_all_types_return_array(get_seq_idx_func);
    }
    // get sequence idx map of an entire batch for fast caching of component sequences
    // (only applicable for independent update dataset)
    SequenceIdx get_sequence_idx_map(ConstDataset const& update_data) const {
        assert(is_update_independent(update_data));
        return get_sequence_idx_map(update_data, 0);
    }

  private:

  public:
    template <class Component> using UpdateType = typename Component::UpdateType;

    static bool is_update_independent(ConstDataset const& update_data) {
        // If the batch size is (0 or) 1, then the update data for this component is 'independent'
        if (update_data.batch_size() <= 1) {
            return true;
        }

        auto const process_buffer_span = []<typename CT>(auto const& all_spans) -> bool {
            // Remember the first batch size, then loop over the remaining batches and check if they are of the same
            // length
            auto const elements_per_scenario = static_cast<Idx>(all_spans.front().size());
            bool const uniform_batch = std::ranges::all_of(all_spans, [elements_per_scenario](auto const& span) {
                return static_cast<Idx>(span.size()) == elements_per_scenario;
            });
            if (!uniform_batch) {
                return false;
            }
            if (elements_per_scenario == 0) {
                return true;
            }
            // Remember the begin iterator of the first scenario, then loop over the remaining scenarios and check the
            // ids
            auto const first_span = all_spans[0];
            // check the subsequent scenarios
            // only return true if all scenarios match the ids of the first batch
            return std::all_of(all_spans.cbegin() + 1, all_spans.cend(), [&first_span](auto const& current_span) {
                return std::ranges::equal(
                    current_span, first_span,
                    [](UpdateType<CT> const& obj, UpdateType<CT> const& first) { return obj.id == first.id; });
            });
        };

        auto const is_component_update_independent = [&update_data, &process_buffer_span]<typename CT>() -> bool {
            // get span of all the update data
            if (update_data.is_columnar(CT::name)) {
                return process_buffer_span.template operator()<CT>(
                    update_data.get_columnar_buffer_span_all_scenarios<meta_data::update_getter_s, CT>());
            }
            return process_buffer_span.template operator()<CT>(
                update_data.get_buffer_span_all_scenarios<meta_data::update_getter_s, CT>());
        };

        // check all components
        auto const update_independent = run_functor_with_all_types_return_array(is_component_update_independent);
        return std::ranges::all_of(update_independent, [](bool const is_independent) { return is_independent; });
    }

  private:

    double system_frequency_;
    meta_data::MetaData const* meta_data_;

    MainModelState state_;
    // math model

    OwnedUpdateDataset cached_inverse_update_{};
    UpdateChange cached_state_changes_{};
    std::array<std::vector<Idx2D>, n_types> parameter_changed_components_{};
#ifndef NDEBUG
    // construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif // !NDEBUG

    static constexpr auto include_all = [](Idx) { return true; };
};

} // namespace power_grid_model
