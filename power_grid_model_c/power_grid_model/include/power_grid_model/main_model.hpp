// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_CXX_MODULE_PURVIEW
#include "all_components.hpp"
#include "job_adapter.hpp"
#include "job_dispatch.hpp"
#include "main_model_impl.hpp"
#endif

#ifndef PGM_CXX_MODULE_PURVIEW
#include "auxiliary/dataset.hpp"
#include "auxiliary/meta_data.hpp"
#include "batch_parameter.hpp"
#include "calculation_preparation.hpp"
#include "common/common.hpp"
#include "common/logging.hpp"
#include "common/multi_threaded_logging.hpp"
#include "main_core/main_model_type.hpp"
#include "main_model_fwd.hpp"
#include "math_solver/math_solver_dispatch.hpp"
#endif

#ifndef PGM_CXX_MODULE_PURVIEW
#include <cassert>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>
#endif

namespace power_grid_model {

// main model class

class MainModel {
  private:
    using Impl = MainModelImpl<main_core::MainModelType<AllExtraRetrievableTypes, AllComponents>>;
    using MultiThreadedLogger = common::logging::MultiThreadedLogger;
    using NoMultiThreadedLogger = common::logging::NoMultiThreadedLogger;

  public:
    using Options = MainModelOptions;

    explicit MainModel(double system_frequency, ConstDataset const& input_data,
                       MathSolverDispatcher const& math_solver_dispatcher, Idx pos = 0,
                       MultiThreadedLogger& logger = no_logger_);
    explicit MainModel(double system_frequency, meta_data::MetaData const& meta_data,
                       MathSolverDispatcher const& math_solver_dispatcher, MultiThreadedLogger& logger = no_logger_);

    // deep copy
    MainModel(MainModel const& other);
    MainModel(MainModel other, MultiThreadedLogger& logger);
    MainModel& operator=(MainModel const& other);
    MainModel(MainModel&& other) noexcept;
    MainModel& operator=(MainModel&& other) noexcept;
    ~MainModel();

    void get_indexer(std::string_view component_type, ID const* id_begin, Idx size, Idx* indexer_begin) const;

    void set_logger(MultiThreadedLogger& logger) { logger_ = logger; }

    bool logger_empty() const noexcept { return std::addressof(logger_.get()) == std::addressof(no_logger_); }

    void reset_logger() noexcept { logger_ = no_logger_; }

    template <cache_type_c CacheType> void update_components(ConstDataset const& update_data);

    /*
    Batch calculation, propagating the results to result_data

    Run the calculation function in batch on the provided update data.

    The calculation function should be able to run standalone.

    threading
        < 0 sequential
        = 0 parallel, use number of hardware threads
        > 0 specify number of parallel threads
    raise a BatchCalculationError if any of the calculations in the batch raised an exception
    */
    BatchParameter calculate(Options const& options, MutableDataset const& result_data,
                             ConstDataset const& update_data);

    void check_no_experimental_features_used(Options const& options, ConstDataset const* batch_dataset) const;

    void check_no_future_deprecations(Options const& options, ConstDataset const* batch_dataset) const;

  private:
    Impl& impl() {
        assert(impl_ != nullptr);
        return *impl_;
    }
    Impl const& impl() const {
        assert(impl_ != nullptr);
        return *impl_;
    }

    std::unique_ptr<Impl> impl_;
    inline static NoMultiThreadedLogger no_logger_{};
    std::reference_wrapper<MultiThreadedLogger> logger_{no_logger_};
};

// With C++ modules, the module interface only declares the class; the (expensive to instantiate) member definitions
// are compiled separately by defining PGM_MAIN_MODEL_DEFINITIONS
#if !defined(PGM_CXX_MODULE_PURVIEW) || defined(PGM_MAIN_MODEL_DEFINE_REST) || defined(PGM_MAIN_MODEL_DEFINE_CALCULATE)
#if defined(PGM_MAIN_MODEL_DEFINE_REST) || defined(PGM_MAIN_MODEL_DEFINE_CALCULATE)
#define PGM_MAIN_MODEL_INLINE
#else
#define PGM_MAIN_MODEL_INLINE inline
#endif
#if !defined(PGM_CXX_MODULE_PURVIEW) || defined(PGM_MAIN_MODEL_DEFINE_REST)
#define PGM_MAIN_MODEL_DEFINE_REST_SECTION
#endif
#if !defined(PGM_CXX_MODULE_PURVIEW) || defined(PGM_MAIN_MODEL_DEFINE_CALCULATE)
#define PGM_MAIN_MODEL_DEFINE_CALCULATE_SECTION
#endif

#ifdef PGM_MAIN_MODEL_DEFINE_REST_SECTION
template <cache_type_c CacheType>
PGM_MAIN_MODEL_INLINE void MainModel::update_components(ConstDataset const& update_data) {
    impl().update_components<CacheType>(update_data.get_individual_scenario(0));
}
#ifdef PGM_MAIN_MODEL_DEFINE_REST
template void MainModel::update_components<permanent_update_t>(ConstDataset const&);
#endif

PGM_MAIN_MODEL_INLINE MainModel::MainModel(double system_frequency, ConstDataset const& input_data,
                                           MathSolverDispatcher const& math_solver_dispatcher, Idx pos,
                                           MultiThreadedLogger& logger)
    : impl_{std::make_unique<Impl>(
          system_frequency, input_data,
          SolverPreparationContext{.math_state = {}, .math_solver_dispatcher = &math_solver_dispatcher}, pos)},
      logger_{logger} {}
PGM_MAIN_MODEL_INLINE MainModel::MainModel(double system_frequency, meta_data::MetaData const& meta_data,
                                           MathSolverDispatcher const& math_solver_dispatcher,
                                           MultiThreadedLogger& logger)
    : impl_{std::make_unique<Impl>(
          system_frequency, meta_data,
          SolverPreparationContext{.math_state = {}, .math_solver_dispatcher = &math_solver_dispatcher})},
      logger_{logger} {}

PGM_MAIN_MODEL_INLINE MainModel::MainModel(MainModel const& other) {
    if (other.impl_ != nullptr) {
        impl_ = std::make_unique<Impl>(*other.impl_);
        logger_ = other.logger_;
    }
}
PGM_MAIN_MODEL_INLINE MainModel::MainModel(MainModel other, MultiThreadedLogger& logger) : MainModel{std::move(other)} {
    logger_ = logger;
}
PGM_MAIN_MODEL_INLINE MainModel& MainModel::operator=(MainModel const& other) {
    if (this != &other) {
        impl_.reset();
        if (other.impl_ != nullptr) {
            impl_ = std::make_unique<Impl>(*other.impl_);
            logger_ = other.logger_;
        }
    }
    return *this;
}
PGM_MAIN_MODEL_INLINE MainModel::MainModel(MainModel&& other) noexcept
    : impl_{std::move(other.impl_)}, logger_{other.logger_} {}
PGM_MAIN_MODEL_INLINE MainModel& MainModel::operator=(MainModel&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
        logger_ = other.logger_;
    }
    return *this;
}
PGM_MAIN_MODEL_INLINE MainModel::~MainModel() { impl_.reset(); }

PGM_MAIN_MODEL_INLINE void MainModel::get_indexer(std::string_view component_type, ID const* id_begin, Idx size,
                                                  Idx* indexer_begin) const {
    impl().get_indexer(component_type, id_begin, size, indexer_begin);
}

#endif // PGM_MAIN_MODEL_DEFINE_REST_SECTION

#ifdef PGM_MAIN_MODEL_DEFINE_CALCULATE_SECTION
PGM_MAIN_MODEL_INLINE BatchParameter MainModel::calculate(Options const& options, MutableDataset const& result_data,
                                                          ConstDataset const& update_data) {
    JobAdapter<Impl> adapter{std::ref(impl()), std::ref(options)};
    return JobDispatch::batch_calculation(adapter, result_data, update_data, options.threading, logger_.get());
}
#endif // PGM_MAIN_MODEL_DEFINE_CALCULATE_SECTION

#ifdef PGM_MAIN_MODEL_DEFINE_REST_SECTION
PGM_MAIN_MODEL_INLINE void MainModel::check_no_experimental_features_used(Options const& options,
                                                                          ConstDataset const* batch_dataset) const {
    impl().check_no_experimental_features_used(options, batch_dataset);
}

PGM_MAIN_MODEL_INLINE void MainModel::check_no_future_deprecations(Options const& options,
                                                                   ConstDataset const* batch_dataset) const {
    impl().check_no_future_deprecations(options, batch_dataset);
}
#endif // PGM_MAIN_MODEL_DEFINE_REST_SECTION

#undef PGM_MAIN_MODEL_INLINE
#undef PGM_MAIN_MODEL_DEFINE_REST_SECTION
#undef PGM_MAIN_MODEL_DEFINE_CALCULATE_SECTION
#endif

} // namespace power_grid_model
