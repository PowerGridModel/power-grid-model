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
#include "main_core/calculation_info.hpp"
#include "main_core/calculation_input_preparation.hpp"
#include "main_core/core_utils.hpp"
#include "main_core/input.hpp"
#include "main_core/math_state.hpp"
#include "main_core/output.hpp"
#include "main_core/topology.hpp"
#include "main_core/update.hpp"

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

// main model implementation template
template <class T, class U> class MainModelImpl;

template <class... ExtraRetrievableType, class... ComponentType>
class MainModelImpl<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {
  private:
    // internal type traits
    // container class
    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    using MainModelState = main_core::MainModelState<ComponentContainer>;

    using SequenceIdxView = std::array<std::span<Idx2D const>, main_core::utils::n_types<ComponentType...>>;
    using OwnedUpdateDataset = std::tuple<std::vector<typename ComponentType::UpdateType>...>;

    static constexpr Idx isolated_component{main_core::isolated_component};
    static constexpr Idx not_connected{main_core::not_connected};
    static constexpr Idx sequential{JobDispatch::sequential};

  public:
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

    MainModelImpl(MainModelImpl const& other)
        : calculation_info_{}, // calculation info should not be copied, because it may result in race conditions
          system_frequency_{other.system_frequency_},
          meta_data_{other.meta_data_},
          math_solver_dispatcher_{other.math_solver_dispatcher_},
          state_{other.state_},
          math_state_{other.math_state_},
          n_math_solvers_{other.n_math_solvers_},
          is_topology_up_to_date_{other.is_topology_up_to_date_},
          is_sym_parameter_up_to_date_{other.is_sym_parameter_up_to_date_},
          is_asym_parameter_up_to_date_{other.is_asym_parameter_up_to_date_},
          is_accumulated_component_updated_{other.is_accumulated_component_updated_},
          last_updated_calculation_symmetry_mode_{other.last_updated_calculation_symmetry_mode_},
          cached_inverse_update_{other.cached_inverse_update_},
          cached_state_changes_{other.cached_state_changes_},
          parameter_changed_components_{other.parameter_changed_components_}
#ifndef NDEBUG
          ,
          // construction_complete is used for debug assertions only
          construction_complete_{other.construction_complete_}
#endif // !NDEBUG
    {
    }
    MainModelImpl& operator=(MainModelImpl const& other) {
        if (this != &other) {
            calculation_info_ = {}; // calculation info should be reset, because it may result in race conditions
            system_frequency_ = other.system_frequency_;
            meta_data_ = other.meta_data_;
            math_solver_dispatcher_ = other.math_solver_dispatcher_;
            state_ = other.state_;
            math_state_ = other.math_state_;
            n_math_solvers_ = other.n_math_solvers_;
            is_topology_up_to_date_ = other.is_topology_up_to_date_;
            is_sym_parameter_up_to_date_ = other.is_sym_parameter_up_to_date_;
            is_asym_parameter_up_to_date_ = other.is_asym_parameter_up_to_date_;
            is_accumulated_component_updated_ = other.is_accumulated_component_updated_;
            last_updated_calculation_symmetry_mode_ = other.last_updated_calculation_symmetry_mode_;
            cached_inverse_update_ = other.cached_inverse_update_;
            cached_state_changes_ = other.cached_state_changes_;
            parameter_changed_components_ = other.parameter_changed_components_;
#ifndef NDEBUG
            // construction_complete is used for debug assertions only
            construction_complete_ = other.construction_complete_;
#endif // !NDEBUG
        }
        return *this;
    }
    MainModelImpl(MainModelImpl&& /*other*/) noexcept = default;
    MainModelImpl& operator=(MainModelImpl&& /*other*/) noexcept = default;
    ~MainModelImpl() = default;

    // helper function to get what components are present in the update data
    std::array<bool, main_core::utils::n_types<ComponentType...>>
    get_components_to_update(ConstDataset const& update_data) const {
        return main_core::utils::run_functor_with_all_types_return_array<ComponentType...>(
            [&update_data]<typename CompType>() {
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
        main_core::add_component<CompType>(state_, std::forward<Inputs>(components), system_frequency_);
    }

    void add_components(ConstDataset const& input_data, Idx pos = 0) {
        auto const add_func = [this, pos, &input_data]<typename CT>() {
            if (input_data.is_columnar(CT::name)) {
                this->add_component<CT>(input_data.get_columnar_buffer_span<meta_data::input_getter_s, CT>(pos));
            } else {
                this->add_component<CT>(input_data.get_buffer_span<meta_data::input_getter_s, CT>(pos));
            }
        };
        main_core::utils::run_functor_with_all_types_return_void<ComponentType...>(add_func);
    }

    // template to update components
    // using forward interators
    // different selection based on component type
    // if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
    template <class CompType, cache_type_c CacheType, std::ranges::viewable_range Updates>
    void update_component(Updates&& updates, std::span<Idx2D const> sequence_idx) {
        constexpr auto comp_index = main_core::utils::index_of_component<CompType, ComponentType...>;

        assert(construction_complete_);
        assert(std::ranges::ssize(sequence_idx) == std::ranges::ssize(updates));

        if constexpr (CacheType::value) {
            main_core::update::update_inverse<CompType>(
                state_, updates, std::back_inserter(std::get<comp_index>(cached_inverse_update_)), sequence_idx);
        }

        UpdateChange const changed = main_core::update::update_component<CompType>(
            state_, std::forward<Updates>(updates),
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
        requires(std::same_as<SequenceIdxMap, main_core::utils::SequenceIdx<ComponentType...>> ||
                 std::same_as<SequenceIdxMap, SequenceIdxView>)
    void update_components(ConstDataset const& update_data, Idx pos, SequenceIdxMap const& sequence_idx_map) {
        main_core::utils::run_functor_with_all_types_return_void<ComponentType...>(
            [this, pos, &update_data, &sequence_idx_map]<typename CT>() {
                this->update_component_row_col<CT, CacheType>(
                    update_data, pos,
                    std::get<main_core::utils::index_of_component<CT, ComponentType...>>(sequence_idx_map));
            });
    }

    // overload to update all components in the first scenario (e.g. permanent update)
    template <cache_type_c CacheType> void update_components(ConstDataset const& update_data) {
        auto const components_to_update = get_components_to_update(update_data);
        auto const update_independence =
            main_core::update::independence::check_update_independence<ComponentType...>(state_, update_data);
        auto const sequence_idx_map = main_core::update::get_all_sequence_idx_map<ComponentType...>(
            state_, update_data, 0, components_to_update, update_independence, false);
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
        main_core::register_topology_components<GenericCurrentSensor>(state_, comp_topo);
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

  public:
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
        main_core::utils::run_functor_with_all_types_return_void<ComponentType...>(get_index_func);
    }

  private:
    // Entry point for main_model.hpp
    main_core::utils::SequenceIdx<ComponentType...> get_all_sequence_idx_map(ConstDataset const& update_data) {
        auto const components_to_update = get_components_to_update(update_data);
        auto const update_independence =
            main_core::update::independence::check_update_independence<ComponentType...>(state_, update_data);
        return main_core::update::get_all_sequence_idx_map<ComponentType...>(
            state_, update_data, 0, components_to_update, update_independence, false);
    }

    void update_state(UpdateChange const& changes) {
        // if topology changed, everything is not up to date
        // if only param changed, set param to not up to date
        is_topology_up_to_date_ = is_topology_up_to_date_ && !changes.topo;
        is_sym_parameter_up_to_date_ = is_sym_parameter_up_to_date_ && !changes.topo && !changes.param;
        is_asym_parameter_up_to_date_ = is_asym_parameter_up_to_date_ && !changes.topo && !changes.param;
    }

    template <typename CompType> void restore_component(SequenceIdxView const& sequence_idx) {
        constexpr auto component_index = main_core::utils::index_of_component<CompType, ComponentType...>;

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
        (restore_component<ComponentType>(sequence_idx), ...);

        update_state(cached_state_changes_);
        cached_state_changes_ = {};
    }

  private:
    void restore_components(std::array<std::reference_wrapper<std::vector<Idx2D> const>,
                                       main_core::utils::n_types<ComponentType...>> const& sequence_idx) {
        restore_components(std::array{std::span<Idx2D const>{
            std::get<main_core::utils::index_of_component<ComponentType, ComponentType...>>(sequence_idx).get()}...});
    }
    void restore_components(main_core::utils::SequenceIdx<ComponentType...> const& sequence_idx) {
        restore_components(std::array{std::span<Idx2D const>{
            std::get<main_core::utils::index_of_component<ComponentType, ComponentType...>>(sequence_idx)}...});
    }

    template <solver_output_type SolverOutputType, typename MathSolverType, typename YBus, typename InputType,
              typename PrepareInputFn, typename SolveFn>
        requires std::invocable<std::remove_cvref_t<PrepareInputFn>, Idx /*n_math_solvers*/> &&
                 std::invocable<std::remove_cvref_t<SolveFn>, MathSolverType&, YBus const&, InputType const&> &&
                 std::same_as<std::invoke_result_t<PrepareInputFn, Idx /*n_math_solvers*/>, std::vector<InputType>> &&
                 std::same_as<std::invoke_result_t<SolveFn, MathSolverType&, YBus const&, InputType const&>,
                              SolverOutputType>
    std::vector<SolverOutputType> calculate_(PrepareInputFn&& prepare_input, SolveFn&& solve) {
        using sym = typename SolverOutputType::sym;

        assert(construction_complete_);
        calculation_info_ = CalculationInfo{};
        // prepare
        auto const& input = [this, prepare_input_ = std::forward<PrepareInputFn>(prepare_input)] {
            Timer const timer(calculation_info_, 2100, "Prepare");
            prepare_solvers<sym>();
            assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
            return prepare_input_(n_math_solvers_);
        }();
        // calculate
        return [this, &input, solve_ = std::forward<SolveFn>(solve)] {
            Timer const timer(calculation_info_, 2200, "Math Calculation");
            auto& solvers = get_solvers<sym>();
            auto& y_bus_vec = get_y_bus<sym>();
            std::vector<SolverOutputType> solver_output;
            solver_output.reserve(n_math_solvers_);
            for (Idx i = 0; i != n_math_solvers_; ++i) {
                solver_output.emplace_back(solve_(solvers[i], y_bus_vec[i], input[i]));
            }
            return solver_output;
        }();
    }

    template <symmetry_tag sym> auto calculate_power_flow_(double err_tol, Idx max_iter) {
        return [this, err_tol, max_iter](MainModelState const& state,
                                         CalculationMethod calculation_method) -> std::vector<SolverOutput<sym>> {
            return calculate_<SolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, PowerFlowInput<sym>>(
                [&state](Idx n_math_solvers) {
                    return main_core::prepare_power_flow_input<sym>(state, n_math_solvers);
                },
                [this, err_tol, max_iter, calculation_method](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus,
                                                              PowerFlowInput<sym> const& input) {
                    return solver.get().run_power_flow(input, err_tol, max_iter, calculation_info_, calculation_method,
                                                       y_bus);
                });
        };
    }

    template <symmetry_tag sym> auto calculate_state_estimation_(double err_tol, Idx max_iter) {
        return [this, err_tol, max_iter](MainModelState const& state,
                                         CalculationMethod calculation_method) -> std::vector<SolverOutput<sym>> {
            return calculate_<SolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, StateEstimationInput<sym>>(
                [&state](Idx n_math_solvers) {
                    return main_core::prepare_state_estimation_input<sym>(state, n_math_solvers);
                },
                [this, err_tol, max_iter, calculation_method](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus,
                                                              StateEstimationInput<sym> const& input) {
                    return solver.get().run_state_estimation(input, err_tol, max_iter, calculation_info_,
                                                             calculation_method, y_bus);
                });
        };
    }

    template <symmetry_tag sym> auto calculate_short_circuit_(ShortCircuitVoltageScaling voltage_scaling) {
        return [this,
                voltage_scaling](MainModelState const& state,
                                 CalculationMethod calculation_method) -> std::vector<ShortCircuitSolverOutput<sym>> {
            (void)state; // to avoid unused-lambda-capture when in Release build
            assert(&state == &state_);

            return calculate_<ShortCircuitSolverOutput<sym>, MathSolverProxy<sym>, YBus<sym>, ShortCircuitInput>(
                [this, voltage_scaling](Idx n_math_solvers) {
                    assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
                    return main_core::prepare_short_circuit_input<sym>(state_, state_.comp_coup, n_math_solvers,
                                                                       voltage_scaling);
                },
                [this, calculation_method](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus,
                                           ShortCircuitInput const& input) {
                    return solver.get().run_short_circuit(input, calculation_info_, calculation_method, y_bus);
                });
        };
    }

    // Calculate with optimization, e.g., automatic tap changer
    template <calculation_type_tag calculation_type, symmetry_tag sym> auto calculate(Options const& options) {
        auto const calculator = [this, &options] {
            if constexpr (std::derived_from<calculation_type, power_flow_t>) {
                return calculate_power_flow_<sym>(options.err_tol, options.max_iter);
            }
            assert(options.optimizer_type == OptimizerType::no_optimization);
            if constexpr (std::derived_from<calculation_type, state_estimation_t>) {
                return calculate_state_estimation_<sym>(options.err_tol, options.max_iter);
            }
            if constexpr (std::derived_from<calculation_type, short_circuit_t>) {
                return calculate_short_circuit_<sym>(options.short_circuit_voltage_scaling);
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
    void calculate(Options options, MutableDataset const& result_data) {
        assert(construction_complete_);

        if (options.calculation_type == CalculationType::short_circuit) {
            auto const faults = state_.components.template citer<Fault>();
            auto const is_three_phase = std::ranges::all_of(
                faults, [](Fault const& fault) { return fault.get_fault_type() == FaultType::three_phase; });
            options.calculation_symmetry =
                is_three_phase ? CalculationSymmetry::symmetric : CalculationSymmetry::asymmetric;
        };

        calculation_type_symmetry_func_selector(
            options.calculation_type, options.calculation_symmetry,
            []<calculation_type_tag calculation_type, symmetry_tag sym>(
                MainModelImpl& main_model_, Options const& options_, MutableDataset const& result_data_) {
                auto const math_output = main_model_.calculate<calculation_type, sym>(options_);
                main_model_.output_result(math_output, result_data_);
            },
            *this, options, result_data);
    }

  public:
    static auto calculator(Options const& options) {
        return [options](MainModelImpl& model, MutableDataset const& target_data, bool cache_run) {
            auto sub_opt = options; // copy
            sub_opt.err_tol = cache_run ? std::numeric_limits<double>::max() : options.err_tol;
            sub_opt.max_iter = cache_run ? 1 : options.max_iter;

            model.calculate(sub_opt, target_data);
        };
    }

    CalculationInfo calculation_info() const { return calculation_info_; }
    void merge_calculation_info(CalculationInfo const& info) {
        assert(construction_complete_);
        main_core::merge_into(calculation_info_, info);
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
    void output_result(MathOutput<std::vector<SolverOutputType>> const& math_output,
                       MutableDataset const& result_data) const {
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

        Timer const t_output(calculation_info_, 3000, "Produce output");
        main_core::utils::run_functor_with_all_types_return_void<ComponentType...>(output_func);
    }

    mutable CalculationInfo calculation_info_; // needs to be first due to padding override
                                               // may be changed in const functions for metrics

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
    std::array<std::vector<Idx2D>, main_core::utils::n_types<ComponentType...>> parameter_changed_components_{};
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

    template <symmetry_tag sym> std::vector<MathSolverProxy<sym>>& get_solvers() {
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
            if (math_idx.group == isolated_component) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].branch_param[math_idx.pos] =
                state_.components.template get_item_by_seq<Branch>(i).template calc_param<sym>();
        }
        // loop all branch3
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->branch3_node_idx.size()); ++i) {
            Idx2DBranch3 const math_idx = state_.topo_comp_coup->branch3[i];
            if (math_idx.group == isolated_component) {
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
            if (math_idx.group == isolated_component) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].shunt_param[math_idx.pos] =
                state_.components.template get_item_by_seq<Shunt>(i).template calc_param<sym>();
        }
        // loop all source
        for (Idx i = 0; i != static_cast<Idx>(state_.comp_topo->source_node_idx.size()); ++i) {
            Idx2D const math_idx = state_.topo_comp_coup->source[i];
            if (math_idx.group == isolated_component) {
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

        static constexpr std::array<AddToIncrement, main_core::utils::n_types<ComponentType...>> add_to_increments{
            [](std::vector<MathModelParamIncrement>& increments, MainModelState const& state,
               Idx2D const& changed_component_idx) {
                if constexpr (std::derived_from<ComponentType, Branch>) {
                    Idx2D const math_idx =
                        state.topo_comp_coup
                            ->branch[main_core::get_component_sequence_idx<Branch>(state, changed_component_idx)];
                    if (math_idx.group == isolated_component) {
                        return;
                    }
                    // assign parameters
                    increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos);
                } else if constexpr (std::derived_from<ComponentType, Branch3>) {
                    Idx2DBranch3 const math_idx =
                        state.topo_comp_coup
                            ->branch3[main_core::get_component_sequence_idx<Branch3>(state, changed_component_idx)];
                    if (math_idx.group == isolated_component) {
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
                            ->shunt[main_core::get_component_sequence_idx<Shunt>(state, changed_component_idx)];
                    if (math_idx.group == isolated_component) {
                        return;
                    }
                    // assign parameters
                    increments[math_idx.group].shunt_param_to_change.push_back(math_idx.pos);
                }
            }...};

        std::vector<MathModelParamIncrement> math_param_increment(n_math_solvers_);

        for (size_t i = 0; i < main_core::utils::n_types<ComponentType...>; ++i) {
            auto const& changed_type_components = parameter_changed_components_[i];
            auto const& add_type_to_increment = add_to_increments[i];
            for (auto const& changed_component : changed_type_components) {
                add_type_to_increment(math_param_increment, state_, changed_component);
            }
        }

        return math_param_increment;
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
                std::array{main_core::utils::index_of_component<Line, ComponentType...>,
                           main_core::utils::index_of_component<Link, ComponentType...>,
                           main_core::utils::index_of_component<Transformer, ComponentType...>};
            constexpr auto shunt_param_in_seq_map =
                std::array{main_core::utils::index_of_component<Shunt, ComponentType...>};

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
        std::vector<MathSolverProxy<sym>>& solvers = get_solvers<sym>();
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
            std::ranges::transform(state_.math_topology, std::back_inserter(solvers), [this](auto const& math_topo) {
                return MathSolverProxy<sym>{math_solver_dispatcher_, math_topo};
            });
            for (Idx idx = 0; idx < n_math_solvers_; ++idx) {
                get_y_bus<sym>()[idx].register_parameters_changed_callback(
                    [solver = std::ref(solvers[idx])](bool changed) {
                        solver.get().get().parameters_changed(changed);
                    });
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

} // namespace power_grid_model
