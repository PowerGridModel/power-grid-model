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

#include <atomic>
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

struct CallCounter {
    std::atomic<Idx> calculate_calls{};
    std::atomic<Idx> cache_calculate_calls{};
    std::atomic<Idx> setup_calls{};
    std::atomic<Idx> winddown_calls{};
    std::atomic<Idx> thread_safe_add_calculation_info_calls{};

    void reset_counters() {
        calculate_calls = 0;
        cache_calculate_calls = 0;
        setup_calls = 0;
        winddown_calls = 0;
        thread_safe_add_calculation_info_calls = 0;
    }
};

class JobAdapterMock : public JobInterface<JobAdapterMock> {
  public:
    JobAdapterMock(std::shared_ptr<CallCounter> counter) : counter_{std::move(counter)} {}
    JobAdapterMock(JobAdapterMock const&) = default;
    JobAdapterMock& operator=(JobAdapterMock const&) = default;
    JobAdapterMock(JobAdapterMock&&) noexcept = default;
    JobAdapterMock& operator=(JobAdapterMock&&) noexcept = default;
    ~JobAdapterMock() = default;

    void reset_counters() { counter_->reset_counters(); }
    Idx get_calculate_counter() const { return counter_->calculate_calls; }
    Idx get_cache_calculate_counter() const { return counter_->cache_calculate_calls; }
    Idx get_setup_counter() const { return counter_->setup_calls; }
    Idx get_winddown_counter() const { return counter_->winddown_calls; }
    Idx get_thread_safe_add_calculation_info_counter() const {
        return counter_->thread_safe_add_calculation_info_calls;
    }

  private:
    friend class JobInterface<JobAdapterMock>;

    std::shared_ptr<CallCounter> counter_;

    void calculate_impl(MockResultDataset const& /*result_data*/, Idx /*scenario_idx*/) { ++counter_->calculate_calls; }
    void cache_calculate_impl() { ++counter_->cache_calculate_calls; }
    void prepare_job_dispatch_impl(MockUpdateDataset const& /*update_data*/) {}
    void setup_impl(MockUpdateDataset const& /*update_data*/, Idx /*scenario_idx*/) { ++counter_->setup_calls; }
    void winddown_impl() { ++counter_->winddown_calls; }
    CalculationInfo get_calculation_info_impl() const { return CalculationInfo{{"default", 0.0}}; }
    void thread_safe_add_calculation_info_impl(CalculationInfo const& /*info*/) {
        ++counter_->thread_safe_add_calculation_info_calls;
    }
    auto get_current_scenario_sequence_view_() {}
};
} // namespace

TEST_CASE("Test job dispatch logic") {
    Idx n_scenarios{};
    std::vector<std::string> exceptions(n_scenarios, "");

    SUBCASE("Test batch_calculation") {
        auto counter = std::make_shared<CallCounter>();
        auto adapter = JobAdapterMock{counter};
        auto result_data = MockResultDataset{};
        auto const expected_result = BatchParameter{};
        bool has_data{};
        SUBCASE("No update data") {
            has_data = false;
            n_scenarios = 9; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
            CHECK(adapter.get_calculate_counter() == 1);
            CHECK(adapter.get_cache_calculate_counter() == 0); // no cache calculation in this case
        }
        SUBCASE("No scenarios") {
            has_data = true;
            n_scenarios = 0;
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data);
            CHECK(expected_result == actual_result);
            // no calculations should be done
            CHECK(adapter.get_calculate_counter() == 0);
            CHECK(adapter.get_cache_calculate_counter() == 0);
        }
        SUBCASE("With scenarios and update data") {
            has_data = true;
            n_scenarios = 7; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result =
                JobDispatch::batch_calculation(adapter, result_data, update_data, JobDispatch::sequential);
            CHECK(expected_result == actual_result);
            // n_scenarios calculations should be done as we run sequentially
            CHECK(adapter.get_calculate_counter() == n_scenarios);
            CHECK(adapter.get_cache_calculate_counter() == 1); // cache calculation is done
        }
    }
    SUBCASE("Test single_thread_job") {
        auto counter = std::make_shared<CallCounter>();
        auto adapter = JobAdapterMock{counter};
        auto result_data = MockResultDataset{};
        bool has_data{false};
        n_scenarios = 9; // arbitrary non-zero value
        auto const update_data = MockUpdateDataset(has_data, n_scenarios);
        exceptions.resize(n_scenarios);
        Idx start{};
        Idx stride{};
        Idx call_number{};

        auto get_call_number = [](Idx start, Idx stride, Idx n_scenarios) {
            if (stride == 0) {
                FAIL("Can't have stride of zero");
            }
            return (n_scenarios - start + stride - 1) / stride;
        };

        auto check_call_numbers = [](JobAdapterMock& adapter, Idx expected_calls) {
            CHECK(adapter.get_setup_counter() == expected_calls);
            CHECK(adapter.get_winddown_counter() == expected_calls);
            CHECK(adapter.get_calculate_counter() == expected_calls);
            CHECK(adapter.get_thread_safe_add_calculation_info_counter() == 1); // always called once
        };

        adapter.prepare_job_dispatch(update_data); // replicate preparation step from batch_calculation
        auto single_job = JobDispatch::single_thread_job(adapter, result_data, update_data, exceptions);

        adapter.reset_counters();
        start = 0;
        stride = 1;
        call_number = get_call_number(start, stride, n_scenarios);
        CAPTURE(call_number);
        CHECK_NOTHROW(single_job(start, stride, n_scenarios));
        check_call_numbers(adapter, call_number);

        adapter.reset_counters();
        start = 0;
        stride = 4;
        call_number = get_call_number(start, stride, n_scenarios);
        CAPTURE(call_number);
        CHECK_NOTHROW(single_job(start, stride, n_scenarios));
        check_call_numbers(adapter, call_number);

        adapter.reset_counters();
        start = 3;
        stride = 2;
        call_number = get_call_number(start, stride, n_scenarios);
        CAPTURE(call_number);
        CHECK_NOTHROW(single_job(start, stride, n_scenarios));
        check_call_numbers(adapter, call_number);
    }
    SUBCASE("Test job_dispatch") {
        Idx threading{};
        std::vector<Idx> calls;
        std::mutex calls_mutex;
        auto single_job = [&calls, &calls_mutex](Idx /*start*/, Idx /*stride*/, Idx /*n_scenarios*/) {
            std::lock_guard<std::mutex> lock(calls_mutex);
            calls.emplace_back(0);
        };

        SUBCASE("Sequential") {
            n_scenarios = 10; // arbitrary non-zero value
            threading = JobDispatch::sequential;
            calls.clear();
            JobDispatch::job_dispatch(single_job, n_scenarios, threading);
            CHECK(calls.size() == 1);
        }

        SUBCASE("Multi-threaded") {
            auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
            n_scenarios = hardware_thread + 1; // larger than hardware threads
            threading = 0;
            calls.clear();
            CAPTURE(hardware_thread);
            CHECK(hardware_thread == JobDispatch::n_threads(n_scenarios, threading));
            JobDispatch::job_dispatch(single_job, n_scenarios, threading);
            CHECK(calls.size() == hardware_thread);

            n_scenarios = hardware_thread - 1; // smaller than hardware threads
            calls.clear();
            CAPTURE(hardware_thread);
            CHECK(n_scenarios == JobDispatch::n_threads(n_scenarios, threading));
            JobDispatch::job_dispatch(single_job, n_scenarios, threading);
            CHECK(calls.size() == n_scenarios);
        }
    }
    SUBCASE("Test n_threads") {
        auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
        CAPTURE(hardware_thread);
        n_scenarios = 14; // arbitrary non-zero value
        Idx threading{};
        SUBCASE("Sequential threading") {
            threading = JobDispatch::sequential;
            CHECK(JobDispatch::n_threads(n_scenarios, threading) == 1);

            threading = 1;
            CHECK(JobDispatch::n_threads(n_scenarios, threading) == 1);

            if (hardware_thread < 2) {
                threading = 0;
                CHECK(JobDispatch::n_threads(n_scenarios, threading) == 1);
            }
        }
        SUBCASE("Parallel threading") {
            if (hardware_thread >= 2) {
                threading = 0; // use hardware threads
                CHECK(JobDispatch::n_threads(n_scenarios, threading) == hardware_thread);
            }

            threading = n_scenarios - 1; // use specified threading
            CHECK(JobDispatch::n_threads(n_scenarios, threading) == threading);

            threading = n_scenarios + 1; // use n_scenarios as threading
            CHECK(JobDispatch::n_threads(n_scenarios, threading) == n_scenarios);
        }
    }
    SUBCASE("Test call_with") {
        // These call counters are local as are unrelated to the adapter mock
        // and are only used to test the call_with functionality
        Idx setup_called{0};
        Idx run_called{0};
        Idx winddown_called{0};
        Idx handle_exception_called{0};
        Idx recover_from_bad_called{0};

        auto setup_fn = [&setup_called](Idx) { setup_called++; };
        auto run_fn_no_throw = [&run_called](Idx) { run_called++; };
        auto run_fn_throw = [&run_called](Idx) {
            run_called++;
            throw std::runtime_error("Run error");
        };
        auto winddown_fn_no_throw = [&winddown_called]() { winddown_called++; };
        auto winddown_fn_throw = [&winddown_called]() {
            winddown_called++;
            throw std::runtime_error("Winddown error");
        };
        auto handle_exception_fn = [&handle_exception_called](Idx) { handle_exception_called++; };
        auto recover_from_bad_fn = [&recover_from_bad_called]() { recover_from_bad_called++; };

        SUBCASE("No exceptions") {
            auto call_with = JobDispatch::call_with<Idx>(run_fn_no_throw, setup_fn, winddown_fn_no_throw,
                                                         handle_exception_fn, recover_from_bad_fn);
            call_with(1);
            CHECK(setup_called == 1);
            CHECK(run_called == 1);
            CHECK(winddown_called == 1);
            CHECK(handle_exception_called == 0);
            CHECK(recover_from_bad_called == 0);
        }
        SUBCASE("With run exception") {
            auto call_with = JobDispatch::call_with<Idx>(run_fn_throw, setup_fn, winddown_fn_no_throw,
                                                         handle_exception_fn, recover_from_bad_fn);
            call_with(2);
            CHECK(setup_called == 1);
            CHECK(run_called == 1);
            CHECK(winddown_called == 1);
            CHECK(handle_exception_called == 1);
            CHECK(recover_from_bad_called == 0);
        }
        SUBCASE("With winddown exception") {
            auto call_with = JobDispatch::call_with<Idx>(run_fn_no_throw, setup_fn, winddown_fn_throw,
                                                         handle_exception_fn, recover_from_bad_fn);
            call_with(3);
            CHECK(setup_called == 1);
            CHECK(run_called == 1);
            CHECK(winddown_called == 2);
            CHECK(handle_exception_called == 1);
            CHECK(recover_from_bad_called == 1);
        }
    }
    SUBCASE("Test scenario_exception_handler") {
        auto counter = std::make_shared<CallCounter>();
        auto adapter = JobAdapterMock{counter};
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
                handler(scenario_idx);
            }
            CHECK(messages[scenario_idx] == "unknown exception");
            CHECK(info.at("default") == 0.0);
        }
    }
    SUBCASE("Test handle_batch_exceptions") {
        n_scenarios = 5; // arbitrary non-zero value
        exceptions.resize(n_scenarios);
        SUBCASE("No exceptions") { CHECK_NOTHROW(JobDispatch::handle_batch_exceptions(exceptions)); }
        SUBCASE("With exceptions") {
            exceptions[0] = "Error in scenario 0";
            exceptions[3] = "Error in scenario 3";
            CHECK_THROWS_AS(JobDispatch::handle_batch_exceptions(exceptions), BatchCalculationError);
        }
    }
}
} // namespace power_grid_model
