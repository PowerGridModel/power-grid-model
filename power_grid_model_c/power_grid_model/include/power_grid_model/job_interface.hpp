// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch interface class

#include "common/calculation_info.hpp"
#include "common/common.hpp"

#include <concepts>
#include <mutex>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Adapter> class JobDispatchInterface {
  public:
    template <typename Calculate, typename ResultDataset>
        requires requires(Adapter& adapter, Calculate&& calculation_fn, ResultDataset const& result_data,
                          Idx scenario_idx) {
            {
                adapter.calculate_impl(std::forward<Calculate>(calculation_fn), result_data, scenario_idx)
            } -> std::same_as<void>;
        }
    void calculate(Calculate&& calculation_fn, ResultDataset const& result_data, Idx scenario_idx = 0) {
        return static_cast<Adapter*>(this)->calculate_impl(std::forward<Calculate>(calculation_fn), result_data,
                                                           scenario_idx);
    }

    template <typename Calculate>
        requires requires(Adapter& adapter, Calculate&& calculation_fn) {
            { adapter.cache_calculate_impl(std::forward<Calculate>(calculation_fn)) } -> std::same_as<void>;
        }
    void cache_calculate(Calculate&& calculation_fn) {
        return static_cast<Adapter*>(this)->cache_calculate_impl(std::forward<Calculate>(calculation_fn));
    }

    template <typename UpdateDataset> void prepare_job_dispatch(UpdateDataset const& update_data) {
        return static_cast<Adapter*>(this)->prepare_job_dispatch_impl(update_data);
    }

    template <typename UpdateDataset> void setup(UpdateDataset const& update_data, Idx scenario_idx) {
        return static_cast<Adapter*>(this)->setup_impl(update_data, scenario_idx);
    }

    void winddown() { return static_cast<Adapter*>(this)->winddown_impl(); }

    CalculationInfo get_calculation_info() const {
        return static_cast<const Adapter*>(this)->get_calculation_info_impl();
    }

    void thread_safe_add_calculation_info(CalculationInfo const& info) {
        static_cast<Adapter*>(this)->thread_safe_add_calculation_info_impl(info);
    }

  protected:
    // Protected & defaulted special members — CRTP: only the derived can create/copy/move this base
    JobDispatchInterface() = default;
    JobDispatchInterface(const JobDispatchInterface& /*other*/) = default;
    JobDispatchInterface& operator=(const JobDispatchInterface& /*other*/) = default;
    JobDispatchInterface(JobDispatchInterface&& /*other*/) noexcept = default;
    JobDispatchInterface& operator=(JobDispatchInterface&& /*other*/) noexcept = default;
    ~JobDispatchInterface() = default;
};

} // namespace power_grid_model
