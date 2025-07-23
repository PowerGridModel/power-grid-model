// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch interface class

#include <concepts>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Adapter> class JobDispatchInterface {
  public:
    template <typename Calculate, typename ResultDataset>
        requires requires(Adapter& adapter, Calculate&& calculation_fn, ResultDataset const& result_data, Idx pos) {
            { adapter.calculate_impl(std::forward<Calculate>(calculation_fn), result_data, pos) } -> std::same_as<void>;
        }
    void calculate(Calculate&& calculation_fn, ResultDataset const& result_data, Idx pos = 0) {
        return static_cast<Adapter*>(this)->calculate_impl(std::forward<Calculate>(calculation_fn), result_data, pos);
    }

    template <typename Calculate>
        requires requires(Adapter& adapter, Calculate&& calculation_fn) {
            { adapter.cache_calculate_impl(std::forward<Calculate>(calculation_fn)) } -> std::same_as<void>;
        }
    void cache_calculate(Calculate&& calculation_fn) {
        return static_cast<Adapter*>(this)->cache_calculate_impl(std::forward<Calculate>(calculation_fn));
    }

    template <typename UpdateDataset> void prepare_job(UpdateDataset const& update_data) {
        return static_cast<Adapter*>(this)->prepare_job_impl(update_data);
    }

    template <typename UpdateDataset> void setup(UpdateDataset const& update_data, Idx scenario_idx) {
        return static_cast<Adapter*>(this)->setup_impl(update_data, scenario_idx);
    }

    void winddown() { return static_cast<Adapter*>(this)->winddown_impl(); }

    CalculationInfo get_calculation_info() const {
        return static_cast<const Adapter*>(this)->get_calculation_info_impl();
    }

    void merge_calculation_infos(std::vector<CalculationInfo> const& info) {
        static_cast<Adapter*>(this)->merge_calculation_infos_impl(info);
    }
};

} // namespace power_grid_model
