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
        requires std::invocable<Calculate> &&
                 requires(Derived& adapter, Calculate&& calculation_fn, ResultDataset const& result_data) {
                     {
                         adapter.calculate_impl(std::forward<Calculate>(calculation_fn), result_data)
                     } -> std::same_as<BatchParameter>; // maybe it returns something different, just check the type on
                                                        // main_model_impl
                 }
    BatchParameter calculate(Calculate&& calculation_fn, ResultDataset const& result_data) {
        return static_cast<Derived*>(this)->calculate_impl(std::forward<Calculate>(calculation_fn), result_data);
    }
};

class BatchDispatchAdapter : BatchDispatchInterface<BatchDispatchAdapter> {
  public:
    BatchDispatchAdapter(MainModelImpl& model) : model_(model) {}

  private:
    friend class BatchDispatchInterface;
    MainModelImpl& model_;
    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModelImpl&, MutableDataset const&, Idx>
    BatchParameter
    calculate_impl(Calculate&& calculation_fn, // change this Calculate to a differnt one cause is confusing from above
                   MutableDataset const& result_data) {
        return std::forward<Calculate>(calculation_fn)(model_, result_data, 0);
    }
};

// Then the BatchDispatch class takes an interface as a a function argument for when calling thebatch dispatch, but
// pass the adapter as the interface, then make each call directly inside BatchDispatch
} // namespace power_grid_model
