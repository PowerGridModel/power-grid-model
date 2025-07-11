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
    template <typename Calculate, typename ResultDataset> // a concept for ResultDataset can be added if needed
        requires requires(Derived& adapter, Calculate&& calculation_fn, ResultDataset const& result_data) {
            { adapter.calculate_1_impl(std::forward<Calculate>(calculation_fn), result_data) } -> std::same_as<void>;
        }
    void calculate_1(Calculate&& calculation_fn, ResultDataset const& result_data) {
        return static_cast<Derived*>(this)->calculate_1_impl(std::forward<Calculate>(calculation_fn), result_data);
    }
};

} // namespace power_grid_model
