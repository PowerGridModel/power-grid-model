// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// handle dataset and buffer related stuff

#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../common/exception.hpp"
#include "../common/iterator_facade.hpp"
#include "dataset_fwd.hpp"
#include "meta_data.hpp"

#include <span>
#include <string_view>

namespace power_grid_model {

namespace meta_data {

template <dataset_type_tag T>
constexpr bool is_data_mutable_v = std::same_as<T, mutable_dataset_t> || std::same_as<T, writable_dataset_t>;
template <dataset_type_tag T> constexpr bool is_indptr_mutable_v = std::same_as<T, writable_dataset_t>;

static_assert(dataset_type_tag<const_dataset_t>);
static_assert(dataset_type_tag<mutable_dataset_t>);
static_assert(dataset_type_tag<writable_dataset_t>);
static_assert(!is_data_mutable_v<const_dataset_t>);
static_assert(is_data_mutable_v<mutable_dataset_t>);
static_assert(is_data_mutable_v<writable_dataset_t>);
static_assert(!is_indptr_mutable_v<const_dataset_t>);
static_assert(!is_indptr_mutable_v<mutable_dataset_t>);
static_assert(is_indptr_mutable_v<writable_dataset_t>);

struct ComponentInfo {
    MetaComponent const* component;
    // for non-uniform component, this is -1, we use indptr to describe the elements per scenario
    Idx elements_per_scenario{};
    Idx total_elements{};
    // whether there is a subset of meaningful attributes that was deduced from the dataset
    bool has_attribute_indications{false};
    // this is not redundant as we use them for aggregate intialization
    // NOLINTNEXTLINE(readability-redundant-member-init)
    std::vector<MetaAttribute const*> attribute_indications{};
};

struct DatasetInfo {
    bool is_batch{false};
    Idx batch_size{1}; // for single dataset, the batch size is one
    MetaDataset const* dataset{nullptr};
    std::vector<ComponentInfo> component_info;
};
template <class Data> struct AttributeBuffer {
    Data* data{nullptr};
    MetaAttribute const* meta_attribute{nullptr};
    bool is_c_order{true};
    Idx stride{1};
};

template <typename T, dataset_type_tag dataset_type> class ColumnarAttributeRange {
  public:
    using Data = std::conditional_t<is_data_mutable_v<dataset_type>, void, void const>;

    class iterator;

    class Proxy {
      public:
        using value_type = std::remove_const_t<T>;

        Proxy() = default;
        Proxy(Idx idx, std::span<AttributeBuffer<Data> const> attribute_buffers)
            : idx_{idx}, attribute_buffers_{std::move(attribute_buffers)} {}

        Proxy const& operator=(value_type const& value) const
            requires is_data_mutable_v<dataset_type>
        {
            for (auto const& attribute_buffer : attribute_buffers_) {
                assert(attribute_buffer.meta_attribute != nullptr);
                auto const& meta_attribute = *attribute_buffer.meta_attribute;
                ctype_func_selector(
                    meta_attribute.ctype, [&value, &attribute_buffer, &meta_attribute, this]<typename AttributeType> {
                        AttributeType* buffer_ptr = reinterpret_cast<AttributeType*>(attribute_buffer.data) + idx_;
                        auto const& attribute_ref = meta_attribute.template get_attribute<AttributeType const>(
                            reinterpret_cast<RawDataConstPtr>(&value));
                        *buffer_ptr = attribute_ref;
                    });
            }
            return *this;
        }
        Proxy& operator=(value_type const& value)
            requires is_data_mutable_v<dataset_type>
        {
            static_cast<Proxy const&>(*this) = value;
            return *this;
        }
        operator value_type() const { return get(); }
        value_type get() const {
            value_type result{};
            for (auto const& attribute_buffer : attribute_buffers_) {
                assert(attribute_buffer.meta_attribute != nullptr);
                auto const& meta_attribute = *attribute_buffer.meta_attribute;
                ctype_func_selector(
                    meta_attribute.ctype, [&result, &attribute_buffer, &meta_attribute, this]<typename AttributeType> {
                        AttributeType const* buffer_ptr =
                            reinterpret_cast<AttributeType const*>(attribute_buffer.data) + idx_;
                        auto& attribute_ref =
                            meta_attribute.template get_attribute<AttributeType>(reinterpret_cast<RawDataPtr>(&result));
                        attribute_ref = *buffer_ptr;
                    });
            }
            return result;
        }

      private:
        friend class ColumnarAttributeRange;
        friend class iterator;

        Idx idx_{};
        std::span<AttributeBuffer<Data> const> attribute_buffers_{};
    };

    class iterator
        : public IteratorFacade<iterator, std::conditional_t<is_data_mutable_v<dataset_type>, Proxy, Proxy const>,
                                Idx> {
      public:
        using value_type = Proxy;
        using difference_type = Idx;

        iterator() = default;
        iterator(difference_type idx, std::span<AttributeBuffer<Data> const> attribute_buffers)
            : current_{idx, attribute_buffers} {}

      private:
        friend class IteratorFacade<iterator, std::conditional_t<is_data_mutable_v<dataset_type>, Proxy, Proxy const>,
                                    Idx>;

        constexpr auto dereference() -> value_type& { return current_; }
        constexpr auto dereference() const -> std::add_lvalue_reference_t<std::add_const_t<value_type>> {
            return current_;
        }
        constexpr auto three_way_compare(iterator const& other) const { return current_.idx_ <=> other.current_.idx_; }
        constexpr auto distance_to(iterator const& other) const { return other.current_.idx_ - current_.idx_; }
        constexpr void advance(difference_type n) { current_.idx_ += n; }

        Proxy current_;
    };

    ColumnarAttributeRange() = default;
    ColumnarAttributeRange(Idx size, std::span<AttributeBuffer<Data> const> attribute_buffers)
        : size_{size}, attribute_buffers_{std::move(attribute_buffers)} {}
    ColumnarAttributeRange(ColumnarAttributeRange::iterator begin, ColumnarAttributeRange::iterator end)
        : size_{std::distance(begin, end)},
          start_{begin->idx_},
          attribute_buffers_{begin->attribute_buffers_.begin(), begin->attribute_buffers_.end()} {
        assert(begin + std::distance(begin, end) == end);
    }

    constexpr Idx size() const { return size_; }
    constexpr bool empty() const { return size_ == 0; }
    iterator begin() const { return get(0); }
    iterator end() const { return get(size_); }
    auto iter() const { return std::ranges::subrange{begin(), end()}; }
    auto operator[](Idx idx) const { return *get(idx); }

  private:
    iterator get(Idx idx) const { return iterator{start_ + idx, attribute_buffers_}; }

    Idx size_{};
    Idx start_{};
    std::span<AttributeBuffer<Data> const> attribute_buffers_;
};

template <typename T> using const_range_object = ColumnarAttributeRange<T, const_dataset_t>;
template <typename T> using mutable_range_object = ColumnarAttributeRange<T, mutable_dataset_t>;

template <dataset_type_tag dataset_type_> class Dataset {
    struct immutable_t {};
    struct mutable_t {};

  public:
    using dataset_type = dataset_type_;
    using Data = std::conditional_t<is_data_mutable_v<dataset_type>, void, void const>;
    using Indptr = std::conditional_t<is_indptr_mutable_v<dataset_type>, Idx, Idx const>;
    template <class StructType>
    using DataStruct = std::conditional_t<is_data_mutable_v<dataset_type>, StructType, StructType const>;

    // for columnar buffers, Data* data is empty and attributes.data is filled
    // for uniform buffers, indptr is empty
    struct Buffer {
        using Data = Dataset::Data;

        Data* data{nullptr};
        std::vector<AttributeBuffer<Data>> attributes{};
        std::span<Indptr> indptr{};
    };

    template <class StructType>
    using RangeObject = std::conditional_t<is_data_mutable_v<dataset_type>, mutable_range_object<StructType>,
                                           const_range_object<StructType>>;

    static constexpr Idx invalid_index{-1};

    Dataset(bool is_batch, Idx batch_size, std::string_view dataset_name, MetaData const& meta_data)
        : meta_data_{&meta_data},
          dataset_info_{.is_batch = is_batch,
                        .batch_size = batch_size,
                        .dataset = &meta_data.get_dataset(dataset_name),
                        .component_info = {}} {
        if (dataset_info_.batch_size < 0) {
            throw DatasetError{"Batch size cannot be negative!\n"};
        }
        if (!dataset_info_.is_batch && (dataset_info_.batch_size != 1)) {
            throw DatasetError{"For non-batch dataset, batch size should be one!\n"};
        }
    }

    // implicit conversion constructor from writable to mutable, from writable and mutable to const
    template <dataset_type_tag other_dataset_type>
        requires((is_data_mutable_v<other_dataset_type> && !is_data_mutable_v<dataset_type>) ||
                 (is_indptr_mutable_v<other_dataset_type> && !is_indptr_mutable_v<dataset_type>))
    Dataset(Dataset<other_dataset_type> const& other)
        : meta_data_{&other.meta_data()}, dataset_info_{other.get_description()} {
        for (Idx i{}; i != other.n_components(); ++i) {
            auto const& buffer = other.get_buffer(i);
            Buffer new_buffer{.data = buffer.data, .indptr = buffer.indptr};
            for (auto const& attribute_buffer : buffer.attributes) {

                AttributeBuffer<Data> const new_attribute_buffer{.data = attribute_buffer.data,
                                                                 .meta_attribute = attribute_buffer.meta_attribute};
                new_buffer.attributes.emplace_back(new_attribute_buffer);
            }
            buffers_.push_back(new_buffer);
        }
    }

    MetaData const& meta_data() const { return *meta_data_; }
    bool empty() const { return dataset_info_.component_info.empty(); }
    bool is_batch() const { return dataset_info_.is_batch; }
    Idx batch_size() const { return dataset_info_.batch_size; }
    MetaDataset const& dataset() const { return *dataset_info_.dataset; }
    Idx n_components() const { return static_cast<Idx>(buffers_.size()); }
    DatasetInfo const& get_description() const { return dataset_info_; }
    Buffer const& get_buffer(std::string_view component) const { return get_buffer(find_component(component, true)); }
    Buffer const& get_buffer(Idx i) const { return buffers_[i]; }

    ComponentInfo const& get_component_info(std::string_view component) const {
        return get_component_info(find_component(component, true));
    }
    ComponentInfo const& get_component_info(Idx i) const { return dataset_info_.component_info[i]; }

    constexpr bool is_row_based(std::string_view component) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return false;
        }
        return is_row_based(idx);
    }
    constexpr bool is_row_based(Idx const i) const { return is_row_based(buffers_[i]); }
    constexpr bool is_row_based(Buffer const& buffer) const { return buffer.data != nullptr; }
    constexpr bool is_columnar(std::string_view component, bool with_attribute_buffers = false) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return false;
        }
        return is_columnar(idx, with_attribute_buffers);
    }
    constexpr bool is_columnar(Idx const i, bool with_attribute_buffers = false) const {
        return is_columnar(buffers_[i], with_attribute_buffers);
    }
    constexpr bool is_columnar(Buffer const& buffer, bool with_attribute_buffers = false) const {
        return !is_row_based(buffer) && !(with_attribute_buffers && buffer.attributes.empty());
    }

    constexpr bool is_dense(std::string_view component) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return true; // by definition
        }
        return is_dense(idx);
    }
    constexpr bool is_dense(Idx const i) const { return is_dense(buffers_[i]); }
    constexpr bool is_dense(Buffer const& buffer) const { return buffer.indptr.empty(); }
    constexpr bool is_sparse(std::string_view component, bool with_attribute_buffers = false) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return false;
        }
        return is_sparse(idx, with_attribute_buffers);
    }
    constexpr bool is_sparse(Idx const i, bool with_attribute_buffers = false) const {
        return is_sparse(buffers_[i], with_attribute_buffers);
    }
    constexpr bool is_sparse(Buffer const& buffer) const { return !is_dense(buffer); }

    constexpr bool is_uniform(std::string_view component) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return true; // by definition
        }
        return is_uniform(idx);
    }
    constexpr bool is_uniform(Idx const i) const { return is_uniform(buffers_[i]); }
    constexpr bool is_uniform(Buffer const& buffer) const {
        if (is_dense(buffer)) {
            return true;
        }
        assert(buffer.indptr.size() > 1);
        auto const first_scenario_size = buffer.indptr[1] - buffer.indptr[0];
        return std::ranges::adjacent_find(buffer.indptr, [first_scenario_size](Idx start, Idx stop) {
                   return stop - start != first_scenario_size;
               }) == buffer.indptr.end();
    }

    constexpr Idx uniform_elements_per_scenario(std::string_view component) const {
        Idx const idx = find_component(component, false);
        if (idx == invalid_index) {
            return 0;
        }
        return uniform_elements_per_scenario(idx);
    }
    constexpr Idx uniform_elements_per_scenario(Idx const i) const {
        assert(is_uniform(i));
        if (is_dense(i)) {
            return get_component_info(i).elements_per_scenario;
        }
        auto const& indptr = buffers_[i].indptr;
        assert(indptr.size() > 1);
        return indptr[1] - indptr[0];
    }

    Idx find_component(std::string_view component, bool required = false) const {
        auto const found = std::ranges::find_if(dataset_info_.component_info, [component](ComponentInfo const& x) {
            return x.component->name == component;
        });
        if (found == dataset_info_.component_info.cend()) {
            if (required) {
                using namespace std::string_literals;
                throw DatasetError{"Cannot find component '"s + std::string{component} + "'!\n"s};
            }
            return invalid_index;
        }
        return std::distance(dataset_info_.component_info.cbegin(), found);
    }
    bool contains_component(std::string_view component) const { return find_component(component) >= 0; }

    void add_component_info(std::string_view component, Idx elements_per_scenario, Idx total_elements)
        requires is_indptr_mutable_v<dataset_type>
    {
        add_component_info_impl(component, elements_per_scenario, total_elements);
    }

    void enable_attribute_indications(std::string_view component)
        requires is_indptr_mutable_v<dataset_type>
    {
        Idx const idx = find_component(component, true);
        dataset_info_.component_info[idx].has_attribute_indications = true;
    }

    void set_attribute_indications(std::string_view component, std::span<MetaAttribute const*> attribute_indications)
        requires is_indptr_mutable_v<dataset_type>
    {
        Idx const idx = find_component(component, true);
        dataset_info_.component_info[idx].attribute_indications = {attribute_indications.begin(),
                                                                   attribute_indications.end()};
    }

    void add_buffer(std::string_view component, std::integral auto elements_per_scenario_,
                    std::integral auto total_elements_, Indptr* indptr, Data* data)
        requires(!is_indptr_mutable_v<dataset_type>)
    {
        auto const elements_per_scenario = static_cast<Idx>(elements_per_scenario_);
        auto const total_elements = static_cast<Idx>(total_elements_);
        check_non_uniform_integrity<immutable_t>(elements_per_scenario, total_elements, indptr);
        add_component_info_impl(component, elements_per_scenario, total_elements);
        buffers_.back().data = data;
        if (indptr) {
            buffers_.back().indptr = get_indptr_span(indptr);
        } else {
            buffers_.back().indptr = {};
        }
    }

    void set_buffer(std::string_view component, Indptr* indptr, Data* data)
        requires is_indptr_mutable_v<dataset_type>
    {
        Idx const idx = find_component(component, true);
        ComponentInfo const& info = dataset_info_.component_info[idx];
        check_non_uniform_integrity<mutable_t>(info.elements_per_scenario, info.total_elements, indptr);
        buffers_[idx].data = data;
        if (indptr) {
            buffers_[idx].indptr = get_indptr_span(indptr);
        } else {
            buffers_[idx].indptr = {};
        }
    }

    void add_attribute_buffer(std::string_view component, std::string_view attribute, Data* data)
        requires(!is_indptr_mutable_v<dataset_type>)
    {
        add_attribute_buffer_impl(component, attribute, data);
    }

    /*
    we decided to go with the same behavior between `add_attribute_buffer` and `set_attribute_buffer` (but different
    entrypoints). The behavior of `set_attribute_buffer` therefore differs from the one of `set_buffer`. The reasoning
    is as follows:

    For components:
    - the deserializer tells the user via the dataset info that a certain component is present in the serialized
    data.
    - It is possible to efficiently determine whether that is the case.
    - The user can then only call `set_buffer` for those components that are already present
    For attributes:
    - the deserializer would need to go over the entire dataset to look for components with the map serialization
        representation to determine whether an attribute is present.
    - this is expensive.
    - the deserializer therefore cannot let the user know beforehand which attributes are present.
    - `set_attribute_buffer` therefore should only be called if it has not been set yet.
    - this is the same behavior as `add_attribute_buffer`.
    */
    void set_attribute_buffer(std::string_view component, std::string_view attribute, Data* data)
        requires is_indptr_mutable_v<dataset_type>
    {
        add_attribute_buffer_impl(component, attribute, data);
    }

    // get buffer by component type
    template <class type_getter, class ComponentType,
              class StructType = DataStruct<typename type_getter::template type<ComponentType>>>
    std::span<StructType> get_buffer_span(Idx scenario = invalid_index) const {
        assert(scenario < batch_size());

        if (!is_batch() && scenario > 0) {
            throw DatasetError{"Cannot export a single dataset with specified scenario\n"};
        }

        Idx const idx = find_component(ComponentType::name, false);
        return get_buffer_span_impl<StructType>(scenario, idx);
    }

    template <class type_getter, class ComponentType,
              class StructType = DataStruct<typename type_getter::template type<ComponentType>>>
    RangeObject<StructType> get_columnar_buffer_span(Idx scenario = invalid_index) const {
        assert(scenario < batch_size());

        if (!is_batch() && scenario > 0) {
            throw DatasetError{"Cannot export a single dataset with specified scenario\n"};
        }

        Idx const idx = find_component(ComponentType::name, false);
        return get_columnar_buffer_span_impl<StructType>(scenario, idx);
    }

    // get buffer by component type for all scenarios in vector span
    template <class type_getter, class ComponentType,
              class StructType = DataStruct<typename type_getter::template type<ComponentType>>>
    std::vector<std::span<StructType>> get_buffer_span_all_scenarios() const {
        Idx const idx = find_component(ComponentType::name, false);
        std::vector<std::span<StructType>> result(batch_size());
        for (Idx scenario{}; scenario != batch_size(); scenario++) {
            result[scenario] = get_buffer_span_impl<StructType>(scenario, idx);
        }
        return result;
    }

    template <class type_getter, class ComponentType,
              class StructType = DataStruct<typename type_getter::template type<ComponentType>>>
    std::vector<RangeObject<StructType>> get_columnar_buffer_span_all_scenarios() const {
        Idx const idx = find_component(ComponentType::name, false);
        std::vector<RangeObject<StructType>> result(batch_size());
        for (Idx scenario{}; scenario != batch_size(); scenario++) {
            result[scenario] = get_columnar_buffer_span_impl<StructType>(scenario, idx);
        }
        return result;
    }

    // get individual dataset from batch
    Dataset get_individual_scenario(Idx scenario) const
        requires(!is_indptr_mutable_v<dataset_type>)
    {
        using AdvanceablePtr = std::conditional_t<is_data_mutable_v<dataset_type>, char*, char const*>;

        assert(0 <= scenario && scenario < batch_size());

        Dataset result{false, 1, dataset().name, meta_data()};
        for (Idx i{}; i != n_components(); ++i) {
            auto const& buffer = get_buffer(i);
            auto const& component_info = get_component_info(i);
            Idx const size = component_info.elements_per_scenario >= 0
                                 ? component_info.elements_per_scenario
                                 : buffer.indptr[scenario + 1] - buffer.indptr[scenario];
            Idx const offset = component_info.elements_per_scenario >= 0 ? size * scenario : buffer.indptr[scenario];
            if (is_columnar(buffer)) {
                result.add_buffer(component_info.component->name, size, size, nullptr, nullptr);
                for (auto const& attribute_buffer : buffer.attributes) {
                    result.add_attribute_buffer(component_info.component->name, attribute_buffer.meta_attribute->name,
                                                static_cast<Data*>(static_cast<AdvanceablePtr>(attribute_buffer.data)));
                }
            } else {
                Data* data = component_info.component->advance_ptr(buffer.data, offset);
                result.add_buffer(component_info.component->name, size, size, nullptr, data);
            }
        }
        return result;
    }

  private:
    MetaData const* meta_data_;
    DatasetInfo dataset_info_;
    std::vector<Buffer> buffers_;

    std::span<Indptr> get_indptr_span(Indptr* indptr) const {
        return std::span{indptr, static_cast<size_t>(batch_size() + 1)};
    }

    void check_uniform_integrity(Idx elements_per_scenario, Idx total_elements) {
        if ((elements_per_scenario >= 0) && (elements_per_scenario * batch_size() != total_elements)) {
            throw DatasetError{
                "For a uniform buffer, total_elements should be equal to elements_per_scenario * batch_size!\n"};
        }
    }

    template <typename check_indptr_content>
        requires std::same_as<check_indptr_content, mutable_t> || std::same_as<check_indptr_content, immutable_t>
    void check_non_uniform_integrity(Idx elements_per_scenario, Idx total_elements, Indptr* indptr) {
        if (elements_per_scenario < 0) {
            if (!indptr) {
                throw DatasetError{"For a non-uniform buffer, indptr should be supplied!\n"};
            }
            if constexpr (std::same_as<check_indptr_content, immutable_t>) {
                if (indptr[0] != 0 || indptr[batch_size()] != total_elements) {
                    throw DatasetError{
                        "For a non-uniform buffer, indptr should begin with 0 and end with total_elements!\n"};
                }
            }
        } else if (indptr) {
            throw DatasetError{"For a uniform buffer, indptr should be nullptr!\n"};
        }
    }

    void add_component_info_impl(std::string_view component, Idx elements_per_scenario, Idx total_elements) {
        if (find_component(component) >= 0) {
            throw DatasetError{"Cannot have duplicated components!\n"};
        }
        check_uniform_integrity(elements_per_scenario, total_elements);
        dataset_info_.component_info.push_back({.component = &dataset_info_.dataset->get_component(component),
                                                .elements_per_scenario = elements_per_scenario,
                                                .total_elements = total_elements});
        buffers_.push_back(Buffer{});
    }

    void add_attribute_buffer_impl(std::string_view component, std::string_view attribute, Data* data) {
        Idx const idx = find_component(component, true);
        Buffer& buffer = buffers_[idx];
        if (!is_columnar(buffer)) {
            throw DatasetError{"Cannot add attribute buffers to row-based dataset!\n"};
        }
        if (std::ranges::find_if(buffer.attributes, [&attribute](auto const& buffer_attribute) {
                return buffer_attribute.meta_attribute->name == attribute;
            }) != buffer.attributes.end()) {
            throw DatasetError{"Cannot have duplicated attribute buffers!\n"};
        }
        AttributeBuffer<Data> const attribute_buffer{
            .data = data, .meta_attribute = &dataset_info_.component_info[idx].component->get_attribute(attribute)};
        buffer.attributes.emplace_back(attribute_buffer);
    }

    template <class RangeType>
    RangeType get_span_impl(RangeType const& total_range, Idx scenario, Buffer const& buffer,
                            ComponentInfo const& info) const {
        if (scenario < 0) {
            return RangeType{std::begin(total_range), std::end(total_range)};
        }
        if (info.elements_per_scenario < 0) {
            return RangeType{std::begin(total_range) + buffer.indptr[scenario],
                             std::begin(total_range) + buffer.indptr[scenario + 1]};
        }
        return RangeType{std::begin(total_range) + info.elements_per_scenario * scenario,
                         std::begin(total_range) + info.elements_per_scenario * (scenario + 1)};
    }

    // get non-empty row buffer
    template <class StructType> std::span<StructType> get_buffer_span_impl(Idx scenario, Idx component_idx) const {
        // return empty span if the component does not exist
        if (component_idx < 0) {
            return {};
        }
        // return span based on uniform or non-uniform buffer
        ComponentInfo const& info = dataset_info_.component_info[component_idx];
        Buffer const& buffer = buffers_[component_idx];
        auto const ptr = reinterpret_cast<StructType*>(buffer.data);
        return get_span_impl(std::span<StructType>{ptr, ptr + info.total_elements}, scenario, buffer, info);
    }

    // get non-empty columnar buffer
    template <class StructType>
    RangeObject<StructType> get_columnar_buffer_span_impl(Idx scenario, Idx component_idx) const {
        // return empty span if the component does not exist
        if (component_idx < 0) {
            return {};
        }
        // return span based on uniform or non-uniform buffer
        ComponentInfo const& info = dataset_info_.component_info[component_idx];
        Buffer const& buffer = buffers_[component_idx];
        assert(is_columnar(buffer));
        return get_span_impl(RangeObject<StructType>{info.total_elements, buffer.attributes}, scenario, buffer, info);
    }
};

} // namespace meta_data

template <dataset_type_tag dataset_type> using Dataset = meta_data::Dataset<dataset_type>;
using ConstDataset = Dataset<const_dataset_t>;
using MutableDataset = Dataset<mutable_dataset_t>;
using WritableDataset = Dataset<writable_dataset_t>;

} // namespace power_grid_model
