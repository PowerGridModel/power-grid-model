// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch interface class

#include "common/calculation_info.hpp"
#include "common/common.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Adapter> class JobDispatchInterface {
  public:
    template <typename ResultDataset>
    void calculate(ResultDataset const& result_data, Idx pos = 0)
        requires requires(Adapter& adapter) {
            { adapter.calculate_impl(result_data, pos) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->calculate_impl(result_data, pos);
    }

    void cache_calculate()
        requires requires(Adapter& adapter) {
            { adapter.cache_calculate_impl() } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->cache_calculate_impl();
    }

    template <typename UpdateDataset>
    void prepare_job_dispatch(UpdateDataset const& update_data)
        requires requires(Adapter& adapter) {
            { adapter.prepare_job_dispatch_impl(update_data) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->prepare_job_dispatch_impl(update_data);
    }

    template <typename UpdateDataset>
    void setup(UpdateDataset const& update_data, Idx scenario_idx)
        requires requires(Adapter& adapter) {
            { adapter.setup_impl(update_data, scenario_idx) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->setup_impl(update_data, scenario_idx);
    }

    void winddown()
        requires requires(Adapter& adapter) {
            { adapter.winddown_impl() } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->winddown_impl();
    }

    CalculationInfo get_calculation_info() const
        requires requires(Adapter& adapter) {
            { adapter.get_calculation_info_impl() } -> std::same_as<CalculationInfo>;
        }
    {
        return static_cast<const Adapter*>(this)->get_calculation_info_impl();
    }

    void thread_safe_add_calculation_info(CalculationInfo const& info)
        requires requires(Adapter& adapter) {
            { adapter.thread_safe_add_calculation_info_impl(info) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->thread_safe_add_calculation_info_impl(info);
    }
};

} // namespace power_grid_model
