// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Derived> class BatchDispatchInterface {
  public:
    // a concept for ResultDataset can be added if needed or MutableDataset can be used directly
    template <typename Calculate, typename ResultDataset>
        requires requires(Derived& adapter, Calculate&& calculation_fn, ResultDataset const& result_data, Idx pos) {
            { adapter.calculate_impl(std::forward<Calculate>(calculation_fn), result_data, pos) } -> std::same_as<void>;
        }
    void calculate(Calculate&& calculation_fn, ResultDataset const& result_data, Idx pos = 0) {
        return static_cast<Derived*>(this)->calculate_impl(std::forward<Calculate>(calculation_fn), result_data, pos);
    }

    template <typename Calculate>
        requires requires(Derived& adapter, Calculate&& calculation_fn) {
            { adapter.cache_calculate_impl(std::forward<Calculate>(calculation_fn)) } -> std::same_as<void>;
        }
    void cache_calculate(Calculate&& calculation_fn) {
        return static_cast<Derived*>(this)->cache_calculate_impl(std::forward<Calculate>(calculation_fn));
    }
};

} // namespace power_grid_model
