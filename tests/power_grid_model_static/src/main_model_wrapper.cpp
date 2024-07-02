// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_static/main_model_wrapper.hpp>

#include <power_grid_model/main_model.hpp>

namespace power_grid_model::pgm_static {
namespace {
using SymMathOutput = MathOutput<std::vector<SolverOutput<symmetric_t>>>;
using AsymMathOutput = MathOutput<std::vector<SolverOutput<asymmetric_t>>>;
// using SymScMathOutput = MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>>;
// using AsymScMathOutput = MathOutput<std::vector<ShortCircuitSolverOutput<asymmetric_t>>>;
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

template <class CompType>
void MainModelWrapper::add_component(std::span<typename CompType::InputType const> components) {
    assert(impl_ != nullptr);
    impl_->add_component<CompType>(components);
}

template void MainModelWrapper::add_component<Node>(std::span<typename Node::InputType const> components);
template void MainModelWrapper::add_component<Line>(std::span<typename Line::InputType const> components);
template void MainModelWrapper::add_component<Link>(std::span<typename Link::InputType const> components);
template void MainModelWrapper::add_component<Transformer>(std::span<typename Transformer::InputType const> components);
template void MainModelWrapper::add_component<ThreeWindingTransformer>(
    std::span<typename ThreeWindingTransformer::InputType const> components);
template void MainModelWrapper::add_component<Shunt>(std::span<typename Shunt::InputType const> components);
template void MainModelWrapper::add_component<Source>(std::span<typename Source::InputType const> components);
template void
MainModelWrapper::add_component<SymGenerator>(std::span<typename SymGenerator::InputType const> components);
template void
MainModelWrapper::add_component<AsymGenerator>(std::span<typename AsymGenerator::InputType const> components);
template void MainModelWrapper::add_component<SymLoad>(std::span<typename SymLoad::InputType const> components);
template void MainModelWrapper::add_component<AsymLoad>(std::span<typename AsymLoad::InputType const> components);
template void
MainModelWrapper::add_component<SymPowerSensor>(std::span<typename SymPowerSensor::InputType const> components);
template void
MainModelWrapper::add_component<AsymPowerSensor>(std::span<typename AsymPowerSensor::InputType const> components);
template void
MainModelWrapper::add_component<SymVoltageSensor>(std::span<typename SymVoltageSensor::InputType const> components);
template void
MainModelWrapper::add_component<AsymVoltageSensor>(std::span<typename AsymVoltageSensor::InputType const> components);
template void MainModelWrapper::add_component<Fault>(std::span<typename Fault::InputType const> components);
template void MainModelWrapper::add_component<TransformerTapRegulator>(
    std::span<typename TransformerTapRegulator::InputType const> components);

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

} // namespace power_grid_model::pgm_static
