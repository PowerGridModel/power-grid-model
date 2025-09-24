// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// batch dispatch interface class

#include "common/common.hpp"
#include "common/logging.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace power_grid_model {
template <typename Adapter> class JobInterface {
  public:
    // the multiple  NOSONARs are used to avoid the complaints about the unnamed concepts
    template <typename ResultDataset>
    void calculate(ResultDataset const& result_data, Idx pos, Logger& logger)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.calculate_impl(result_data, pos, logger) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->calculate_impl(result_data, pos, logger);
    }
    template <typename ResultDataset> void calculate(ResultDataset const& result_data, Logger& logger) {
        calculate(result_data, Idx{}, logger);
    }

    void cache_calculate(Logger& logger)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.cache_calculate_impl(logger) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->cache_calculate_impl(logger);
    }

    template <typename UpdateDataset>
    void prepare_job_dispatch(UpdateDataset const& update_data)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.prepare_job_dispatch_impl(update_data) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->prepare_job_dispatch_impl(update_data);
    }

    template <typename UpdateDataset>
    void setup(UpdateDataset const& update_data, Idx scenario_idx)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.setup_impl(update_data, scenario_idx) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->setup_impl(update_data, scenario_idx);
    }

    void winddown()
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.winddown_impl() } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->winddown_impl();
    }

  private:
    friend Adapter;
    JobInterface() = default;
    JobInterface(const JobInterface& /*other*/) = default;
    JobInterface& operator=(const JobInterface& /*other*/) = default;
    JobInterface(JobInterface&& /*other*/) noexcept = default;
    JobInterface& operator=(JobInterface&& /*other*/) noexcept = default;
    ~JobInterface() = default;
};

} // namespace power_grid_model
