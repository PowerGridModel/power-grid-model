// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "main_model_fwd.hpp"

#include "main_core/calculation_info.hpp"
#include "main_core/update.hpp"

#include <mutex>
#include <thread>

namespace power_grid_model {

template <class MainModel, class... ComponentType> class JobDispatch {
  private:
    using SequenceIdxView = std::array<std::span<Idx2D const>, main_core::utils::n_types<ComponentType...>>;

  public:
    static constexpr Idx ignore_output{-1};
    static constexpr Idx sequential{-1};

    template <typename Calculate>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    static BatchParameter batch_calculation_(MainModel& model, CalculationInfo& calculation_info,
                                             Calculate&& calculation_fn, MutableDataset const& result_data,
                                             ConstDataset const& update_data, Idx threading = sequential) {
        // if the update dataset is empty without any component
        // execute one power flow in the current instance, no batch calculation is needed
        if (update_data.empty()) {
            std::forward<Calculate>(calculation_fn)(model, result_data, 0);
            return BatchParameter{};
        }

        // get batch size
        Idx const n_scenarios = update_data.batch_size();

        // if the batch_size is zero, it is a special case without doing any calculations at all
        // we consider in this case the batch set is independent but not topology cacheable
        if (n_scenarios == 0) {
            return BatchParameter{};
        }

        // calculate once to cache topology, ignore results, all math solvers are initialized
        try {
            calculation_fn(model,
                           {
                               false,
                               1,
                               "sym_output",
                               model.meta_data(),
                           },
                           ignore_output);
        } catch (SparseMatrixError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        } catch (NotObservableError const&) { // NOLINT(bugprone-empty-catch) // NOSONAR
            // missing entries are provided in the update data
        }

        // error messages
        std::vector<std::string> exceptions(n_scenarios, "");

        // thread-safe handling of calculation info
        std::mutex calculation_info_mutex;
        auto const thread_safe_add_calculation_info = [&calculation_info,
                                                       &calculation_info_mutex](CalculationInfo const& info) {
            std::lock_guard const lock{calculation_info_mutex};
            main_core::merge_into(calculation_info, info);
        };

        // lambda for sub batch calculation
        main_core::utils::SequenceIdx<ComponentType...> all_scenarios_sequence;
        auto sub_batch =
            sub_batch_calculation_(model, std::forward<Calculate>(calculation_fn), result_data, update_data,
                                   all_scenarios_sequence, exceptions, thread_safe_add_calculation_info);

        job_dispatch(sub_batch, n_scenarios, threading);

        handle_batch_exceptions(exceptions);

        return BatchParameter{};
    }

    template <typename Calculate, typename AddCalculationInfo>
        requires std::invocable<std::remove_cvref_t<Calculate>, MainModel&, MutableDataset const&, Idx>
    static auto sub_batch_calculation_(MainModel const& base_model, Calculate&& calculation_fn,
                                       MutableDataset const& result_data, ConstDataset const& update_data,
                                       main_core::utils::SequenceIdx<ComponentType...>& all_scenarios_sequence,
                                       std::vector<std::string>& exceptions,
                                       AddCalculationInfo&& thread_safe_add_calculation_info) {
        // cache component update order where possible.
        // the order for a cacheable (independent) component by definition is the same across all scenarios
        auto const components_to_update = base_model.get_components_to_update(update_data);
        auto const update_independence = main_core::update::independence::check_update_independence<ComponentType...>(
            base_model.state(), update_data);
        all_scenarios_sequence = main_core::update::get_all_sequence_idx_map<ComponentType...>(
            base_model.state(), update_data, 0, components_to_update, update_independence, false);

        return [&base_model, &exceptions, &thread_safe_add_calculation_info,
                calculation_fn_ = std::forward<Calculate>(calculation_fn), &result_data, &update_data,
                &all_scenarios_sequence_ = std::as_const(all_scenarios_sequence), components_to_update,
                update_independence](Idx start, Idx stride, Idx n_scenarios) {
            assert(n_scenarios <= narrow_cast<Idx>(exceptions.size()));

            CalculationInfo thread_info;

            Timer t_total(thread_info, 0000, "Total in thread");

            auto const copy_model_functor = [&base_model, &thread_info] {
                Timer const t_copy_model_functor(thread_info, 1100, "Copy model");
                return MainModel{base_model};
            };
            auto model = copy_model_functor();

            auto current_scenario_sequence_cache = main_core::utils::SequenceIdx<ComponentType...>{};
            auto [setup, winddown] =
                scenario_update_restore(model, update_data, components_to_update, update_independence,
                                        all_scenarios_sequence_, current_scenario_sequence_cache, thread_info);

            auto calculate_scenario = JobDispatch::call_with<Idx>(
                [&model, &calculation_fn_, &result_data, &thread_info](Idx scenario_idx) {
                    calculation_fn_(model, result_data, scenario_idx);
                    main_core::merge_into(thread_info, model.calculation_info());
                },
                std::move(setup), std::move(winddown), scenario_exception_handler(model, exceptions, thread_info),
                [&model, &copy_model_functor](Idx /*scenario_idx*/) { model = copy_model_functor(); });

            for (Idx scenario_idx = start; scenario_idx < n_scenarios; scenario_idx += stride) {
                Timer const t_total_single(thread_info, 0100, "Total single calculation in thread");
                calculate_scenario(scenario_idx);
            }

            t_total.stop();
            thread_safe_add_calculation_info(thread_info);
        };
    }

    template <typename RunSubBatchFn>
        requires std::invocable<std::remove_cvref_t<RunSubBatchFn>, Idx /*start*/, Idx /*stride*/, Idx /*n_scenarios*/>
    static void job_dispatch(RunSubBatchFn sub_batch, Idx n_scenarios, Idx threading) {
        // run batches sequential or parallel
        auto const n_thread = n_threads(n_scenarios, threading);
        if (n_thread == 1) {
            // run all in sequential
            sub_batch(0, 1, n_scenarios);
        } else {
            // create parallel threads
            std::vector<std::thread> threads;
            threads.reserve(n_thread);
            for (Idx thread_number = 0; thread_number < n_thread; ++thread_number) {
                // compute each sub batch with stride
                threads.emplace_back(sub_batch, thread_number, n_thread, n_scenarios);
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

    static auto scenario_update_restore(
        MainModel& model, ConstDataset const& update_data,
        main_core::utils::ComponentFlags<ComponentType...> const& components_to_store,
        main_core::update::independence::UpdateIndependence<ComponentType...> const& do_update_cache,
        main_core::utils::SequenceIdx<ComponentType...> const& all_scenario_sequence,
        main_core::utils::SequenceIdx<ComponentType...>& current_scenario_sequence_cache,
        CalculationInfo& info) noexcept {
        main_core::utils::ComponentFlags<ComponentType...> independence_flags{};
        std::ranges::transform(do_update_cache, independence_flags.begin(),
                               [](auto const& comp) { return comp.is_independent(); });
        auto const scenario_sequence = [&all_scenario_sequence, &current_scenario_sequence_cache,
                                        independence_flags_ = std::move(independence_flags)]() -> SequenceIdxView {
            return main_core::utils::run_functor_with_all_types_return_array<ComponentType...>(
                [&all_scenario_sequence, &current_scenario_sequence_cache, &independence_flags_]<typename CT>() {
                    constexpr auto comp_idx = main_core::utils::index_of_component<CT, ComponentType...>;
                    if (std::get<comp_idx>(independence_flags_)) {
                        return std::span<Idx2D const>{std::get<comp_idx>(all_scenario_sequence)};
                    }
                    return std::span<Idx2D const>{std::get<comp_idx>(current_scenario_sequence_cache)};
                });
        };

        return std::make_pair(
            [&model, &update_data, scenario_sequence, &current_scenario_sequence_cache, &components_to_store,
             do_update_cache_ = std::move(do_update_cache), &info](Idx scenario_idx) {
                Timer const t_update_model(info, 1200, "Update model");
                current_scenario_sequence_cache = main_core::update::get_all_sequence_idx_map<ComponentType...>(
                    model.state(), update_data, scenario_idx, components_to_store, do_update_cache_, true);

                model.template update_components<cached_update_t>(update_data, scenario_idx, scenario_sequence());
            },
            [&model, scenario_sequence, &current_scenario_sequence_cache, &info](Idx /*scenario_idx*/) {
                Timer const t_update_model(info, 1201, "Restore model");

                model.restore_components(scenario_sequence());
                std::ranges::for_each(current_scenario_sequence_cache,
                                      [](auto& comp_seq_idx) { comp_seq_idx.clear(); });
            });
    }

    // Lippincott pattern
    static auto scenario_exception_handler(MainModel& model, std::vector<std::string>& messages,
                                           CalculationInfo& info) {
        return [&model, &messages, &info](Idx scenario_idx) {
            std::exception_ptr const ex_ptr = std::current_exception();
            try {
                std::rethrow_exception(ex_ptr);
            } catch (std::exception const& ex) {
                messages[scenario_idx] = ex.what();
            } catch (...) {
                messages[scenario_idx] = "unknown exception";
            }
            info.merge(model.calculation_info());
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
