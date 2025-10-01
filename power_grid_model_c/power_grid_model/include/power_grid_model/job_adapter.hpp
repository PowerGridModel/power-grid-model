// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Adapter that connects the JobDispatch to the MainModelImpl

#include "job_interface.hpp"
#include "main_model_fwd.hpp"

#include "auxiliary/dataset.hpp"
#include "main_core/update.hpp"

namespace power_grid_model {

template <class MainModel> class JobAdapter;

template <class MainModel> class JobAdapter : public JobInterface {
  public:
    using ModelType = typename MainModel::ImplType;

    JobAdapter(std::reference_wrapper<MainModel> model_reference,
               std::reference_wrapper<MainModelOptions const> options)
        : model_reference_{model_reference}, options_{options} {}
    JobAdapter(JobAdapter const& other)
        : model_copy_{std::make_unique<MainModel>(other.model_reference_.get())},
          model_reference_{std::ref(*model_copy_)},
          options_{std::ref(other.options_)},
          components_to_update_{other.components_to_update_},
          update_independence_{other.update_independence_},
          independence_flags_{other.independence_flags_},
          all_scenarios_sequence_{other.all_scenarios_sequence_} {}
    JobAdapter& operator=(JobAdapter const& other) {
        if (this != &other) {
            model_copy_ = std::make_unique<MainModel>(other.model_reference_.get());
            model_reference_ = std::ref(*model_copy_);
            options_ = std::ref(other.options_);
            components_to_update_ = other.components_to_update_;
            update_independence_ = other.update_independence_;
            independence_flags_ = other.independence_flags_;
            all_scenarios_sequence_ = other.all_scenarios_sequence_;
        }
        return *this;
    }
    JobAdapter(JobAdapter&& other) noexcept
        : model_copy_{std::move(other.model_copy_)},
          model_reference_{model_copy_ ? std::ref(*model_copy_) : std::move(other.model_reference_)},
          options_{other.options_},
          components_to_update_{std::move(other.components_to_update_)},
          update_independence_{std::move(other.update_independence_)},
          independence_flags_{std::move(other.independence_flags_)},
          all_scenarios_sequence_{std::move(other.all_scenarios_sequence_)} {}
    JobAdapter& operator=(JobAdapter&& other) noexcept {
        if (this != &other) {
            model_copy_ = std::move(other.model_copy_);
            model_reference_ = model_copy_ ? std::ref(*model_copy_) : std::move(other.model_reference_);
            options_ = other.options_;
            components_to_update_ = std::move(other.components_to_update_);
            update_independence_ = std::move(other.update_independence_);
            independence_flags_ = std::move(other.independence_flags_);
            all_scenarios_sequence_ = std::move(other.all_scenarios_sequence_);
        }
        return *this;
    }
    ~JobAdapter() { model_copy_.reset(); }

  private:
    friend class JobInterface;

    std::unique_ptr<MainModel> model_copy_;
    std::reference_wrapper<MainModel> model_reference_;
    std::reference_wrapper<MainModelOptions const> options_;

    typename ModelType::ComponentFlags components_to_update_{};
    typename ModelType::UpdateIndependence update_independence_{};
    typename ModelType::ComponentFlags independence_flags_{};
    std::shared_ptr<typename ModelType::SequenceIdx> all_scenarios_sequence_;
    // current_scenario_sequence_cache_ is calculated per scenario, so it is excluded from the constructors.
    typename ModelType::SequenceIdx current_scenario_sequence_cache_{};

    void calculate_impl(MutableDataset const& result_data, Idx scenario_idx, Logger& logger) const {
        MainModel::calculator(options_.get(), model_reference_.get(), result_data.get_individual_scenario(scenario_idx),
                              false, logger);
    }

    void cache_calculate_impl(Logger& logger) const {
        // calculate once to cache topology, ignore results, all math solvers are initialized
        try {
            MainModel::calculator(options_.get(), model_reference_.get(),
                                  {
                                      false,
                                      1,
                                      "sym_output",
                                      model_reference_.get().meta_data(),
                                  },
                                  true, logger);
        } catch (SparseMatrixError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        } catch (NotObservableError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        }
    }

    void prepare_job_dispatch_impl(ConstDataset const& update_data) {
        // cache component update order where possible.
        // the order for a cacheable (independent) component by definition is the same across all scenarios
        components_to_update_ = model_reference_.get().get_components_to_update(update_data);
        update_independence_ = main_core::update::independence::check_update_independence<ModelType>(
            model_reference_.get().state().components, update_data);
        std::ranges::transform(update_independence_, independence_flags_.begin(),
                               [](auto const& comp) { return comp.is_independent(); });
        all_scenarios_sequence_ =
            std::make_shared<typename ModelType::SequenceIdx>(main_core::update::get_all_sequence_idx_map<ModelType>(
                model_reference_.get().state().components, update_data, 0, components_to_update_, update_independence_,
                false));
    }

    void setup_impl(ConstDataset const& update_data, Idx scenario_idx) {
        current_scenario_sequence_cache_ = main_core::update::get_all_sequence_idx_map<ModelType>(
            model_reference_.get().state().components, update_data, scenario_idx, components_to_update_,
            update_independence_, true);
        auto const current_scenario_sequence = get_current_scenario_sequence_view_();
        model_reference_.get().template update_components<cached_update_t>(update_data, scenario_idx,
                                                                           current_scenario_sequence);
    }

    void winddown_impl() {
        model_reference_.get().restore_components(get_current_scenario_sequence_view_());
        std::ranges::for_each(current_scenario_sequence_cache_, [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
    }

    auto get_current_scenario_sequence_view_() const {
        return ModelType::run_functor_with_all_component_types_return_array([this]<typename CT>() {
            constexpr auto comp_idx = ModelType::template index_of_component<CT>;
            if (std::get<comp_idx>(independence_flags_)) {
                return std::span<Idx2D const>{std::get<comp_idx>(*all_scenarios_sequence_)};
            }
            return std::span<Idx2D const>{std::get<comp_idx>(current_scenario_sequence_cache_)};
        });
    }
};
} // namespace power_grid_model
