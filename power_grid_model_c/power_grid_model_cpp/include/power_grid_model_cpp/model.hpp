// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_MODEL_HPP
#define POWER_GRID_MODEL_CPP_MODEL_HPP

#include "basics.hpp"
#include "dataset.hpp"
#include "handle.hpp"
#include "logger.hpp"
#include "options.hpp"

#include "power_grid_model_c/model.h"

#include <vector>

namespace power_grid_model_cpp {
struct StateOutputRequest {
        bool y_bus{};
        bool jacobian{};
};

class StateOutput {
    public:
        StateOutput(StateOutput&&) noexcept = default;
        StateOutput& operator=(StateOutput&&) noexcept = default;
        StateOutput(StateOutput const&) = delete;
        StateOutput& operator=(StateOutput const&) = delete;

        Idx scenario_count() const { return handle_.call_with(PGM_state_output_scenario_count, output_.get()); }
        PGM_StateScenarioView scenario(Idx scenario_idx) const {
                PGM_StateScenarioView view{};
                handle_.call_with(PGM_state_output_get_scenario, output_.get(), scenario_idx, &view);
                return view;
        }
        PGM_StateGroupView group(Idx scenario_idx, Idx group_idx) const {
                PGM_StateGroupView view{};
                handle_.call_with(PGM_state_output_get_group, output_.get(), scenario_idx, group_idx, &view);
                return view;
        }
        Idx jacobian_history_count(Idx scenario_idx, Idx group_idx) const {
            return handle_.call_with(PGM_state_output_jacobian_history_count, output_.get(), scenario_idx,
                         group_idx);
        }
        PGM_StateJacobianView jacobian_at_iteration(Idx scenario_idx, Idx group_idx, Idx iteration_idx) const {
            PGM_StateJacobianView view{};
            handle_.call_with(PGM_state_output_get_jacobian_history, output_.get(), scenario_idx, group_idx,
                      iteration_idx, &view);
            return view;
        }

    private:
        friend class Model;
        explicit StateOutput(RawStateOutput* output) : output_{output} {}

        Handle handle_{};
        detail::UniquePtr<RawStateOutput, &PGM_destroy_state_output> output_;
};

class Model {
  public:
    Model(double system_frequency, DatasetConst const& input_dataset)
        : model_{handle_.call_with(PGM_create_model, system_frequency, input_dataset.get())} {}
    // Copy construction binds the copy to a fresh Handle: the new Model starts with no
    // logger registrations, even if `other` has loggers registered.
    Model(Model const& other) : model_{handle_.call_with(PGM_copy_model, other.get())} {}
    // Copy assignment keeps this Model's Handle (and its existing logger registrations);
    // only the underlying model data is replaced.
    Model& operator=(Model const& other) {
        if (this != &other) {
            model_.reset(handle_.call_with(PGM_copy_model, other.get()));
        }
        return *this;
    }
    // Move construction/assignment transfers the Handle, including any logger registrations.
    Model(Model&& other) noexcept : handle_{std::move(other.handle_)}, model_{std::move(other.model_)} {}
    Model& operator=(Model&& other) noexcept {
        if (this != &other) {
            handle_ = std::move(other.handle_);
            model_ = std::move(other.model_);
        }
        return *this;
    }
    ~Model() = default;

    PowerGridModel const* get() const { return model_.get(); }
    PowerGridModel* get() { return model_.get(); }

    void update(DatasetConst const& update_dataset) {
        handle_.call_with(PGM_update_model, get(), update_dataset.get());
    }

    void get_indexer(std::string const& component, Idx size, ID const* ids, Idx* indexer) const {
        handle_.call_with(PGM_get_indexer, get(), component.c_str(), size, ids, indexer);
    }

    void calculate(Options const& opt, DatasetMutable const& output_dataset, DatasetConst const& batch_dataset) {
        handle_.call_with(PGM_calculate, get(), opt.get(), output_dataset.get(), batch_dataset.get());
    }

    void calculate(Options const& opt, DatasetMutable const& output_dataset) {
        handle_.call_with(PGM_calculate, get(), opt.get(), output_dataset.get(), nullptr);
    }

    StateOutput calculate_with_state(Options const& opt, DatasetMutable const& output_dataset,
                                     std::vector<StateOutputRequest> const& requests) {
        return calculate_with_state_impl(opt, output_dataset, nullptr, requests);
    }

    StateOutput calculate_with_state(Options const& opt, DatasetMutable const& output_dataset,
                                     DatasetConst const& batch_dataset,
                                     std::vector<StateOutputRequest> const& requests) {
        return calculate_with_state_impl(opt, output_dataset, batch_dataset.get(), requests);
    }

    // Attach a logger so it receives output from calculations performed on this model.
    // Attaching the same logger twice is a no-op. See logger.hpp for lifetime notes: the
    // logger may safely be destroyed while still registered, but it can then no longer be
    // targeted individually via remove_logger() (use remove_all_loggers() instead).
    void add_logger(Logger& logger) { handle_.register_logger(logger); }

    // Detach a specific logger from this model. A no-op if it is not registered.
    void remove_logger(Logger& logger) { handle_.unregister_logger(logger); }

    // Detach every logger currently registered to this model.
    void remove_all_loggers() { handle_.unregister_all_loggers(); }

  private:
        StateOutput calculate_with_state_impl(Options const& opt, DatasetMutable const& output_dataset,
                                                                                    RawConstDataset const* batch_dataset,
                                                                                    std::vector<StateOutputRequest> const& requests) {
                std::vector<PGM_StateOutputRequest> raw_requests;
                raw_requests.reserve(requests.size());
                for (auto const& request : requests) {
                    raw_requests.push_back({static_cast<Idx>(request.y_bus), static_cast<Idx>(request.jacobian)});
                }
                RawStateOutput* state_output{};
                handle_.call_with(PGM_calculate_with_state, get(), opt.get(), output_dataset.get(), batch_dataset,
                                                    raw_requests.data(), static_cast<Idx>(raw_requests.size()), &state_output);
                return StateOutput{state_output};
        }

    Handle handle_{};
    detail::UniquePtr<PowerGridModel, &PGM_destroy_model> model_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_MODEL_HPP
