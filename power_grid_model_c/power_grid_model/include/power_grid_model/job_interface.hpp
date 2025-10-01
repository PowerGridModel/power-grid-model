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
class JobInterface {
  public:
    // the multiple  NOSONARs are used to avoid the complaints about the unnamed concepts
    template <typename Self, typename ResultDataset>
    void calculate(this Self&& self, ResultDataset const& result_data, Idx pos, Logger& logger)
        requires requires { // NOSONAR
            { std::forward<Self>(self).calculate_impl(result_data, pos, logger) } -> std::same_as<void>;
        }
    {
        return std::forward<Self>(self).calculate_impl(result_data, pos, logger);
    }

    template <typename Self, typename ResultDataset>
    void calculate(this Self&& self, ResultDataset const& result_data, Logger& logger) {
        std::forward<Self>(self).calculate(result_data, Idx{}, logger);
    }

    template <typename Self>
    void cache_calculate(this Self&& self, Logger& logger)
        requires requires { // NOSONAR
            { std::forward<Self>(self).cache_calculate_impl(logger) } -> std::same_as<void>;
        }
    {
        return std::forward<Self>(self).cache_calculate_impl(logger);
    }

    template <typename Self, typename UpdateDataset>
    void prepare_job_dispatch(this Self&& self, UpdateDataset const& update_data)
        requires requires { // NOSONAR
            { std::forward<Self>(self).prepare_job_dispatch_impl(update_data) } -> std::same_as<void>;
        }
    {
        return std::forward<Self>(self).prepare_job_dispatch_impl(update_data);
    }

    template <typename Self, typename UpdateDataset>
    void setup(this Self&& self, UpdateDataset const& update_data, Idx scenario_idx)
        requires requires { // NOSONAR
            { std::forward<Self>(self).setup_impl(update_data, scenario_idx) } -> std::same_as<void>;
        }
    {
        return std::forward<Self>(self).setup_impl(update_data, scenario_idx);
    }

    template <typename Self>
    void winddown(this Self&& self)
        requires requires { // NOSONAR
            { std::forward<Self>(self).winddown_impl() } -> std::same_as<void>;
        }
    {
        return std::forward<Self>(self).winddown_impl();
    }

  protected:
    JobInterface() = default;
    JobInterface(const JobInterface& /*other*/) = default;
    JobInterface& operator=(const JobInterface& /*other*/) = default;
    JobInterface(JobInterface&& /*other*/) noexcept = default;
    JobInterface& operator=(JobInterface&& /*other*/) noexcept = default;
    ~JobInterface() = default;
};

} // namespace power_grid_model
