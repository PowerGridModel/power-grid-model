// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/job_dispatch.hpp>
#include <power_grid_model/job_interface.hpp>

#include <doctest/doctest.h>
namespace power_grid_model {
namespace {
struct MockUpdateDataset {
    MockUpdateDataset(bool data, Idx n_scenarios) : data{data}, n_scenarios{n_scenarios} {}
    bool data;
    Idx n_scenarios;
    bool empty() const { return !data; }
    Idx batch_size() const { return n_scenarios; }
};

struct MockResultDataset {};

class JobAdapterMock : public JobInterface<JobAdapterMock> {
  public:
    JobAdapterMock() = default;
    JobAdapterMock(JobAdapterMock const&) = default;
    JobAdapterMock& operator=(JobAdapterMock const&) = default;
    JobAdapterMock(JobAdapterMock&&) noexcept = default;
    JobAdapterMock& operator=(JobAdapterMock&&) noexcept = default;
    ~JobAdapterMock() = default;

  private:
    friend class JobInterface<JobAdapterMock>;

    void calculate_impl(MockResultDataset const& /*result_data*/, Idx /*scenario_idx*/) const {}
    void cache_calculate_impl() const {}
    void prepare_job_dispatch_impl(MockUpdateDataset const& /*update_data*/) {}
    void setup_impl(MockUpdateDataset const& /*update_data*/, Idx /*scenario_idx*/) {}
    void winddown_impl() {}
    CalculationInfo get_calculation_info_impl() const { return CalculationInfo{{"default", 0.0}}; }
    void thread_safe_add_calculation_info_impl(CalculationInfo const& /*info*/) {}
    auto get_current_scenario_sequence_view_() const {}
};
} // namespace

TEST_CASE("Test job dispatch logic") {
    bool has_data{};
    Idx n_scenarios{};
    std::vector<std::string> exceptions(n_scenarios, "");

    SUBCASE("Test batch_calculation") {
        auto adapter = JobAdapterMock{};
        auto result_data = MockResultDataset{};
        auto const expected_result = BatchParameter{};
        SUBCASE("No update data") {
            has_data = false;
            n_scenarios = 9; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
        }
        SUBCASE("No scenarios") {
            has_data = true;
            n_scenarios = 0;
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
        }
        SUBCASE("With scenarios and update data") {
            has_data = true;
            n_scenarios = 7; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
        }
    }
    SUBCASE("single_thread_job") {}
    SUBCASE("job_dispatch") {}
    SUBCASE("n_threads") {}
    SUBCASE("call_with") {}
    SUBCASE("scenario_exception_handler") {
        auto adapter = JobAdapterMock{};
        n_scenarios = 11; // arbitrary non-zero value
        auto messages = std::vector<std::string>(n_scenarios, "");
        auto info = CalculationInfo{};
        auto handler = JobDispatch::scenario_exception_handler(adapter, messages, info);
        SUBCASE("Known exception") {
            std::string const expected_message = "Test exception";
            Idx const scenario_idx = 7; // arbitrary index
            try {
                throw std::runtime_error(expected_message);
            } catch (...) {
                handler(scenario_idx);
            }
            CHECK(messages[scenario_idx] == expected_message);
            CHECK(info.at("default") == 0.0);
        }
        SUBCASE("Unknown exception") {
            Idx const scenario_idx = 3; // arbitrary index
            try {
                throw 4; // arbitrary non-exception type
            } catch (...) {
                handler(scenario_idx); // simulate handling for scenario index 0
            }
            CHECK(messages[scenario_idx] == "unknown exception");
            CHECK(info.at("default") == 0.0);
        }
    }
    SUBCASE("handle_batch_exceptions") {
        SUBCASE("No exceptions") {
            std::vector<std::string> const empty_exceptions{};
            CHECK_NOTHROW(JobDispatch::handle_batch_exceptions(empty_exceptions));
        }
        SUBCASE("With exceptions") {
            Idx n_scenarios = 5; // arbitrary non-zero value
            std::vector<std::string> exceptions(n_scenarios, "");
            exceptions[0] = "Error in scenario 0";
            exceptions[3] = "Error in scenario 3";
            CHECK_THROWS_AS(JobDispatch::handle_batch_exceptions(exceptions), BatchCalculationError);
        }
    }
}
} // namespace power_grid_model
