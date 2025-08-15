// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging_impl.hpp"

#include <cstddef>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

namespace power_grid_model {
namespace common::logging {
class CalculationInfo : public DefaultLogger {
    using Data = std::map<LoggingTag, double>;

  public:
    using const_iterator = Data::const_iterator;

    void log(LoggingTag tag, double value) override { log_impl(tag, value); }
    void log(LoggingTag tag, Idx value) override { log_impl(tag, static_cast<double>(value)); }
    void merge(const CalculationInfo& other) {
        for (const auto& [tag, value] : other.data_) {
            log(tag, value);
        }
    }

  private:
    Data data_;

    void log_impl(LoggingTag tag, double value) {
        using enum LoggingTag;

        switch (tag) {
        case timer:
            return accumulate_log(tag, value);
        case iterative_pf_solver_max_num_iter:
        case ilse_max_num_iter:
        case nrse_max_num_iter:
            return maximize_log(tag, value);
        default:
            return;
        }
    }
    void accumulate_log(LoggingTag tag, double value) { data_[tag] += value; }
    void maximize_log(LoggingTag tag, double value) { data_[tag] = std::max(data_[tag], value); }

  public:
    static std::string make_key(LoggingTag code) { // TODO(mgovers): make private + only use for reporting
        auto const name = common::logging::to_string(code);

        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << static_cast<std::underlying_type_t<LoggingTag>>(code) << ".";
        auto key = ss.str();
        for (size_t i = 0, n = key.length() - 1; i < n; ++i) {
            if (key[i] == '0') {
                break;
            }
            key += "\t";
        }
        key += name;
        return key;
    }

    auto begin() { return std::ranges::begin(data_); }
    auto begin() const { return std::ranges::begin(data_); }
    auto end() { return std::ranges::end(data_); }
    auto end() const { return std::ranges::end(data_); }
};
} // namespace common::logging

using common::logging::CalculationInfo;
} // namespace power_grid_model
