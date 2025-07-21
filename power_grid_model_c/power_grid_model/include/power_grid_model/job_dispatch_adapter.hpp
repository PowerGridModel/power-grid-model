// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch adapter class

#include "auxiliary/dataset.hpp"
#include "job_dispatch_interface.hpp"
#include "main_model_fwd.hpp"

#include "main_core/calculation_info.hpp"
#include "main_core/update.hpp"

namespace power_grid_model {

template <class MainModel, class... ComponentType>
class JobDispatchAdapter : public JobDispatchInterface<JobDispatchAdapter<MainModel>> {
  public:
    JobDispatchAdapter(std::reference_wrapper<MainModel> model) : model_{std::move(model)} {}

    JobDispatchAdapter(JobDispatchAdapter const& other)
        : model_copy_{std::make_unique<MainModel>(other.model_.get())}, model_{std::ref(*model_copy_)} {}

    JobDispatchAdapter& operator=(JobDispatchAdapter const& other) {
        if (this != &other) {
            model_copy_ = std::make_unique<MainModel>(other.model_.get());
            model_ = std::ref(*model_copy_);
        }
        return *this;
    }

  private:
    static constexpr Idx ignore_output{-1};

    friend class JobDispatchInterface<JobDispatchAdapter>;
    std::unique_ptr<MainModel> model_copy_;
    std::reference_wrapper<MainModel> model_;

    main_core::utils::SequenceIdx<ComponentType...> current_scenario_sequence_cache_{};
    std::shared_ptr<const power_grid_model::main_core::utils::ComponentFlags<ComponentType...>> components_to_update_;
    std::shared_ptr<const power_grid_model::main_core::update::independence::UpdateIndependence<ComponentType...>>
        update_independence_;
    std::shared_ptr<const main_core::utils::SequenceIdx<ComponentType...>> all_scenarios_sequence_;
    main_core::utils::ComponentFlags<ComponentType...> independence_flags_{};

    // TODO(figueroa1395): Keep calculation_fn at the adapter level only

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    void calculate_impl(Calculate&& calculation_fn, MutableDataset const& result_data, Idx scenario_idx) const {
        return std::forward<Calculate>(calculation_fn)(model_.get(), result_data, scenario_idx);
    }

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    void cache_calculate_impl(Calculate&& calculation_fn) const {
        return std::forward<Calculate>(calculation_fn)(model_.get(),
                                                       {
                                                           false,
                                                           1,
                                                           "sym_output",
                                                           model_.get().meta_data(),
                                                       },
                                                       ignore_output);
    }

    void prepare_sub_batch_calculation_impl(ConstDataset const& update_data) {
        // cache component update order where possible.
        // the order for a cacheable (independent) component by definition is the same across all scenarios
        components_to_update_ =
            std::make_shared<const power_grid_model::main_core::utils::ComponentFlags<ComponentType...>>(
                model_.get().get_components_to_update(update_data));
        update_independence_ = std::make_shared<
            const power_grid_model::main_core::update::independence::UpdateIndependence<ComponentType...>>(
            main_core::update::independence::check_update_independence<ComponentType...>(model_.get().state(),
                                                                                         update_data));
        assert(components_to_update_ != nullptr);
        assert(update_independence_ != nullptr);
        all_scenarios_sequence_ = std::make_shared<const main_core::utils::SequenceIdx<ComponentType...>>(
            main_core::update::get_all_sequence_idx_map<ComponentType...>(
                model_.get().state(), update_data, 0, *components_to_update_, *update_independence_, false));

        std::ranges::transform(*update_independence_, independence_flags_.begin(),
                               [](auto const& comp) { return comp.is_independent(); });
    }

    void setup_impl(ConstDataset const& update_data, Idx scenario_idx) {
        assert(components_to_update_ != nullptr);
        assert(update_independence_ != nullptr);
        current_scenario_sequence_cache_ = main_core::update::get_all_sequence_idx_map<ComponentType...>(
            model_.get().state(), update_data, scenario_idx, *components_to_update_, *update_independence_, true);
        model_.get().template update_components<cached_update_t>(update_data, scenario_idx,
                                                                 get_current_scenario_sequence_view_());
    }

    void winddown_impl() {
        model_.get().restore_components(get_current_scenario_sequence_view_());
        std::ranges::for_each(current_scenario_sequence_cache_, [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
    }

    CalculationInfo get_calculation_info_impl() const { return model_.get().calculation_info(); }

    void merge_calculation_infos_impl(std::vector<CalculationInfo> const& infos) {
        model_.get().set_calculation_info(main_core::merge_calculation_info(infos));
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
