// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// main model class

// main include
#include "batch_parameter.hpp"
#include "calculation_parameters.hpp"
#include "container.hpp"
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

// main model implementation
#include "main_core/calculation_info.hpp"
#include "main_core/input.hpp"
#include "main_core/math_state.hpp"
#include "main_core/output.hpp"
#include "main_core/topology.hpp"
#include "main_core/update.hpp"

// threading
#include <thread>

namespace power_grid_model {

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

    // trait on type list
    // struct of entry
    // name of the component, and the index in the list
    struct ComponentEntry {
        char const* name;
        size_t index;
    };

    template <class T, class U> struct component_list_generator_impl;
    template <class... C, size_t... index>
    struct component_list_generator_impl<ComponentList<C...>, std::index_sequence<index...>> {
        using AllTypes = ComponentList<C...>;
        static constexpr std::array component_index_map{ComponentEntry{C::name, index}...};
        static constexpr size_t n_types = sizeof...(C);

        static size_t find_index(std::string const& name) {
            auto const found = std::ranges::find_if(component_index_map, [&name](auto x) { return x == name; });
            assert(found != component_index_map.cend());
            return found->index;
        }

        template <class Comp> static constexpr size_t index_of() { return container_impl::get_cls_pos_v<Comp, C...>; }
    };
    using AllComponents = component_list_generator_impl<ComponentList<ComponentType...>,
                                                        std::make_index_sequence<sizeof...(ComponentType)>>;
    static constexpr size_t n_types = AllComponents::n_types;

    using SequenceIdx = std::array<std::vector<Idx2D>, n_types>;

    using OwnedUpdateDataset = std::tuple<std::vector<typename ComponentType::UpdateType>...>;

    static constexpr Idx ignore_output{-1};

  public:
    struct cached_update_t : std::true_type {};

    struct permanent_update_t : std::false_type {};

    // constructor with data
    explicit MainModelImpl(double system_frequency, ConstDataset const& input_data, Idx pos = 0)
        : system_frequency_{system_frequency} {
        using InputFunc = void (*)(MainModelImpl & x, ConstDataPointer const& data_ptr, Idx position);

        static constexpr std::array<InputFunc, n_types> add{
            [](MainModelImpl& model, ConstDataPointer const& data_ptr, Idx position) {
                auto const [begin, end] = data_ptr.get_iterators<typename ComponentType::InputType>(position);
                model.add_component<ComponentType>(begin, end);
            }...};
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            auto const found = input_data.find(entry.name);
            // skip if component does not exist
            if (found == input_data.cend()) {
                continue;
            }
            // add
            add[entry.index](*this, found->second, pos);
        }
        set_construction_complete();
    }

    // constructor with only frequency
    explicit MainModelImpl(double system_frequency) : system_frequency_{system_frequency} {}

    // get number
    template <class CompType> Idx component_count() const {
        assert(construction_complete_);
        return state_.components.template size<CompType>();
    }

    // all component count
    std::map<std::string, Idx> all_component_count() const {
        std::map<std::string, Idx> map;
        static constexpr std::array counter{&MainModelImpl::component_count<ComponentType>...};
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            Idx const size = std::invoke(counter[entry.index], this);
            if (size > 0) {
                map[entry.name] = size;
            }
        }
        return map;
    }

    // helper function to add vectors of components
    template <class CompType> void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(components.cbegin(), components.cend());
    }

    // template to construct components
    // using forward interators
    // different selection based on component type
    template <std::derived_from<Base> CompType, std::forward_iterator ForwardIterator>
    void add_component(ForwardIterator begin, ForwardIterator end) {
        assert(!construction_complete_);
        main_core::add_component<CompType>(state_, begin, end, system_frequency_);
    }

    // template to update components
    // using forward interators
    // different selection based on component type
    // if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
    template <class CompType, class CacheType, std::forward_iterator ForwardIterator>
    void update_component(ForwardIterator begin, ForwardIterator end, std::vector<Idx2D> const& sequence_idx) {
        constexpr auto comp_index = AllComponents::template index_of<CompType>();

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
    template <class CompType, class CacheType>
    void update_component(std::vector<typename CompType::UpdateType> const& components,
                          std::vector<Idx2D> const& sequence_idx) {
        if (!components.empty()) {
            update_component<CompType, CacheType>(components.cbegin(), components.cend(), sequence_idx);
        }
    }

    // update all components
    template <class CacheType>
    void update_component(ConstDataset const& update_data, Idx pos, SequenceIdx const& sequence_idx_map) {
        using UpdateFunc = void (*)(MainModelImpl & x, ConstDataPointer const& data_ptr, Idx position,
                                    std::vector<Idx2D> const& sequence_idx);

        static constexpr std::array<UpdateFunc, n_types> update{[](MainModelImpl& model,
                                                                   ConstDataPointer const& data_ptr, Idx position,
                                                                   std::vector<Idx2D> const& sequence_idx) {
            auto const [begin, end] = data_ptr.get_iterators<typename ComponentType::UpdateType>(position);
            model.update_component<ComponentType, CacheType>(begin, end, sequence_idx);
        }...};
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            // skip if component does not exist
            if (auto const found = update_data.find(entry.name); found != update_data.cend()) {
                update[entry.index](*this, found->second, pos, sequence_idx_map[entry.index]);
            }
        }
    }

    // update all components
    template <class CacheType> void update_component(ConstDataset const& update_data, Idx pos = 0) {
        update_component<CacheType>(update_data, pos, get_sequence_idx_map(update_data));
    }

    template <typename CompType> void restore_component(SequenceIdx const& sequence_idx) {
        constexpr auto component_index = AllComponents::template index_of<CompType>();

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
    void get_indexer(std::string const& component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        using GetIndexerFunc = void (*)(MainModelState const& state, ID const* id_begin, Idx size, Idx* indexer_begin);

        // static function array
        static constexpr std::array<GetIndexerFunc, n_types> get_indexer_func{
            [](MainModelState const& state, ID const* id_begin_, Idx size_, Idx* indexer_begin_) {
                std::transform(id_begin_, id_begin_ + size_, indexer_begin_, [&state](ID id) {
                    return state.components.template get_idx_by_id<ComponentType>(id).pos;
                });
            }...};
        // search component type name
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            if (entry.name == component_type) {
                return get_indexer_func[entry.index](state_, id_begin, size, indexer_begin);
            }
        }
    }

    // get sequence idx map of a certain batch scenario
    SequenceIdx get_sequence_idx_map(ConstDataset const& update_data, Idx scenario_idx) const {
        using GetSeqIdxFunc =
            std::vector<Idx2D> (*)(MainModelState const& state, ConstDataPointer const& component_update, Idx pos);

        // function pointer array to get cached idx
        static constexpr std::array<GetSeqIdxFunc, n_types> get_seq_idx{
            [](MainModelState const& state, ConstDataPointer const& component_update, Idx pos) -> std::vector<Idx2D> {
                using UpdateType = typename ComponentType::UpdateType;

                auto const [it_begin, it_end] = component_update.template get_iterators<UpdateType>(pos);
                return main_core::get_component_sequence<ComponentType>(state, it_begin, it_end);
            }...};

        // fill in the map per component type
        SequenceIdx sequence_idx_map;
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            // skip if component does not exist
            if (auto const found = update_data.find(entry.name); found != update_data.cend()) {
                // add
                sequence_idx_map[entry.index] = get_seq_idx[entry.index](state_, found->second, scenario_idx);
            }
        }
        return sequence_idx_map;
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

    template <math_output_type MathOutputType, typename MathSolverType, typename YBus, typename InputType,
              typename PrepareInputFn, typename SolveFn>
        requires std::invocable<std::remove_cvref_t<PrepareInputFn>> &&
                 std::invocable<std::remove_cvref_t<SolveFn>, MathSolverType&, YBus const&, InputType const&> &&
                 std::same_as<std::invoke_result_t<PrepareInputFn>, std::vector<InputType>> &&
                 std::same_as<std::invoke_result_t<SolveFn, MathSolverType&, YBus const&, InputType const&>,
                              MathOutputType>
    std::vector<MathOutputType> calculate_(PrepareInputFn&& prepare_input, SolveFn&& solve) {
        using sym = typename MathOutputType::sym;

        assert(construction_complete_);
        calculation_info_ = CalculationInfo{};
        // prepare
        auto const& input = [this, &prepare_input] {
            Timer const timer(calculation_info_, 2100, "Prepare");
            prepare_solvers<sym>();
            return prepare_input();
        }();
        // calculate
        return [this, &input, &solve] {
            Timer const timer(calculation_info_, 2200, "Math Calculation");
            auto& solvers = get_solvers<sym>();
            auto& y_bus_vec = get_y_bus<sym>();
            std::vector<MathOutputType> math_output;
            math_output.reserve(n_math_solvers_);
            for (Idx i = 0; i != n_math_solvers_; ++i) {
                math_output.emplace_back(solve(solvers[i], y_bus_vec[i], input[i]));
            }
            return math_output;
        }();
    }

    template <symmetry_tag sym>
    std::vector<MathOutput<sym>> calculate_power_flow_(double err_tol, Idx max_iter,
                                                       CalculationMethod calculation_method) {
        return calculate_<MathOutput<sym>, MathSolver<sym>, YBus<sym>, PowerFlowInput<sym>>(
            [this] { return prepare_power_flow_input<sym>(); },
            [this, err_tol, max_iter, calculation_method](MathSolver<sym>& solver, YBus<sym> const& y_bus,
                                                          PowerFlowInput<sym> const& input) {
                return solver.run_power_flow(input, err_tol, max_iter, calculation_info_, calculation_method, y_bus);
            });
    }

    template <symmetry_tag sym>
    std::vector<MathOutput<sym>> calculate_state_estimation_(double err_tol, Idx max_iter,
                                                             CalculationMethod calculation_method) {
        return calculate_<MathOutput<sym>, MathSolver<sym>, YBus<sym>, StateEstimationInput<sym>>(
            [this] { return prepare_state_estimation_input<sym>(); },
            [this, err_tol, max_iter, calculation_method](MathSolver<sym>& solver, YBus<sym> const& y_bus,
                                                          StateEstimationInput<sym> const& input) {
                return solver.run_state_estimation(input, err_tol, max_iter, calculation_info_, calculation_method,
                                                   y_bus);
            });
    }

    template <symmetry_tag sym>
    std::vector<ShortCircuitMathOutput<sym>> calculate_short_circuit_(ShortCircuitVoltageScaling voltage_scaling,
                                                                      CalculationMethod calculation_method) {
        return calculate_<ShortCircuitMathOutput<sym>, MathSolver<sym>, YBus<sym>, ShortCircuitInput>(
            [this, voltage_scaling] { return prepare_short_circuit_input<sym>(voltage_scaling); },
            [this, calculation_method](MathSolver<sym>& solver, YBus<sym> const& y_bus,
                                       ShortCircuitInput const& input) {
                return solver.run_short_circuit(input, calculation_info_, calculation_method, y_bus);
            });
    }

    /*
    run the calculation function in batch on the provided update data.

    The calculation function should be able to run standalone.
    It should output to the provided result_data if the trailing argument is not ignore_output.

    threading
        < 0 sequential
        = 0 parallel, use number of hardware threads
        > 0 specify number of parallel threads
    raise a BatchCalculationError if any of the calculations in the batch raised an exception
    */
    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModelImpl&, Dataset const&, Idx>
    BatchParameter batch_calculation_(Calculate&& calculation_fn, Dataset const& result_data,
                                      ConstDataset const& update_data, Idx threading = -1) {
        // if the update batch is one empty map without any component
        // execute one power flow in the current instance, no batch calculation is needed
        // NOTE: if the map is not empty but the datasets inside are empty
        //     that will be considered as a zero batch_size
        if (update_data.empty()) {
            calculation_fn(*this, result_data, 0);
            return BatchParameter{};
        }

        // get batch size (can't be empty; see previous check)
        Idx const n_scenarios = update_data.cbegin()->second.batch_size();
        // assert if all component types have the same number of batches
        assert(std::all_of(update_data.cbegin(), update_data.cend(),
                           [n_scenarios](auto const& x) { return x.second.batch_size() == n_scenarios; }));

        // if the batch_size is zero, it is a special case without doing any calculations at all
        // we consider in this case the batch set is independent and but not topology cachable
        if (n_scenarios == 0) {
            return BatchParameter{};
        }

        // calculate once to cache topology, ignore results, all math solvers are initialized
        try {
            calculation_fn(*this, {}, ignore_output);
        } catch (const SparseMatrixError&) {
            // missing entries are provided in the update data
        }

        // error messages
        std::vector<std::string> exceptions(n_scenarios, "");
        std::vector<CalculationInfo> infos(n_scenarios);

        // lambda for sub batch calculation
        auto sub_batch = sub_batch_calculation_(calculation_fn, result_data, update_data, exceptions, infos);

        batch_dispatch(sub_batch, n_scenarios, threading);

        handle_batch_exceptions(exceptions);
        calculation_info_ = main_core::merge_calculation_info(infos);

        return BatchParameter{};
    }

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModelImpl&, Dataset const&, Idx>
    auto sub_batch_calculation_(Calculate&& calculation_fn, Dataset const& result_data, ConstDataset const& update_data,
                                std::vector<std::string>& exceptions, std::vector<CalculationInfo>& infos) {
        // const ref of current instance
        MainModelImpl const& base_model = *this;

        // cache component update order if possible
        bool const is_independent = MainModelImpl::is_update_independent(update_data);

        return [&base_model, &exceptions, &infos, &calculation_fn, &result_data, &update_data,
                is_independent](Idx start, Idx stride, Idx n_scenarios) {
            assert(n_scenarios <= narrow_cast<Idx>(exceptions.size()));
            assert(n_scenarios <= narrow_cast<Idx>(infos.size()));

            Timer const t_total(infos[start], 0000, "Total in thread");

            auto copy_model = [&base_model, &infos](Idx scenario_idx) {
                Timer const t_copy_model(infos[scenario_idx], 1100, "Copy model");
                return MainModelImpl{base_model};
            };
            auto model = copy_model(start);

            SequenceIdx scenario_sequence = is_independent ? model.get_sequence_idx_map(update_data) : SequenceIdx{};

            auto [setup, winddown] =
                scenario_update_restore(model, update_data, is_independent, scenario_sequence, infos);

            auto calculate_scenario = MainModelImpl::call_with<Idx>(
                [&model, &calculation_fn, &result_data, &infos](Idx scenario_idx) {
                    calculation_fn(model, result_data, scenario_idx);
                    infos[scenario_idx].merge(model.calculation_info_);
                },
                std::move(setup), std::move(winddown), scenario_exception_handler(model, exceptions, infos),
                [&model, &copy_model](Idx scenario_idx) { model = copy_model(scenario_idx); });

            for (Idx scenario_idx = start; scenario_idx < n_scenarios; scenario_idx += stride) {
                Timer const t_total_single(infos[scenario_idx], 0100, "Total single calculation in thread");

                calculate_scenario(scenario_idx);
            }
        };
    }

    // run sequential if
    //    specified threading < 0
    //    use hardware threads, but it is either unknown (0) or only has one thread (1)
    //    specified threading = 1
    template <typename RunSubBatchFn>
        requires std::invocable<std::remove_cvref_t<RunSubBatchFn>, Idx /*start*/, Idx /*stride*/, Idx /*n_scenarios*/>
    static void batch_dispatch(RunSubBatchFn sub_batch, Idx n_scenarios, Idx threading) {
        // run batches sequential or parallel
        auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
        if (threading < 0 || threading == 1 || (threading == 0 && hardware_thread < 2)) {
            // run all in sequential
            sub_batch(0, 1, n_scenarios);
        } else {
            // create parallel threads
            Idx const n_thread = std::min(threading == 0 ? hardware_thread : threading, n_scenarios);
            std::vector<std::thread> threads;
            threads.reserve(n_thread);
            for (Idx thread_number = 0; thread_number < n_thread; ++thread_number) {
                // compute each sub batch with stride
                threads.emplace_back(sub_batch, thread_number, n_thread, n_scenarios);
            }
            for (auto& thread : threads) {
                thread.join();
            }
        }
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
                                        bool const is_independent, SequenceIdx& scenario_sequence,
                                        std::vector<CalculationInfo>& infos) {
        return std::make_pair(
            [&model, &update_data, &scenario_sequence, is_independent, &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1200, "Update model");
                if (!is_independent) {
                    scenario_sequence = model.get_sequence_idx_map(update_data, scenario_idx);
                }
                model.template update_component<cached_update_t>(update_data, scenario_idx, scenario_sequence);
            },
            [&model, &scenario_sequence, is_independent, &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1201, "Restore model");
                model.restore_components(scenario_sequence);
                if (!is_independent) {
                    std::ranges::for_each(scenario_sequence, [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
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
        // check all components
        return std::ranges::all_of(AllComponents::component_index_map, [&update_data](ComponentEntry const& entry) {
            static constexpr std::array check_component_update_independent{
                &is_component_update_independent<ComponentType>...};
            auto const found = update_data.find(entry.name);
            // return true if this component update does not exist
            if (found == update_data.cend()) {
                return true;
            }
            // check for this component update
            return check_component_update_independent[entry.index](found->second);
        });
    }

    template <class Component> static bool is_component_update_independent(ConstDataPointer const& component_update) {
        // If the batch size is (0 or) 1, then the update data for this component is 'independent'
        if (component_update.batch_size() <= 1) {
            return true;
        }

        // Remember the first batch size, then loop over the remaining batches and check if they are of the same length
        Idx const elements_per_scenario = component_update.elements_per_scenario(0);
        for (Idx batch = 1; batch != component_update.batch_size(); ++batch) {
            if (elements_per_scenario != component_update.elements_per_scenario(batch)) {
                return false;
            }
        }

        // Remember the begin iterator of the first batch, then loop over the remaining batches and check the ids
        UpdateType<Component> const* it_first_begin =
            component_update.template get_iterators<UpdateType<Component>>(0).first;
        // check the subsequent batches
        // only return true if all batches match the ids of the first batch
        return std::all_of(
            IdxCount{1}, IdxCount{component_update.batch_size()}, [it_first_begin, &component_update](Idx batch) {
                auto const [it_begin, it_end] = component_update.template get_iterators<UpdateType<Component>>(batch);
                return std::equal(it_begin, it_end, it_first_begin,
                                  [](UpdateType<Component> const& obj, UpdateType<Component> const& first) {
                                      return obj.id == first.id;
                                  });
            });
    }

    // Single load flow calculation, returning math output results
    template <symmetry_tag sym>
    std::vector<MathOutput<sym>> calculate_power_flow(double err_tol, Idx max_iter,
                                                      CalculationMethod calculation_method) {
        return calculate_power_flow_<sym>(err_tol, max_iter, calculation_method);
    }

    // Single load flow calculation, propagating the results to result_data
    template <symmetry_tag sym>
    void calculate_power_flow(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                              Dataset const& result_data, Idx pos = 0) {
        assert(construction_complete_);
        auto const math_output = calculate_power_flow_<sym>(err_tol, max_iter, calculation_method);
        if (pos != ignore_output) {
            output_result(math_output, result_data, pos);
        }
    }

    // Batch load flow calculation, propagating the results to result_data
    template <symmetry_tag sym>
    BatchParameter calculate_power_flow(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                        Dataset const& result_data, ConstDataset const& update_data,
                                        Idx threading = -1) {
        return batch_calculation_(
            [err_tol, max_iter, calculation_method](MainModelImpl& model, Dataset const& target_data, Idx pos) {
                auto const err_tol_ = pos != ignore_output ? err_tol : std::numeric_limits<double>::max();
                auto const max_iter_ = pos != ignore_output ? max_iter : 1;

                model.calculate_power_flow<sym>(err_tol_, max_iter_, calculation_method, target_data, pos);
            },
            result_data, update_data, threading);
    }

    // Single state estimation calculation, returning math output results
    template <symmetry_tag sym>
    std::vector<MathOutput<sym>> calculate_state_estimation(double err_tol, Idx max_iter,
                                                            CalculationMethod calculation_method) {
        return calculate_state_estimation_<sym>(err_tol, max_iter, calculation_method);
    }

    // Single state estimation calculation, propagating the results to result_data
    template <symmetry_tag sym>
    void calculate_state_estimation(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                    Dataset const& result_data, Idx pos = 0) {
        assert(construction_complete_);
        auto const math_output = calculate_state_estimation_<sym>(err_tol, max_iter, calculation_method);
        output_result(math_output, result_data, pos);
    }

    // Batch state estimation calculation, propagating the results to result_data
    template <symmetry_tag sym>
    BatchParameter calculate_state_estimation(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                              Dataset const& result_data, ConstDataset const& update_data,
                                              Idx threading = -1) {
        return batch_calculation_(
            [err_tol, max_iter, calculation_method](MainModelImpl& model, Dataset const& target_data, Idx pos) {
                auto const err_tol_ = pos != ignore_output ? err_tol : std::numeric_limits<double>::max();
                auto const max_iter_ = pos != ignore_output ? max_iter : 1;

                model.calculate_state_estimation<sym>(err_tol_, max_iter_, calculation_method, target_data, pos);
            },
            result_data, update_data, threading);
    }

    // Single short circuit calculation, returning short circuit math output results
    template <symmetry_tag sym>
    std::vector<ShortCircuitMathOutput<sym>> calculate_short_circuit(ShortCircuitVoltageScaling voltage_scaling,
                                                                     CalculationMethod calculation_method) {
        return calculate_short_circuit_<sym>(voltage_scaling, calculation_method);
    }

    // Single short circuit calculation, propagating the results to result_data
    void calculate_short_circuit(ShortCircuitVoltageScaling voltage_scaling, CalculationMethod calculation_method,
                                 Dataset const& result_data, Idx pos = 0) {
        assert(construction_complete_);
        if (std::all_of(state_.components.template citer<Fault>().begin(),
                        state_.components.template citer<Fault>().end(),
                        [](Fault const& fault) { return fault.get_fault_type() == FaultType::three_phase; })) {
            auto const math_output = calculate_short_circuit_<symmetric_t>(voltage_scaling, calculation_method);
            output_result(math_output, result_data, pos);
        } else {
            auto const math_output = calculate_short_circuit_<asymmetric_t>(voltage_scaling, calculation_method);
            output_result(math_output, result_data, pos);
        }
    }

    // Batch short circuit calculation, propagating the results to result_data
    BatchParameter calculate_short_circuit(ShortCircuitVoltageScaling voltage_scaling,
                                           CalculationMethod calculation_method, Dataset const& result_data,
                                           ConstDataset const& update_data, Idx threading = -1) {
        return batch_calculation_(
            [voltage_scaling, calculation_method](MainModelImpl& model, Dataset const& target_data, Idx pos) {
                if (pos != ignore_output) {
                    model.calculate_short_circuit(voltage_scaling, calculation_method, target_data, pos);
                }
            },
            result_data, update_data, threading);
    }

    template <typename Component, math_output_type MathOutputType, std::forward_iterator ResIt>
    ResIt output_result(std::vector<MathOutputType> const& math_output, ResIt res_it) const {
        assert(construction_complete_);
        return main_core::output_result<Component, ComponentContainer>(state_, math_output, res_it);
    }

    template <math_output_type MathOutputType>
    void output_result(std::vector<MathOutputType> const& math_output, Dataset const& result_data, Idx pos = 0) {
        using OutputFunc = void (*)(MainModelImpl & x, std::vector<MathOutputType> const& math_output,
                                    MutableDataPointer const& data_ptr, Idx position);

        static constexpr std::array<OutputFunc, n_types> get_result{[](MainModelImpl& model,
                                                                       std::vector<MathOutputType> const& math_output_,
                                                                       MutableDataPointer const& data_ptr,
                                                                       Idx position) {
            auto const begin =
                data_ptr
                    .get_iterators<
                        std::conditional_t<steady_state_math_output_type<MathOutputType>,
                                           typename ComponentType::template OutputType<typename MathOutputType::sym>,
                                           typename ComponentType::ShortCircuitOutputType>>(position)
                    .first;
            model.output_result<ComponentType>(math_output_, begin);
        }...};

        Timer const t_output(calculation_info_, 3000, "Produce output");
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            auto const found = result_data.find(entry.name);
            // skip if component does not exist
            if (found == result_data.cend()) {
                continue;
            }
            // update
            get_result[entry.index](*this, math_output, found->second, pos);
        }
    }

    CalculationInfo calculation_info() const { return calculation_info_; }

  private:
    CalculationInfo calculation_info_; // needs to be first due to padding override

    double system_frequency_;

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
                        state.topo_comp_coup->branch[state.components.template get_seq<Branch>(changed_component_idx)];
                    if (math_idx.group == -1) {
                        return;
                    }
                    // assign parameters
                    increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos);
                } else if constexpr (std::derived_from<ComponentType, Branch3>) {
                    Idx2DBranch3 const math_idx =
                        state.topo_comp_coup
                            ->branch3[state.components.template get_seq<Branch3>(changed_component_idx)];
                    if (math_idx.group == -1) {
                        return;
                    }
                    // assign parameters, branch3 param consists of three branch parameters
                    // auto const branch3_param =
                    //   state.components.template get_item<Branch3>(changed_component_idx).template calc_param<sym>();
                    for (size_t branch2 = 0; branch2 < 3; ++branch2) {
                        increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos[branch2]);
                    }
                } else if constexpr (std::same_as<ComponentType, Shunt>) {
                    Idx2D const math_idx =
                        state.topo_comp_coup->shunt[state.components.template get_seq<Shunt>(changed_component_idx)];
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

    /** This is a heavily templated member function because it operates on many different variables of many different
     * types, but the essence is ever the same: filling one member (vector) of the calculation calc_input struct
     * (soa) with the right calculation symmetric or asymmetric calculation parameters, in the same order as the
     * corresponding component are stored in the component topology. There is one such struct for each sub graph / math
     * model and all of them are filled within the same function call (i.e. Notice that calc_input is a vector).
     *
     *  1. For each component, check if it should be included.
     *     By default, all component are included, except for some cases, like power sensors. For power sensors, the
     *     list of component contains all power sensors, but the preparation should only be done for one type of power
     *     sensors at a time. Therefore, `included` will be a lambda function, such as:
     *
     *       [this](Idx i) { return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::source; }
     *
     *  2. Find the original component in the topology and retrieve its calculation parameters.
     *
     *  3. Fill the calculation parameters of the right math model.
     *
     *  @tparam CalcStructOut
     *      The calculation input type (soa) for the desired calculation (e.g. PowerFlowInput<sym> or
     *StateEstimationInput<sym>).
     *
     * @tparam CalcParamOut
     *      The data type for the desired calculation for the given ComponentIn (e.g. SourceCalcParam<sym> or
     *      VoltageSensorCalcParam<sym>).
     *
     * @tparam comp_vect
     *      The (pre-allocated and resized) vector of CalcParamOuts which shall be filled with component specific
     *      calculation parameters. Note that this is a pointer to member
     *
     * @tparam ComponentIn
     * 	    The component type for which we are collecting calculation parameters
     *
     * @tparam PredicateIn
     * 	    The lambda function type. The actual type depends on the captured variables, and will be automatically
     * 	    deduced.
     *
     * @param component[in]
     *      The vector of component math indices to consider (e.g. state_.topo_comp_coup->source).
     *      When idx.group = -1, the original component is not assigned to a math model, so we can skip it.
     *
     * @param calc_input[out]
     *		Although this variable is called `input`, it is actually the output of this function, it stored the
     *		calculation parameters for each math model, for each component of type ComponentIn.
     *
     * @param include
     *      A lambda function (Idx i -> bool) which returns true if the component at Idx i should be included.
     * 	    The default lambda `include_all` always returns `true`.
     */
    template <calculation_input_type CalcStructOut, typename CalcParamOut,
              std::vector<CalcParamOut>(CalcStructOut::*comp_vect), class ComponentIn,
              std::invocable<Idx> PredicateIn = decltype(include_all)>
        requires std::convertible_to<std::invoke_result_t<PredicateIn, Idx>, bool>
    void prepare_input(std::vector<Idx2D> const& components, std::vector<CalcStructOut>& calc_input,
                       PredicateIn include = include_all) {
        for (Idx i = 0, n = (Idx)components.size(); i != n; ++i) {
            if (include(i)) {
                Idx2D const math_idx = components[i];
                if (math_idx.group != -1) {
                    auto const& component = state_.components.template get_item_by_seq<ComponentIn>(i);
                    CalcStructOut& math_model_input = calc_input[math_idx.group];
                    std::vector<CalcParamOut>& math_model_input_vect = math_model_input.*comp_vect;
                    math_model_input_vect[math_idx.pos] = calculate_param<CalcStructOut>(component);
                }
            }
        }
    }

    template <calculation_input_type CalcStructOut, typename CalcParamOut,
              std::vector<CalcParamOut>(CalcStructOut::*comp_vect), class ComponentIn,
              std::invocable<Idx> PredicateIn = decltype(include_all)>
        requires std::convertible_to<std::invoke_result_t<PredicateIn, Idx>, bool>
    void prepare_input(std::vector<Idx2D> const& components, std::vector<CalcStructOut>& calc_input,
                       std::invocable<ComponentIn const&> auto extra_args, PredicateIn include = include_all) {
        for (Idx i = 0, n = (Idx)components.size(); i != n; ++i) {
            if (include(i)) {
                Idx2D const math_idx = components[i];
                if (math_idx.group != -1) {
                    auto const& component = state_.components.template get_item_by_seq<ComponentIn>(i);
                    CalcStructOut& math_model_input = calc_input[math_idx.group];
                    std::vector<CalcParamOut>& math_model_input_vect = math_model_input.*comp_vect;
                    math_model_input_vect[math_idx.pos] =
                        calculate_param<CalcStructOut>(component, extra_args(component));
                }
            }
        }
    }

    template <calculation_input_type CalcInputType>
    auto calculate_param(auto const& c, auto const&... extra_args)
        requires requires {
                     { c.calc_param(extra_args...) };
                 }
    {
        return c.calc_param(extra_args...);
    }

    template <calculation_input_type CalcInputType>
    auto calculate_param(auto const& c, auto const&... extra_args)
        requires requires {
                     { c.template calc_param<typename CalcInputType::sym>(extra_args...) };
                 }
    {
        return c.template calc_param<typename CalcInputType::sym>(extra_args...);
    }

    template <symmetry_tag sym, IntSVector(StateEstimationInput<sym>::*component), class Component>
    void prepare_input_status(std::vector<Idx2D> const& objects, std::vector<StateEstimationInput<sym>>& input) {
        for (Idx i = 0, n = (Idx)objects.size(); i != n; ++i) {
            Idx2D const math_idx = objects[i];
            if (math_idx.group == -1) {
                continue;
            }
            (input[math_idx.group].*component)[math_idx.pos] =
                state_.components.template get_item_by_seq<Component>(i).status();
        }
    }

    template <symmetry_tag sym> std::vector<PowerFlowInput<sym>> prepare_power_flow_input() {
        assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
        std::vector<PowerFlowInput<sym>> pf_input(n_math_solvers_);
        for (Idx i = 0; i != n_math_solvers_; ++i) {
            pf_input[i].s_injection.resize(state_.math_topology[i]->n_load_gen());
            pf_input[i].source.resize(state_.math_topology[i]->n_source());
        }
        prepare_input<PowerFlowInput<sym>, DoubleComplex, &PowerFlowInput<sym>::source, Source>(
            state_.topo_comp_coup->source, pf_input);

        prepare_input<PowerFlowInput<sym>, ComplexValue<sym>, &PowerFlowInput<sym>::s_injection, GenericLoadGen>(
            state_.topo_comp_coup->load_gen, pf_input);

        return pf_input;
    }

    template <symmetry_tag sym> std::vector<StateEstimationInput<sym>> prepare_state_estimation_input() {
        assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());

        std::vector<StateEstimationInput<sym>> se_input(n_math_solvers_);

        for (Idx i = 0; i != n_math_solvers_; ++i) {
            se_input[i].shunt_status.resize(state_.math_topology[i]->n_shunt());
            se_input[i].load_gen_status.resize(state_.math_topology[i]->n_load_gen());
            se_input[i].source_status.resize(state_.math_topology[i]->n_source());
            se_input[i].measured_voltage.resize(state_.math_topology[i]->n_voltage_sensor());
            se_input[i].measured_source_power.resize(state_.math_topology[i]->n_source_power_sensor());
            se_input[i].measured_load_gen_power.resize(state_.math_topology[i]->n_load_gen_power_sensor());
            se_input[i].measured_shunt_power.resize(state_.math_topology[i]->n_shunt_power_power_sensor());
            se_input[i].measured_branch_from_power.resize(state_.math_topology[i]->n_branch_from_power_sensor());
            se_input[i].measured_branch_to_power.resize(state_.math_topology[i]->n_branch_to_power_sensor());
            se_input[i].measured_bus_injection.resize(state_.math_topology[i]->n_bus_power_sensor());
        }

        prepare_input_status<sym, &StateEstimationInput<sym>::shunt_status, Shunt>(state_.topo_comp_coup->shunt,
                                                                                   se_input);
        prepare_input_status<sym, &StateEstimationInput<sym>::load_gen_status, GenericLoadGen>(
            state_.topo_comp_coup->load_gen, se_input);
        prepare_input_status<sym, &StateEstimationInput<sym>::source_status, Source>(state_.topo_comp_coup->source,
                                                                                     se_input);

        prepare_input<StateEstimationInput<sym>, VoltageSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_voltage, GenericVoltageSensor>(
            state_.topo_comp_coup->voltage_sensor, se_input);
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_source_power, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input,
            [this](Idx i) { return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::source; });
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_load_gen_power, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input, [this](Idx i) {
                return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::load ||
                       state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::generator;
            });
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_shunt_power, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input,
            [this](Idx i) { return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::shunt; });
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_branch_from_power, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input, [this](Idx i) {
                return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch_from ||
                       // all branch3 sensors are at from side in the mathematical model
                       state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_1 ||
                       state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_2 ||
                       state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_3;
            });
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_branch_to_power, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input, [this](Idx i) {
                return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch_to;
            });
        prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_bus_injection, GenericPowerSensor>(
            state_.topo_comp_coup->power_sensor, se_input,
            [this](Idx i) { return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::node; });

        return se_input;
    }

    template <symmetry_tag sym>
    std::vector<ShortCircuitInput> prepare_short_circuit_input(ShortCircuitVoltageScaling voltage_scaling) {
        assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());

        std::vector<IdxVector> topo_fault_indices(state_.math_topology.size());
        std::vector<IdxVector> topo_bus_indices(state_.math_topology.size());

        for (Idx fault_idx{0}; fault_idx < state_.components.template size<Fault>(); ++fault_idx) {
            auto const& fault = state_.components.template get_item_by_seq<Fault>(fault_idx);
            if (fault.status()) {
                auto const node_idx = state_.components.template get_seq<Node>(fault.get_fault_object());
                auto const topo_bus_idx = state_.topo_comp_coup->node[node_idx];

                if (topo_bus_idx.group >= 0) { // Consider non-isolated objects only
                    topo_fault_indices[topo_bus_idx.group].push_back(fault_idx);
                    topo_bus_indices[topo_bus_idx.group].push_back(topo_bus_idx.pos);
                }
            }
        }

        auto fault_coup = std::vector<Idx2D>(state_.components.template size<Fault>(), Idx2D{-1, -1});

        std::vector<ShortCircuitInput> sc_input(n_math_solvers_);
        for (Idx i = 0; i != n_math_solvers_; ++i) {
            auto map = build_sparse_mapping(topo_bus_indices[i], state_.math_topology[i]->n_bus());

            for (Idx reordered_idx{0}; reordered_idx < static_cast<Idx>(map.reorder.size()); ++reordered_idx) {
                fault_coup[topo_fault_indices[i][map.reorder[reordered_idx]]] = Idx2D{i, reordered_idx};
            }

            sc_input[i].fault_buses = {from_sparse, std::move(map.indptr)};
            sc_input[i].faults.resize(state_.components.template size<Fault>());
            sc_input[i].source.resize(state_.math_topology[i]->n_source());
        }

        state_.comp_coup = ComponentToMathCoupling{.fault = std::move(fault_coup)};

        prepare_input<ShortCircuitInput, FaultCalcParam, &ShortCircuitInput::faults, Fault>(
            state_.comp_coup.fault, sc_input, [this](Fault const& fault) {
                return state_.components.template get_item<Node>(fault.get_fault_object()).u_rated();
            });
        prepare_input<ShortCircuitInput, DoubleComplex, &ShortCircuitInput::source, Source>(
            state_.topo_comp_coup->source, sc_input, [this, voltage_scaling](Source const& source) {
                return std::pair{state_.components.template get_item<Node>(source.node()).u_rated(), voltage_scaling};
            });

        return sc_input;
    }

    template <symmetry_tag sym> void prepare_y_bus() {
        std::vector<YBus<sym>>& y_bus_vec = get_y_bus<sym>();
        // also get the vector of other Y_bus (sym -> asym, or asym -> sym)
        std::vector<YBus<other_symmetry_t<sym>>>& other_y_bus_vec = get_y_bus<other_symmetry_t<sym>>();
        // If no Ybus exists, build them
        if (y_bus_vec.empty()) {
            bool const other_y_bus_exist = (!other_y_bus_vec.empty());
            y_bus_vec.reserve(n_math_solvers_);
            auto math_params = get_math_param<sym>();

            // Check the branch and shunt indices
            constexpr auto branch_param_in_seq_map =
                std::array{AllComponents::template index_of<Line>(), AllComponents::template index_of<Link>(),
                           AllComponents::template index_of<Transformer>()};
            constexpr auto shunt_param_in_seq_map = std::array{AllComponents::template index_of<Shunt>()};

            for (Idx i = 0; i != n_math_solvers_; ++i) {
                // construct from existing Y_bus structure if possible
                if (other_y_bus_exist) {
                    y_bus_vec.emplace_back(state_.math_topology[i],
                                           std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])),
                                           other_y_bus_vec[i].get_y_bus_structure());
                } else {
                    y_bus_vec.emplace_back(state_.math_topology[i],
                                           std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])));
                }

                y_bus_vec.back().set_branch_param_idx(
                    IdxVector{branch_param_in_seq_map.begin(), branch_param_in_seq_map.end()});
                y_bus_vec.back().set_shunt_param_idx(
                    IdxVector{shunt_param_in_seq_map.begin(), shunt_param_in_seq_map.end()});
            }
        }
    }

    template <symmetry_tag sym> void prepare_solvers() {
        std::vector<MathSolver<sym>>& solvers = get_solvers<sym>();
        // rebuild topology if needed
        if (!is_topology_up_to_date_) {
            rebuild_topology();
        }
        prepare_y_bus<sym>();

        if (n_math_solvers_ != static_cast<Idx>(solvers.size())) {
            assert(solvers.empty());
            assert(n_math_solvers_ == static_cast<Idx>(state_.math_topology.size()));
            assert(n_math_solvers_ == static_cast<Idx>(get_y_bus<sym>().size()));

            solvers.clear();
            solvers.reserve(n_math_solvers_);
            std::ranges::transform(state_.math_topology, std::back_inserter(solvers),
                                   [](auto math_topo) { return MathSolver<sym>{std::move(math_topo)}; });
            for (Idx idx = 0; idx < n_math_solvers_; ++idx) {
                get_y_bus<sym>()[idx].register_parameters_changed_callback(
                    [solver = std::ref(solvers[idx])](bool changed) { solver.get().parameters_changed(changed); });
            }
        } else if (!is_parameter_up_to_date<sym>()) {
            std::vector<MathModelParam<sym>> const math_params = get_math_param<sym>();
            std::vector<MathModelParamIncrement> const math_param_increments = get_math_param_increment<sym>();
            if (last_updated_calculation_symmetry_mode_ == is_symmetric_v<sym>) {
                main_core::update_y_bus(math_state_, math_params, math_param_increments);
            } else {
                main_core::update_y_bus(math_state_, math_params);
            }
        }
        // else do nothing, set everything up to date
        is_parameter_up_to_date<sym>() = true;
        std::ranges::for_each(parameter_changed_components_, [](auto& comps) { comps.clear(); });
        last_updated_calculation_symmetry_mode_ = is_symmetric_v<sym>;
    }
};

using MainModel =
    MainModelImpl<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, GenericLoadGen, GenericLoad,
                                        GenericGenerator, GenericPowerSensor, GenericVoltageSensor>,
                  AllComponents>;

} // namespace power_grid_model
