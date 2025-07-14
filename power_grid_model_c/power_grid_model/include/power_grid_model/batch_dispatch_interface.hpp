// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch interface class

#include <concepts>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Adapter> class BatchDispatchInterface {
  public:
    // a concept for ResultDataset can be added if needed or MutableDataset can be used directly
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

    CalculationInfo get_calculation_info() const { return static_cast<Adapter*>(this)->get_calculation_info_impl(); }

    void set_calculation_info(CalculationInfo const& info) {
        static_cast<Adapter*>(this)->set_calculation_info_impl(info);
    }
};

} // namespace power_grid_model
