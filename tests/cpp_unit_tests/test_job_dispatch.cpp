// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/job_dispatch.hpp>
#include <power_grid_model/job_interface.hpp>
#include <power_grid_model/main_core/core_utils.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <ranges>
#include <variant>

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
    std::atomic<Idx> set_logger_calls{};
    std::atomic<Idx> reset_logger_calls{};

    void reset_counters() {
        calculate_calls = 0;
        cache_calculate_calls = 0;
        setup_calls = 0;
        winddown_calls = 0;
        set_logger_calls = 0;
        reset_logger_calls = 0;
    }
};

class JobAdapterMock : public JobInterface {
  public:
    JobAdapterMock(std::shared_ptr<CallCounter> counter) : counter_{std::move(counter)} {
        REQUIRE_MESSAGE(counter_ != nullptr, "Counter must not be null or all getters will fail later on");
    }
    JobAdapterMock(JobAdapterMock const& other) : counter_{other.counter_} {}
    JobAdapterMock& operator=(JobAdapterMock const& other) {
        if (this != &other) {
            counter_ = other.counter_;
        }
        return *this;
    };
    JobAdapterMock(JobAdapterMock&& other) noexcept : counter_{std::exchange(other.counter_, nullptr)} {}
    JobAdapterMock& operator=(JobAdapterMock&& other) noexcept {
        if (this != &other) {
            counter_ = std::exchange(other.counter_, nullptr);
        }
        return *this;
    }
    ~JobAdapterMock() noexcept { counter_ = nullptr; }

    void reset_counters() const { counter_->reset_counters(); }
    Idx get_calculate_counter() const { return counter_->calculate_calls; }
    Idx get_cache_calculate_counter() const { return counter_->cache_calculate_calls; }
    Idx get_setup_counter() const { return counter_->setup_calls; }
    Idx get_winddown_counter() const { return counter_->winddown_calls; }

  private:
    friend class JobInterface;

    std::shared_ptr<CallCounter> counter_;

    void calculate_impl(MockResultDataset const& /*result_data*/, Idx /*scenario_idx*/,
                        Logger const& /*logger*/) const {
        ++(counter_->calculate_calls);
    }
    void cache_calculate_impl(Logger const& /*logger*/) const { ++(counter_->cache_calculate_calls); }
    void prepare_job_dispatch_impl(MockUpdateDataset const& /*update_data*/) const { /* patch base class function */ }
    void setup_impl(MockUpdateDataset const& /*update_data*/, Idx /*scenario_idx*/) const { ++(counter_->setup_calls); }
    void winddown_impl() const { ++(counter_->winddown_calls); }
};

class SomeTestException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class TestLogger : public common::logging::Logger {
  public:
    struct EmptyEvent {};
    static constexpr EmptyEvent empty_event{};

    struct Entry {
        LogEvent event;
        std::variant<EmptyEvent, std::string, double, Idx> data;
    };
    using Data = std::vector<Entry>;

    // Mock logger for testing
    void log(LogEvent event) override { log_.emplace_back(event, empty_event); }
    void log(LogEvent event, std::string_view message) override { log_.emplace_back(event, std::string{message}); }
    void log(LogEvent event, double value) override { log_.emplace_back(event, value); }
    void log(LogEvent event, Idx value) override { log_.emplace_back(event, value); }

    Data const& report() const { return log_; }

    template <std::derived_from<Logger> T> T& merge_into(T& destination) const {
        if (&destination == this) {
            return destination; // nothing to do
        }
        for (const auto& entry : report()) {
            std::visit(
                [&destination, event = entry.event](auto&& arg) {
                    using U = std::decay_t<decltype(arg)>;
                    if constexpr (std::same_as<U, TestLogger::EmptyEvent>) {
                        destination.log(event);
                    } else {
                        destination.log(event, arg);
                    }
                },
                entry.data);
        }
        return destination;
    }

  private:
    Data log_;
};

class MultiThreadedTestLogger : public common::logging::MultiThreadedLoggerImpl<TestLogger> {
  public:
    using MultiThreadedLoggerImpl::MultiThreadedLoggerImpl;
    using Data = TestLogger::Data;

    Data const& report() const { return get().report(); }
};

using common::logging::MultiThreadedLogger;

MultiThreadedLogger& no_logger() {
    static common::logging::NoMultiThreadedLogger instance;
    return instance;
}
} // namespace

TEST_CASE("Test job dispatch logic") {
    std::vector<std::string> exceptions(0, "");

    SUBCASE("Test batch_calculation") {
        auto counter = std::make_shared<CallCounter>();
        auto adapter = JobAdapterMock{counter};
        auto result_data = MockResultDataset{};
        auto const expected_result = BatchParameter{};
        SUBCASE("No update data") {
            bool const has_data = false;
            Idx const n_scenarios = 9; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data,
                                                                      main_core::utils::sequential, no_logger());
            CHECK(expected_result == actual_result);
            CHECK(adapter.get_calculate_counter() == 1);
            CHECK(adapter.get_cache_calculate_counter() == 0); // no cache calculation in this case
        }
        SUBCASE("No scenarios") {
            bool const has_data = true;
            Idx const n_scenarios = 0;
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data,
                                                                      main_core::utils::sequential, no_logger());
            CHECK(expected_result == actual_result);
            // no calculations should be done
            CHECK(adapter.get_calculate_counter() == 0);
            CHECK(adapter.get_cache_calculate_counter() == 0);
        }
        SUBCASE("With scenarios and update data") {
            bool const has_data = true;
            Idx const n_scenarios = 7; // arbitrary non-zero value
            auto const update_data = MockUpdateDataset(has_data, n_scenarios);
            adapter.reset_counters();
            auto const actual_result = JobDispatch::batch_calculation(adapter, result_data, update_data,
                                                                      main_core::utils::sequential, no_logger());
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
        bool const has_data{false};
        Idx const n_scenarios = 9; // arbitrary non-zero value
        auto const update_data = MockUpdateDataset(has_data, n_scenarios);
        exceptions.resize(n_scenarios);
        Idx start{};
        Idx stride{};
        Idx call_number{};

        auto get_call_number = [](Idx start_, Idx stride_, Idx n_scenarios_) {
            REQUIRE_MESSAGE(
                stride_ > 0,
                "Can't have stride of (less than) zero; this should be caught by a different job dispatch handling");
            return (n_scenarios_ - start_ + stride_ - 1) / stride_;
        };

        auto check_call_numbers = [](JobAdapterMock const& adapter_, Idx expected_calls) {
            CHECK(adapter_.get_setup_counter() == expected_calls);
            CHECK(adapter_.get_winddown_counter() == expected_calls);
            CHECK(adapter_.get_calculate_counter() == expected_calls);
        };

        adapter.prepare_job_dispatch(update_data); // replicate preparation step from batch_calculation
        common::logging::NoMultiThreadedLogger no_log;
        auto single_job = JobDispatch::single_thread_job(adapter, result_data, update_data, exceptions, no_log);

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
        struct JobArguments {
            Idx start;
            Idx stride;
            Idx n_scenarios;
        };
        std::vector<JobArguments> calls;
        std::mutex calls_mutex;
        auto single_job = [&calls, &calls_mutex](Idx start, Idx stride, Idx n_scenarios) {
            std::lock_guard<std::mutex> const lock(calls_mutex);
            calls.emplace_back(start, stride, n_scenarios);
        };

        SUBCASE("Sequential") {
            Idx const n_scenarios = 10; // arbitrary non-zero value
            Idx const threading = main_core::utils::sequential;
            calls.clear();
            JobDispatch::job_dispatch(single_job, n_scenarios, threading);
            CHECK(calls.size() == 1);
            CHECK(calls.front().start == 0);
            CHECK(calls.front().stride == 1);
            CHECK(calls.front().n_scenarios == 10);
        }

        SUBCASE("Multi-threaded") {
            static_assert(std::is_unsigned_v<decltype(std::thread::hardware_concurrency())>);

            auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
            Idx const threading = 0;

            SUBCASE("More scenarios than hardware threads") {
                Idx const n_scenarios = hardware_thread + 1; // larger than hardware threads
                calls.clear();
                CAPTURE(hardware_thread);
                CHECK(hardware_thread == JobDispatch::n_threads(n_scenarios, threading));
                JobDispatch::job_dispatch(single_job, n_scenarios, threading);
                CHECK(calls.size() == hardware_thread);
                auto const n_threads = static_cast<Idx>(calls.size());
                CHECK(std::ranges::all_of(std::views::iota(Idx{0}, n_threads), [&calls](Idx i) {
                    return std::ranges::any_of(calls, [i](JobArguments const& call) { return call.start == i; });
                }));
                CHECK(std::ranges::all_of(calls,
                                          [n_threads](JobArguments const& call) { return call.stride == n_threads; }));
                CHECK(std::ranges::all_of(
                    calls, [n_scenarios](JobArguments const& call) { return call.n_scenarios == n_scenarios; }));
            }
            SUBCASE("Less scenarios than hardware threads") {
                Idx const n_scenarios = std::max(Idx{0}, hardware_thread - 1);
                CAPTURE(n_scenarios);
                calls.clear();
                CAPTURE(hardware_thread);
                CHECK(n_scenarios == JobDispatch::n_threads(n_scenarios, threading));
                JobDispatch::job_dispatch(single_job, n_scenarios, threading);
                CHECK(calls.size() == n_scenarios);
                auto const n_threads = static_cast<Idx>(calls.size());
                CHECK(std::ranges::all_of(std::views::iota(0, n_threads), [&calls](Idx i) {
                    return std::ranges::any_of(calls, [i](JobArguments const& call) { return call.start == i; });
                }));
                CHECK(std::ranges::all_of(calls,
                                          [n_threads](JobArguments const& call) { return call.stride == n_threads; }));
                CHECK(std::ranges::all_of(
                    calls, [n_scenarios](JobArguments const& call) { return call.n_scenarios == n_scenarios; }));
            }
        }
    }
    SUBCASE("Test n_threads") {
        static_assert(std::is_unsigned_v<decltype(std::thread::hardware_concurrency())>);

        auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
        CAPTURE(hardware_thread);
        Idx const n_scenarios = 14; // arbitrary non-zero value

        SUBCASE("Sequential threading") {
            CHECK(JobDispatch::n_threads(n_scenarios, main_core::utils::sequential) == 1);
            CHECK(JobDispatch::n_threads(n_scenarios, 1) == 1);
        }
        SUBCASE("Parallel threading") {
            SUBCASE("Use specified threading when less than num scenarios") {
                Idx const threading = n_scenarios - 1;
                CHECK(JobDispatch::n_threads(n_scenarios, threading) == threading);
            }
            SUBCASE("Use num scenarios threading when requesting more threads") {
                Idx const threading = n_scenarios + 1;
                CHECK(JobDispatch::n_threads(n_scenarios, threading) == n_scenarios);
            }
        }
        SUBCASE("Hardware threading") {
            if (hardware_thread < 2) {
                CHECK(JobDispatch::n_threads(n_scenarios, 0) == 1);
            } else if (hardware_thread <= n_scenarios) {
                CHECK(JobDispatch::n_threads(n_scenarios, 0) == hardware_thread);
            } else {
                CHECK(JobDispatch::n_threads(n_scenarios, 0) == n_scenarios);
            }
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
            throw SomeTestException{"Run error"};
        };
        auto winddown_fn_no_throw = [&winddown_called]() { winddown_called++; };
        auto winddown_fn_throw = [&winddown_called]() {
            winddown_called++;
            throw SomeTestException{"Winddown error"};
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
        Idx const n_scenarios = 11; // arbitrary non-zero value
        auto messages = std::vector<std::string>(n_scenarios, "");
        auto handler = JobDispatch::scenario_exception_handler(messages);
        SUBCASE("Known exception") {
            std::string const expected_message = "Test exception";
            Idx const scenario_idx = 7; // arbitrary index
            try {
                throw SomeTestException{expected_message};
            } catch (...) { // NOSONAR // the handler should work with arbitrary exception types
                handler(scenario_idx);
            }
            CHECK(messages[scenario_idx] == expected_message);
        }
        SUBCASE("Unknown exception") {
            Idx const scenario_idx = 3; // arbitrary index
            try {
                throw 4;    // arbitrary non-exception type  // NOLINT(hicpp-exception-baseclass)
            } catch (...) { // NOSONAR // the handler should work with arbitrary exception types
                handler(scenario_idx);
            }
            CHECK(messages[scenario_idx] == "unknown exception");
        }
    }
    SUBCASE("Test handle_batch_exceptions") {
        Idx const n_scenarios = 5; // arbitrary non-zero value
        exceptions.resize(n_scenarios);
        SUBCASE("No exceptions") { CHECK_NOTHROW(JobDispatch::handle_batch_exceptions(exceptions)); }
        SUBCASE("With exceptions") {
            exceptions[0] = "Error in scenario 0";
            exceptions[3] = "Error in scenario 3";
            CHECK_THROWS_WITH_AS(
                [&exceptions] {
                    try {
                        JobDispatch::handle_batch_exceptions(exceptions);
                        FAIL("should have thrown here");
                    } catch (BatchCalculationError const& e) {
                        using namespace std::string_literals;
                        REQUIRE(e.err_msgs().size() == 2);
                        CHECK(e.err_msgs() == std::vector{"Error in scenario 0"s, "Error in scenario 3"s});
                        REQUIRE(e.failed_scenarios().size() == 2);
                        CHECK(e.failed_scenarios() == std::vector{Idx{0}, Idx{3}});
                        throw;
                    }
                }(),
                "Error in batch #0: Error in scenario 0\nError in batch #3: Error in scenario 3\n",
                BatchCalculationError);
        }
    }
}
} // namespace power_grid_model
