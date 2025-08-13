// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Adapter that connects the JobDispatch to the MainModelImpl

#include "auxiliary/dataset.hpp"
#include "job_interface.hpp"
#include "main_model_fwd.hpp"

#include "main_core/calculation_info.hpp"
#include "main_core/update.hpp"

namespace power_grid_model {

template <class MainModel, typename... ComponentType> class JobDispatchAdapter;

template <class MainModel, class... ComponentType>
class JobDispatchAdapter<MainModel, ComponentList<ComponentType...>>
    : public JobDispatchInterface<JobDispatchAdapter<MainModel, ComponentList<ComponentType...>>> {
  public:
    JobDispatchAdapter(std::reference_wrapper<MainModel> model) : model_{std::move(model)} {}
    JobDispatchAdapter(JobDispatchAdapter const& other)
        : model_copy_{std::make_unique<MainModel>(other.model_.get())},
          model_{std::ref(*model_copy_)},
          components_to_update_{other.components_to_update_},
          update_independence_{other.update_independence_},
          independence_flags_{other.independence_flags_},
          all_scenarios_sequence_{other.all_scenarios_sequence_} {}
    JobDispatchAdapter& operator=(JobDispatchAdapter const& other) {
        if (this != &other) {
            model_copy_ = std::make_unique<MainModel>(other.model_.get());
            model_ = std::ref(*model_copy_);
            components_to_update_ = other.components_to_update_;
            update_independence_ = other.update_independence_;
            independence_flags_ = other.independence_flags_;
            all_scenarios_sequence_ = other.all_scenarios_sequence_;
        }
        return *this;
    }
    JobDispatchAdapter(JobDispatchAdapter&& other) noexcept
        : model_copy_{std::move(other.model_copy_)},
          model_{model_copy_ ? std::ref(*model_copy_) : std::move(other.model_)},
          components_to_update_{std::move(other.components_to_update_)},
          update_independence_{std::move(other.update_independence_)},
          independence_flags_{std::move(other.independence_flags_)},
          all_scenarios_sequence_{std::move(other.all_scenarios_sequence_)} {}
    JobDispatchAdapter& operator=(JobDispatchAdapter&& other) noexcept {
        if (this != &other) {
            model_copy_ = std::move(other.model_copy_);
            model_ = model_copy_ ? std::ref(*model_copy_) : std::move(other.model_);
            components_to_update_ = std::move(other.components_to_update_);
            update_independence_ = std::move(other.update_independence_);
            independence_flags_ = std::move(other.independence_flags_);
            all_scenarios_sequence_ = std::move(other.all_scenarios_sequence_);
        }
        return *this;
    }
    ~JobDispatchAdapter() { model_copy_.reset(); }

  private:
    // Grant the CRTP base (JobDispatchInterface<JobDispatchAdapter>) access to
    // JobDispatchAdapter's private members. This allows the base class template
    // to call derived-class implementation details as part of the CRTP pattern.
    friend class JobDispatchInterface<JobDispatchAdapter>;

    static constexpr Idx ignore_output{-1};

    std::unique_ptr<MainModel> model_copy_;
    std::reference_wrapper<MainModel> model_;

    main_core::utils::ComponentFlags<ComponentType...> components_to_update_{};
    main_core::update::independence::UpdateIndependence<ComponentType...> update_independence_{};
    main_core::utils::ComponentFlags<ComponentType...> independence_flags_{};
    std::shared_ptr<main_core::utils::SequenceIdx<ComponentType...>> all_scenarios_sequence_;
    // current_scenario_sequence_cache_ is calculated per scenario, so it is excluded from the constructors.
    main_core::utils::SequenceIdx<ComponentType...> current_scenario_sequence_cache_{};

    std::mutex calculation_info_mutex_;

    // TODO(figueroa1395): Keep calculation_fn at the adapter level only
    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, bool>
    void calculate_impl(Calculate&& calculation_fn, MutableDataset const& result_data, Idx scenario_idx) const {
        std::forward<Calculate>(calculation_fn)(model_.get(), result_data.get_individual_scenario(scenario_idx), false);
    }

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    void cache_calculate_impl(Calculate&& calculation_fn) const {
        // calculate once to cache topology, ignore results, all math solvers are initialized
        try {
            std::forward<Calculate>(calculation_fn)(model_.get(),
                                                    {
                                                        false,
                                                        1,
                                                        "sym_output",
                                                        model_.get().meta_data(),
                                                    },
                                                    true);
        } catch (SparseMatrixError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        } catch (NotObservableError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        }
    }

    void prepare_job_dispatch_impl(ConstDataset const& update_data) {
        // cache component update order where possible.
        // the order for a cacheable (independent) component by definition is the same across all scenarios
        components_to_update_ = model_.get().get_components_to_update(update_data);
        update_independence_ = main_core::update::independence::check_update_independence<ComponentType...>(
            model_.get().state(), update_data);
        std::ranges::transform(update_independence_, independence_flags_.begin(),
                               [](auto const& comp) { return comp.is_independent(); });
        all_scenarios_sequence_ = std::make_shared<main_core::utils::SequenceIdx<ComponentType...>>(
            main_core::update::get_all_sequence_idx_map<ComponentType...>(
                model_.get().state(), update_data, 0, components_to_update_, update_independence_, false));
    }

    void setup_impl(ConstDataset const& update_data, Idx scenario_idx) {
        current_scenario_sequence_cache_ = main_core::update::get_all_sequence_idx_map<ComponentType...>(
            model_.get().state(), update_data, scenario_idx, components_to_update_, update_independence_, true);
        auto const current_scenario_sequence = get_current_scenario_sequence_view_();
        model_.get().template update_components<cached_update_t>(update_data, scenario_idx, current_scenario_sequence);
    }

    void winddown_impl() {
        model_.get().restore_components(get_current_scenario_sequence_view_());
        std::ranges::for_each(current_scenario_sequence_cache_, [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
    }

    CalculationInfo get_calculation_info_impl() const { return model_.get().calculation_info(); }

    void thread_safe_add_calculation_info_impl(CalculationInfo const& info) {
        std::lock_guard const lock{calculation_info_mutex_};
        model_.get().merge_calculation_info(info);
    }

    auto get_current_scenario_sequence_view_() const {
        return main_core::utils::run_functor_with_all_types_return_array<ComponentType...>([this]<typename CT>() {
            constexpr auto comp_idx = main_core::utils::index_of_component<CT, ComponentType...>;
            if (std::get<comp_idx>(independence_flags_)) {
                return std::span<Idx2D const>{std::get<comp_idx>(*all_scenarios_sequence_)};
            }
            return std::span<Idx2D const>{std::get<comp_idx>(current_scenario_sequence_cache_)};
        });
    }
};
} // namespace power_grid_model
