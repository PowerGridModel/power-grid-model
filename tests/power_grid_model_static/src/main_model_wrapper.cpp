// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_static/main_model_wrapper.hpp>

#include <power_grid_model/main_model.hpp>

namespace power_grid_model::pgm_static {
namespace {
using SymMathOutput = MathOutput<std::vector<SolverOutput<symmetric_t>>>;
using AsymMathOutput = MathOutput<std::vector<SolverOutput<asymmetric_t>>>;
using SymScMathOutput = MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>>;
using AsymScMathOutput = MathOutput<std::vector<ShortCircuitSolverOutput<asymmetric_t>>>;
} // namespace

class MainModelWrapper::Impl : public MainModel {
  public:
    using MainModel::MainModel;
};

MainModelWrapper::MainModelWrapper(double system_frequency, ConstDataset const& input_data, Idx pos)
    : impl_{std::make_unique<Impl>(system_frequency, input_data, pos)} {}

MainModelWrapper::MainModelWrapper(double system_frequency, meta_data::MetaData const& meta_data)
    : impl_{std::make_unique<Impl>(system_frequency, meta_data)} {}

// deep copy
MainModelWrapper::MainModelWrapper(MainModelWrapper const& other)
    : impl_{other.impl_ == nullptr ? nullptr : new Impl{*other.impl_}} {}
MainModelWrapper& MainModelWrapper::operator=(MainModelWrapper& other) {
    impl_.reset(other.impl_ == nullptr ? nullptr : new Impl{*other.impl_});
    return *this;
}
MainModelWrapper::MainModelWrapper(MainModelWrapper&& other) = default;
MainModelWrapper& MainModelWrapper::operator=(MainModelWrapper&& /* other */) = default;
MainModelWrapper::~MainModelWrapper() = default;

bool MainModelWrapper::is_update_independent(ConstDataset const& update_data) {
    return Impl::is_update_independent(update_data);
}

std::map<std::string, Idx> MainModelWrapper::all_component_count() const {
    assert(impl_ != nullptr);
    return impl_->all_component_count();
}

void MainModelWrapper::get_indexer(std::string_view component_type, ID const* id_begin, Idx size,
                                   Idx* indexer_begin) const {
    assert(impl_ != nullptr);
    impl_->get_indexer(component_type, id_begin, size, indexer_begin);
}

void MainModelWrapper::set_construction_complete() {
    assert(impl_ != nullptr);
    impl_->set_construction_complete();
}

void MainModelWrapper::restore_components(ConstDataset const& update_data) {
    assert(impl_ != nullptr);
    impl_->restore_components(impl_->get_sequence_idx_map(update_data));
}

void MainModelWrapper::add_components(ConstDataset const& input_data, Idx pos) {
    assert(impl_ != nullptr);
    impl_->add_components(input_data, pos);
}

template <solver_output_type SolverOutputType>
void MainModelWrapper::output_result(MathOutput<std::vector<SolverOutputType>> const& math_output,
                                     MutableDataset const& result_data, Idx pos) const {
    assert(impl_ != nullptr);
    impl_->output_result(math_output, result_data, pos);
}
template void MainModelWrapper::output_result(MathOutput<std::vector<SolverOutput<symmetric_t>>> const& math_output,
                                              MutableDataset const& result_data, Idx pos) const;
template void MainModelWrapper::output_result(MathOutput<std::vector<SolverOutput<asymmetric_t>>> const& math_output,
                                              MutableDataset const& result_data, Idx pos) const;
template void
MainModelWrapper::output_result(MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>> const& math_output,
                                MutableDataset const& result_data, Idx pos) const;
template void
MainModelWrapper::output_result(MathOutput<std::vector<ShortCircuitSolverOutput<asymmetric_t>>> const& math_output,
                                MutableDataset const& result_data, Idx pos) const;

template <cache_type_c CacheType> void MainModelWrapper::update_component(ConstDataset const& update_data, Idx pos) {
    assert(impl_ != nullptr);
    impl_->update_component<CacheType>(update_data, pos);
}
template void MainModelWrapper::update_component<cached_update_t>(ConstDataset const& update_data, Idx pos);
template void MainModelWrapper::update_component<permanent_update_t>(ConstDataset const& update_data, Idx pos);

template <symmetry_tag sym>
MathOutput<std::vector<SolverOutput<sym>>> MainModelWrapper::calculate_power_flow(Options const& options) {
    assert(impl_ != nullptr);
    return impl_->calculate_power_flow<sym>(options);
}
template SymMathOutput MainModelWrapper::calculate_power_flow<symmetric_t>(Options const& options);
template AsymMathOutput MainModelWrapper::calculate_power_flow<asymmetric_t>(Options const& option);
template <symmetry_tag sym>
void MainModelWrapper::calculate_power_flow(Options const& options, MutableDataset const& result_data, Idx pos) {
    assert(impl_ != nullptr);
    return impl_->calculate_power_flow<sym>(options, result_data, pos);
}
template void MainModelWrapper::calculate_power_flow<symmetric_t>(Options const& options,
                                                                  MutableDataset const& result_data, Idx pos);
template void MainModelWrapper::calculate_power_flow<asymmetric_t>(Options const& option,
                                                                   MutableDataset const& result_data, Idx pos);
template <symmetry_tag sym>
BatchParameter MainModelWrapper::calculate_power_flow(Options const& options, MutableDataset const& result_data,
                                                      ConstDataset const& update_data) {
    assert(impl_ != nullptr);
    return impl_->calculate_power_flow<sym>(options, result_data, update_data);
}
template BatchParameter MainModelWrapper::calculate_power_flow<symmetric_t>(Options const& options,
                                                                            MutableDataset const& result_data,
                                                                            ConstDataset const& update_data);
template BatchParameter MainModelWrapper::calculate_power_flow<asymmetric_t>(Options const& options,
                                                                             MutableDataset const& result_data,
                                                                             ConstDataset const& update_data);

template <symmetry_tag sym>
MathOutput<std::vector<SolverOutput<sym>>> MainModelWrapper::calculate_state_estimation(Options const& options) {
    assert(impl_ != nullptr);
    return impl_->calculate_state_estimation<sym>(options);
}
template SymMathOutput MainModelWrapper::calculate_state_estimation<symmetric_t>(Options const& options);
template AsymMathOutput MainModelWrapper::calculate_state_estimation<asymmetric_t>(Options const& option);
template <symmetry_tag sym>
BatchParameter MainModelWrapper::calculate_state_estimation(Options const& options, MutableDataset const& result_data,
                                                            ConstDataset const& update_data) {
    assert(impl_ != nullptr);
    return impl_->calculate_state_estimation<sym>(options, result_data, update_data);
}
template BatchParameter MainModelWrapper::calculate_state_estimation<symmetric_t>(Options const& options,
                                                                                  MutableDataset const& result_data,
                                                                                  ConstDataset const& update_data);
template BatchParameter MainModelWrapper::calculate_state_estimation<asymmetric_t>(Options const& options,
                                                                                   MutableDataset const& result_data,
                                                                                   ConstDataset const& update_data);
template <symmetry_tag sym>
MathOutput<std::vector<ShortCircuitSolverOutput<sym>>>
MainModelWrapper::calculate_short_circuit(Options const& options) {
    assert(impl_ != nullptr);
    return impl_->calculate_short_circuit<sym>(options);
}
template SymScMathOutput MainModelWrapper::calculate_short_circuit<symmetric_t>(Options const& options);
template AsymScMathOutput MainModelWrapper::calculate_short_circuit<asymmetric_t>(Options const& option);
void MainModelWrapper::calculate_short_circuit(Options const& options, MutableDataset const& result_data, Idx pos) {
    assert(impl_ != nullptr);
    return impl_->calculate_short_circuit(options, result_data, pos);
}
BatchParameter MainModelWrapper::calculate_short_circuit(Options const& options, MutableDataset const& result_data,
                                                         ConstDataset const& update_data) {
    assert(impl_ != nullptr);
    return impl_->calculate_short_circuit(options, result_data, update_data);
}

template <typename CompType, typename MathOutputType, typename OutputType>
void MainModelWrapper::output_extra_retreivable_result(MathOutputType const& math_output,
                                                       std::span<OutputType> target) const {
    assert(impl_ != nullptr);
    impl_->output_result<CompType>(math_output, target.begin());
}

template void
MainModelWrapper::output_extra_retreivable_result<Branch, SymMathOutput, typename Branch::OutputType<symmetric_t>>(
    SymMathOutput const& math_output, std::span<typename Branch::OutputType<symmetric_t>> target) const;
template void
MainModelWrapper::output_extra_retreivable_result<Branch3, SymMathOutput, typename Branch3::OutputType<symmetric_t>>(
    SymMathOutput const& math_output, std::span<typename Branch3::OutputType<symmetric_t>> target) const;
template void MainModelWrapper::output_extra_retreivable_result<Appliance, SymMathOutput,
                                                                typename Appliance::OutputType<symmetric_t>>(
    SymMathOutput const& math_output, std::span<typename Appliance::OutputType<symmetric_t>> target) const;

template void
MainModelWrapper::output_extra_retreivable_result<Branch, AsymMathOutput, typename Branch::OutputType<asymmetric_t>>(
    AsymMathOutput const& math_output, std::span<typename Branch::OutputType<asymmetric_t>> target) const;
template void
MainModelWrapper::output_extra_retreivable_result<Branch3, AsymMathOutput, typename Branch3::OutputType<asymmetric_t>>(
    AsymMathOutput const& math_output, std::span<typename Branch3::OutputType<asymmetric_t>> target) const;
template void MainModelWrapper::output_extra_retreivable_result<Appliance, AsymMathOutput,
                                                                typename Appliance::OutputType<asymmetric_t>>(
    AsymMathOutput const& math_output, std::span<typename Appliance::OutputType<asymmetric_t>> target) const;

} // namespace power_grid_model::pgm_static
