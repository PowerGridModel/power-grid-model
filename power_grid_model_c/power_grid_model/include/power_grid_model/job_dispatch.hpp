// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "batch_parameter.hpp"
#include "job_interface.hpp"

#include "common/exception.hpp"
#include "common/timer.hpp"
#include "common/typing.hpp"

#include <thread>

namespace power_grid_model {

class JobDispatch {
  public:
    template <typename Adapter, typename ResultDataset, typename UpdateDataset>
        requires std::is_base_of_v<JobInterface, Adapter>
    static BatchParameter batch_calculation(Adapter& adapter, ResultDataset const& result_data,
                                            UpdateDataset const& update_data, Idx threading,
                                            common::logging::MultiThreadedLogger& log) {
        if (update_data.empty()) {
            adapter.calculate(result_data, log);
            return BatchParameter{};
        }

        // get batch size
        Idx const n_scenarios = update_data.batch_size();

        // if the batch_size is zero, it is a special case without doing any calculations at all
        // we consider in this case the batch set is independent but not topology cacheable
        if (n_scenarios == 0) {
            return BatchParameter{};
        }

        // calculate once to cache, ignore results
        adapter.cache_calculate(log);

        // error messages
        std::vector<std::string> exceptions(n_scenarios, "");

        adapter.prepare_job_dispatch(update_data);
        auto single_job = JobDispatch::single_thread_job(adapter, result_data, update_data, exceptions, log);

        job_dispatch(single_job, n_scenarios, threading);

        handle_batch_exceptions(exceptions);

        return BatchParameter{};
    }

    // Lippincott pattern
    static auto scenario_exception_handler(std::vector<std::string>& messages) {
        return [&messages](Idx scenario_idx) -> void {
            assert(0 <= scenario_idx);
            assert(scenario_idx < std::ssize(messages));

            std::exception_ptr const ex_ptr = std::current_exception();
            try {
                std::rethrow_exception(ex_ptr);
            } catch (std::exception const& ex) {
                messages[scenario_idx] = ex.what();
            } catch (...) {
                messages[scenario_idx] = "unknown exception";
            }
        };
    }

    template <typename Adapter, typename ResultDataset, typename UpdateDataset>
    static auto single_thread_job(Adapter& base_adapter, ResultDataset const& result_data,
                                  UpdateDataset const& update_data, std::vector<std::string>& exceptions,
                                  common::logging::MultiThreadedLogger& base_log) {
        return [&base_adapter, &exceptions, &result_data, &update_data, &base_log](Idx start, Idx stride,
                                                                                   Idx n_scenarios) {
            assert(n_scenarios <= narrow_cast<Idx>(exceptions.size()));
            auto thread_log_ptr = base_log.create_child();
            Logger& thread_log = *thread_log_ptr;

            Timer t_total{thread_log, LogEvent::total_batch_calculation_in_thread};

            auto const copy_adapter_functor = [&base_adapter, &thread_log]() {
                Timer const t_copy_adapter_functor{thread_log, LogEvent::copy_model};
                auto result = Adapter{base_adapter};
                return result;
            };

            auto adapter = copy_adapter_functor();

            auto setup = [&adapter, &update_data, &thread_log](Idx scenario_idx) {
                Timer const t_update_model{thread_log, LogEvent::update_model};
                adapter.setup(update_data, scenario_idx);
            };

            auto winddown = [&adapter, &thread_log]() {
                Timer const t_restore_model{thread_log, LogEvent::restore_model};
                adapter.winddown();
            };

            auto recover_from_bad = [&adapter, &copy_adapter_functor]() {
                // TODO(figueroa1395): Time this step
                // how do we want to deal with exceptions and timing?
                adapter = copy_adapter_functor();
            };

            auto run = [&adapter, &result_data, &thread_log](Idx scenario_idx) {
                adapter.calculate(result_data, scenario_idx, thread_log);
            };

            auto calculate_scenario = JobDispatch::call_with<Idx>(std::move(run), std::move(setup), std::move(winddown),
                                                                  JobDispatch::scenario_exception_handler(exceptions),
                                                                  std::move(recover_from_bad));

            for (Idx scenario_idx = start; scenario_idx < n_scenarios; scenario_idx += stride) {
                Timer const t_total_single{thread_log, LogEvent::total_single_calculation_in_thread};
                calculate_scenario(scenario_idx);
            }

            t_total.stop();
        };
    }

    template <typename RunSingleJobFn>
        requires std::invocable<std::remove_cvref_t<RunSingleJobFn>, Idx /*start*/, Idx /*stride*/, Idx /*n_scenarios*/>
    static void job_dispatch(RunSingleJobFn single_thread_job, Idx n_scenarios, Idx threading) {
        // run batches sequential or parallel
        auto const n_thread = n_threads(n_scenarios, threading);
        if (n_thread == 1) {
            // run all in sequential
            single_thread_job(0, 1, n_scenarios);
        } else {
            // create parallel threads
            std::vector<std::thread> threads;
            threads.reserve(n_thread);
            for (Idx thread_number = 0; thread_number < n_thread; ++thread_number) {
                // compute each single thread job with stride
                threads.emplace_back(single_thread_job, thread_number, n_thread, n_scenarios);
            }
            for (auto& thread : threads) {
                thread.join();
            }
        }
    }

    // run sequential if
    //    specified threading < 0
    //    use hardware threads, but it is either unknown (0) or only has one thread (1)
    //    specified threading = 1
    static Idx n_threads(Idx n_scenarios, Idx threading) {
        auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
        if (threading < 0 || threading == 1 || (threading == 0 && hardware_thread < 2)) {
            return 1; // sequential
        }
        return std::min(threading == 0 ? hardware_thread : threading, n_scenarios);
    }

    template <typename... Args, typename RunFn, typename SetupFn, typename WinddownFn, typename HandleExceptionFn,
              typename RecoverFromBadFn>
        requires std::invocable<std::remove_cvref_t<RunFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<SetupFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<WinddownFn>> &&
                 std::invocable<std::remove_cvref_t<HandleExceptionFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<RecoverFromBadFn>>
    static auto call_with(RunFn run, SetupFn setup, WinddownFn winddown, HandleExceptionFn handle_exception,
                          RecoverFromBadFn recover_from_bad) {
        return [setup_ = std::move(setup), run_ = std::move(run), winddown_ = std::move(winddown),
                handle_exception_ = std::move(handle_exception),
                recover_from_bad_ = std::move(recover_from_bad)](Args const&... args) {
            try {
                setup_(args...);
                run_(args...);
                winddown_();
            } catch (...) {
                handle_exception_(args...);
                try {
                    winddown_();
                } catch (...) {
                    recover_from_bad_();
                }
            }
        };
    }

    static void handle_batch_exceptions(std::vector<std::string> const& exceptions) {
        std::string combined_error_message;
        IdxVector failed_scenarios;
        std::vector<std::string> err_msgs;
        for (Idx batch = 0; batch < static_cast<Idx>(exceptions.size()); ++batch) {
            // append exception if it is not empty
            if (!exceptions[batch].empty()) {
                combined_error_message =
                    std::format("{}Error in batch #{}: {}\n", combined_error_message, batch, exceptions[batch]);
                failed_scenarios.push_back(batch);
                err_msgs.push_back(exceptions[batch]);
            }
        }
        if (!combined_error_message.empty()) {
            throw BatchCalculationError(combined_error_message, failed_scenarios, err_msgs);
        }
    }
};

} // namespace power_grid_model
