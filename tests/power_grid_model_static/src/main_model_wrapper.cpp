// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_static/main_model_wrapper.hpp>

#include <power_grid_model/main_model.hpp>

namespace power_grid_model::pgm_static {

class MainModelWrapper::Impl : public MainModel {
  public:
    using MainModel::MainModel;
};

MainModelWrapper::MainModelWrapper(double system_frequency, ConstDataset const& input_data, Idx pos)
    : impl_{std::make_unique<MainModelWrapper::Impl>(system_frequency, input_data, pos)} {}

MainModelWrapper::MainModelWrapper(double system_frequency, meta_data::MetaData const& meta_data)
    : impl_{std::make_unique<MainModelWrapper::Impl>(system_frequency, meta_data)} {}

// deep copy
MainModelWrapper::MainModelWrapper(MainModelWrapper const& other)
    : impl_{other.impl_ == nullptr ? nullptr : new MainModelWrapper::Impl{*other.impl_}} {}
MainModelWrapper& MainModelWrapper::operator=(MainModelWrapper& other) {
    impl_.reset(other.impl_ == nullptr ? nullptr : new MainModelWrapper::Impl{*other.impl_});
    return *this;
}
MainModelWrapper::MainModelWrapper(MainModelWrapper&& other) = default;
MainModelWrapper& MainModelWrapper::operator=(MainModelWrapper&& /* other */) = default;
MainModelWrapper::~MainModelWrapper() = default;

template <cache_type_c CacheType> void MainModelWrapper::update_component(ConstDataset const& update_data, Idx pos) {
    assert(impl_ != nullptr);
    impl_->update_component<CacheType>(update_data, pos);
}
template void MainModelWrapper::update_component<cached_update_t>(ConstDataset const& update_data, Idx pos);
template void MainModelWrapper::update_component<permanent_update_t>(ConstDataset const& update_data, Idx pos);

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

} // namespace power_grid_model::pgm_static
