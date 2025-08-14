// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/common.hpp>
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
    CalculationInfo get_calculation_info_impl() const { return CalculationInfo{}; }
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
            n_scenarios = 999; // arbitrary non-zero value
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
            n_scenarios = 999; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
        }
    }
    SUBCASE("single_thread_job") {}
    SUBCASE("job_dispatch") {}
    SUBCASE("n_threads") {}
    SUBCASE("call_with") {}
    SUBCASE("scenario_exception_handler") {}
    SUBCASE("handle_batch_exceptions") {}
}
} // namespace power_grid_model
