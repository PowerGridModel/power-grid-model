// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_gen/gen_getters.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/typing.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <numeric>

TYPE_TO_STRING_AS("ConstDataset", power_grid_model::ConstDataset);
TYPE_TO_STRING_AS("MutableDataset", power_grid_model::MutableDataset);
TYPE_TO_STRING_AS("WritableDataset", power_grid_model::WritableDataset);

namespace power_grid_model::meta_data {

namespace {
struct AInput {
    static constexpr auto id_name = "id";
    static constexpr auto a0_name = "a0";
    static constexpr auto a1_name = "a1";

    ID id{na_IntID};
    double a0{nan};
    double a1{nan};
};
struct AUpdate {
    static constexpr auto id_name = "id";
    static constexpr auto a0_name = "a0";

    ID id{na_IntID};
    double a0{nan};
};
template <symmetry_tag symmetry> struct AOutput {
    using sym = symmetry;

    static constexpr auto id_name = "id";
    static constexpr auto a2_name = "a2";
    static constexpr auto a3_name = "a3";

    ID id{na_IntID};
    double a2{nan};
    double a3{nan};
};
struct AScOutput {
    static constexpr auto id_name = "id";

    ID id{na_IntID};
};
struct BInput {};
struct BUpdate {};
template <symmetry_tag symmetry> struct BOutput {
    using sym = symmetry;
};
struct BScOutput {};
} // namespace

template <> struct get_attributes_list<AInput> {
    static constexpr std::array<MetaAttribute, 3> value{
        meta_data_gen::get_meta_attribute<&AInput::id>(offsetof(AInput, id), AInput::id_name),
        meta_data_gen::get_meta_attribute<&AInput::a0>(offsetof(AInput, a0), AInput::a0_name),
        meta_data_gen::get_meta_attribute<&AInput::a1>(offsetof(AInput, a1), AInput::a1_name),
    };
};
template <> struct get_attributes_list<AUpdate> {
    static constexpr std::array<MetaAttribute, 2> value{
        meta_data_gen::get_meta_attribute<&AUpdate::id>(offsetof(AUpdate, id), AUpdate::id_name),
        meta_data_gen::get_meta_attribute<&AUpdate::a0>(offsetof(AUpdate, a0), AUpdate::a0_name),
    };
};
template <> struct get_attributes_list<AOutput<symmetric_t>> {
    static constexpr std::array<MetaAttribute, 3> value{
        meta_data_gen::get_meta_attribute<&AOutput<symmetric_t>::id>(offsetof(AOutput<symmetric_t>, id),
                                                                     AOutput<symmetric_t>::id_name),
        meta_data_gen::get_meta_attribute<&AOutput<symmetric_t>::a2>(offsetof(AOutput<symmetric_t>, a2),
                                                                     AOutput<symmetric_t>::a2_name),
        meta_data_gen::get_meta_attribute<&AOutput<symmetric_t>::a3>(offsetof(AOutput<symmetric_t>, a3),
                                                                     AOutput<symmetric_t>::a3_name),
    };
};
template <> struct get_attributes_list<AOutput<asymmetric_t>> {
    static constexpr std::array<MetaAttribute, 3> value{
        meta_data_gen::get_meta_attribute<&AOutput<asymmetric_t>::id>(offsetof(AOutput<asymmetric_t>, id),
                                                                      AOutput<asymmetric_t>::id_name),
        meta_data_gen::get_meta_attribute<&AOutput<asymmetric_t>::a2>(offsetof(AOutput<asymmetric_t>, a2),
                                                                      AOutput<asymmetric_t>::a2_name),
        meta_data_gen::get_meta_attribute<&AOutput<asymmetric_t>::a3>(offsetof(AOutput<asymmetric_t>, a3),
                                                                      AOutput<asymmetric_t>::a3_name),
    };
};
template <> struct get_attributes_list<AScOutput> {
    static constexpr std::array<MetaAttribute, 1> value{
        meta_data_gen::get_meta_attribute<&AScOutput::id>(offsetof(AScOutput, id), AScOutput::id_name),
    };
};
template <> struct get_attributes_list<BInput> {
    static constexpr std::array<MetaAttribute, 0> value{};
};
template <> struct get_attributes_list<BUpdate> {
    static constexpr std::array<MetaAttribute, 0> value{};
};
template <> struct get_attributes_list<BOutput<symmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value{};
};
template <> struct get_attributes_list<BOutput<asymmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value{};
};
template <> struct get_attributes_list<BScOutput> {
    static constexpr std::array<MetaAttribute, 0> value{};
};

namespace {
struct A {
    using InputType = AInput;
    using UpdateType = AUpdate;
    template <symmetry_tag sym> using OutputType = AOutput<sym>;
    using ShortCircuitOutputType = AScOutput;

    static constexpr auto name = "A";
};
struct B {
    using InputType = BInput;
    using UpdateType = BUpdate;
    template <symmetry_tag sym> using OutputType = BOutput<sym>;
    using ShortCircuitOutputType = BScOutput;

    static constexpr auto name = "B";
};

constexpr MetaData test_meta_data = meta_data_gen::get_meta_data<ComponentList<A, B>, // all components list
                                                                 input_getter_s, update_getter_s
                                                                 // end list of all marks
                                                                 >::value;

constexpr MetaData test_meta_data_all =
    meta_data_gen::get_meta_data<ComponentList<A, B>, // all components list
                                 input_getter_s, update_getter_s, sym_output_getter_s, asym_output_getter_s,
                                 sc_output_getter_s
                                 // end list of all marks
                                 >::value;
} // namespace

} // namespace power_grid_model::meta_data

TYPE_TO_STRING_AS("const_range_object<A::InputType>",
                  power_grid_model::meta_data::const_range_object<power_grid_model::meta_data::A::InputType>);
TYPE_TO_STRING_AS("mutable_range_object<A::InputType>",
                  power_grid_model::meta_data::mutable_range_object<power_grid_model::meta_data::A::InputType>);

namespace power_grid_model::meta_data {

namespace test {
namespace {
template <typename DatasetType>
DatasetType create_dataset(bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
    auto dataset = DatasetType{is_batch, batch_size, dataset_type.name, test_meta_data_all};
    CHECK(&dataset.meta_data() == static_cast<MetaData const*>(&test_meta_data_all));
    CHECK(dataset.empty());
    CHECK(dataset.is_batch() == is_batch);
    CHECK(dataset.batch_size() == batch_size);
    CHECK(dataset.n_components() == 0);

    auto const& info = dataset.get_description();
    CHECK(info.is_batch == dataset.is_batch());
    CHECK(info.batch_size == dataset.batch_size());
    CHECK(info.dataset == &dataset.dataset());
    CHECK(info.component_info.empty());
    return dataset;
};

void check_nan_or_equal(double first, double second) {
    CHECK(((first == second) || (is_nan(first) == is_nan(second))));
}

void check_nan(double value) { CHECK(is_nan(value)); }

void check_equal(A::InputType const& first, A::InputType const& second) {
    CHECK(first.id == second.id);
    check_nan_or_equal(first.a0, second.a0);
    check_nan_or_equal(first.a1, second.a1);
}
} // namespace
} // namespace test

namespace {
template <typename DatasetType, typename BufferSpan, typename Idx>
auto get_colummnar_element(BufferSpan const& buffer_span, Idx idx) {
    using ProxyType =
        std::conditional_t<std::same_as<DatasetType, ConstDataset>, const_range_object<A::InputType const>::Proxy,
                           mutable_range_object<A::InputType>::Proxy>;
    static_assert(std::same_as<decltype(buffer_span[idx]), ProxyType>);
    return static_cast<A::InputType>(buffer_span[idx]);
}

template <typename BufferSpan>
void check_row_span(BufferSpan const& buffer_span, Idx const& total_elements,
                    std::vector<A::InputType> const& a_buffer) {
    CHECK(std::size(buffer_span) == total_elements);
    CHECK(std::data(buffer_span) == a_buffer.data());
}

} // namespace

TEST_CASE_TEMPLATE("Test range object", RangeObjectType, const_range_object<A::InputType>,
                   mutable_range_object<A::InputType>) {
    using Data = std::conditional_t<std::same_as<RangeObjectType, const_range_object<A::InputType>>, void const, void>;

    auto const& all_attributes = test_meta_data.datasets.front().get_component(A::name);

    auto id_buffer = std::vector<ID>{0, 1, 2};
    auto a1_buffer = std::vector<double>{0.0, 1.0, nan};
    auto const total_elements = narrow_cast<Idx>(id_buffer.size());
    REQUIRE(narrow_cast<Idx>(a1_buffer.size()) >= total_elements);
    AttributeBuffer<Data> const attribute_id{.data = static_cast<Data*>(id_buffer.data()),
                                             .meta_attribute = &all_attributes.get_attribute("id")};
    AttributeBuffer<Data> const attribute_a1{.data = static_cast<Data*>(a1_buffer.data()),
                                             .meta_attribute = &all_attributes.get_attribute("a1")};
    std::vector<AttributeBuffer<Data>> const elements{attribute_id, attribute_a1};
    RangeObjectType range_object{total_elements, elements};

    static_assert(std::convertible_to<decltype(*range_object.begin()), A::InputType>);

    auto const check_buffer = [total_elements, &id_buffer, &a1_buffer](auto const& object) {
        CHECK(object.size() == total_elements);
        for (Idx idx = 0; idx < object.size(); ++idx) {
            auto const element = static_cast<A::InputType>(object[idx]);
            CHECK(element.id == id_buffer[idx]);
            test::check_nan_or_equal(element.a1, a1_buffer[idx]);
            test::check_nan(element.a0);
            test::check_equal(object[idx], *(object.begin() + idx));
        }
    };

    SUBCASE("Constructor") {
        id_buffer = {0, 1, 2, 3, 4};
        auto const elements_total = narrow_cast<Idx>(id_buffer.size());
        AttributeBuffer<Data> const id_attribute{.data = static_cast<Data*>(id_buffer.data()),
                                                 .meta_attribute = &all_attributes.get_attribute("id")};
        std::vector<AttributeBuffer<Data>> const element{id_attribute};
        RangeObjectType total_range{elements_total, element};
        auto const start = total_range.begin() + 2;
        auto const stop = total_range.begin() + 4;
        RangeObjectType sub_range{start, stop};
        CHECK(sub_range[0].get().id == total_range[2].get().id);
    }

    SUBCASE("Read access") {
        check_buffer(range_object);
        id_buffer = {2, 3, 4};
        a1_buffer = {6.0, -2.0, nan};
        check_buffer(range_object);
    }

    if constexpr (std::same_as<RangeObjectType, mutable_range_object<A::InputType>>) {
        SUBCASE("Write access") {
            A::InputType const new_values{.id = 20, .a0 = -10.0, .a1 = nan};
            A::InputType const expected{.id = new_values.id, .a0 = nan, .a1 = new_values.a1};
            Idx const size = range_object.size();
            for (Idx idx = 0; idx < size; ++idx) {
                check_buffer(range_object);
                range_object[idx] = new_values;
                check_buffer(range_object);
                test::check_equal(range_object[idx].get(), expected);
            }
            for (auto proxy : range_object) {
                check_buffer(range_object);
                proxy = new_values;
                check_buffer(range_object);
                test::check_equal(proxy.get(), expected);
            }
        }
    }

    SUBCASE("Iterator access") {
        SUBCASE("Distance") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                CHECK(std::distance(range_object.begin(), range_object.begin() + idx) == idx);
                CHECK(std::distance(range_object.begin() + idx, range_object.end()) == range_object.size() - idx);
            }
        }
        SUBCASE("Equal") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                CHECK(range_object.begin() + idx == range_object.end() - range_object.size() + idx);
                CHECK(range_object.begin() + idx != range_object.begin() + idx + 1);
                CHECK(range_object.begin() + idx != range_object.begin() + idx - 1);
            }
        }
        SUBCASE("Prefix increment") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                auto to_prefix_increment = range_object.begin() + idx;
                CHECK(range_object.begin() + idx + 1 == ++to_prefix_increment);
                CHECK(range_object.begin() + idx + 1 == to_prefix_increment);
            }
        }
        SUBCASE("Prefix decrement") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                auto to_prefix_decrement = range_object.begin() + idx;
                CHECK(range_object.begin() + idx - 1 == --to_prefix_decrement);
                CHECK(range_object.begin() + idx - 1 == to_prefix_decrement);
            }
        }
        SUBCASE("Suffix increment") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                auto to_suffix_increment = range_object.begin() + idx;
                CHECK(range_object.begin() + idx == to_suffix_increment++);
                CHECK(range_object.begin() + idx + 1 == to_suffix_increment);
            }
        }
        SUBCASE("Suffix decrement") {
            for (Idx idx = 0; idx < range_object.size(); ++idx) {
                auto to_suffix_decrement = range_object.begin() + idx;
                CHECK(range_object.begin() + idx == to_suffix_decrement--);
                CHECK(range_object.begin() + idx - 1 == to_suffix_decrement);
            }
        }
        SUBCASE("Iteration") {
            Idx count = 0;
            for (auto const& element : range_object) {
                test::check_equal(element, range_object[count]);
                ++count;
            }
        }
    }
}

TEST_CASE_TEMPLATE("Test dataset (common)", DatasetType, ConstDataset, MutableDataset, WritableDataset) {
    std::vector<Idx> fake_data;
    std::vector<Idx> fake_indptr;

    constexpr auto create_dataset = [](bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
        return test::create_dataset<DatasetType>(is_batch, batch_size, dataset_type);
    };
    auto const add_buffer = [](DatasetType& dataset, std::string_view name, Idx elements_per_scenario,
                               Idx total_elements, Idx* indptr, void* data) {
        if constexpr (std::same_as<DatasetType, WritableDataset>) {
            dataset.add_component_info(name, elements_per_scenario, total_elements); // in deserializer
            dataset.set_buffer(name, indptr, data);                                  // by end-user
        } else {
            dataset.add_buffer(name, elements_per_scenario, total_elements, indptr, data);
        }
    };
    auto const add_attribute_buffer = [](DatasetType& dataset, std::string_view name, std::string_view attribute,
                                         auto* data) {
        if constexpr (std::same_as<DatasetType, WritableDataset>) {
            dataset.set_attribute_buffer(name, attribute, data);
        } else {
            dataset.add_attribute_buffer(name, attribute, data);
        }
    };
    auto const add_homogeneous_buffer = [&add_buffer](DatasetType& dataset, std::string_view name,
                                                      Idx elements_per_scenario, void* data) {
        add_buffer(dataset, name, elements_per_scenario, elements_per_scenario * dataset.batch_size(), nullptr, data);
    };
    auto const add_inhomogeneous_buffer = [&add_buffer](DatasetType& dataset, std::string_view name, Idx total_elements,
                                                        Idx* indptr, void* data) {
        add_buffer(dataset, name, -1, total_elements, indptr, data);
    };
    auto get_data_buffer = [&fake_data](bool const& is_columnar, Idx const& total_elements) -> Idx* {
        if (is_columnar) {
            return nullptr;
        }
        fake_data.resize(std::max(narrow_cast<Idx>(fake_data.size()), total_elements));
        return fake_data.data();
    };
    auto get_indptr_buffer = [&fake_indptr](Idx const& elements_per_scenario, Idx const& total_elements,
                                            DatasetType const& dataset) -> Idx* {
        if (elements_per_scenario != -1) {
            return nullptr;
        }
        fake_indptr.resize(std::max(narrow_cast<Idx>(fake_indptr.size()), dataset.batch_size() + 1));
        std::ranges::fill(fake_indptr, Idx{0});
        fake_indptr.back() = total_elements;
        return fake_indptr.data();
    };
    auto const add_component_info = [&add_buffer, &get_data_buffer, &get_indptr_buffer](
                                        DatasetType& dataset, std::string_view name, Idx elements_per_scenario,
                                        Idx total_elements, bool is_columnar = false) {
        if constexpr (std::same_as<DatasetType, WritableDataset>) {
            (void)add_buffer;
            (void)get_data_buffer;
            (void)get_indptr_buffer;
            dataset.add_component_info(name, elements_per_scenario, total_elements);
        } else {
            void* data_buffer = get_data_buffer(is_columnar, total_elements);
            Idx* indptr_buffer = get_indptr_buffer(elements_per_scenario, total_elements, dataset);
            add_buffer(dataset, name, elements_per_scenario, total_elements, indptr_buffer, data_buffer);
        }
    };

    SUBCASE("Constructor") {
        SUBCASE("Single dataset") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(std::string_view{dataset_type.name});
                create_dataset(false, 1, dataset_type);
            }
        }
        SUBCASE("Batch dataset") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(std::string_view{dataset_type.name});
                for (auto batch_size : {0, 1, 2}) {
                    CAPTURE(batch_size);
                    create_dataset(true, batch_size, dataset_type);
                }
                CHECK_THROWS_AS(create_dataset(true, -1, dataset_type), DatasetError);
            }
        }
        SUBCASE("Unknown dataset name") {
            constexpr auto construct = [] { return DatasetType{false, 1, "sym_output", test_meta_data}; };
            CHECK_THROWS_AS(construct(), std::out_of_range);
        }
        SUBCASE("Single dataset with wrong batch size") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(std::string_view{dataset_type.name});
                auto const construct = [&dataset_type] {
                    return DatasetType{false, 0, dataset_type.name, test_meta_data_all};
                };
                CHECK_THROWS_AS(construct(), DatasetError);
            }
        }
    }

    SUBCASE("Component info") {
        auto const& dataset_type = test_meta_data_all.datasets.front();
        CAPTURE(std::string_view{dataset_type.name});

        for (auto const batch_size : {0, 1, 2}) {
            CAPTURE(batch_size);

            SUBCASE("No component added") {
                CAPTURE(batch_size);
                auto dataset = create_dataset(true, batch_size, dataset_type);

                CHECK(dataset.n_components() == 0);
                CHECK_FALSE(dataset.contains_component(A::name));
                CHECK(dataset.get_description().component_info.empty());
                CHECK_THROWS_AS(dataset.get_component_info(A::name), DatasetError);
            }
            SUBCASE("Add homogeneous component info") {
                for (auto const elements_per_scenario : {0, 1, 2}) {
                    CAPTURE(elements_per_scenario);
                    auto const total_elements = elements_per_scenario * batch_size;

                    auto dataset = create_dataset(true, batch_size, dataset_type);
                    CHECK_FALSE(dataset.contains_component(A::name));

                    add_component_info(dataset, A::name, elements_per_scenario, total_elements);
                    CHECK(dataset.n_components() == 1);
                    CHECK(dataset.contains_component(A::name));

                    auto const& component_info = dataset.get_component_info(A::name);
                    CHECK(component_info.component == &dataset_type.get_component(A::name));
                    CHECK(component_info.elements_per_scenario == elements_per_scenario);
                    CHECK(component_info.total_elements == total_elements);

                    CHECK_FALSE(dataset.get_description().component_info.empty());
                }
            }
            SUBCASE("Add inhomogeneous component info") {
                for (auto const total_elements : {0, 1, 2}) {
                    CAPTURE(total_elements);
                    constexpr auto elements_per_scenario = -1;

                    auto dataset = create_dataset(true, batch_size, dataset_type);
                    if (batch_size == 0 && total_elements > 0 && !std::same_as<DatasetType, WritableDataset>) {
                        CHECK_THROWS_AS(add_component_info(dataset, A::name, elements_per_scenario, total_elements),
                                        DatasetError);
                    } else {
                        add_component_info(dataset, A::name, elements_per_scenario, total_elements);
                        CHECK(dataset.n_components() == 1);
                        CHECK(dataset.contains_component(A::name));

                        auto const& component_info = dataset.get_component_info(A::name);
                        CHECK(component_info.component == &dataset_type.get_component(A::name));
                        CHECK(component_info.elements_per_scenario == elements_per_scenario);
                        CHECK(component_info.total_elements == total_elements);

                        CHECK_FALSE(dataset.get_description().component_info.empty());
                    }
                }
            }
            SUBCASE("Add unknown component info") {
                auto dataset = create_dataset(true, batch_size, dataset_type);
                CHECK_THROWS_AS(add_component_info(dataset, "unknown", 0, 0), std::out_of_range);
            }
            SUBCASE("Add duplicate component info") {
                auto dataset = create_dataset(true, batch_size, dataset_type);
                CHECK_NOTHROW(add_component_info(dataset, A::name, 0, 0));
                CHECK_THROWS_AS(add_component_info(dataset, A::name, 0, 0), DatasetError);
            }
            SUBCASE("Add inconsistent component info") {
                auto dataset = create_dataset(true, batch_size, dataset_type);
                CHECK_THROWS_AS(add_component_info(dataset, A::name, 1, batch_size + 1), DatasetError);
            }
        }
    }

    SUBCASE("Component query") {
        auto const& dataset_type = test_meta_data_all.datasets.front();
        CAPTURE(std::string_view{dataset_type.name});
        auto dataset = create_dataset(true, 1, dataset_type);

        auto const check_has_no_component = [&dataset](std::string_view name) {
            CHECK(dataset.find_component(name) == DatasetType::invalid_index);
            CHECK(dataset.find_component(name, false) == DatasetType::invalid_index);
            CHECK_THROWS_AS(dataset.find_component(name, true), DatasetError);
        };
        auto const check_has_component_at_index = [&dataset](std::string_view name, Idx index) {
            CHECK(dataset.find_component(name) == index);
            CHECK(dataset.find_component(name, false) == index);
            CHECK(dataset.find_component(name, true) == index);
        };

        check_has_no_component(A::name);
        check_has_no_component(B::name);
        add_component_info(dataset, B::name, 0, 0);
        check_has_no_component(A::name);
        check_has_component_at_index(B::name, 0);
        add_component_info(dataset, A::name, 0, 0);
        check_has_component_at_index(A::name, 1);
        check_has_component_at_index(B::name, 0);
    }

    SUBCASE("Buffer query") {
        auto const& dataset_type = test_meta_data_all.datasets.front();
        CAPTURE(std::string_view{dataset_type.name});

        SUBCASE("Homogeneous buffer") {
            SUBCASE("Single dataset") {
                for (auto const elements_per_scenario : {0, 1, 2}) {
                    CAPTURE(elements_per_scenario);
                    auto const total_elements = elements_per_scenario;

                    auto dataset = create_dataset(false, 1, dataset_type);

                    auto a_buffer = std::vector<A::InputType>(total_elements);
                    add_homogeneous_buffer(dataset, A::name, elements_per_scenario,
                                           static_cast<void*>(a_buffer.data()));

                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(), total_elements, a_buffer);
                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index),
                                   total_elements, a_buffer);
                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(0), total_elements, a_buffer);

                    auto const all_scenario_spans = dataset.template get_buffer_span_all_scenarios<input_getter_s, A>();
                    CHECK(all_scenario_spans.size() == 1);
                    check_row_span(all_scenario_spans[0], total_elements, a_buffer);
                }
            }
            SUBCASE("Batch dataset") {
                for (auto const batch_size : {0, 1, 2}) {
                    CAPTURE(batch_size);
                    for (auto const elements_per_scenario : {0, 1, 2}) {
                        CAPTURE(elements_per_scenario);
                        auto const total_elements = elements_per_scenario * batch_size;

                        auto dataset = create_dataset(true, batch_size, dataset_type);

                        auto a_buffer = std::vector<A::InputType>(4);
                        add_homogeneous_buffer(dataset, A::name, elements_per_scenario,
                                               static_cast<void*>(a_buffer.data()));

                        CHECK(dataset.template get_buffer_span<input_getter_s, A>().data() == a_buffer.data());
                        CHECK(dataset.template get_buffer_span<input_getter_s, A>().size() == total_elements);
                        CHECK(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index).data() ==
                              a_buffer.data());
                        CHECK(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index).size() ==
                              total_elements);

                        auto const all_scenario_spans =
                            dataset.template get_buffer_span_all_scenarios<input_getter_s, A>();
                        CHECK(all_scenario_spans.size() == batch_size);

                        for (Idx scenario : {0, 1, 2, 3}) {
                            CAPTURE(scenario);
                            if (scenario < batch_size) {
                                auto const scenario_span =
                                    dataset.template get_buffer_span<input_getter_s, A>(scenario);

                                CHECK(scenario_span.data() == a_buffer.data() + scenario * elements_per_scenario);
                                CHECK(scenario_span.size() == elements_per_scenario);
                                CHECK(all_scenario_spans[scenario].data() == scenario_span.data());
                                CHECK(all_scenario_spans[scenario].size() == scenario_span.size());
                            }
                        }
                    }
                }
            }
        }
        SUBCASE("Inhomogeneous buffer") {
            SUBCASE("Single dataset") {
                for (auto const total_elements : {0, 1, 2}) {
                    CAPTURE(total_elements);

                    auto dataset = create_dataset(false, 1, dataset_type);

                    auto a_buffer = std::vector<A::InputType>(total_elements);
                    auto a_indptr = std::vector<Idx>{0, total_elements};
                    add_inhomogeneous_buffer(dataset, A::name, total_elements, a_indptr.data(),
                                             static_cast<void*>(a_buffer.data()));

                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(), total_elements, a_buffer);
                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index),
                                   total_elements, a_buffer);
                    check_row_span(dataset.template get_buffer_span<input_getter_s, A>(0), total_elements, a_buffer);

                    auto const all_scenario_spans = dataset.template get_buffer_span_all_scenarios<input_getter_s, A>();
                    CHECK(all_scenario_spans.size() == 1);
                    check_row_span(all_scenario_spans[0], total_elements, a_buffer);
                }
            }
            SUBCASE("Batch dataset") {
                for (auto const& elements_per_scenarios :
                     {std::vector<Idx>{}, std::vector<Idx>{4}, std::vector<Idx>{1, 1, 2},
                      std::vector<Idx>{0, 2, 0, 1, 1, 0}, std::vector<Idx>{2, 2}}) {
                    auto const batch_size = static_cast<Idx>(elements_per_scenarios.size());
                    auto const total_elements =
                        std::accumulate(elements_per_scenarios.begin(), elements_per_scenarios.end(), Idx{0});
                    CAPTURE(batch_size);
                    CAPTURE(total_elements);
                    CAPTURE(elements_per_scenarios);

                    auto dataset = create_dataset(true, batch_size, dataset_type);

                    auto a_buffer = std::vector<A::InputType>(total_elements);
                    auto a_indptr = std::vector<Idx>{};
                    std::exclusive_scan(elements_per_scenarios.begin(), elements_per_scenarios.end(),
                                        std::back_insert_iterator(a_indptr), Idx{0});
                    a_indptr.push_back(total_elements);

                    add_inhomogeneous_buffer(dataset, A::name, total_elements, a_indptr.data(),
                                             static_cast<void*>(a_buffer.data()));

                    CHECK(dataset.template get_buffer_span<input_getter_s, A>().data() == a_buffer.data());
                    CHECK(dataset.template get_buffer_span<input_getter_s, A>().size() == total_elements);
                    CHECK(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index).data() ==
                          a_buffer.data());
                    CHECK(dataset.template get_buffer_span<input_getter_s, A>(DatasetType::invalid_index).size() ==
                          total_elements);

                    auto const all_scenario_spans = dataset.template get_buffer_span_all_scenarios<input_getter_s, A>();
                    CHECK(all_scenario_spans.size() == batch_size);

                    for (Idx scenario : {0, 1, 2, 3}) {
                        CAPTURE(scenario);
                        if (scenario < batch_size) {
                            auto const scenario_span = dataset.template get_buffer_span<input_getter_s, A>(scenario);

                            CHECK(scenario_span.data() == a_buffer.data() + a_indptr[scenario]);
                            CHECK(scenario_span.size() == elements_per_scenarios[scenario]);
                            CHECK(all_scenario_spans[scenario].data() == scenario_span.data());
                            CHECK(all_scenario_spans[scenario].size() == scenario_span.size());
                        }
                    }
                }
            }
        }
        SUBCASE("Homogeneous columnar buffer") {
            SUBCASE("Single dataset") {
                for (auto const elements_per_scenario : {0, 1, 2}) {
                    CAPTURE(elements_per_scenario);
                    auto const total_elements = elements_per_scenario;

                    auto dataset = create_dataset(false, 1, dataset_type);

                    auto id_buffer = std::vector<ID>(total_elements);
                    auto a1_buffer = std::vector<double>(total_elements);

                    add_homogeneous_buffer(dataset, A::name, elements_per_scenario, nullptr);

                    auto const check_span = [&](auto const& buffer_span) {
                        CHECK(buffer_span.size() == total_elements);
                        for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                            auto const element = get_colummnar_element<DatasetType>(buffer_span, idx);
                            CHECK(element.id == id_buffer[idx]);
                            CHECK(element.a1 == a1_buffer[idx]);
                            CHECK(is_nan(element.a0));
                        }
                    };
                    auto const check_all_spans = [&] {
                        check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>());
                        check_span(
                            dataset.template get_columnar_buffer_span<input_getter_s, A>(DatasetType::invalid_index));

                        check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>(0));

                        auto const all_scenario_spans =
                            dataset.template get_columnar_buffer_span_all_scenarios<input_getter_s, A>();
                        CHECK(all_scenario_spans.size() == 1);
                        check_span(all_scenario_spans[0]);
                    };

                    add_attribute_buffer(dataset, A::name, A::InputType::a1_name, a1_buffer.data());
                    add_attribute_buffer(dataset, A::name, A::InputType::id_name, id_buffer.data());

                    check_all_spans();

                    std::ranges::fill(id_buffer, 1);
                    check_all_spans();

                    std::ranges::transform(std::ranges::iota_view{ID{0}, total_elements}, id_buffer.begin(),
                                           [](ID value) { return value * 2; });

                    check_all_spans();
                    std::ranges::transform(id_buffer, a1_buffer.begin(),
                                           [](ID value) { return static_cast<double>(value); });
                    check_all_spans();

                    if constexpr (!std::same_as<DatasetType, ConstDataset>) {
                        auto const buffer_span = dataset.template get_columnar_buffer_span<input_getter_s, A>();
                        for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                            buffer_span[idx] = A::InputType{.id = -10, .a0 = -1.0, .a1 = -2.0};
                            CHECK(id_buffer[idx] == -10);
                            CHECK(a1_buffer[idx] == -2.0);
                            check_all_spans();
                        }
                    }
                }
            }
            SUBCASE("Batch dataset") {
                for (auto const batch_size : {0, 1, 2}) {
                    CAPTURE(batch_size);
                    for (auto const elements_per_scenario : {0, 1, 2}) {
                        CAPTURE(elements_per_scenario);
                        auto const total_elements = elements_per_scenario * batch_size;

                        auto dataset = create_dataset(true, batch_size, dataset_type);

                        auto id_buffer = std::vector<ID>(total_elements);
                        auto a1_buffer = std::vector<double>(total_elements);
                        add_homogeneous_buffer(dataset, A::name, elements_per_scenario, nullptr);

                        auto const check_span = [&](auto const& buffer_span, Idx const& scenario = -1) {
                            auto element_number = total_elements;
                            Idx aux_idx = 0;
                            if (scenario != -1) {
                                element_number = elements_per_scenario;
                                aux_idx = scenario * elements_per_scenario;
                            }
                            CHECK(buffer_span.size() == element_number);
                            for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                                auto const element = get_colummnar_element<DatasetType>(buffer_span, idx);
                                CHECK(element.id == id_buffer[aux_idx + idx]);
                                CHECK(element.a1 == a1_buffer[aux_idx + idx]);
                                CHECK(is_nan(element.a0));
                            }
                        };
                        auto const check_all_spans = [&](auto const& scenario) {
                            check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>());
                            check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>(
                                DatasetType::invalid_index));

                            auto const all_scenario_spans =
                                dataset.template get_columnar_buffer_span_all_scenarios<input_getter_s, A>();
                            CHECK(all_scenario_spans.size() == batch_size);

                            auto const scenario_span =
                                dataset.template get_columnar_buffer_span<input_getter_s, A>(scenario);
                            check_span(scenario_span, scenario);
                            CHECK(all_scenario_spans[scenario].size() == scenario_span.size());
                            check_span(all_scenario_spans[scenario], scenario);
                        };
                        add_attribute_buffer(dataset, A::name, A::InputType::a1_name, a1_buffer.data());
                        add_attribute_buffer(dataset, A::name, A::InputType::id_name, id_buffer.data());
                        for (Idx scenario : {0, 1, 2, 3}) {
                            CAPTURE(scenario);
                            if (scenario < batch_size) {
                                check_all_spans(scenario);

                                std::ranges::fill(id_buffer, 1);
                                check_all_spans(scenario);

                                std::ranges::transform(std::ranges::iota_view{ID{0}, total_elements}, id_buffer.begin(),
                                                       [](ID value) { return value * 2; });
                                check_all_spans(scenario);

                                std::ranges::transform(id_buffer, a1_buffer.begin(),
                                                       [](ID value) { return static_cast<double>(value); });
                                check_all_spans(scenario);

                                if constexpr (!std::same_as<DatasetType, ConstDataset>) {
                                    auto buffer_span =
                                        dataset.template get_columnar_buffer_span<input_getter_s, A>(scenario);
                                    Idx const size = buffer_span.size();
                                    for (Idx idx = 0; idx < size; ++idx) {
                                        buffer_span[idx] = A::InputType{.id = -10, .a0 = -1.0, .a1 = -2.0};
                                        CHECK(id_buffer[idx + (scenario * elements_per_scenario)] == -10);
                                        CHECK(a1_buffer[idx + (scenario * elements_per_scenario)] == -2.0);
                                        check_all_spans(scenario);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        SUBCASE("Inhomogeneous columnar buffer") {
            SUBCASE("Single dataset") {
                for (auto const elements_per_scenario : {0, 1, 2}) {
                    CAPTURE(elements_per_scenario);
                    auto const total_elements = elements_per_scenario;

                    auto dataset = create_dataset(false, 1, dataset_type);

                    auto id_buffer = std::vector<ID>(total_elements);
                    auto a1_buffer = std::vector<double>(total_elements);
                    auto a_indptr = std::vector<Idx>{0, total_elements};

                    add_inhomogeneous_buffer(dataset, A::name, total_elements, a_indptr.data(), nullptr);

                    auto const check_span = [&](auto const& buffer_span) {
                        CHECK(buffer_span.size() == total_elements);
                        for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                            auto const element = get_colummnar_element<DatasetType>(buffer_span, idx);
                            CHECK(element.id == id_buffer[idx]);
                            CHECK(element.a1 == a1_buffer[idx]);
                            CHECK(is_nan(element.a0));
                        }
                    };
                    auto const check_all_spans = [&] {
                        check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>());
                        check_span(
                            dataset.template get_columnar_buffer_span<input_getter_s, A>(DatasetType::invalid_index));

                        check_span(dataset.template get_columnar_buffer_span<input_getter_s, A>(0));

                        auto const all_scenario_spans =
                            dataset.template get_columnar_buffer_span_all_scenarios<input_getter_s, A>();
                        CHECK(all_scenario_spans.size() == 1);
                        check_span(all_scenario_spans[0]);
                    };

                    add_attribute_buffer(dataset, A::name, A::InputType::a1_name, a1_buffer.data());
                    add_attribute_buffer(dataset, A::name, A::InputType::id_name, id_buffer.data());
                    check_all_spans();

                    std::ranges::fill(id_buffer, 1);
                    check_all_spans();

                    std::ranges::transform(std::ranges::iota_view{ID{0}, total_elements}, id_buffer.begin(),
                                           [](ID value) { return value * 2; });
                    check_all_spans();

                    std::ranges::transform(id_buffer, a1_buffer.begin(),
                                           [](ID value) { return static_cast<double>(value); });
                    check_all_spans();

                    if constexpr (!std::same_as<DatasetType, ConstDataset>) {
                        auto const buffer_span = dataset.template get_columnar_buffer_span<input_getter_s, A>();
                        for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                            buffer_span[idx] = A::InputType{.id = -10, .a0 = -1.0, .a1 = -2.0};
                            CHECK(id_buffer[idx] == -10);
                            CHECK(a1_buffer[idx] == -2.0);
                            check_all_spans();
                        }
                    }
                }
            }
            SUBCASE("Batch dataset") {
                for (auto const& elements_per_scenarios :
                     {std::vector<Idx>{}, std::vector<Idx>{4}, std::vector<Idx>{1, 1, 2},
                      std::vector<Idx>{0, 2, 0, 1, 1, 0}, std::vector<Idx>{2, 2}}) {
                    auto const batch_size = static_cast<Idx>(elements_per_scenarios.size());
                    auto const total_elements =
                        std::accumulate(elements_per_scenarios.begin(), elements_per_scenarios.end(), Idx{0});
                    CAPTURE(batch_size);
                    CAPTURE(total_elements);
                    CAPTURE(elements_per_scenarios);

                    auto dataset = create_dataset(true, batch_size, dataset_type);

                    auto id_buffer = std::vector<ID>(total_elements);
                    auto a1_buffer = std::vector<double>(total_elements);
                    auto a_indptr = std::vector<Idx>{};
                    std::exclusive_scan(elements_per_scenarios.begin(), elements_per_scenarios.end(),
                                        std::back_insert_iterator(a_indptr), Idx{0});
                    a_indptr.push_back(total_elements);

                    add_inhomogeneous_buffer(dataset, A::name, total_elements, a_indptr.data(), nullptr);

                    auto const check_span = [&total_elements, &elements_per_scenarios, &a_indptr, &id_buffer,
                                             &a1_buffer]<typename T>(auto const& buffer_span,
                                                                     Idx const& scenario = -1) {
                        auto element_number = total_elements;
                        Idx aux_idx = 0;
                        if (scenario != -1) {
                            element_number = elements_per_scenarios[scenario];
                            aux_idx = a_indptr[scenario];
                        }
                        CHECK(buffer_span.size() == element_number);
                        for (Idx idx = 0; idx < buffer_span.size(); ++idx) {
                            auto const element = get_colummnar_element<T>(buffer_span, idx);
                            CHECK(element.id == id_buffer[aux_idx + idx]);
                            CHECK(element.a1 == a1_buffer[aux_idx + idx]);
                            CHECK(is_nan(element.a0));
                        }
                    };
                    auto const check_all_spans = [&check_span, &batch_size]<typename T>(auto& any_dataset,
                                                                                        auto const& scenario) {
                        check_span.template operator()<T>(
                            any_dataset.template get_columnar_buffer_span<input_getter_s, A>());
                        check_span.template operator()<T>(
                            any_dataset.template get_columnar_buffer_span<input_getter_s, A>(T::invalid_index));

                        auto const all_scenario_spans =
                            any_dataset.template get_columnar_buffer_span_all_scenarios<input_getter_s, A>();
                        CHECK(all_scenario_spans.size() == batch_size);

                        auto const scenario_span =
                            any_dataset.template get_columnar_buffer_span<input_getter_s, A>(scenario);
                        check_span.template operator()<T>(scenario_span, scenario);
                        CHECK(all_scenario_spans[scenario].size() == scenario_span.size());
                        check_span.template operator()<T>(all_scenario_spans[scenario], scenario);
                    };
                    add_attribute_buffer(dataset, A::name, A::InputType::a1_name, a1_buffer.data());
                    add_attribute_buffer(dataset, A::name, A::InputType::id_name, id_buffer.data());
                    for (Idx scenario : {0, 1, 2, 3}) {
                        CAPTURE(scenario);
                        if (scenario < batch_size) {
                            check_all_spans.template operator()<DatasetType>(dataset, scenario);

                            std::ranges::fill(id_buffer, 1);
                            check_all_spans.template operator()<DatasetType>(dataset, scenario);

                            std::ranges::transform(std::ranges::iota_view{ID{0}, static_cast<ID>(total_elements)},
                                                   id_buffer.begin(), [](ID value) { return value * 2; });
                            check_all_spans.template operator()<DatasetType>(dataset, scenario);

                            std::ranges::transform(id_buffer, a1_buffer.begin(),
                                                   [](ID value) { return static_cast<double>(value); });
                            check_all_spans.template operator()<DatasetType>(dataset, scenario);

                            auto dataset_copy = ConstDataset{dataset};
                            check_all_spans.template operator()<ConstDataset>(dataset_copy, scenario);

                            if constexpr (!std::same_as<DatasetType, ConstDataset>) {
                                auto buffer_span =
                                    dataset.template get_columnar_buffer_span<input_getter_s, A>(scenario);
                                Idx const size = buffer_span.size();
                                for (Idx idx = 0; idx < size; ++idx) {
                                    buffer_span[idx] = A::InputType{.id = -10, .a0 = -1.0, .a1 = -2.0};
                                    CHECK(id_buffer[idx + (a_indptr[scenario])] == -10);
                                    CHECK(a1_buffer[idx + (a_indptr[scenario])] == -2.0);
                                    check_all_spans.template operator()<DatasetType>(dataset, scenario);
                                }
                            }
                        }
                    }
                }
            }
        }
        SUBCASE("Duplicate buffer entry") {
            auto const& dataset_typ = test_meta_data_all.datasets.front();
            CAPTURE(std::string_view{dataset_typ.name});

            auto dataset = create_dataset(true, 0, dataset_typ);
            auto a_buffer = std::vector<A::InputType>(1);
            auto a_indptr = std::vector<Idx>{0};
            SUBCASE("Homogeneous buffer") {
                add_homogeneous_buffer(dataset, A::name, 0, static_cast<void*>(a_buffer.data()));
                CHECK_THROWS_AS(add_homogeneous_buffer(dataset, A::name, 0, static_cast<void*>(a_buffer.data())),
                                DatasetError);
            }
            SUBCASE("Inhomogeneous buffer") {
                add_homogeneous_buffer(dataset, A::name, 0, static_cast<void*>(a_buffer.data()));
                CHECK_THROWS_AS(
                    add_inhomogeneous_buffer(dataset, A::name, 0, a_indptr.data(), static_cast<void*>(a_buffer.data())),
                    DatasetError);
            }
            SUBCASE("Mixed buffer types") {
                auto a_indptr = std::vector<Idx>{0, 0};
                add_homogeneous_buffer(dataset, A::name, 0, static_cast<void*>(a_buffer.data()));
                CHECK_THROWS_AS(
                    add_inhomogeneous_buffer(dataset, A::name, 0, a_indptr.data(), static_cast<void*>(a_buffer.data())),
                    DatasetError);
            }
        }
    }

    if constexpr (std::same_as<DatasetType, ConstDataset> || std::same_as<DatasetType, MutableDataset>) {
        SUBCASE("Get individual scenario") {
            auto const& dataset_type = test_meta_data_all.datasets.front();
            CAPTURE(std::string_view{dataset_type.name});

            Idx const batch_size{2};
            Idx const a_elements_per_scenario{3};

            auto dataset = create_dataset(true, batch_size, dataset_type);

            auto const check_get_individual_scenario = [&] {
                for (auto scenario = 0; scenario < batch_size; ++scenario) {
                    CAPTURE(scenario);
                    auto const scenario_dataset = dataset.get_individual_scenario(scenario);

                    CHECK(&scenario_dataset.meta_data() == &dataset.meta_data());
                    CHECK(!scenario_dataset.empty());
                    CHECK(scenario_dataset.is_batch() == false);
                    CHECK(scenario_dataset.batch_size() == 1);
                    CHECK(scenario_dataset.n_components() == dataset.n_components());

                    CHECK(scenario_dataset.get_component_info(A::name).component ==
                          &dataset_type.get_component(A::name));
                    CHECK(scenario_dataset.get_component_info(A::name).elements_per_scenario ==
                          a_elements_per_scenario);
                    CHECK(scenario_dataset.get_component_info(A::name).total_elements == a_elements_per_scenario);

                    CHECK(scenario_dataset.get_component_info(B::name).component ==
                          &dataset_type.get_component(B::name));
                    auto const expected_size =
                        dataset.is_row_based(dataset.get_buffer(B::name))
                            ? dataset.template get_buffer_span<input_getter_s, B>(scenario).size()
                            : dataset.template get_columnar_buffer_span<input_getter_s, B>(scenario).size();
                    CHECK(scenario_dataset.get_component_info(B::name).elements_per_scenario == expected_size);
                    CHECK(scenario_dataset.get_component_info(B::name).total_elements ==
                          scenario_dataset.get_component_info(B::name).elements_per_scenario);

                    if (dataset.is_row_based(dataset.get_buffer(A::name))) {
                        auto const scenario_span_a = scenario_dataset.template get_buffer_span<input_getter_s, A>();
                        auto const dataset_span_a = dataset.template get_buffer_span<input_getter_s, A>(scenario);
                        CHECK(scenario_span_a.data() == dataset_span_a.data());
                        CHECK(scenario_span_a.size() == dataset_span_a.size());
                    } else {
                        auto const scenario_span_a =
                            scenario_dataset.template get_columnar_buffer_span<input_getter_s, A>();
                        auto const dataset_span_a =
                            dataset.template get_columnar_buffer_span<input_getter_s, A>(scenario);
                        REQUIRE(scenario_span_a.size() == dataset_span_a.size());
                        for (Idx idx = 0; idx < scenario_span_a.size(); ++idx) {
                            auto const scenario_element = scenario_span_a[idx].get();
                            auto const& dataset_element = dataset_span_a[idx].get();
                            CHECK(scenario_element.id == dataset_element.id);
                            CHECK(scenario_element.a1 == dataset_element.a1);
                        }
                    }
                    if (dataset.is_row_based(dataset.get_buffer(B::name))) {
                        auto const scenario_span_b = scenario_dataset.template get_buffer_span<input_getter_s, B>();
                        auto const dataset_span_b = dataset.template get_buffer_span<input_getter_s, B>(scenario);
                        CHECK(scenario_span_b.data() == dataset_span_b.data());
                        CHECK(scenario_span_b.size() == dataset_span_b.size());
                    } else {
                        auto const scenario_span_b =
                            scenario_dataset.template get_columnar_buffer_span<input_getter_s, B>();
                        auto const dataset_span_b =
                            dataset.template get_columnar_buffer_span<input_getter_s, B>(scenario);
                        CHECK(scenario_span_b.begin() == dataset_span_b.begin());
                        CHECK(scenario_span_b.size() == dataset_span_b.size());
                    }
                }
            };

            SUBCASE("row-based") {
                auto a_buffer = std::vector<A::InputType>(a_elements_per_scenario * batch_size);
                auto b_buffer = std::vector<A::InputType>(3);
                auto b_indptr = std::vector<Idx>{0, 0, narrow_cast<Idx>(b_buffer.size())};
                std::ranges::transform(IdxRange(0, std::ssize(a_buffer)), a_buffer.begin(), [](Idx idx) {
                    return A::InputType{.id = static_cast<ID>(idx), .a1 = static_cast<double>(idx)};
                });
                add_homogeneous_buffer(dataset, A::name, a_elements_per_scenario, static_cast<void*>(a_buffer.data()));
                add_inhomogeneous_buffer(dataset, B::name, b_buffer.size(), b_indptr.data(),
                                         static_cast<void*>(b_buffer.data()));

                check_get_individual_scenario();
            }
            SUBCASE("columnar") {
                auto a_id_buffer =
                    std::views::iota(0, a_elements_per_scenario * batch_size) | std::ranges::to<std::vector<ID>>();
                auto a_a1_buffer =
                    std::views::iota(0, a_elements_per_scenario * batch_size) | std::ranges::to<std::vector<double>>();
                auto b_indptr = std::vector<Idx>{0, 0, 3};

                add_homogeneous_buffer(dataset, A::name, a_elements_per_scenario, nullptr);
                add_attribute_buffer(dataset, A::name, "id", static_cast<void*>(a_id_buffer.data()));
                add_attribute_buffer(dataset, A::name, "a1", static_cast<void*>(a_a1_buffer.data()));
                add_inhomogeneous_buffer(dataset, B::name, b_indptr.back(), b_indptr.data(), nullptr);

                check_get_individual_scenario();
            }
        }
    }
}

TEST_CASE("Test writable dataset") {
    using DatasetType = WritableDataset;

    constexpr auto create_dataset = [](bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
        return test::create_dataset<DatasetType>(is_batch, batch_size, dataset_type);
    };
    auto const& dataset_type = test_meta_data_all.datasets.front();
    CAPTURE(std::string_view{dataset_type.name});

    for (auto const batch_size : {0, 1, 2}) {
        CAPTURE(batch_size);

        SUBCASE("Add homogeneous component info") {
            for (auto const elements_per_scenario : {-1, 0, 1, 2}) {
                CAPTURE(elements_per_scenario);
                auto const total_elements = elements_per_scenario * batch_size;

                auto dataset = create_dataset(true, batch_size, dataset_type);
                CHECK_FALSE(dataset.contains_component(A::name));

                dataset.add_component_info(A::name, elements_per_scenario, total_elements);
                CHECK(dataset.n_components() == 1);
                CHECK(dataset.contains_component(A::name));

                auto const& component_info = dataset.get_component_info(A::name);
                CHECK(component_info.component == &dataset_type.get_component(A::name));
                CHECK(component_info.elements_per_scenario == elements_per_scenario);
                CHECK(component_info.total_elements == total_elements);

                CHECK_FALSE(dataset.get_description().component_info.empty());
            }
        }
        SUBCASE("Add inhomogeneous component info") {
            for (auto const total_elements : {0, 1, 2}) {
                CAPTURE(total_elements);
                constexpr auto elements_per_scenario = -1;

                auto dataset = create_dataset(true, batch_size, dataset_type);
                dataset.add_component_info(A::name, elements_per_scenario, total_elements);
                CHECK(dataset.n_components() == 1);
                CHECK(dataset.contains_component(A::name));

                auto const& component_info = dataset.get_component_info(A::name);
                CHECK(component_info.component == &dataset_type.get_component(A::name));
                CHECK(component_info.elements_per_scenario == elements_per_scenario);
                CHECK(component_info.total_elements == total_elements);

                CHECK_FALSE(dataset.get_description().component_info.empty());
            }
        }
        SUBCASE("Add unknown component info") {
            auto dataset = create_dataset(true, batch_size, dataset_type);
            CHECK_THROWS_AS(dataset.add_component_info("unknown", 0, 0), std::out_of_range);
        }
        SUBCASE("Add duplicate component info") {
            auto dataset = create_dataset(true, batch_size, dataset_type);
            CHECK_NOTHROW(dataset.add_component_info(A::name, 0, 0));
            CHECK_THROWS_AS(dataset.add_component_info(A::name, 0, 0), DatasetError);
        }
        SUBCASE("Add inconsistent component info") {
            auto dataset = create_dataset(true, batch_size, dataset_type);
            CHECK_THROWS_AS(dataset.add_component_info(A::name, 1, batch_size + 1), DatasetError);
        }
    }
}

} // namespace power_grid_model::meta_data
