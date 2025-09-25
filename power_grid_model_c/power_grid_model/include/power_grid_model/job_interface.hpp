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
    void calculate(this Self const& self, ResultDataset const& result_data, Idx pos = 0)
        requires requires { // NOSONAR
            { self.calculate_impl(result_data, pos) } -> std::same_as<void>;
        }
    {
        return self.calculate_impl(result_data, pos);
    }

    template <typename Self>
    void cache_calculate(this Self const& self)
        requires requires { // NOSONAR
            { self.cache_calculate_impl() } -> std::same_as<void>;
        }
    {
        return self.cache_calculate_impl();
    }

    template <typename Self, typename UpdateDataset>
    void prepare_job_dispatch(this Self& self, UpdateDataset const& update_data)
        requires requires { // NOSONAR
            { self.prepare_job_dispatch_impl(update_data) } -> std::same_as<void>;
        }
    {
        return self.prepare_job_dispatch_impl(update_data);
    }

    template <typename Self, typename UpdateDataset>
    void setup(this Self& self, UpdateDataset const& update_data, Idx scenario_idx)
        requires requires { // NOSONAR
            { self.setup_impl(update_data, scenario_idx) } -> std::same_as<void>;
        }
    {
        return self.setup_impl(update_data, scenario_idx);
    }

    template <typename Self>
    void winddown(this Self& self)
        requires requires { // NOSONAR
            { self.winddown_impl() } -> std::same_as<void>;
        }
    {
        return self.winddown_impl();
    }

    template <typename Self>
    void reset_logger(this Self& self)
        requires requires { // NOSONAR
            { self.reset_logger_impl() } -> std::same_as<void>;
        }
    {
        self.reset_logger_impl();
    }

    template <typename Self>
    void set_logger(this Self& self, Logger& log)
        requires requires { // NOSONAR
            { self.set_logger_impl(log) } -> std::same_as<void>;
        }
    {
        self.set_logger_impl(log);
    }
};

} // namespace power_grid_model
