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
#include "math_solver/math_solver_dispatch.hpp"

#include "optimizer/optimizer.hpp"

// main model implementation
#include "main_core/calculation_input_preparation.hpp"
#include "main_core/input.hpp"
#include "main_core/main_model_type.hpp"
#include "main_core/math_state.hpp"
#include "main_core/output.hpp"
#include "main_core/topology.hpp"
#include "main_core/update.hpp"
#include "main_core/y_bus.hpp"

// stl library
#include <memory>
#include <span>

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

template <class ModelType>
    requires(main_core::is_main_model_type_v<ModelType>)
class MainModelImpl {

  private:
    // internal type traits
    using ComponentContainer = typename ModelType::ComponentContainer;
    using MainModelState = typename ModelType::MainModelState;

    using SequenceIdx = typename ModelType::SequenceIdx;
    using SequenceIdxRefWrappers = ModelType::SequenceIdxRefWrappers;
    using SequenceIdxView = typename ModelType::SequenceIdxView;
    using OwnedUpdateDataset = typename ModelType::OwnedUpdateDataset;
    using ComponentFlags = typename ModelType::ComponentFlags;

    static constexpr Idx isolated_component{main_core::isolated_component};
    static constexpr Idx not_connected{main_core::not_connected};
    static constexpr Idx sequential{main_core::utils::sequential};

  public:
    using ImplType = ModelType;
    using Options = MainModelOptions;
    using MathState = main_core::MathState;
    using MetaData = meta_data::MetaData;

    // constructor with data
    explicit MainModelImpl(double system_frequency, ConstDataset const& input_data,
                           MathSolverDispatcher const& math_solver_dispatcher, Idx pos = 0)
        : system_frequency_{system_frequency},
          meta_data_{&input_data.meta_data()},
          math_solver_dispatcher_{&math_solver_dispatcher} {
        assert(input_data.get_description().dataset->name == std::string_view("input"));
        add_components(input_data, pos);
        set_construction_complete();
    }

    // constructor with only frequency
    explicit MainModelImpl(double system_frequency, meta_data::MetaData const& meta_data,
                           MathSolverDispatcher const& math_solver_dispatcher)
        : system_frequency_{system_frequency},
          meta_data_{&meta_data},
          math_solver_dispatcher_{&math_solver_dispatcher} {}

    // helper function to get what components are present in the update data
    ComponentFlags get_components_to_update(ConstDataset const& update_data) const {
        return ModelType::run_functor_with_all_component_types_return_array([&update_data]<typename CompType>() {
            return (update_data.find_component(CompType::name, false) != main_core::utils::invalid_index);
        });
    }

  private:
    // template to construct components
    // using forward interators
    // different selection based on component type
    template <std::derived_from<Base> CompType, std::ranges::viewable_range Inputs>
    void add_component(Inputs&& components) {
        assert(!construction_complete_);
        main_core::add_component<CompType>(state_.components, std::forward<Inputs>(components), system_frequency_);
    }

    void add_components(ConstDataset const& input_data, Idx pos = 0) {
        auto const add_func = [this, pos, &input_data]<typename CT>() {
            if (input_data.is_columnar(CT::name)) {
                this->add_component<CT>(input_data.get_columnar_buffer_span<meta_data::input_getter_s, CT>(pos));
            } else {
                this->add_component<CT>(input_data.get_buffer_span<meta_data::input_getter_s, CT>(pos));
            }
        };
        ModelType::run_functor_with_all_component_types_return_void(add_func);
    }

    // template to update components
    // using forward interators
    // different selection based on component type
    // if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
    template <class CompType, cache_type_c CacheType, std::ranges::viewable_range Updates>
    void update_component(Updates&& updates, std::span<Idx2D const> sequence_idx) {
        constexpr auto comp_index = ModelType::template index_of_component<CompType>;

        assert(construction_complete_);
        assert(std::ranges::ssize(sequence_idx) == std::ranges::ssize(updates));

        if constexpr (CacheType::value) {
            main_core::update::update_inverse<CompType>(
                state_.components, updates, std::back_inserter(std::get<comp_index>(cached_inverse_update_)),
                sequence_idx);
        }

        UpdateChange const changed = main_core::update::update_component<CompType>(
            state_.components, std::forward<Updates>(updates),
            std::back_inserter(std::get<comp_index>(parameter_changed_components_)), sequence_idx);

        // update, get changed variable
        update_state(changed);
        if constexpr (CacheType::value) {
            cached_state_changes_ = cached_state_changes_ || changed;
        }
    }

    // entry point overload to update one row or column based component type
    template <class CompType, cache_type_c CacheType>
    void update_component_row_col(ConstDataset const& update_data, Idx pos, std::span<Idx2D const> sequence_idx) {
        assert(construction_complete_);
        assert(update_data.get_description().dataset->name == std::string_view("update"));

        if (update_data.is_columnar(CompType::name)) {
            this->update_component<CompType, CacheType>(
                update_data.get_columnar_buffer_span<meta_data::update_getter_s, CompType>(pos), sequence_idx);
        } else {
            this->update_component<CompType, CacheType>(
                update_data.get_buffer_span<meta_data::update_getter_s, CompType>(pos), sequence_idx);
        }
    }

  public:
    // overload to update all components across all scenarios
    template <cache_type_c CacheType, typename SequenceIdxMap>
        requires(std::same_as<SequenceIdxMap, SequenceIdx> || std::same_as<SequenceIdxMap, SequenceIdxView>)
    void update_components(ConstDataset const& update_data, Idx pos, SequenceIdxMap const& sequence_idx_map) {
        ModelType::run_functor_with_all_component_types_return_void(
            [this, pos, &update_data, &sequence_idx_map]<typename CT>() {
                this->update_component_row_col<CT, CacheType>(
                    update_data, pos, std::get<ModelType::template index_of_component<CT>>(sequence_idx_map));
            });
    }

    // overload to update all components in the first scenario (e.g. permanent update)
    template <cache_type_c CacheType> void update_components(ConstDataset const& update_data) {
        auto const components_to_update = get_components_to_update(update_data);
        auto const update_independence =
            main_core::update::independence::check_update_independence<ModelType>(state_.components, update_data);
        auto const sequence_idx_map = main_core::update::get_all_sequence_idx_map<ModelType>(
            state_.components, update_data, 0, components_to_update, update_independence, false);
        update_components<CacheType>(update_data, 0, sequence_idx_map);
    }

  private:
    // set complete construction
    // initialize internal arrays
    void set_construction_complete() {
        assert(!construction_complete_);
#ifndef NDEBUG
        // set construction_complete for debug assertions
        construction_complete_ = true;
#endif // !NDEBUG
        state_.components.set_construction_complete();
        state_.comp_topo =
            std::make_shared<ComponentTopology const>(main_core::construct_topology<ModelType>(state_.components));
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

  public:
    /*
    the the sequence indexer given an input array of ID's for a given component type
    */
    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const {
        auto const get_index_func = [&components = this->state_.components, component_type, id_begin, size,
                                     indexer_begin]<typename CT>() {
            if (component_type == CT::name) {
                std::transform(id_begin, id_begin + size, indexer_begin, [&components](ID id) {
                    return main_core::get_component_idx_by_id<CT>(components, id).pos;
                });
            }
        };
        ModelType::run_functor_with_all_component_types_return_void(get_index_func);
    }

  private:
    // Entry point for main_model.hpp
    SequenceIdx get_all_sequence_idx_map(ConstDataset const& update_data) {
        auto const components_to_update = get_components_to_update(update_data);
        auto const update_independence =
            main_core::update::independence::check_update_independence<ModelType>(state_.components, update_data);
        return main_core::update::get_all_sequence_idx_map<ModelType>(state_.components, update_data, 0,
                                                                      components_to_update, update_independence, false);
    }

    void update_state(UpdateChange const& changes) {
        // if topology changed, everything is not up to date
        // if only param changed, set param to not up to date
        is_topology_up_to_date_ = is_topology_up_to_date_ && !changes.topo;
        is_sym_parameter_up_to_date_ = is_sym_parameter_up_to_date_ && !changes.topo && !changes.param;
        is_asym_parameter_up_to_date_ = is_asym_parameter_up_to_date_ && !changes.topo && !changes.param;
    }

    template <typename CompType> void restore_component(SequenceIdxView const& sequence_idx) {
        constexpr auto component_index = ModelType::template index_of_component<CompType>;

        auto& cached_inverse_update = std::get<component_index>(cached_inverse_update_);
        auto const& component_sequence = std::get<component_index>(sequence_idx);

        if (!cached_inverse_update.empty()) {
            update_component<CompType, permanent_update_t>(cached_inverse_update, component_sequence);
            cached_inverse_update.clear();
        }
    }

  public:
    // restore the initial values of all components
    void restore_components(SequenceIdxView const& sequence_idx) {
        ModelType::run_functor_with_all_component_types_return_void(
            [this, &sequence_idx]<typename CompType>() { this->restore_component<CompType>(sequence_idx); });

        update_state(cached_state_changes_);
        cached_state_changes_ = {};
    }

  private:
    void restore_components(SequenceIdxRefWrappers const& sequence_idx) {
        ModelType::run_functor_with_all_component_types_return_void([this, &sequence_idx]<typename CompType>() {
            this->restore_component<CompType>(std::array{std::span<Idx2D const>{
                std::get<ModelType::template index_of_component<CompType>>(sequence_idx).get()}});
        });
    }

    void restore_components(ModelType::SequenceIdx const& sequence_idx) {
        ModelType::run_functor_with_all_component_types_return_void([this, &sequence_idx]<typename CompType>() {
            this->restore_component<CompType>(std::array{
                std::span<Idx2D const>{std::get<ModelType::template index_of_component<CompType>>(sequence_idx)}});
        });
    }

    template <solver_output_type SolverOutputType, typename MathSolverType, typename YBus, typename InputType,
              typename PrepareInputFn, typename SolveFn>
        requires std::invocable<std::remove_cvref_t<PrepareInputFn>, Idx /*n_math_solvers*/> &&
                 std::invocable<std::remove_cvref_t<SolveFn>, MathSolverType&, YBus const&, InputType const&> &&
                 std::same_as<std::invoke_result_t<PrepareInputFn, Idx /*n_math_solvers*/>, std::vector<InputType>> &&
                 std::same_as<std::invoke_result_t<SolveFn, MathSolverType&, YBus const&, InputType const&>,
                              SolverOutputType>
    std::vector<SolverOutputType> calculate_(PrepareInputFn&& prepare_input, SolveFn&& solve, Logger& logger) {
        using sym = typename SolverOutputType::sym;

        assert(construction_complete_);
        // prepare
        auto const& input = [this, &logger, prepare_input_ = std::forward<PrepareInputFn>(prepare_input)] {
            Timer const timer{logger, LogEvent::prepare};
            prepare_solvers<sym>();
            assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
            return prepare_input_(n_math_solvers_);
        }();
        // calculate
        return [this, &logger, &input, solve_ = std::forward<SolveFn>(solve)] {
            Timer const timer{logger, LogEvent::math_calculation};
            auto& solvers = main_core::get_solvers<sym>(math_state_);
            auto& y_bus_vec = main_core::get_y_bus<sym>(math_state_);
            std::vector<SolverOutputType> solver_output;
            solver_output.reserve(n_math_solvers_);
            for (Idx i = 0; i != n_math_solvers_; ++i) {
                solver_output.emplace_back(solve_(solvers[i], y_bus_vec[i], input[i]));
            }
            return solver_output;
        }();
    }

    template <symmetry_tag sym> auto calculate_power_flow_(double err_tol, Idx max_iter, Logger& logger) {
        return
            [this, err_tol, max_iter, &logger](MainModelState const& state,
                                               CalculationMethod calculation_method) -> std::vector<SolverOutput<sym>> {
                return calculate_<SolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, PowerFlowInput<sym>>(
                    [&state](Idx n_math_solvers) {
                        return main_core::prepare_power_flow_input<sym>(state, n_math_solvers);
                    },
                    [err_tol, max_iter, calculation_method,
                     &logger](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
                        return solver.get().run_power_flow(input, err_tol, max_iter, logger, calculation_method, y_bus);
                    },
                    logger);
            };
    }

    template <symmetry_tag sym> auto calculate_state_estimation_(double err_tol, Idx max_iter, Logger& logger) {
        return
            [this, err_tol, max_iter, &logger](MainModelState const& state,
                                               CalculationMethod calculation_method) -> std::vector<SolverOutput<sym>> {
                return calculate_<SolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, StateEstimationInput<sym>>(
                    [&state](Idx n_math_solvers) {
                        return main_core::prepare_state_estimation_input<sym>(state, n_math_solvers);
                    },
                    [err_tol, max_iter, calculation_method, &logger](
                        MathSolverProxy<sym>& solver, YBus<sym> const& y_bus, StateEstimationInput<sym> const& input) {
                        return solver.get().run_state_estimation(input, err_tol, max_iter, logger, calculation_method,
                                                                 y_bus);
                    },
                    logger);
            };
    }

    template <symmetry_tag sym>
    auto calculate_short_circuit_(ShortCircuitVoltageScaling voltage_scaling, Logger& logger) {
        return [this, voltage_scaling,
                &logger](MainModelState const& state,
                         CalculationMethod calculation_method) -> std::vector<ShortCircuitSolverOutput<sym>> {
            (void)state; // to avoid unused-lambda-capture when in Release build
            assert(&state == &state_);

            return calculate_<ShortCircuitSolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, ShortCircuitInput>(
                [this, voltage_scaling](Idx n_math_solvers) {
                    assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
                    return main_core::prepare_short_circuit_input<sym>(state_, state_.comp_coup, n_math_solvers,
                                                                       voltage_scaling);
                },
                [calculation_method, &logger](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus,
                                              ShortCircuitInput const& input) {
                    return solver.get().run_short_circuit(input, logger, calculation_method, y_bus);
                },
                logger);
        };
    }

    // Calculate with optimization, e.g., automatic tap changer
    template <calculation_type_tag calculation_type, symmetry_tag sym>
    auto calculate(Options const& options, Logger& logger) {
        auto const calculator = [this, &options, &logger] {
            if constexpr (std::derived_from<calculation_type, power_flow_t>) {
                return calculate_power_flow_<sym>(options.err_tol, options.max_iter, logger);
            }
            assert(options.optimizer_type == OptimizerType::no_optimization);
            if constexpr (std::derived_from<calculation_type, state_estimation_t>) {
                return calculate_state_estimation_<sym>(options.err_tol, options.max_iter, logger);
            }
            if constexpr (std::derived_from<calculation_type, short_circuit_t>) {
                return calculate_short_circuit_<sym>(options.short_circuit_voltage_scaling, logger);
            }
            throw UnreachableHit{"MainModelImpl::calculate", "Unknown calculation type"};
        }();

        SearchMethod const& search_method = options.optimizer_strategy == OptimizerStrategy::any
                                                ? SearchMethod::linear_search
                                                : SearchMethod::binary_search;

        return optimizer::get_optimizer<MainModelState, ConstDataset>(
                   options.optimizer_type, options.optimizer_strategy, calculator,
                   [this](ConstDataset const& update_data) {
                       this->update_components<permanent_update_t>(update_data);
                   },
                   *meta_data_, search_method)
            ->optimize(state_, options.calculation_method);
    }

    // Single calculation, propagating the results to result_data
    void calculate(Options options, MutableDataset const& result_data, Logger& logger) {
        assert(construction_complete_);

        if (options.calculation_type == CalculationType::short_circuit) {
            auto const faults = state_.components.template citer<Fault>();
            auto const is_three_phase = std::ranges::all_of(
                faults, [](Fault const& fault) { return fault.get_fault_type() == FaultType::three_phase; });
            options.calculation_symmetry =
                is_three_phase ? CalculationSymmetry::symmetric : CalculationSymmetry::asymmetric;
        }

        calculation_type_symmetry_func_selector(
            options.calculation_type, options.calculation_symmetry,
            []<calculation_type_tag calculation_type, symmetry_tag sym>(
                MainModelImpl& main_model_, Options const& options_, MutableDataset const& result_data_,
                Logger& logger) {
                auto const math_output = main_model_.calculate<calculation_type, sym>(options_, logger);
                main_model_.output_result(math_output, result_data_, logger);
            },
            *this, options, result_data, logger);
    }

  public:
    static auto calculator(Options const& options, MainModelImpl& model, MutableDataset const& target_data,
                           bool cache_run, Logger& logger) {
        auto sub_opt = options; // copy
        sub_opt.err_tol = cache_run ? std::numeric_limits<double>::max() : options.err_tol;
        sub_opt.max_iter = cache_run ? 1 : options.max_iter;

        model.calculate(sub_opt, target_data, logger);
    }

    auto const& state() const {
        assert(construction_complete_);
        return state_;
    }
    auto const& meta_data() const {
        assert(construction_complete_);
        assert(meta_data_ != nullptr);
        return *meta_data_;
    }

    void check_no_experimental_features_used(Options const& /*options*/) const {}

  private:
    template <solver_output_type SolverOutputType>
    void output_result(MathOutput<std::vector<SolverOutputType>> const& math_output, MutableDataset const& result_data,
                       Logger& logger) const {
        assert(!result_data.is_batch());

        auto const output_func = [this, &math_output, &result_data]<typename CT>() {
            auto process_output_span = [this, &math_output](auto const& span) {
                if (std::empty(span)) {
                    return;
                }
                main_core::output_result<CT>(state_, math_output, span);
            };

            if (result_data.is_columnar(CT::name)) {
                auto const span =
                    result_data.get_columnar_buffer_span<typename output_type_getter<SolverOutputType>::type, CT>();
                process_output_span(span);
            } else {
                auto const span =
                    result_data.get_buffer_span<typename output_type_getter<SolverOutputType>::type, CT>();
                process_output_span(span);
            }
        };

        Timer const t_output{logger, LogEvent::produce_output};
        ModelType::run_functor_with_all_component_types_return_void(output_func);
    }

    double system_frequency_;
    MetaData const* meta_data_;
    MathSolverDispatcher const* math_solver_dispatcher_;

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
    SequenceIdx parameter_changed_components_{};
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

    void rebuild_topology() {
        assert(construction_complete_);
        // clear old solvers
        reset_solvers();
        ComponentConnections const comp_conn =
            main_core::construct_components_connections<ModelType>(state_.components);
        // re build
        Topology topology{*state_.comp_topo, comp_conn};
        std::tie(state_.math_topology, state_.topo_comp_coup) = topology.build_topology();
        n_math_solvers_ = static_cast<Idx>(state_.math_topology.size());
        is_topology_up_to_date_ = true;
        is_sym_parameter_up_to_date_ = false;
        is_asym_parameter_up_to_date_ = false;
    }

    template <symmetry_tag sym> void prepare_solvers() {
        std::vector<MathSolverProxy<sym>>& solvers = main_core::get_solvers<sym>(math_state_);
        // rebuild topology if needed
        if (!is_topology_up_to_date_) {
            rebuild_topology();
        }
        main_core::prepare_y_bus<sym, ModelType>(state_, n_math_solvers_, math_state_);

        if (n_math_solvers_ != static_cast<Idx>(solvers.size())) {
            assert(solvers.empty());
            assert(n_math_solvers_ == static_cast<Idx>(state_.math_topology.size()));
            assert(n_math_solvers_ == static_cast<Idx>(main_core::get_y_bus<sym>(math_state_).size()));

            solvers.clear();
            solvers.reserve(n_math_solvers_);
            std::ranges::transform(state_.math_topology, std::back_inserter(solvers), [this](auto const& math_topo) {
                return MathSolverProxy<sym>{math_solver_dispatcher_, math_topo};
            });
            for (Idx idx = 0; idx < n_math_solvers_; ++idx) {
                main_core::get_y_bus<sym>(math_state_)[idx].register_parameters_changed_callback(
                    [solver = std::ref(solvers[idx])](bool changed) {
                        solver.get().get().parameters_changed(changed);
                    });
            }
        } else if (!is_parameter_up_to_date<sym>()) {
            std::vector<MathModelParam<sym>> const math_params =
                main_core::get_math_param<sym>(state_, n_math_solvers_);
            std::vector<MathModelParamIncrement> const math_param_increments =
                main_core::get_math_param_increment<ModelType>(state_, n_math_solvers_, parameter_changed_components_);
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

} // namespace power_grid_model
