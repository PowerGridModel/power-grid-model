// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch adapter class

#include "batch_dispatch_interface.hpp"
#include "main_core/calculation_info.hpp"
#include "main_core/update.hpp"

namespace power_grid_model {

template <class MainModel> class BatchDispatchAdapter : public BatchDispatchInterface<BatchDispatchAdapter<MainModel>> {
  public:
    BatchDispatchAdapter(std::reference_wrapper<MainModel> model) : model_{std::move(model)} {} //, owns_model_copy_{false} {}

    BatchDispatchAdapter(BatchDispatchAdapter const& other)
        : model_copy_{new MainModel{other.model_.get()}}, model_{std::ref(*model_copy_)}, owns_model_copy_{true} {}

    BatchDispatchAdapter& operator=(BatchDispatchAdapter const& other) {
        if (this != &other) {
            model_copy_ = std::make_unique<MainModel>(other.model_.get());
            model_ = std::ref(*model_copy_);
            owns_model_copy_ = true;
        }
        return *this;
    }

  private:
    static constexpr Idx ignore_output{-1};

    friend class BatchDispatchInterface<BatchDispatchAdapter>;
    std::unique_ptr<MainModel> model_copy_;
    std::reference_wrapper<MainModel> model_;
    bool owns_model_copy_{false};

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    void calculate_impl(Calculate&& calculation_fn, MutableDataset const& result_data, Idx pos) {
        return std::forward<Calculate>(calculation_fn)(model_.get(), result_data, pos);
    }

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    void cache_calculate_impl(Calculate&& calculation_fn) {
        return std::forward<Calculate>(calculation_fn)(model_.get(),
                                                       {
                                                           false,
                                                           1,
                                                           "sym_output",
                                                           model_.get().meta_data(),
                                                       },
                                                       ignore_output);
    }

    CalculationInfo calculation_info_impl() const { return model_.get().calculation_info(); }

    void set_calculation_info_impl(CalculationInfo const& info) { model_.get().set_calculation_info(info); }
};
} // namespace power_grid_model
