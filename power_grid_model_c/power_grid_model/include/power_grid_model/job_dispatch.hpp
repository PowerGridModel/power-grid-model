// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "job_dispatch_interface.hpp"

#include "main_core/calculation_info.hpp"
#include "main_core/update.hpp"

#include <thread>

namespace power_grid_model {

class JobDispatch {
  public:
    static constexpr Idx ignore_output{-1};
    static constexpr Idx sequential{-1};

    // TODO(figueroa1395): remove calculation_fn dependency
    // TODO(figueroa1395): add concept to Adapter template parameter
    // TODO(figueroa1395): add generic template parameters for update_data and result_data
    template <typename Adapter, typename Calculate>
    static BatchParameter batch_calculation(Adapter& adapter, Calculate&& calculation_fn,
                                            MutableDataset const& result_data, ConstDataset const& update_data,
                                            Idx threading = sequential) {
        if (update_data.empty()) {
            adapter.calculate(std::forward<Calculate>(calculation_fn), result_data);
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
        adapter.cache_calculate(std::forward<Calculate>(calculation_fn));

        // error messages
        std::vector<std::string> exceptions(n_scenarios, "");
        std::vector<CalculationInfo> infos(n_scenarios);

        auto single_job = single_thread_job(adapter, std::forward<Calculate>(calculation_fn), result_data, update_data,
                                            exceptions, infos);

        job_dispatch(single_job, n_scenarios, threading);

        handle_batch_exceptions(exceptions);
        adapter.merge_calculation_infos(infos);

        return BatchParameter{};
    }

  private:
    template <typename Adapter, typename Calculate>
    static auto single_thread_job(Adapter& base_adapter, Calculate&& calculation_fn, MutableDataset const& result_data,
                                  ConstDataset const& update_data, std::vector<std::string>& exceptions,
                                  std::vector<CalculationInfo>& infos) {
        return [&base_adapter, &exceptions, &infos, calculation_fn_ = std::forward<Calculate>(calculation_fn),
                &result_data, &update_data](Idx start, Idx stride, Idx n_scenarios) {
            assert(n_scenarios <= narrow_cast<Idx>(exceptions.size()));
            assert(n_scenarios <= narrow_cast<Idx>(infos.size()));

            Timer const t_total(infos[start], 0000, "Total in thread");

            auto const copy_adapter_functor = [&base_adapter, &infos](Idx scenario_idx) {
                Timer const t_copy_adapter_functor(infos[scenario_idx], 1100, "Copy model");
                return Adapter{base_adapter};
            };
            auto adapter = copy_adapter_functor(start);

            adapter.prepare_job(update_data);
            auto setup = [&adapter, &update_data, &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1200, "Update model");
                adapter.setup(update_data, scenario_idx);
            };

            auto winddown = [&adapter, &infos](Idx scenario_idx) {
                Timer const t_update_model(infos[scenario_idx], 1201, "Restore model");
                adapter.winddown();
            };

            auto calculate_scenario = JobDispatch::call_with<Idx>(
                [&adapter, &calculation_fn_, &result_data, &infos](Idx scenario_idx) {
                    adapter.calculate(calculation_fn_, result_data, scenario_idx);
                    infos[scenario_idx].merge(adapter.get_calculation_info());
                },
                std::move(setup), std::move(winddown), scenario_exception_handler(adapter, exceptions, infos),
                [&adapter, &copy_adapter_functor](Idx scenario_idx) { adapter = copy_adapter_functor(scenario_idx); });

            for (Idx scenario_idx = start; scenario_idx < n_scenarios; scenario_idx += stride) {
                Timer const t_total_single(infos[scenario_idx], 0100, "Total single calculation in thread");

                calculate_scenario(scenario_idx);
            }
        };
    }

    // run sequential if
    //    specified threading < 0
    //    use hardware threads, but it is either unknown (0) or only has one thread (1)
    //    specified threading = 1
    template <typename RunSingleJobFn>
        requires std::invocable<std::remove_cvref_t<RunSingleJobFn>, Idx /*start*/, Idx /*stride*/, Idx /*n_scenarios*/>
    static void job_dispatch(RunSingleJobFn single_thread_job, Idx n_scenarios, Idx threading) {
        // run batches sequential or parallel
        auto const hardware_thread = static_cast<Idx>(std::thread::hardware_concurrency());
        if (threading < 0 || threading == 1 || (threading == 0 && hardware_thread < 2)) {
            // run all in sequential
            single_thread_job(0, 1, n_scenarios);
        } else {
            // create parallel threads
            Idx const n_thread = std::min(threading == 0 ? hardware_thread : threading, n_scenarios);
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

    template <typename... Args, typename RunFn, typename SetupFn, typename WinddownFn, typename HandleExceptionFn,
              typename RecoverFromBadFn>
        requires std::invocable<std::remove_cvref_t<RunFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<SetupFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<WinddownFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<HandleExceptionFn>, Args const&...> &&
                 std::invocable<std::remove_cvref_t<RecoverFromBadFn>, Args const&...>
    static auto call_with(RunFn run, SetupFn setup, WinddownFn winddown, HandleExceptionFn handle_exception,
                          RecoverFromBadFn recover_from_bad) {
        return [setup_ = std::move(setup), run_ = std::move(run), winddown_ = std::move(winddown),
                handle_exception_ = std::move(handle_exception),
                recover_from_bad_ = std::move(recover_from_bad)](Args const&... args) {
            try {
                setup_(args...);
                run_(args...);
                winddown_(args...);
            } catch (...) {
                handle_exception_(args...);
                try {
                    winddown_(args...);
                } catch (...) {
                    recover_from_bad_(args...);
                }
            }
        };
    }

    // Lippincott pattern
    template <typename Adapter>
    static auto scenario_exception_handler(Adapter& adapter, std::vector<std::string>& messages,
                                           std::vector<CalculationInfo>& infos) {
        return [&adapter, &messages, &infos](Idx scenario_idx) {
            std::exception_ptr const ex_ptr = std::current_exception();
            try {
                std::rethrow_exception(ex_ptr);
            } catch (std::exception const& ex) {
                messages[scenario_idx] = ex.what();
            } catch (...) {
                messages[scenario_idx] = "unknown exception";
            }
            infos[scenario_idx].merge(adapter.get_calculation_info());
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
