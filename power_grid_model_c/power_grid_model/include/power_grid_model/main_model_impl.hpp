// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// main model class

// main include
#include "batch_parameter.hpp"
#include "calculation_parameters.hpp"
#include "container.hpp"
#include "main_model_fwd.hpp"
#include "topology.hpp"

// common
#include "common/common.hpp"
#include "common/exception.hpp"
#include "common/timer.hpp"

// component include
#include "all_components.hpp"
#include "auxiliary/dataset.hpp"
#include "auxiliary/input.hpp"
#include "auxiliary/output.hpp"

// math model include
#include "math_solver/math_solver.hpp"

#include "optimizer/optimizer.hpp"

// main model implementation
#include "main_core/calculation_info.hpp"
#include "main_core/input.hpp"
#include "main_core/math_state.hpp"
#include "main_core/output.hpp"
#include "main_core/topology.hpp"
#include "main_core/update.hpp"

// stl library
#include <memory>
#include <span>
#include <thread>

namespace power_grid_model {

// solver output type to output type getter meta function

template <solver_output_type SolverOutputType> struct output_type_getter;
template <short_circuit_solver_output_type SolverOutputType> struct output_type_getter<SolverOutputType> {
    using type = meta_data::sc_output_getter_s;
};
template <> struct output_type_getter<SolverOutput<symmetric_t>> {
    using type = meta_data::sym_output_getter_s;
};
template <> struct output_type_getter<SolverOutput<asymmetric_t>> {
    using type = meta_data::asym_output_getter_s;
};

struct power_flow_t {};
struct state_estimation_t {};
struct short_circuit_t {};

template <typename T>
concept calculation_type_tag = std::derived_from<T, power_flow_t> || std::derived_from<T, state_estimation_t> ||
                               std::derived_from<T, short_circuit_t>;

template <class Functor, class... Args>
decltype(auto) calculation_symmetry_func_selector(CalculationSymmetry calculation_symmetry, Functor&& f,
                                                  Args&&... args) {
    using enum CalculationSymmetry;

    switch (calculation_symmetry) {
    case symmetric:
        return std::forward<Functor>(f).template operator()<symmetric_t>(std::forward<Args>(args)...);
    case asymmetric:
        return std::forward<Functor>(f).template operator()<asymmetric_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"Calculation symmetry selector", calculation_symmetry};
    }
}

template <class Functor, class... Args>
decltype(auto) calculation_type_func_selector(CalculationType calculation_type, Functor&& f, Args&&... args) {
    using enum CalculationType;

    switch (calculation_type) {
    case CalculationType::power_flow:
        return std::forward<Functor>(f).template operator()<power_flow_t>(std::forward<Args>(args)...);
    case CalculationType::state_estimation:
        return std::forward<Functor>(f).template operator()<state_estimation_t>(std::forward<Args>(args)...);
    case CalculationType::short_circuit:
        return std::forward<Functor>(f).template operator()<short_circuit_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"CalculationType", calculation_type};
    }
}

template <class Functor, class... Args>
decltype(auto) calculation_type_symmetry_func_selector(CalculationType calculation_type,
                                                       CalculationSymmetry calculation_symmetry, Functor&& f,
                                                       Args&&... args) {
    calculation_type_func_selector(
        calculation_type,
        []<calculation_type_tag calculation_type, typename Functor_, typename... Args_>(
            CalculationSymmetry calculation_symmetry_, Functor_&& f_, Args_&&... args_) {
            calculation_symmetry_func_selector(
                calculation_symmetry_,
                []<symmetry_tag sym, typename SubFunctor, typename... SubArgs>(SubFunctor&& sub_f,
                                                                               SubArgs&&... sub_args) {
                    std::forward<SubFunctor>(sub_f).template operator()<calculation_type, sym>(
                        std::forward<SubArgs>(sub_args)...);
                },
                std::forward<Functor_>(f_), std::forward<Args_>(args_)...);
        },
        calculation_symmetry, std::forward<Functor>(f), std::forward<Args>(args)...);
}

// main model implementation template
template <class T, class U> class MainModelImpl;

template <class... ExtraRetrievableType, class... ComponentType>
class MainModelImpl<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {
  private:
    // internal type traits
    // container class
    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    using MainModelState = main_core::MainModelState<ComponentContainer>;
    using MathState = main_core::MathState;

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

    // all component count
    std::map<std::string, Idx, std::less<>> all_component_count() const {
        auto const get_comp_count = [this]<typename CT>() -> std::pair<std::string, Idx> {
            return make_pair(std::string{CT::name}, this->component_count<CT>());
        };
        auto const all_count = run_functor_with_all_types_return_array(get_comp_count);
        std::map<std::string, Idx, std::less<>> result;
        for (auto const& [name, count] : all_count) {
            if (count > 0) {
                // only add if count is greater than 0
                result[name] = count;
            }
        }
        return result;
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
        update_state(changed);
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

    template <typename CompType> void restore_component(SequenceIdx const& sequence_idx) {
        constexpr auto component_index = index_of_component<CompType>;

        auto& cached_inverse_update = std::get<component_index>(cached_inverse_update_);
        auto const& component_sequence = std::get<component_index>(sequence_idx);

        if (!cached_inverse_update.empty()) {
            update_component<CompType, permanent_update_t>(cached_inverse_update, component_sequence);
            cached_inverse_update.clear();
        }
    }

    // restore the initial values of all components
    void restore_components(SequenceIdx const& sequence_idx) {
        (restore_component<ComponentType>(sequence_idx), ...);

        update_state(cached_state_changes_);
        cached_state_changes_ = {};
    }

    void restore_components(ConstDataset const& update_data) { restore_components(get_sequence_idx_map(update_data)); }

    // set complete construction
    // initialize internal arrays
    void set_construction_complete() {
        assert(!construction_complete_);
#ifndef NDEBUG
        // set construction_complete for debug assertions
        construction_complete_ = true;
#endif // !NDEBUG
        state_.components.set_construction_complete();
        construct_topology();
    }

    void construct_topology() {
        ComponentTopology comp_topo;
        main_core::register_topology_components<Node>(state_, comp_topo);
        main_core::register_topology_components<Branch>(state_, comp_topo);
        main_core::register_topology_components<Branch3>(state_, comp_topo);
        main_core::register_topology_components<Source>(state_, comp_topo);
        main_core::register_topology_components<Shunt>(state_, comp_topo);
        main_core::register_topology_components<GenericLoadGen>(state_, comp_topo);
        main_core::register_topology_components<GenericVoltageSensor>(state_, comp_topo);
        main_core::register_topology_components<GenericPowerSensor>(state_, comp_topo);
        main_core::register_topology_components<Regulator>(state_, comp_topo);
        state_.comp_topo = std::make_shared<ComponentTopology const>(std::move(comp_topo));
    }

    void reset_solvers() {
        assert(construction_complete_);
        is_topology_up_to_date_ = false;
        is_sym_parameter_up_to_date_ = false;
        is_asym_parameter_up_to_date_ = false;
        n_math_solvers_ = 0;
        main_core::clear(math_state_);
        state_.math_topology.clear();
        state_.topo_comp_coup.reset();
        state_.comp_coup = {};
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
    void update_state(const UpdateChange& changes) {
        // if topology changed, everything is not up to date
        // if only param changed, set param to not up to date
        is_topology_up_to_date_ = is_topology_up_to_date_ && !changes.topo;
        is_sym_parameter_up_to_date_ = is_sym_parameter_up_to_date_ && !changes.topo && !changes.param;
        is_asym_parameter_up_to_date_ = is_asym_parameter_up_to_date_ && !changes.topo && !changes.param;
    }

    template <typename... Args, typename RunFn, typename SetupFn, typename WinddownFn, typename HandleExceptionFn,
              typename RecoverFromBadFn>
        requires std::invocable<std::remove_cvref_t<RunFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<SetupFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<WinddownFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<HandleExceptionFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<RecoverFromBadFn>, Args const&...>
    static auto call_with(RunFn run, SetupFn setup, WinddownFn winddown, HandleExceptionFn handle_exception,
                          RecoverFromBadFn recover_from_bad) {
        return [setup_ = std::move(setup), run_ = std::move(run), winddown_ = std::move(winddown),
                handle_exception_ = std::move(handle_exception),
                recover_from_bad_ = std::move(recover_from_bad)](Args const&... args) {
            try {
                setup_(args...);
                run_(args...);
                winddown_(args...);
            } catch (...) {
                handle_exception_(args...);
                try {
                    winddown_(args...);
                } catch (...) {
                    recover_from_bad_(args...);
                }
            }
        };
    }

    static auto scenario_update_restore(MainModelImpl& model, ConstDataset const& update_data,
                                        SequenceIdx const& scenario_sequence, SequenceIdx& scenario_sequence_cache,
                                        bool is_independent, std::vector<CalculationInfo>& infos) noexcept {
        bool const do_update_cache = !is_independent;
        return std::make_pair(
            [&model, &update_data, &scenario_sequence, &scenario_sequence_cache, do_update_cache,
             &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1200, "Update model");
                if (do_update_cache) {
                    scenario_sequence_cache = model.get_sequence_idx_map(update_data, scenario_idx);
                }
                model.template update_component<cached_update_t>(update_data, scenario_idx, scenario_sequence);
            },
            [&model, &scenario_sequence, &scenario_sequence_cache, do_update_cache, &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1201, "Restore model");
                model.restore_components(scenario_sequence);
                if (do_update_cache) {
                    std::ranges::for_each(scenario_sequence_cache, [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
                }
            });
    }

    // Lippincott pattern
    static auto scenario_exception_handler(MainModelImpl& model, std::vector<std::string>& messages,
                                           std::vector<CalculationInfo>& infos) {
        return [&model, &messages, &infos](Idx scenario_idx) {
            std::exception_ptr const ex_ptr = std::current_exception();
            try {
                std::rethrow_exception(ex_ptr);
            } catch (std::exception const& ex) {
                messages[scenario_idx] = ex.what();
            } catch (...) {
                messages[scenario_idx] = "unknown exception";
            }
            infos[scenario_idx].merge(model.calculation_info_);
        };
    }

    static void handle_batch_exceptions(std::vector<std::string> const& exceptions) {
        std::string combined_error_message;
        IdxVector failed_scenarios;
        std::vector<std::string> err_msgs;
        for (Idx batch = 0; batch < static_cast<Idx>(exceptions.size()); ++batch) {
            // append exception if it is not empty
            if (!exceptions[batch].empty()) {
                combined_error_message += "Error in batch #" + std::to_string(batch) + ": " + exceptions[batch];
                failed_scenarios.push_back(batch);
                err_msgs.push_back(exceptions[batch]);
            }
        }
        if (!combined_error_message.empty()) {
            throw BatchCalculationError(combined_error_message, failed_scenarios, err_msgs);
        }
    }

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
    mutable CalculationInfo calculation_info_; // needs to be first due to padding override
                                               // may be changed in const functions for metrics

    double system_frequency_;
    meta_data::MetaData const* meta_data_;

    MainModelState state_;
    // math model
    MathState math_state_;
    Idx n_math_solvers_{0};
    bool is_topology_up_to_date_{false};
    bool is_sym_parameter_up_to_date_{false};
    bool is_asym_parameter_up_to_date_{false};
    bool is_accumulated_component_updated_{true};
    bool last_updated_calculation_symmetry_mode_{false};

    OwnedUpdateDataset cached_inverse_update_{};
    UpdateChange cached_state_changes_{};
    std::array<std::vector<Idx2D>, n_types> parameter_changed_components_{};
#ifndef NDEBUG
    // construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif // !NDEBUG

    template <symmetry_tag sym> bool& is_parameter_up_to_date() {
        if constexpr (is_symmetric_v<sym>) {
            return is_sym_parameter_up_to_date_;
        } else {
            return is_asym_parameter_up_to_date_;
        }
    }

    template <symmetry_tag sym> std::vector<MathSolver<sym>>& get_solvers() {
        if constexpr (is_symmetric_v<sym>) {
            return math_state_.math_solvers_sym;
        } else {
            return math_state_.math_solvers_asym;
        }
    }

    template <symmetry_tag sym> std::vector<YBus<sym>>& get_y_bus() {
        if constexpr (is_symmetric_v<sym>) {
            return math_state_.y_bus_vec_sym;
        } else {
            return math_state_.y_bus_vec_asym;
        }
    }

    void rebuild_topology() {
        assert(construction_complete_);
        // clear old solvers
        reset_solvers();
        // get connection info
        ComponentConnections comp_conn;
        comp_conn.branch_connected.resize(state_.comp_topo->branch_node_idx.size());
        comp_conn.branch_phase_shift.resize(state_.comp_topo->branch_node_idx.size());
        comp_conn.branch3_connected.resize(state_.comp_topo->branch3_node_idx.size());
        comp_conn.branch3_phase_shift.resize(state_.comp_topo->branch3_node_idx.size());
        comp_conn.source_connected.resize(state_.comp_topo->source_node_idx.size());
        std::transform(
            state_.components.template citer<Branch>().begin(), state_.components.template citer<Branch>().end(),
            comp_conn.branch_connected.begin(), [](Branch const& branch) {
                return BranchConnected{static_cast<IntS>(branch.from_status()), static_cast<IntS>(branch.to_status())};
            });
        std::transform(state_.components.template citer<Branch>().begin(),
                       state_.components.template citer<Branch>().end(), comp_conn.branch_phase_shift.begin(),
                       [](Branch const& branch) { return branch.phase_shift(); });
        std::transform(
            state_.components.template citer<Branch3>().begin(), state_.components.template citer<Branch3>().end(),
            comp_conn.branch3_connected.begin(), [](Branch3 const& branch3) {
                return Branch3Connected{static_cast<IntS>(branch3.status_1()), static_cast<IntS>(branch3.status_2()),
                                        static_cast<IntS>(branch3.status_3())};
            });
        std::transform(state_.components.template citer<Branch3>().begin(),
                       state_.components.template citer<Branch3>().end(), comp_conn.branch3_phase_shift.begin(),
                       [](Branch3 const& branch3) { return branch3.phase_shift(); });
        std::transform(state_.components.template citer<Source>().begin(),
                       state_.components.template citer<Source>().end(), comp_conn.source_connected.begin(),
                       [](Source const& source) { return source.status(); });
        // re build
        Topology topology{*state_.comp_topo, comp_conn};
        std::tie(state_.math_topology, state_.topo_comp_coup) = topology.build_topology();
        n_math_solvers_ = static_cast<Idx>(state_.math_topology.size());
        is_topology_up_to_date_ = true;
        is_sym_parameter_up_to_date_ = false;
        is_asym_parameter_up_to_date_ = false;
    }

    template <symmetry_tag sym> std::vector<MathModelParam<sym>> get_math_param() {
        std::vector<MathModelParam<sym>> math_param(n_math_solvers_);
        for (Idx i = 0; i != n_math_solvers_; ++i) {
            math_param[i].branch_param.resize(state_.math_topology[i]->n_branch());
            math_param[i].shunt_param.resize(state_.math_topology[i]->n_shunt());
            math_param[i].source_param.resize(state_.math_topology[i]->n_source());
        }
        // loop all branch
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->branch_node_idx.size()); ++i) {
            Idx2D const math_idx = state_.topo_comp_coup->branch[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].branch_param[math_idx.pos] =
                state_.components.template get_item_by_seq<Branch>(i).template calc_param<sym>();
        }
        // loop all branch3
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->branch3_node_idx.size()); ++i) {
            Idx2DBranch3 const math_idx = state_.topo_comp_coup->branch3[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters, branch3 param consists of three branch parameters
            auto const branch3_param =
                state_.components.template get_item_by_seq<Branch3>(i).template calc_param<sym>();
            for (size_t branch2 = 0; branch2 < 3; ++branch2) {
                math_param[math_idx.group].branch_param[math_idx.pos[branch2]] = branch3_param[branch2];
            }
        }
        // loop all shunt
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->shunt_node_idx.size()); ++i) {
            Idx2D const math_idx = state_.topo_comp_coup->shunt[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].shunt_param[math_idx.pos] =
                state_.components.template get_item_by_seq<Shunt>(i).template calc_param<sym>();
        }
        // loop all source
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->source_node_idx.size()); ++i) {
            Idx2D const math_idx = state_.topo_comp_coup->source[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].source_param[math_idx.pos] =
                state_.components.template get_item_by_seq<Source>(i).template math_param<sym>();
        }
        return math_param;
    }
    template <symmetry_tag sym> std::vector<MathModelParamIncrement> get_math_param_increment() {
        using AddToIncrement = void (*)(std::vector<MathModelParamIncrement>&, MainModelState const&, Idx2D const&);

        static constexpr std::array<AddToIncrement, n_types> add_to_increments{
            [](std::vector<MathModelParamIncrement>& increments, MainModelState const& state,
               Idx2D const& changed_component_idx) {
                if constexpr (std::derived_from<ComponentType, Branch>) {
                    Idx2D const math_idx =
                        state.topo_comp_coup
                            ->branch[main_core::get_component_sequence<Branch>(state, changed_component_idx)];
                    if (math_idx.group == -1) {
                        return;
                    }
                    // assign parameters
                    increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos);
                } else if constexpr (std::derived_from<ComponentType, Branch3>) {
                    Idx2DBranch3 const math_idx =
                        state.topo_comp_coup
                            ->branch3[main_core::get_component_sequence<Branch3>(state, changed_component_idx)];
                    if (math_idx.group == -1) {
                        return;
                    }
                    // assign parameters, branch3 param consists of three branch parameters
                    // auto const branch3_param =
                    //   get_component<Branch3>(state, changed_component_idx).template calc_param<sym>();
                    for (size_t branch2 = 0; branch2 < 3; ++branch2) {
                        increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos[branch2]);
                    }
                } else if constexpr (std::same_as<ComponentType, Shunt>) {
                    Idx2D const math_idx =
                        state.topo_comp_coup
                            ->shunt[main_core::get_component_sequence<Shunt>(state, changed_component_idx)];
                    if (math_idx.group == -1) {
                        return;
                    }
                    // assign parameters
                    increments[math_idx.group].shunt_param_to_change.push_back(math_idx.pos);
                }
            }...};

        std::vector<MathModelParamIncrement> math_param_increment(n_math_solvers_);

        for (size_t i = 0; i < n_types; ++i) {
            auto const& changed_type_components = parameter_changed_components_[i];
            auto const& add_type_to_increment = add_to_increments[i];
            for (auto const& changed_component : changed_type_components) {
                add_type_to_increment(math_param_increment, state_, changed_component);
            }
        }

        return math_param_increment;
    }

    static constexpr auto include_all = [](Idx) { return true; };
};

} // namespace power_grid_model
