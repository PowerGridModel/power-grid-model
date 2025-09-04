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
    void calculate(ResultDataset const& result_data, Idx pos = 0)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.calculate_impl(result_data, pos) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->calculate_impl(result_data, pos);
    }

    void cache_calculate()
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.cache_calculate_impl() } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->cache_calculate_impl();
    }

    template <typename UpdateDataset>
    void prepare_job_dispatch(UpdateDataset const& update_data)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.prepare_job_dispatch_impl(update_data) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->prepare_job_dispatch_impl(update_data);
    }

    template <typename UpdateDataset>
    void setup(UpdateDataset const& update_data, Idx scenario_idx)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.setup_impl(update_data, scenario_idx) } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->setup_impl(update_data, scenario_idx);
    }

    void winddown()
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.winddown_impl() } -> std::same_as<void>;
        }
    {
        return static_cast<Adapter*>(this)->winddown_impl();
    }

    void reset_logger()
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.reset_logger_impl() } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->reset_logger_impl();
    }
    void set_logger(Logger& log)
        requires requires(Adapter& adapter) { // NOSONAR
            { adapter.set_logger_impl(log) } -> std::same_as<void>;
        }
    {
        static_cast<Adapter*>(this)->set_logger_impl(log);
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
