// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_MODEL_HPP
#define POWER_GRID_MODEL_MAIN_MODEL_HPP

// main model class

// main include
#include "calculation_parameters.hpp"
#include "container.hpp"
#include "exception.hpp"
#include "power_grid_model.hpp"
#include "timer.hpp"
#include "topology.hpp"

// component include
#include "all_components.hpp"
#include "auxiliary/dataset.hpp"
#include "auxiliary/input.hpp"
#include "auxiliary/output.hpp"

// math model include
#include "math_solver/math_solver.hpp"

// threading
#include <thread>

namespace power_grid_model {

// main model implementation template
template <class T, class U>
class MainModelImpl;

template <class... ExtraRetrievableType, class... ComponentType>
class MainModelImpl<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> final {
   private:
    // internal type traits
    // container class
    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;

    // trait on type list
    // struct of entry
    // name of the component, and the index in the list
    struct ComponentEntry {
        char const* name;
        size_t index;
    };

    template <class T, class U>
    struct component_list_generator_impl;
    template <class... C, size_t... index>
    struct component_list_generator_impl<ComponentList<C...>, std::index_sequence<index...>> {
        using AllTypes = ComponentList<C...>;
        static constexpr std::array component_index_map{ComponentEntry{C::name, index}...};
        static constexpr size_t n_types = sizeof...(C);

        static size_t find_index(std::string const& name) {
            auto const found = std::find_if(component_index_map.cbegin(), component_index_map.cend(), [&name](auto x) {
                return x == name;
            });
            assert(found != component_index_map.cend());
            return found->index;
        }
    };
    using AllComponents = component_list_generator_impl<ComponentList<ComponentType...>,
                                                        std::make_index_sequence<sizeof...(ComponentType)>>;
    static constexpr size_t n_types = AllComponents::n_types;

    // function pointer definition
    using InputFunc = void (*)(MainModelImpl& x, DataPointer<true> const& data_ptr, Idx position);
    using UpdateFunc = void (*)(MainModelImpl& x, DataPointer<true> const& data_ptr, Idx position,
                                std::vector<Idx2D> const& sequence_idx);
    template <bool sym>
    using OutputFunc = void (*)(MainModelImpl& x, std::vector<MathOutput<sym>> const& math_output,
                                DataPointer<false> const& data_ptr, Idx position);
    using CheckUpdateFunc = bool (*)(ConstDataPointer const& component_update);
    using GetSeqIdxFunc = std::vector<Idx2D> (*)(MainModelImpl const& x, ConstDataPointer const& component_update);
    using GetIndexerFunc = void (*)(MainModelImpl const& x, ID const* id_begin, Idx size, Idx* indexer_begin);

   public:
    // constructor with data
    explicit MainModelImpl(double system_frequency, ConstDataset const& input_data, Idx pos = 0)
        : system_frequency_{system_frequency} {
        static constexpr std::array<InputFunc, n_types> add{
            [](MainModelImpl& model, DataPointer<true> const& data_ptr, Idx position) {
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
    explicit MainModelImpl(double system_frequency) : system_frequency_{system_frequency} {
    }

    // get number
    template <class CompType>
    Idx component_count() const {
        assert(construction_complete_);
        return components_.template size<CompType>();
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
    template <class CompType>
    void add_component(std::vector<typename CompType::InputType> const& components) {
        add_component<CompType>(components.cbegin(), components.cend());
    }

    // template to construct components
    // using forward interators
    // different selection based on component type
    template <class CompType, class ForwardIterator>
    std::enable_if_t<std::is_base_of_v<Base, CompType>> add_component(ForwardIterator begin, ForwardIterator end) {
        assert(!construction_complete_);
        // check forward iterator
        static_assert(std::is_base_of_v<std::forward_iterator_tag,
                                        typename std::iterator_traits<ForwardIterator>::iterator_category>);
        size_t size = std::distance(begin, end);
        components_.template reserve<CompType>(size);
        // loop to add component
        for (auto it = begin; it != end; ++it) {
            auto const& input = *it;
            ID const id = input.id;
            // construct based on type of component
            if constexpr (std::is_base_of_v<Node, CompType>) {
                components_.template emplace<CompType>(id, input);
            }
            else if constexpr (std::is_base_of_v<Branch, CompType>) {
                double const u1 = components_.template get_item<Node>(input.from_node).u_rated();
                double const u2 = components_.template get_item<Node>(input.to_node).u_rated();
                // set system frequency for line
                if constexpr (std::is_same_v<CompType, Line>) {
                    components_.template emplace<CompType>(id, input, system_frequency_, u1, u2);
                }
                else {
                    components_.template emplace<CompType>(id, input, u1, u2);
                }
            }
            else if constexpr (std::is_base_of_v<Branch3, CompType>) {
                double const u1 = components_.template get_item<Node>(input.node_1).u_rated();
                double const u2 = components_.template get_item<Node>(input.node_2).u_rated();
                double const u3 = components_.template get_item<Node>(input.node_3).u_rated();
                components_.template emplace<CompType>(id, input, u1, u2, u3);
            }
            else if constexpr (std::is_base_of_v<Appliance, CompType>) {
                double const u = components_.template get_item<Node>(input.node).u_rated();
                components_.template emplace<CompType>(id, input, u);
            }
            else if constexpr (std::is_base_of_v<GenericVoltageSensor, CompType>) {
                double const u = components_.template get_item<Node>(input.measured_object).u_rated();
                components_.template emplace<CompType>(id, input, u);
            }
            else if constexpr (std::is_base_of_v<GenericPowerSensor, CompType>) {
                // it is not allowed to place a sensor at a link
                if (components_.get_idx_by_id(input.measured_object).group ==
                    components_.template get_type_idx<Link>()) {
                    throw InvalidMeasuredObject("Link", "PowerSensor");
                }
                ID const measured_object = input.measured_object;
                // check correctness of measured component type based on measured terminal type
                switch (input.measured_terminal_type) {
                    case MeasuredTerminalType::branch_from:
                    case MeasuredTerminalType::branch_to:
                        components_.template get_item<Branch>(measured_object);
                        break;
                    case MeasuredTerminalType::branch3_1:
                    case MeasuredTerminalType::branch3_2:
                    case MeasuredTerminalType::branch3_3:
                        components_.template get_item<Branch3>(measured_object);
                        break;
                    case MeasuredTerminalType::shunt:
                        components_.template get_item<Shunt>(measured_object);
                        break;
                    case MeasuredTerminalType::source:
                        components_.template get_item<Source>(measured_object);
                        break;
                    case MeasuredTerminalType::load:
                        components_.template get_item<GenericLoad>(measured_object);
                        break;
                    case MeasuredTerminalType::generator:
                        components_.template get_item<GenericGenerator>(measured_object);
                        break;
                    default:
                        throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " item retrieval",
                                                      input.measured_terminal_type);
                }

                components_.template emplace<CompType>(id, input);
            }
        }
    }

    // helper function to update vectors of components
    template <class CompType>
    void update_component(std::vector<typename CompType::UpdateType> const& components) {
        update_component<CompType>(components.cbegin(), components.cend());
    }

    // template to update components
    // using forward interators
    // different selection based on component type
    // if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
    template <class CompType, class ForwardIterator>
    void update_component(ForwardIterator begin, ForwardIterator end, std::vector<Idx2D> const& sequence_idx = {}) {
        assert(construction_complete_);
        // check forward iterator
        static_assert(std::is_base_of_v<std::forward_iterator_tag,
                                        typename std::iterator_traits<ForwardIterator>::iterator_category>);
        bool const has_sequence_id = !sequence_idx.empty();
        Idx seq = 0;
        // loop to to update component
        for (auto it = begin; it != end; ++it, ++seq) {
            // get component
            // either using ID via hash map
            // either directly using sequence id
            CompType& comp = has_sequence_id ? components_.template get_item<CompType>(sequence_idx[seq])
                                             : components_.template get_item<CompType>(it->id);
            // update, get changed variable
            UpdateChange changed = comp.update(*it);
            // if topology changed, everything is not up to date
            // if only param changed, set param to not up to date
            is_topology_up_to_date_ = is_topology_up_to_date_ && !changed.topo;
            is_sym_parameter_up_to_date_ = is_sym_parameter_up_to_date_ && !changed.topo && !changed.param;
            is_asym_parameter_up_to_date_ = is_asym_parameter_up_to_date_ && !changed.topo && !changed.param;
        }
    }

    // update all components
    void update_component(ConstDataset const& update_data, Idx pos = 0,
                          std::map<std::string, std::vector<Idx2D>> const& sequence_idx_map = {}) {
        static constexpr std::array<UpdateFunc, n_types> update{[](MainModelImpl& model,
                                                                   DataPointer<true> const& data_ptr, Idx position,
                                                                   std::vector<Idx2D> const& sequence_idx) {
            auto const [begin, end] = data_ptr.get_iterators<typename ComponentType::UpdateType>(position);
            model.update_component<ComponentType>(begin, end, sequence_idx);
        }...};
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            auto const found = update_data.find(entry.name);
            // skip if component does not exist
            if (found == update_data.cend()) {
                continue;
            }
            // try to find sequence idx
            auto const found_seq = sequence_idx_map.find(entry.name);
            if (found_seq == sequence_idx_map.cend()) {
                // if not found sequence, update using IDs
                update[entry.index](*this, found->second, pos, {});
            }
            else {
                // else update using pre-cached sequence number
                update[entry.index](*this, found->second, pos, found_seq->second);
            }
        }
    }

    // set complete construction
    // initialize internal arrays
    void set_construction_complete() {
        assert(!construction_complete_);
#ifndef NDEBUG
        // set construction_complete for debug assertions
        construction_complete_ = true;
#endif  // !NDEBUG
        components_.set_construction_complete();
        // set component topo
        ComponentTopology comp_topo;
        comp_topo.n_node = components_.template size<Node>();
        // fill topology data
        comp_topo.branch_node_idx.resize(components_.template size<Branch>());
        std::transform(components_.template citer<Branch>().begin(), components_.template citer<Branch>().end(),
                       comp_topo.branch_node_idx.begin(), [this](Branch const& branch) {
                           return BranchIdx{components_.template get_seq<Node>(branch.from_node()),
                                            components_.template get_seq<Node>(branch.to_node())};
                       });
        comp_topo.branch3_node_idx.resize(components_.template size<Branch3>());
        std::transform(components_.template citer<Branch3>().begin(), components_.template citer<Branch3>().end(),
                       comp_topo.branch3_node_idx.begin(), [this](Branch3 const& branch3) {
                           return Branch3Idx{components_.template get_seq<Node>(branch3.node_1()),
                                             components_.template get_seq<Node>(branch3.node_2()),
                                             components_.template get_seq<Node>(branch3.node_3())};
                       });
        comp_topo.source_node_idx.resize(components_.template size<Source>());
        std::transform(components_.template citer<Source>().begin(), components_.template citer<Source>().end(),
                       comp_topo.source_node_idx.begin(), [this](Source const& source) {
                           return components_.template get_seq<Node>(source.node());
                       });
        comp_topo.shunt_node_idx.resize(components_.template size<Shunt>());
        std::transform(components_.template citer<Shunt>().begin(), components_.template citer<Shunt>().end(),
                       comp_topo.shunt_node_idx.begin(), [this](Shunt const& shunt) {
                           return components_.template get_seq<Node>(shunt.node());
                       });
        comp_topo.load_gen_node_idx.resize(components_.template size<GenericLoadGen>());
        std::transform(components_.template citer<GenericLoadGen>().begin(),
                       components_.template citer<GenericLoadGen>().end(), comp_topo.load_gen_node_idx.begin(),
                       [this](GenericLoadGen const& load_gen) {
                           return components_.template get_seq<Node>(load_gen.node());
                       });
        comp_topo.load_gen_type.resize(components_.template size<GenericLoadGen>());
        std::transform(components_.template citer<GenericLoadGen>().begin(),
                       components_.template citer<GenericLoadGen>().end(), comp_topo.load_gen_type.begin(),
                       [](GenericLoadGen const& load_gen) {
                           return load_gen.type();
                       });
        comp_topo.voltage_sensor_node_idx.resize(components_.template size<GenericVoltageSensor>());
        std::transform(components_.template citer<GenericVoltageSensor>().begin(),
                       components_.template citer<GenericVoltageSensor>().end(),
                       comp_topo.voltage_sensor_node_idx.begin(), [this](GenericVoltageSensor const& voltage_sensor) {
                           return components_.template get_seq<Node>(voltage_sensor.measured_object());
                       });
        comp_topo.power_sensor_object_idx.resize(components_.template size<GenericPowerSensor>());
        std::transform(components_.template citer<GenericPowerSensor>().begin(),
                       components_.template citer<GenericPowerSensor>().end(),
                       comp_topo.power_sensor_object_idx.begin(), [this](GenericPowerSensor const& power_sensor) {
                           switch (power_sensor.get_terminal_type()) {
                               case MeasuredTerminalType::branch_from:
                               case MeasuredTerminalType::branch_to:
                                   return components_.template get_seq<Branch>(power_sensor.measured_object());
                               case MeasuredTerminalType::source:
                                   return components_.template get_seq<Source>(power_sensor.measured_object());
                               case MeasuredTerminalType::shunt:
                                   return components_.template get_seq<Shunt>(power_sensor.measured_object());
                               case MeasuredTerminalType::load:
                               case MeasuredTerminalType::generator:
                                   return components_.template get_seq<GenericLoadGen>(power_sensor.measured_object());
                               case MeasuredTerminalType::branch3_1:
                               case MeasuredTerminalType::branch3_2:
                               case MeasuredTerminalType::branch3_3:
                                   return components_.template get_seq<Branch3>(power_sensor.measured_object());
                               default:
                                   throw MissingCaseForEnumError("Power sensor idx to seq transformation",
                                                                 power_sensor.get_terminal_type());
                           }
                       });
        comp_topo.power_sensor_terminal_type.resize(components_.template size<GenericPowerSensor>());
        std::transform(components_.template citer<GenericPowerSensor>().begin(),
                       components_.template citer<GenericPowerSensor>().end(),
                       comp_topo.power_sensor_terminal_type.begin(), [](GenericPowerSensor const& power_sensor) {
                           return power_sensor.get_terminal_type();
                       });
        comp_topo_ = std::make_shared<ComponentTopology const>(std::move(comp_topo));
    }

    void reset_solvers() {
        assert(construction_complete_);
        is_topology_up_to_date_ = false;
        is_sym_parameter_up_to_date_ = false;
        is_asym_parameter_up_to_date_ = false;
        n_math_solvers_ = 0;
        sym_solvers_.clear();
        asym_solvers_.clear();
        math_topology_.clear();
        comp_coup_.reset();
    }

    /*
    the the sequence indexer given an input array of ID's for a given component type
    */
    void get_indexer(std::string const& component_type, ID const* id_begin, Idx size, Idx* indexer_begin) {
        // static function array
        static constexpr std::array<GetIndexerFunc, n_types> get_indexer_func{
            [](MainModelImpl const& model, ID const* id_begin, Idx size, Idx* indexer_begin) {
                std::transform(id_begin, id_begin + size, indexer_begin, [&model](ID id) {
                    return model.components_.template get_idx_by_id<ComponentType>(id).pos;
                });
            }...};
        // search component type name
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            if (entry.name == component_type) {
                return get_indexer_func[entry.index](*this, id_begin, size, indexer_begin);
            }
        }
    }

   private:
    template <bool sym, typename InputType, std::vector<InputType> (MainModelImpl::*PrepareInputFn)(),
              MathOutput<sym> (MathSolver<sym>::*SolveFn)(InputType const&, double, Idx, CalculationInfo&,
                                                          CalculationMethod)>
    std::vector<MathOutput<sym>> calculate_(double err_tol, Idx max_iter, CalculationMethod calculation_method) {
        assert(construction_complete_);
        calculation_info_ = CalculationInfo{};
        // prepare
        Timer timer(calculation_info_, 2100, "Prepare");
        prepare_solvers<sym>();
        auto const& input = (this->*PrepareInputFn)();
        // calculate
        timer = Timer(calculation_info_, 2200, "Math Calculation");
        std::vector<MathSolver<sym>>& solvers = get_solvers<sym>();
        std::vector<MathOutput<sym>> math_output(n_math_solvers_);
        std::transform(solvers.begin(), solvers.end(), input.cbegin(), math_output.begin(),
                       [&](MathSolver<sym>& math_solver, InputType const& y) {
                           return (math_solver.*SolveFn)(y, err_tol, max_iter, calculation_info_, calculation_method);
                       });
        return math_output;
    }

    template <bool sym>
    std::vector<MathOutput<sym>> calculate_power_flow_(double err_tol, Idx max_iter,
                                                       CalculationMethod calculation_method) {
        return calculate_<sym, PowerFlowInput<sym>, &MainModelImpl::prepare_power_flow_input,
                          &MathSolver<sym>::run_power_flow>(err_tol, max_iter, calculation_method);
    }

    template <bool sym>
    std::vector<MathOutput<sym>> calculate_state_estimation_(double err_tol, Idx max_iter,
                                                             CalculationMethod calculation_method) {
        return calculate_<sym, StateEstimationInput<sym>, &MainModelImpl::prepare_state_estimation_input,
                          &MathSolver<sym>::run_state_estimation>(err_tol, max_iter, calculation_method);
    }

    // get sequence idx map for fast caching of component sequences
    // only applicable for independent update dataset
    std::map<std::string, std::vector<Idx2D>> get_sequence_idx_map(ConstDataset const& update_data) const {
        // function pointer array to get cached idx
        static constexpr std::array<GetSeqIdxFunc, n_types> get_seq_idx{
            [](MainModelImpl const& model, ConstDataPointer const& component_update) -> std::vector<Idx2D> {
                using UpdateType = typename ComponentType::UpdateType;
                // no batch
                if (component_update.batch_size() < 1) {
                    return {};
                }
                // begin and end of the first batch
                auto const [it_begin, it_end] = component_update.template get_iterators<UpdateType>(0);
                // vector
                std::vector<Idx2D> seq_idx(std::distance(it_begin, it_end));
                std::transform(it_begin, it_end, seq_idx.begin(), [&model](UpdateType const& update) {
                    return model.components_.template get_idx_by_id<ComponentType>(update.id);
                });
                return seq_idx;
            }...};

        // fill in the map per component type
        std::map<std::string, std::vector<Idx2D>> sequence_idx_map;
        for (ComponentEntry const& entry : AllComponents::component_index_map) {
            auto const found = update_data.find(entry.name);
            // skip if component does not exist
            if (found == update_data.cend()) {
                continue;
            }
            // add
            sequence_idx_map[entry.name] = get_seq_idx[entry.index](*this, found->second);
        }
        return sequence_idx_map;
    }

    /*
    calculate power flow or state estimation in batch
    provide update data
    threading
        < 0 sequential
        = 0 parallel, use number of hardware threads
        > 0 specify number of parallel threads
    raise a BatchCalculationError if any of the calculations in the batch raised an exception
    */
    template <bool sym, std::vector<MathOutput<sym>> (MainModelImpl::*calculation_fn)(double, Idx, CalculationMethod)>
    BatchParameter batch_calculation_(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                      Dataset const& result_data, ConstDataset const& update_data, Idx threading = -1) {
        // if the update batch is one empty set per component type
        // execute one power flow in the current instance, no batch calculation is needed
        bool const all_empty = std::all_of(update_data.cbegin(), update_data.cend(), [](auto const& x) {
            return x.second.is_empty();
        });
        if (all_empty) {
            auto const math_output = (this->*calculation_fn)(err_tol, max_iter, calculation_method);
            output_result(math_output, result_data);
            return BatchParameter{};
        }

        // get number of batches (can't be empty, because then all_empty would have been true)
        Idx const n_batch = update_data.cbegin()->second.batch_size();
        // assert if all component types have the same number of batches
        assert(std::all_of(update_data.cbegin(), update_data.cend(), [n_batch](auto const& x) {
            return x.second.batch_size() == n_batch;
        }));

        // if cache_topology, the topology and math solvers will be initialized at base scenario
        // otherwise the topology and math solvers will be reset
        bool const cache_topology = MainModelImpl::is_topology_cacheable(update_data);
        // if independent is true, the base scenario will not be copied in each loop
        // otherwise in each loop a new instance is made with base scenario
        bool const independent = MainModelImpl::is_update_independent(update_data);

        // calculate once for cache topology, ignore results, all math solvers are initialized
        if (cache_topology) {
            (this->*calculation_fn)(err_tol, max_iter, calculation_method);
        }
        else {
            // otherwise reset solvers
            reset_solvers();
        }
        // const ref of current instance
        MainModelImpl const& base_model = *this;

        // get component sequence idx cache if update dataset is independent
        std::map<std::string, std::vector<Idx2D>> const sequence_idx_map =
            independent ? get_sequence_idx_map(update_data) : std::map<std::string, std::vector<Idx2D>>{};

        // error messages
        std::vector<std::string> exceptions(n_batch, "");

        // lambda for sub batch calculation
        auto sub_batch = [&base_model, &exceptions, &result_data, &update_data, &sequence_idx_map, n_batch, independent,
                          err_tol, max_iter, calculation_method](Idx start, Idx stride) {
            // copy base model
            MainModelImpl model{base_model};
            for (Idx batch_number = start; batch_number < n_batch; batch_number += stride) {
                // duplicate model if updates are not independent
                if (!independent) {
                    model = base_model;
                }
                // try to update model and run calculation
                try {
                    model.update_component(update_data, batch_number, sequence_idx_map);
                    auto const math_output = (model.*calculation_fn)(err_tol, max_iter, calculation_method);
                    model.output_result(math_output, result_data, batch_number);
                }
                catch (std::exception const& ex) {
                    exceptions[batch_number] = ex.what();
                }
                catch (...) {
                    exceptions[batch_number] = "unknown exception";
                }
            }
        };

        // run batches sequential or parallel
        Idx const hardware_thread = (Idx)std::thread::hardware_concurrency();
        // run sequential if
        //    specified threading < 0
        //    use hardware threads, but it is either unknown (0) or only has one thread (1)
        //    specified threading = 1
        if (threading < 0 || threading == 1 || (threading == 0 && hardware_thread < 2)) {
            // run all in sequential
            sub_batch(0, 1);
        }
        else {
            // create parallel threads
            Idx const n_thread = threading == 0 ? hardware_thread : threading;
            std::vector<std::thread> threads;
            threads.reserve(n_thread);
            for (Idx thread_number = 0; thread_number != n_thread; ++thread_number) {
                // compute each sub batch with stride
                threads.emplace_back(sub_batch, thread_number, n_thread);
            }
            for (auto& thread : threads) {
                thread.join();
            }
        }

        // handle exception message
        std::string combined_error_message;
        for (Idx batch = 0; batch != n_batch; ++batch) {
            // append exception if it is not empty
            if (!exceptions[batch].empty()) {
                combined_error_message += "Error in batch #" + std::to_string(batch) + ": " + exceptions[batch];
            }
        }
        if (!combined_error_message.empty()) {
            throw BatchCalculationError(combined_error_message);
        }

        return BatchParameter{independent, cache_topology};
    }

   public:
    template <class Component>
    using UpdateType = typename Component::UpdateType;

    static bool is_update_independent(ConstDataset const& update_data) {
        // check all components
        return std::all_of(AllComponents::component_index_map.cbegin(), AllComponents::component_index_map.cend(),
                           [&update_data](ComponentEntry const& entry) {
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

    template <class Component>
    static bool is_component_update_independent(ConstDataPointer const& component_update) {
        // If the batch size is (0 or) 1, then the update data for this component is 'independent'
        if (component_update.batch_size() <= 1) {
            return true;
        }

        // Remember the first batch size, then loop over the remaining batches and check if they are of the same length
        Idx const length_per_batch = component_update.length_per_batch(0);
        for (Idx batch = 1; batch != component_update.batch_size(); ++batch) {
            if (length_per_batch != component_update.length_per_batch(batch)) {
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

    static bool is_topology_cacheable(ConstDataset const& update_data) {
        // check all components
        return std::all_of(AllComponents::component_index_map.cbegin(), AllComponents::component_index_map.cend(),
                           [&update_data](ComponentEntry const& entry) {
                               static constexpr std::array check_component_update_cacheable{
                                   &is_topology_cacheable_component<ComponentType>...};
                               auto const found = update_data.find(entry.name);
                               // return true if this component update does not exist
                               if (found == update_data.cend()) {
                                   return true;
                               }
                               // check for this component update
                               return check_component_update_cacheable[entry.index](found->second);
                           });
    }

    template <class Component>
    static bool is_topology_cacheable_component(ConstDataPointer const& component_update) {
        // The topology is cacheable if there are no changes in the branch and source switching statusses
        auto const [it_begin, it_end] = component_update.template get_iterators<UpdateType<Component>>(-1);
        if constexpr (std::is_base_of_v<Branch, Component>) {
            // Check for all batches
            return std::all_of(it_begin, it_end, [](BranchUpdate const& update) {
                return is_nan(update.from_status) && is_nan(update.to_status);
            });
        }
        else if constexpr (std::is_base_of_v<Branch3, Component>) {
            // Check for all batches
            return std::all_of(it_begin, it_end, [](Branch3Update const& update) {
                return is_nan(update.status_1) && is_nan(update.status_2) && is_nan(update.status_3);
            });
        }
        else if constexpr (std::is_base_of_v<Source, Component>) {
            // Check for all batches
            return std::all_of(it_begin, it_end, [](SourceUpdate const& update) {
                return is_nan(update.status);
            });
        }
        else {
            // Other components have no impact on topology caching
            return true;
        }
    }

    // Single load flow calculation, returning math output results
    template <bool sym>
    std::vector<MathOutput<sym>> calculate_power_flow(double err_tol, Idx max_iter,
                                                      CalculationMethod calculation_method) {
        return calculate_power_flow_<sym>(err_tol, max_iter, calculation_method);
    }

    // Single load flow calculation, propagating the results to result_data
    template <bool sym>
    void calculate_power_flow(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                              Dataset const& result_data, Idx pos = 0) {
        assert(construction_complete_);
        auto const math_output = calculate_power_flow_<sym>(err_tol, max_iter, calculation_method);
        output_result(math_output, result_data, pos);
    }

    // Batch load flow calculation, propagating the results to result_data
    template <bool sym>
    BatchParameter calculate_power_flow(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                        Dataset const& result_data, ConstDataset const& update_data,
                                        Idx threading = -1) {
        return batch_calculation_<sym, &MainModelImpl::calculate_power_flow_<sym>>(
            err_tol, max_iter, calculation_method, result_data, update_data, threading);
    }

    // Single state estimation calculation, returning math output results
    template <bool sym>
    std::vector<MathOutput<sym>> calculate_state_estimation(double err_tol, Idx max_iter,
                                                            CalculationMethod calculation_method) {
        return calculate_state_estimation_<sym>(err_tol, max_iter, calculation_method);
    }

    // Single state estimation calculation, propagating the results to result_data
    template <bool sym>
    void calculate_state_estimation(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                    Dataset const& result_data, Idx pos = 0) {
        assert(construction_complete_);
        auto const math_output = calculate_state_estimation_<sym>(err_tol, max_iter, calculation_method);
        output_result(math_output, result_data, pos);
    }

    // Batch state estimation calculation, propagating the results to result_data
    template <bool sym>
    BatchParameter calculate_state_estimation(double err_tol, Idx max_iter, CalculationMethod calculation_method,
                                              Dataset const& result_data, ConstDataset const& update_data,
                                              Idx threading = -1) {
        return batch_calculation_<sym, &MainModelImpl::calculate_state_estimation_<sym>>(
            err_tol, max_iter, calculation_method, result_data, update_data, threading);
    }

    // output node
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_same_v<Node, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(components_.template citer<Component>().begin(),
                              components_.template citer<Component>().end(), comp_coup_->node.cbegin(), res_it,
                              [&math_output](Node const& node, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return node.get_null_output<sym>();
                                  }
                                  return node.get_output<sym>(math_output[math_id.group].u[math_id.pos]);
                              });
    }

    // output branch
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_base_of_v<Branch, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(components_.template citer<Component>().begin(),
                              components_.template citer<Component>().end(),
                              comp_coup_->branch.cbegin() + components_.template get_start_idx<Branch, Component>(),
                              res_it, [&math_output](Branch const& branch, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return branch.get_null_output<sym>();
                                  }
                                  return branch.get_output<sym>(math_output[math_id.group].branch[math_id.pos]);
                              });
    }

    // output branch3
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_base_of_v<Branch3, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(components_.template citer<Component>().begin(),
                              components_.template citer<Component>().end(),
                              comp_coup_->branch3.cbegin() + components_.template get_start_idx<Branch3, Component>(),
                              res_it, [&math_output](Branch3 const& branch3, Idx2DBranch3 math_id) {
                                  if (math_id.group == -1) {
                                      return branch3.get_null_output<sym>();
                                  }

                                  return branch3.get_output<sym>(math_output[math_id.group].branch[math_id.pos[0]],
                                                                 math_output[math_id.group].branch[math_id.pos[1]],
                                                                 math_output[math_id.group].branch[math_id.pos[2]]);
                              });
    }

    // output source, loadgen, shunt individually
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_same_v<Appliance, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        res_it = output_result<sym, Source>(math_output, res_it);
        res_it = output_result<sym, GenericLoadGen>(math_output, res_it);
        res_it = output_result<sym, Shunt>(math_output, res_it);
        return res_it;
    }

    // output source
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_same_v<Source, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(components_.template citer<Component>().begin(),
                              components_.template citer<Component>().end(), comp_coup_->source.cbegin(), res_it,
                              [&math_output](Source const& source, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return source.get_null_output<sym>();
                                  }
                                  return source.get_output<sym>(math_output[math_id.group].source[math_id.pos]);
                              });
    }

    // output load gen
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_base_of_v<GenericLoadGen, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(
            components_.template citer<Component>().begin(), components_.template citer<Component>().end(),
            comp_coup_->load_gen.cbegin() + components_.template get_start_idx<GenericLoadGen, Component>(), res_it,
            [&math_output](GenericLoadGen const& load_gen, Idx2D math_id) {
                if (math_id.group == -1) {
                    return load_gen.get_null_output<sym>();
                }
                return load_gen.get_output<sym>(math_output[math_id.group].load_gen[math_id.pos]);
            });
    }

    // output shunt
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_same_v<Shunt, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(components_.template citer<Component>().begin(),
                              components_.template citer<Component>().end(), comp_coup_->shunt.cbegin(), res_it,
                              [&math_output](Shunt const& shunt, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return shunt.get_null_output<sym>();
                                  }
                                  return shunt.get_output<sym>(math_output[math_id.group].shunt[math_id.pos]);
                              });
    }

    // output voltage sensor
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_base_of_v<GenericVoltageSensor, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(
            components_.template citer<Component>().begin(), components_.template citer<Component>().end(),
            comp_topo_->voltage_sensor_node_idx.cbegin() +
                components_.template get_start_idx<GenericVoltageSensor, Component>(),
            res_it, [this, &math_output](GenericVoltageSensor const& voltage_sensor, Idx const node_seq) {
                Idx2D const node_math_id = comp_coup_->node[node_seq];
                if (node_math_id.group == -1) {
                    return voltage_sensor.get_null_output<sym>();
                }
                return voltage_sensor.get_output<sym>(math_output[node_math_id.group].u[node_math_id.pos]);
            });
    }

    // output power sensor
    template <bool sym, class Component, class ResIt>
    std::enable_if_t<
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ResIt>::iterator_category> &&
            std::is_base_of_v<GenericPowerSensor, Component>,
        ResIt>
    output_result(std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        assert(construction_complete_);
        return std::transform(
            components_.template citer<Component>().begin(), components_.template citer<Component>().end(),
            comp_topo_->power_sensor_object_idx.cbegin() +
                components_.template get_start_idx<GenericPowerSensor, Component>(),
            res_it, [this, &math_output](GenericPowerSensor const& power_sensor, Idx const obj_seq) {
                auto const terminal_type = power_sensor.get_terminal_type();
                Idx2D const obj_math_id = [&]() {
                    switch (terminal_type) {
                        case MeasuredTerminalType::branch_from:
                        case MeasuredTerminalType::branch_to:
                            return comp_coup_->branch[obj_seq];
                        case MeasuredTerminalType::source:
                            return comp_coup_->source[obj_seq];
                        case MeasuredTerminalType::shunt:
                            return comp_coup_->shunt[obj_seq];
                        case MeasuredTerminalType::load:
                        case MeasuredTerminalType::generator:
                            return comp_coup_->load_gen[obj_seq];
                        // from branch3, get relevant math object branch based on the measured side
                        case MeasuredTerminalType::branch3_1:
                            return Idx2D{comp_coup_->branch3[obj_seq].group, comp_coup_->branch3[obj_seq].pos[0]};
                        case MeasuredTerminalType::branch3_2:
                            return Idx2D{comp_coup_->branch3[obj_seq].group, comp_coup_->branch3[obj_seq].pos[1]};
                        case MeasuredTerminalType::branch3_3:
                            return Idx2D{comp_coup_->branch3[obj_seq].group, comp_coup_->branch3[obj_seq].pos[2]};
                        default:
                            throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " output_result()",
                                                          terminal_type);
                    }
                }();

                if (obj_math_id.group == -1) {
                    return power_sensor.get_null_output<sym>();
                }

                switch (terminal_type) {
                    case MeasuredTerminalType::branch_from:
                    // all power sensors in branch3 are at from side in the mathematical model
                    case MeasuredTerminalType::branch3_1:
                    case MeasuredTerminalType::branch3_2:
                    case MeasuredTerminalType::branch3_3:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].branch[obj_math_id.pos].s_f);
                    case MeasuredTerminalType::branch_to:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].branch[obj_math_id.pos].s_t);
                    case MeasuredTerminalType::source:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].source[obj_math_id.pos].s);
                    case MeasuredTerminalType::shunt:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].shunt[obj_math_id.pos].s);
                    case MeasuredTerminalType::load:
                    case MeasuredTerminalType::generator:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].load_gen[obj_math_id.pos].s);
                    default:
                        throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " output_result()",
                                                      terminal_type);
                }
            });
    }

    template <bool sym>
    void output_result(std::vector<MathOutput<sym>> const& math_output, Dataset const& result_data, Idx pos = 0) {
        static constexpr std::array<OutputFunc<sym>, n_types> get_result{
            [](MainModelImpl& model, std::vector<MathOutput<sym>> const& math_output,
               DataPointer<false> const& data_ptr, Idx position) {
                auto const begin =
                    data_ptr.get_iterators<typename ComponentType::template OutputType<sym>>(position).first;
                model.output_result<sym, ComponentType>(math_output, begin);
            }...};
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

    CalculationInfo calculation_info() {
        return calculation_info_;
    }

   private:
    double system_frequency_;
    ComponentContainer components_;
    // calculation parameters
    std::shared_ptr<ComponentTopology const> comp_topo_;
    std::shared_ptr<ComponentToMathCoupling const> comp_coup_;
    // math model
    std::vector<std::shared_ptr<MathModelTopology const>> math_topology_;
    std::vector<MathSolver<true>> sym_solvers_;
    std::vector<MathSolver<false>> asym_solvers_;
    Idx n_math_solvers_{0};
    bool is_topology_up_to_date_{false};
    bool is_sym_parameter_up_to_date_{false};
    bool is_asym_parameter_up_to_date_{false};
    CalculationInfo calculation_info_;
#ifndef NDEBUG
    // construction_complete is used for debug assertions only
    bool construction_complete_{false};
#endif  // !NDEBUG

    template <bool sym>
    bool& is_parameter_up_to_date() {
        if constexpr (sym) {
            return is_sym_parameter_up_to_date_;
        }
        else {
            return is_asym_parameter_up_to_date_;
        }
    }

    template <bool sym>
    std::vector<MathSolver<sym>>& get_solvers() {
        if constexpr (sym) {
            return sym_solvers_;
        }
        else {
            return asym_solvers_;
        }
    }

    void rebuild_topology() {
        assert(construction_complete_);
        // clear old solvers
        reset_solvers();
        // get connection info
        ComponentConnections comp_conn;
        comp_conn.branch_connected.resize(comp_topo_->branch_node_idx.size());
        comp_conn.branch_phase_shift.resize(comp_topo_->branch_node_idx.size());
        comp_conn.branch3_connected.resize(comp_topo_->branch3_node_idx.size());
        comp_conn.branch3_phase_shift.resize(comp_topo_->branch3_node_idx.size());
        comp_conn.source_connected.resize(comp_topo_->source_node_idx.size());
        std::transform(components_.template citer<Branch>().begin(), components_.template citer<Branch>().end(),
                       comp_conn.branch_connected.begin(), [](Branch const& branch) {
                           return BranchConnected{branch.from_status(), branch.to_status()};
                       });
        std::transform(components_.template citer<Branch>().begin(), components_.template citer<Branch>().end(),
                       comp_conn.branch_phase_shift.begin(), [](Branch const& branch) {
                           return branch.phase_shift();
                       });
        std::transform(components_.template citer<Branch3>().begin(), components_.template citer<Branch3>().end(),
                       comp_conn.branch3_connected.begin(), [](Branch3 const& branch3) {
                           return Branch3Connected{branch3.status_1(), branch3.status_2(), branch3.status_3()};
                       });
        std::transform(components_.template citer<Branch3>().begin(), components_.template citer<Branch3>().end(),
                       comp_conn.branch3_phase_shift.begin(), [](Branch3 const& branch3) {
                           return branch3.phase_shift();
                       });
        std::transform(components_.template citer<Source>().begin(), components_.template citer<Source>().end(),
                       comp_conn.source_connected.begin(), [](Source const& source) {
                           return source.status();
                       });
        // re build
        Topology topology{*comp_topo_, comp_conn};
        std::tie(math_topology_, comp_coup_) = topology.build_topology();
        n_math_solvers_ = (Idx)math_topology_.size();
        is_topology_up_to_date_ = true;
        is_sym_parameter_up_to_date_ = false;
        is_asym_parameter_up_to_date_ = false;
    }

    template <bool sym>
    std::vector<MathModelParam<sym>> get_math_param() {
        std::vector<MathModelParam<sym>> math_param(n_math_solvers_);
        for (Idx i = 0; i != n_math_solvers_; ++i) {
            math_param[i].branch_param.resize(math_topology_[i]->n_branch());
            math_param[i].shunt_param.resize(math_topology_[i]->n_shunt());
            math_param[i].source_param.resize(math_topology_[i]->n_source());
        }
        // loop all branch
        for (Idx i = 0; i != (Idx)comp_topo_->branch_node_idx.size(); ++i) {
            Idx2D const math_idx = comp_coup_->branch[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].branch_param[math_idx.pos] =
                components_.template get_item_by_seq<Branch>(i).template calc_param<sym>();
        }
        // loop all branch3
        for (Idx i = 0; i != (Idx)comp_topo_->branch3_node_idx.size(); ++i) {
            Idx2DBranch3 const math_idx = comp_coup_->branch3[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters, branch3 param consists of three branch parameters
            auto const branch3_param = components_.template get_item_by_seq<Branch3>(i).template calc_param<sym>();
            for (size_t branch2 = 0; branch2 < 3; ++branch2) {
                math_param[math_idx.group].branch_param[math_idx.pos[branch2]] = branch3_param[branch2];
            }
        }
        // loop all shunt
        for (Idx i = 0; i != (Idx)comp_topo_->shunt_node_idx.size(); ++i) {
            Idx2D const math_idx = comp_coup_->shunt[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].shunt_param[math_idx.pos] =
                components_.template get_item_by_seq<Shunt>(i).template calc_param<sym>();
        }
        // loop all source
        for (Idx i = 0; i != (Idx)comp_topo_->source_node_idx.size(); ++i) {
            Idx2D const math_idx = comp_coup_->source[i];
            if (math_idx.group == -1) {
                continue;
            }
            // assign parameters
            math_param[math_idx.group].source_param[math_idx.pos] =
                components_.template get_item_by_seq<Source>(i).template math_param<sym>();
        }
        return math_param;
    }

    static constexpr auto include_all = [](Idx) {
        return true;
    };

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
     *       [&](Idx i) { return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::source; }
     *
     *  2. Find the original component in the topology and retrieve its calculation parameters.
     *
     *  3. Fill the calculation parameters of the right math model.
     *
     *
     *  @tparam sym
     *      Use symmetric vs Asymmetric calculation parameters
     *
     *  @tparam CalcStructOut
     *      The struct (soa) for the desired calculation (e.g. PowerFlowInput<sym> or StateEstimationInput<sym>)
     *
     * @tparam CalcParamOut
     *      The data type for the desired calculation for the given ComponentIn (e.g. SourceCalcParam<sym> or
     *      SensorCalcParam<sym>).
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
     *      The vector of component math indices to consider (e.g. comp_coup_->source).
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
    template <bool sym, class CalcStructOut, typename CalcParamOut,
              std::vector<CalcParamOut>(CalcStructOut::*comp_vect), class ComponentIn,
              typename PredicateIn = decltype(include_all)>
    void prepare_input(std::vector<Idx2D> const& components, std::vector<CalcStructOut>& calc_input,
                       PredicateIn include = include_all) {
        for (Idx i = 0, n = (Idx)components.size(); i != n; ++i) {
            if (include(i)) {
                Idx2D const math_idx = components[i];
                if (math_idx.group != -1) {
                    CalcParamOut const calc_param =
                        components_.template get_item_by_seq<ComponentIn>(i).template calc_param<sym>();
                    CalcStructOut& math_model_input = calc_input[math_idx.group];
                    std::vector<CalcParamOut>& math_model_input_vect = math_model_input.*comp_vect;
                    math_model_input_vect[math_idx.pos] = calc_param;
                }
            }
        }
    }

    template <bool sym, IntSVector(StateEstimationInput<sym>::*component), class Component>
    void prepare_input_status(std::vector<Idx2D> const& objects, std::vector<StateEstimationInput<sym>>& input) {
        for (Idx i = 0, n = (Idx)objects.size(); i != n; ++i) {
            Idx2D const math_idx = objects[i];
            if (math_idx.group == -1) {
                continue;
            }
            (input[math_idx.group].*component)[math_idx.pos] =
                components_.template get_item_by_seq<Component>(i).status();
        }
    }

    template <bool sym>
    std::vector<PowerFlowInput<sym>> prepare_power_flow_input() {
        assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());
        std::vector<PowerFlowInput<sym>> pf_input(n_math_solvers_);
        for (Idx i = 0; i != n_math_solvers_; ++i) {
            pf_input[i].s_injection.resize(math_topology_[i]->n_load_gen());
            pf_input[i].source.resize(math_topology_[i]->n_source());
        }
        prepare_input<sym, PowerFlowInput<sym>, DoubleComplex, &PowerFlowInput<sym>::source, Source>(comp_coup_->source,
                                                                                                     pf_input);

        prepare_input<sym, PowerFlowInput<sym>, ComplexValue<sym>, &PowerFlowInput<sym>::s_injection, GenericLoadGen>(
            comp_coup_->load_gen, pf_input);

        return pf_input;
    }

    template <bool sym>
    std::vector<StateEstimationInput<sym>> prepare_state_estimation_input() {
        assert(is_topology_up_to_date_ && is_parameter_up_to_date<sym>());

        std::vector<StateEstimationInput<sym>> se_input(n_math_solvers_);

        for (Idx i = 0; i != n_math_solvers_; ++i) {
            se_input[i].shunt_status.resize(math_topology_[i]->n_shunt());
            se_input[i].load_gen_status.resize(math_topology_[i]->n_load_gen());
            se_input[i].source_status.resize(math_topology_[i]->n_source());
            se_input[i].measured_voltage.resize(math_topology_[i]->n_voltage_sensor());
            se_input[i].measured_source_power.resize(math_topology_[i]->n_source_power_sensor());
            se_input[i].measured_load_gen_power.resize(math_topology_[i]->n_load_gen_power_sensor());
            se_input[i].measured_shunt_power.resize(math_topology_[i]->n_shunt_power_power_sensor());
            se_input[i].measured_branch_from_power.resize(math_topology_[i]->n_branch_from_power_sensor());
            se_input[i].measured_branch_to_power.resize(math_topology_[i]->n_branch_to_power_sensor());
        }

        prepare_input_status<sym, &StateEstimationInput<sym>::shunt_status, Shunt>(comp_coup_->shunt, se_input);
        prepare_input_status<sym, &StateEstimationInput<sym>::load_gen_status, GenericLoadGen>(comp_coup_->load_gen,
                                                                                               se_input);
        prepare_input_status<sym, &StateEstimationInput<sym>::source_status, Source>(comp_coup_->source, se_input);

        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_voltage, GenericVoltageSensor>(comp_coup_->voltage_sensor,
                                                                                          se_input);
        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_source_power, GenericPowerSensor>(
            comp_coup_->power_sensor, se_input, [&](Idx i) {
                return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::source;
            });
        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_load_gen_power, GenericPowerSensor>(
            comp_coup_->power_sensor, se_input, [&](Idx i) {
                return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::load ||
                       comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::generator;
            });
        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_shunt_power, GenericPowerSensor>(
            comp_coup_->power_sensor, se_input, [&](Idx i) {
                return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::shunt;
            });
        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_branch_from_power, GenericPowerSensor>(
            comp_coup_->power_sensor, se_input, [&](Idx i) {
                return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::branch_from ||
                       // all branch3 sensors are at from side in the mathemtical model
                       comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_1 ||
                       comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_2 ||
                       comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::branch3_3;
            });
        prepare_input<sym, StateEstimationInput<sym>, SensorCalcParam<sym>,
                      &StateEstimationInput<sym>::measured_branch_to_power, GenericPowerSensor>(
            comp_coup_->power_sensor, se_input, [&](Idx i) {
                return comp_topo_->power_sensor_terminal_type[i] == MeasuredTerminalType::branch_to;
            });

        return se_input;
    }

    template <bool sym>
    void prepare_solvers() {
        std::vector<MathSolver<sym>>& solvers = get_solvers<sym>();
        // also get the vector of other solvers (sym -> asym, or asym -> sym)
        std::vector<MathSolver<!sym>>& other_solvers = get_solvers<!sym>();
        // rebuild topology if needed
        if (!is_topology_up_to_date_) {
            rebuild_topology();
        }
        // if solvers do not exist, build them
        if (n_math_solvers_ != (Idx)solvers.size()) {
            // check if other (sym/asym) solver exist
            bool const other_solver_exist = (n_math_solvers_ == (Idx)other_solvers.size());
            assert(solvers.size() == 0);
            solvers.reserve(n_math_solvers_);
            // get param, will be consumed
            std::vector<MathModelParam<sym>> math_params = get_math_param<sym>();
            // loop to build
            for (Idx i = 0; i != n_math_solvers_; ++i) {
                // if other solver exists, construct from existing y bus struct
                if (other_solver_exist) {
                    solvers.emplace_back(math_topology_[i],
                                         std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])),
                                         other_solvers[i].shared_y_bus_struct());
                }
                // else construct from scratch
                else {
                    solvers.emplace_back(math_topology_[i],
                                         std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])));
                }
            }
        }
        // if parameters are not up to date, update them
        else if (!is_parameter_up_to_date<sym>()) {
            // get param, will be consumed
            std::vector<MathModelParam<sym>> math_params = get_math_param<sym>();
            for (Idx i = 0; i != n_math_solvers_; ++i) {
                // move parameter into a shared ownership for the math solver
                solvers[i].update_value(std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])));
            }
        }
        // else do nothing, set everything up to date
        is_parameter_up_to_date<sym>() = true;
    }
};

using MainModel =
    MainModelImpl<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, GenericLoadGen, GenericLoad,
                                        GenericGenerator, GenericPowerSensor, GenericVoltageSensor>,
                  AllComponents>;

}  // namespace power_grid_model

#endif
