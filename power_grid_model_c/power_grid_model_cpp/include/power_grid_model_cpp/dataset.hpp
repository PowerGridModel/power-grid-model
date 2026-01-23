// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "basics.hpp"
#include "buffer.hpp"
#include "handle.hpp"
#include "utils.hpp"

#include "power_grid_model_c/dataset.h"

#include <map>
#include <set>
#include <variant>

namespace power_grid_model_cpp {
class ComponentTypeNotFound : public PowerGridError {
  public:
    ComponentTypeNotFound(std::string const& component)
        : PowerGridError{[&]() {
              using namespace std::string_literals;
              return "ComponentType"s + component + " not found"s;
          }()} {}
    ComponentTypeNotFound(std::string_view component) : ComponentTypeNotFound{std::string{component}} {}
};

class DatasetInfo {

  public:
    DatasetInfo(RawDatasetInfo const* info) noexcept : info_{info} {}
    DatasetInfo(DatasetInfo&&) = default;
    DatasetInfo& operator=(DatasetInfo&&) = default;
    DatasetInfo(DatasetInfo const&) = delete;            // No copy constructor
    DatasetInfo& operator=(DatasetInfo const&) = delete; // No copy assignment
    ~DatasetInfo() = default;

    std::string name() const { return std::string{handle_.call_with(PGM_dataset_info_name, info_)}; }

    bool is_batch() const { return handle_.call_with(PGM_dataset_info_is_batch, info_) != 0; }

    Idx batch_size() const { return handle_.call_with(PGM_dataset_info_batch_size, info_); }

    Idx n_components() const { return handle_.call_with(PGM_dataset_info_n_components, info_); }

    std::string component_name(Idx component_idx) const {
        return std::string{handle_.call_with(PGM_dataset_info_component_name, info_, component_idx)};
    }

    Idx component_elements_per_scenario(Idx component_idx) const {
        return handle_.call_with(PGM_dataset_info_elements_per_scenario, info_, component_idx);
    }

    Idx component_total_elements(Idx component_idx) const {
        return handle_.call_with(PGM_dataset_info_total_elements, info_, component_idx);
    }

    Idx component_idx(std::string_view component) const {
        Idx const n_comp = n_components();
        for (Idx idx = 0; idx < n_comp; ++idx) {
            if (component_name(idx) == component) {
                return idx;
            }
        }
        throw ComponentTypeNotFound{component};
    }

    bool has_attribute_indications(Idx component_idx) const {
        return handle_.call_with(PGM_dataset_info_has_attribute_indications, info_, component_idx) != 0;
    }

    std::vector<std::string> attribute_indications(Idx component_idx) const {
        Idx const n_attributes = handle_.call_with(PGM_dataset_info_n_attribute_indications, info_, component_idx);
        std::vector<std::string> attributes;
        attributes.reserve(n_attributes);
        for (Idx idx = 0; idx < n_attributes; ++idx) {
            attributes.emplace_back(handle_.call_with(PGM_dataset_info_attribute_name, info_, component_idx, idx));
        }
        return attributes;
    }

  private:
    Handle handle_{};
    RawDatasetInfo const* info_;
};

class DatasetWritable {
  public:
    DatasetWritable(RawWritableDataset* dataset)
        : dataset_{dataset}, info_{handle_.call_with(PGM_dataset_writable_get_info, dataset_)} {}
    DatasetWritable(DatasetWritable&&) = default;
    DatasetWritable& operator=(DatasetWritable&&) = default;
    DatasetWritable(DatasetWritable const&) = delete;            // No copy constructor
    DatasetWritable& operator=(DatasetWritable const&) = delete; // No copy assignment
    ~DatasetWritable() = default;

    RawWritableDataset const* get() const { return dataset_; }
    RawWritableDataset* get() { return dataset_; }

    DatasetInfo const& get_info() const { return info_; }

    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        handle_.call_with(PGM_dataset_writable_set_buffer, get(), component.c_str(), indptr, data);
    }

    void set_buffer(std::string const& component, Idx* indptr, Buffer& data) {
        handle_.call_with(PGM_dataset_writable_set_buffer, get(), component.c_str(), indptr, data.get());
    }

    void set_attribute_buffer(std::string const& component, std::string const& attribute, RawDataPtr data) {
        handle_.call_with(PGM_dataset_writable_set_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void set_attribute_buffer(std::string const& component, std::string const& attribute, Buffer& data) {
        handle_.call_with(PGM_dataset_writable_set_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

  private:
    Handle handle_{};
    RawWritableDataset* dataset_;
    DatasetInfo info_;
};

class DatasetMutable {
  public:
    explicit DatasetMutable(std::string const& dataset, bool is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_mutable, dataset.c_str(), (is_batch ? Idx{1} : Idx{0}),
                                     batch_size)},
          info_{handle_.call_with(PGM_dataset_mutable_get_info, get())} {}

    RawMutableDataset const* get() const { return dataset_.get(); }
    RawMutableDataset* get() { return dataset_.get(); }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) {
        handle_.call_with(PGM_dataset_mutable_add_buffer, get(), component.c_str(), elements_per_scenario,
                          total_elements, indptr, data);
    }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer& data) {
        handle_.call_with(PGM_dataset_mutable_add_buffer, get(), component.c_str(), elements_per_scenario,
                          total_elements, indptr, data.get());
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, RawDataPtr data) {
        handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, Buffer& data) {
        handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

    DatasetInfo const& get_info() const { return info_; }

  private:
    Handle handle_{};
    detail::UniquePtr<RawMutableDataset, &PGM_destroy_dataset_mutable> dataset_;
    DatasetInfo info_;
};

class DatasetConst {
  public:
    explicit DatasetConst(std::string const& dataset, bool is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_const, dataset.c_str(), (is_batch ? Idx{1} : Idx{0}),
                                     batch_size)},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}

    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_writable, writable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}

    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_mutable, mutable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}

    RawConstDataset const* get() const { return dataset_.get(); }
    RawConstDataset* get() { return dataset_.get(); }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) {
        handle_.call_with(PGM_dataset_const_add_buffer, get(), component.c_str(), elements_per_scenario, total_elements,
                          indptr, data);
    }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer const& data) {
        handle_.call_with(PGM_dataset_const_add_buffer, get(), component.c_str(), elements_per_scenario, total_elements,
                          indptr, data.get());
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, RawDataConstPtr data) {
        handle_.call_with(PGM_dataset_const_add_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, Buffer const& data) {
        handle_.call_with(PGM_dataset_const_add_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

    void set_next_cartesian_product_dimension(DatasetConst const& next_dataset) {
        handle_.call_with(PGM_dataset_const_set_next_cartesian_product_dimension, get(), next_dataset.get());
    }

    DatasetInfo const& get_info() const { return info_; }

  private:
    Handle handle_{};
    detail::UniquePtr<RawConstDataset, &PGM_destroy_dataset_const> dataset_;
    DatasetInfo info_;
};

class AttributeBuffer {
  private:
    using VariantType = std::variant<std::monostate, std::vector<ID>, std::vector<IntS>, std::vector<double>,
                                     std::vector<std::array<double, 3>>>;

    struct BufferCreator {
        Idx size;
        template <class T> VariantType operator()() const { return std::vector<T>(size, nan_value<T>()); }
    };

    struct PtrGetter {
        AttributeBuffer& buffer;
        template <class T> RawDataPtr operator()() const { return std::get<std::vector<T>>(buffer.buffer_).data(); }
    };

  public:
    AttributeBuffer() = default;

    explicit AttributeBuffer(MetaAttribute const* attribute, Idx size)
        : attribute_{attribute},
          buffer_{pgm_type_func_selector(MetaData::attribute_ctype(attribute), BufferCreator{size})} {}

    RawDataPtr get() { return pgm_type_func_selector(MetaData::attribute_ctype(attribute_), PtrGetter{*this}); }

  private:
    MetaAttribute const* attribute_{nullptr};
    VariantType buffer_;
};

struct OwningMemory {
    std::vector<Buffer> buffers;
    std::vector<std::vector<Idx>> indptrs;
    std::vector<std::vector<AttributeBuffer>> attribute_buffers;
};

struct OwningDataset {
    DatasetMutable dataset;
    OwningMemory storage{};

    OwningDataset(DatasetWritable& writable_dataset, bool enable_columnar_buffers = false)
        : dataset{writable_dataset.get_info().name(), writable_dataset.get_info().is_batch(),
                  writable_dataset.get_info().batch_size()},
          storage{} {
        auto const& info = writable_dataset.get_info();
        Idx const batch_size = info.batch_size();
        auto const& dataset_name = info.name();

        for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
            auto const& component_name = info.component_name(component_idx);
            auto const* const component_meta = MetaData::get_component_by_name(dataset_name, component_name);
            Idx const component_size = info.component_total_elements(component_idx);
            Idx const elements_per_scenario = info.component_elements_per_scenario(component_idx);

            auto& current_indptr = storage.indptrs.emplace_back(elements_per_scenario < 0 ? batch_size + 1 : 0);
            if (!current_indptr.empty()) {
                current_indptr.at(0) = 0;
                current_indptr.at(batch_size) = component_size;
            }
            Idx* const indptr = current_indptr.empty() ? nullptr : current_indptr.data();
            if (info.has_attribute_indications(component_idx) && enable_columnar_buffers) {
                auto& current_buffer = storage.buffers.emplace_back();
                writable_dataset.set_buffer(component_name, indptr, current_buffer);
                dataset.add_buffer(component_name, elements_per_scenario, component_size, indptr, current_buffer);
                auto const& attribute_indications = info.attribute_indications(component_idx);
                auto& current_attribute_buffers = storage.attribute_buffers.emplace_back();
                for (auto const& attribute_name : attribute_indications) {
                    auto const* const attribute_meta =
                        MetaData::get_attribute_by_name(dataset_name, component_name, attribute_name);
                    current_attribute_buffers.emplace_back(attribute_meta, component_size);
                    writable_dataset.set_attribute_buffer(component_name, attribute_name,
                                                          current_attribute_buffers.back().get());
                    dataset.add_attribute_buffer(component_name, attribute_name,
                                                 current_attribute_buffers.back().get());
                }
            } else {
                auto& current_buffer = storage.buffers.emplace_back(component_meta, component_size);
                storage.attribute_buffers.emplace_back(); // empty attribute buffers
                writable_dataset.set_buffer(component_name, indptr, current_buffer);
                dataset.add_buffer(component_name, elements_per_scenario, component_size, indptr, current_buffer);
            }
        }
    }

    OwningDataset(
        OwningDataset const& ref_dataset, std::string const& dataset_name, bool is_batch = false, Idx batch_size = 1,
        std::map<MetaComponent const*, std::set<MetaAttribute const*>> const& output_component_attribute_filters = {})
        : dataset{dataset_name, is_batch, batch_size}, storage{} {
        DatasetInfo const& ref_info = ref_dataset.dataset.get_info();
        bool const enable_filters = !output_component_attribute_filters.empty();

        for (Idx component_idx{}; component_idx != ref_info.n_components(); ++component_idx) {
            auto const& component_name = ref_info.component_name(component_idx);
            auto const& component_meta = MetaData::get_component_by_name(dataset_name, component_name);
            // skip components not in the filter
            if (enable_filters &&
                output_component_attribute_filters.find(component_meta) == output_component_attribute_filters.end()) {
                continue;
            }

            // get size info from reference dataset
            Idx const component_elements_per_scenario = ref_info.component_elements_per_scenario(component_idx);
            if (component_elements_per_scenario < 0) {
                throw PowerGridError{"Cannot create result dataset for component with variable size per scenario"};
            }
            Idx const component_size = component_elements_per_scenario * batch_size;
            storage.indptrs.emplace_back();

            auto const component_filter_it = output_component_attribute_filters.find(component_meta);
            std::set<MetaAttribute const*> const& attribute_filter =
                component_filter_it != output_component_attribute_filters.end() ? component_filter_it->second
                                                                                : std::set<MetaAttribute const*>{};
            if (attribute_filter.empty()) {
                // create full row buffer
                auto& component_buffer = storage.buffers.emplace_back(component_meta, component_size);
                storage.attribute_buffers.emplace_back(); // empty attribute buffers
                dataset.add_buffer(component_name, component_elements_per_scenario, component_size, nullptr,
                                   component_buffer);
            } else {
                // push nullptr as row buffer, and start attribute buffers
                auto& component_buffer = storage.buffers.emplace_back();
                storage.attribute_buffers.emplace_back();
                dataset.add_buffer(component_name, component_elements_per_scenario, component_size, nullptr,
                                   component_buffer);
                for (auto const* const attribute_meta : attribute_filter) {
                    auto const attribute_name = MetaData::attribute_name(attribute_meta);
                    auto& attribute_buffer =
                        storage.attribute_buffers.back().emplace_back(attribute_meta, component_size);
                    dataset.add_attribute_buffer(component_name, attribute_name, attribute_buffer.get());
                }
            }
        }
    }
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
