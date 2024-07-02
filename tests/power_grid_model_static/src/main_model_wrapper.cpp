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

template <typename Component, typename MathOutputType, std::forward_iterator ResIt>
ResIt MainModelWrapper::output_result(MathOutputType const& math_output, ResIt res_it) const {
    assert(impl_ != nullptr);
    return impl_->output_result<Component>(math_output, res_it);
}

template std::vector<typename Node::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Node, SymMathOutput, std::vector<typename Node::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Node::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Line::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Line, SymMathOutput, std::vector<typename Line::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Line::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Link::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Link, SymMathOutput, std::vector<typename Link::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Link::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Transformer::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Transformer, SymMathOutput,
                                std::vector<typename Transformer::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename Transformer::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename ThreeWindingTransformer::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<ThreeWindingTransformer, SymMathOutput,
                                std::vector<typename ThreeWindingTransformer::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename ThreeWindingTransformer::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Shunt::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Shunt, SymMathOutput, std::vector<typename Shunt::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Shunt::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Source::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Source, SymMathOutput, std::vector<typename Source::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Source::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename SymGenerator::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<SymGenerator, SymMathOutput,
                                std::vector<typename SymGenerator::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename SymGenerator::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename AsymGenerator::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<AsymGenerator, SymMathOutput,
                                std::vector<typename AsymGenerator::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename AsymGenerator::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename SymLoad::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<SymLoad, SymMathOutput,
                                std::vector<typename SymLoad::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename SymLoad::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename AsymLoad::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<AsymLoad, SymMathOutput,
                                std::vector<typename AsymLoad::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename AsymLoad::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename SymPowerSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<SymPowerSensor, SymMathOutput,
                                std::vector<typename SymPowerSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename SymPowerSensor::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename AsymPowerSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<AsymPowerSensor, SymMathOutput,
                                std::vector<typename AsymPowerSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename AsymPowerSensor::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename SymVoltageSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<SymVoltageSensor, SymMathOutput,
                                std::vector<typename SymVoltageSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename SymVoltageSensor::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename AsymVoltageSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<AsymVoltageSensor, SymMathOutput,
                                std::vector<typename AsymVoltageSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename AsymVoltageSensor::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Fault::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Fault, SymMathOutput, std::vector<typename Fault::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Fault::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename TransformerTapRegulator::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<TransformerTapRegulator, SymMathOutput,
                                std::vector<typename TransformerTapRegulator::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename TransformerTapRegulator::OutputType<symmetric_t>>::iterator res_it) const;

template std::vector<typename Branch::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Branch, SymMathOutput, std::vector<typename Branch::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Branch::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Branch3::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Branch3, SymMathOutput,
                                std::vector<typename Branch3::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Branch3::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename Appliance::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<Appliance, SymMathOutput,
                                std::vector<typename Appliance::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output, std::vector<typename Appliance::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename GenericLoadGen::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<GenericLoadGen, SymMathOutput,
                                std::vector<typename GenericLoadGen::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename GenericLoadGen::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename GenericLoad::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<GenericLoad, SymMathOutput,
                                std::vector<typename GenericLoad::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename GenericLoad::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename GenericGenerator::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<GenericGenerator, SymMathOutput,
                                std::vector<typename GenericGenerator::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename GenericGenerator::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename GenericPowerSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<GenericPowerSensor, SymMathOutput,
                                std::vector<typename GenericPowerSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename GenericPowerSensor::OutputType<symmetric_t>>::iterator res_it) const;
template std::vector<typename GenericVoltageSensor::OutputType<symmetric_t>>::iterator
MainModelWrapper::output_result<GenericVoltageSensor, SymMathOutput,
                                std::vector<typename GenericVoltageSensor::OutputType<symmetric_t>>::iterator>(
    SymMathOutput const& math_output,
    std::vector<typename GenericVoltageSensor::OutputType<symmetric_t>>::iterator res_it) const;

template std::vector<typename Node::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Node, AsymMathOutput, std::vector<typename Node::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Node::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Line::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Line, AsymMathOutput, std::vector<typename Line::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Line::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Link::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Link, AsymMathOutput, std::vector<typename Link::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Link::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Transformer::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Transformer, AsymMathOutput,
                                std::vector<typename Transformer::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename Transformer::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename ThreeWindingTransformer::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<ThreeWindingTransformer, AsymMathOutput,
                                std::vector<typename ThreeWindingTransformer::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename ThreeWindingTransformer::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Shunt::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Shunt, AsymMathOutput, std::vector<typename Shunt::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Shunt::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Source::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Source, AsymMathOutput,
                                std::vector<typename Source::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Source::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename SymGenerator::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<SymGenerator, AsymMathOutput,
                                std::vector<typename SymGenerator::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename SymGenerator::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename AsymGenerator::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<AsymGenerator, AsymMathOutput,
                                std::vector<typename AsymGenerator::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename AsymGenerator::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename SymLoad::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<SymLoad, AsymMathOutput,
                                std::vector<typename SymLoad::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename SymLoad::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename AsymLoad::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<AsymLoad, AsymMathOutput,
                                std::vector<typename AsymLoad::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename AsymLoad::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename SymPowerSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<SymPowerSensor, AsymMathOutput,
                                std::vector<typename SymPowerSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename SymPowerSensor::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename AsymPowerSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<AsymPowerSensor, AsymMathOutput,
                                std::vector<typename AsymPowerSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename AsymPowerSensor::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename SymVoltageSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<SymVoltageSensor, AsymMathOutput,
                                std::vector<typename SymVoltageSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename SymVoltageSensor::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename AsymVoltageSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<AsymVoltageSensor, AsymMathOutput,
                                std::vector<typename AsymVoltageSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename AsymVoltageSensor::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Fault::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Fault, AsymMathOutput, std::vector<typename Fault::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Fault::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename TransformerTapRegulator::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<TransformerTapRegulator, AsymMathOutput,
                                std::vector<typename TransformerTapRegulator::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename TransformerTapRegulator::OutputType<asymmetric_t>>::iterator res_it) const;

template std::vector<typename Branch::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Branch, AsymMathOutput,
                                std::vector<typename Branch::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Branch::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Branch3::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Branch3, AsymMathOutput,
                                std::vector<typename Branch3::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output, std::vector<typename Branch3::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename Appliance::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<Appliance, AsymMathOutput,
                                std::vector<typename Appliance::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename Appliance::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename GenericLoadGen::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<GenericLoadGen, AsymMathOutput,
                                std::vector<typename GenericLoadGen::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename GenericLoadGen::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename GenericLoad::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<GenericLoad, AsymMathOutput,
                                std::vector<typename GenericLoad::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename GenericLoad::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename GenericGenerator::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<GenericGenerator, AsymMathOutput,
                                std::vector<typename GenericGenerator::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename GenericGenerator::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename GenericPowerSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<GenericPowerSensor, AsymMathOutput,
                                std::vector<typename GenericPowerSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename GenericPowerSensor::OutputType<asymmetric_t>>::iterator res_it) const;
template std::vector<typename GenericVoltageSensor::OutputType<asymmetric_t>>::iterator
MainModelWrapper::output_result<GenericVoltageSensor, AsymMathOutput,
                                std::vector<typename GenericVoltageSensor::OutputType<asymmetric_t>>::iterator>(
    AsymMathOutput const& math_output,
    std::vector<typename GenericVoltageSensor::OutputType<asymmetric_t>>::iterator res_it) const;

template std::vector<typename Node::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Node, SymScMathOutput, std::vector<typename Node::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Node::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Line::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Line, SymScMathOutput, std::vector<typename Line::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Line::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Link::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Link, SymScMathOutput, std::vector<typename Link::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Link::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Transformer::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Transformer, SymScMathOutput,
                                std::vector<typename Transformer::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename Transformer::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<ThreeWindingTransformer, SymScMathOutput,
                                std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Shunt::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Shunt, SymScMathOutput, std::vector<typename Shunt::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Shunt::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Source::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Source, SymScMathOutput,
                                std::vector<typename Source::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Source::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymGenerator, SymScMathOutput,
                                std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymGenerator, SymScMathOutput,
                                std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymLoad, SymScMathOutput,
                                std::vector<typename SymLoad::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename SymLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymLoad, SymScMathOutput,
                                std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymPowerSensor, SymScMathOutput,
                                std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymPowerSensor, SymScMathOutput,
                                std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymVoltageSensor, SymScMathOutput,
                                std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymVoltageSensor, SymScMathOutput,
                                std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Fault::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Fault, SymScMathOutput, std::vector<typename Fault::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Fault::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<TransformerTapRegulator, SymScMathOutput,
                                std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator res_it) const;

template std::vector<typename Branch::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Branch, SymScMathOutput,
                                std::vector<typename Branch::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Branch::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Branch3::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Branch3, SymScMathOutput,
                                std::vector<typename Branch3::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Branch3::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Appliance::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Appliance, SymScMathOutput,
                                std::vector<typename Appliance::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output, std::vector<typename Appliance::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericLoadGen, SymScMathOutput,
                                std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericLoad, SymScMathOutput,
                                std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericGenerator, SymScMathOutput,
                                std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericPowerSensor, SymScMathOutput,
                                std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericVoltageSensor, SymScMathOutput,
                                std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator>(
    SymScMathOutput const& math_output,
    std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;

template std::vector<typename Node::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Node, AsymScMathOutput, std::vector<typename Node::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Node::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Line::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Line, AsymScMathOutput, std::vector<typename Line::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Line::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Link::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Link, AsymScMathOutput, std::vector<typename Link::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Link::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Transformer::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Transformer, AsymScMathOutput,
                                std::vector<typename Transformer::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename Transformer::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<ThreeWindingTransformer, AsymScMathOutput,
                                std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename ThreeWindingTransformer::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Shunt::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Shunt, AsymScMathOutput, std::vector<typename Shunt::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Shunt::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Source::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Source, AsymScMathOutput,
                                std::vector<typename Source::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Source::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymGenerator, AsymScMathOutput,
                                std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename SymGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymGenerator, AsymScMathOutput,
                                std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename AsymGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymLoad, AsymScMathOutput,
                                std::vector<typename SymLoad::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename SymLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymLoad, AsymScMathOutput,
                                std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename AsymLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymPowerSensor, AsymScMathOutput,
                                std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename SymPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymPowerSensor, AsymScMathOutput,
                                std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename AsymPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<SymVoltageSensor, AsymScMathOutput,
                                std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename SymVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<AsymVoltageSensor, AsymScMathOutput,
                                std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename AsymVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Fault::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Fault, AsymScMathOutput, std::vector<typename Fault::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Fault::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<TransformerTapRegulator, AsymScMathOutput,
                                std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename TransformerTapRegulator::ShortCircuitOutputType>::iterator res_it) const;

template std::vector<typename Branch::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Branch, AsymScMathOutput,
                                std::vector<typename Branch::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Branch::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Branch3::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Branch3, AsymScMathOutput,
                                std::vector<typename Branch3::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output, std::vector<typename Branch3::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename Appliance::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<Appliance, AsymScMathOutput,
                                std::vector<typename Appliance::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename Appliance::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericLoadGen, AsymScMathOutput,
                                std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename GenericLoadGen::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericLoad, AsymScMathOutput,
                                std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename GenericLoad::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericGenerator, AsymScMathOutput,
                                std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename GenericGenerator::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericPowerSensor, AsymScMathOutput,
                                std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename GenericPowerSensor::ShortCircuitOutputType>::iterator res_it) const;
template std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator
MainModelWrapper::output_result<GenericVoltageSensor, AsymScMathOutput,
                                std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator>(
    AsymScMathOutput const& math_output,
    std::vector<typename GenericVoltageSensor::ShortCircuitOutputType>::iterator res_it) const;

} // namespace power_grid_model::pgm_static
