// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_gen/gen_getters.hpp>
#include <power_grid_model/common/component_list.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::meta_data {

namespace {
struct AInput {};
struct AUpdate {};
template <symmetry_tag symmetry> struct AOutput {
    using sym = symmetry;
};
struct AScOutput {};
struct BInput {};
struct BUpdate {};
template <symmetry_tag symmetry> struct BOutput {
    using sym = symmetry;
};
struct BScOutput {};
} // namespace

template <> struct get_attributes_list<AInput> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<AUpdate> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<AOutput<symmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<AOutput<asymmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<AScOutput> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<BInput> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<BUpdate> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<BOutput<symmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<BOutput<asymmetric_t>> {
    static constexpr std::array<MetaAttribute, 0> value;
};
template <> struct get_attributes_list<BScOutput> {
    static constexpr std::array<MetaAttribute, 0> value;
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

constexpr MetaData test_meta_data =
    meta_data_gen::get_meta_data<ComponentList<A, B>, // all components list
                                 meta_data_gen::dataset_mark<[] { return "input"; }, input_getter_s>,
                                 meta_data_gen::dataset_mark<[] { return "update"; }, update_getter_s>
                                 // end list of all marks
                                 >::value;

constexpr MetaData test_meta_data_all =
    meta_data_gen::get_meta_data<ComponentList<A, B>, // all components list
                                 meta_data_gen::dataset_mark<[] { return "input"; }, input_getter_s>,
                                 meta_data_gen::dataset_mark<[] { return "update"; }, update_getter_s>,
                                 meta_data_gen::dataset_mark<[] { return "sym_output"; }, sym_output_getter_s>,
                                 meta_data_gen::dataset_mark<[] { return "asym_output"; }, asym_output_getter_s>,
                                 meta_data_gen::dataset_mark<[] { return "sc_output"; }, sc_output_getter_s>
                                 // end list of all marks
                                 >::value;
} // namespace

namespace test {
namespace {
template <typename DatasetType>
DatasetType create_dataset(bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
    auto const dataset = DatasetType{is_batch, batch_size, dataset_type.name, test_meta_data_all};
    CHECK(&dataset.meta_data() == static_cast<MetaData const* const>(&test_meta_data_all));
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
} // namespace
} // namespace test

TEST_CASE_TEMPLATE("Test dataset (common)", DatasetType, ConstDataset, MutableDataset, WritableDataset) {
    constexpr auto create_dataset = [](bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
        return test::create_dataset<DatasetType>(is_batch, batch_size, dataset_type);
    };

    SUBCASE("Constructor") {
        SUBCASE("Single dataset") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(dataset_type.name);
                create_dataset(false, 1, dataset_type);
            }
        }
        SUBCASE("Batch dataset") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(dataset_type.name);
                for (auto batch_size : {0, 1, 2, -1}) {
                    CAPTURE(batch_size);
                    create_dataset(true, batch_size, dataset_type);
                }
            }
        }
        SUBCASE("Unknown dataset name") {
            constexpr auto construct = [] { return DatasetType{false, 1, "sym_output", test_meta_data}; };
            CHECK_THROWS_AS(construct(), std::out_of_range);
        }
        SUBCASE("Single dataset with wrong batch size") {
            for (auto const& dataset_type : test_meta_data_all.datasets) {
                CAPTURE(dataset_type.name);
                auto const construct = [&dataset_type] {
                    return DatasetType{false, 0, dataset_type.name, test_meta_data_all};
                };
                CHECK_THROWS_AS(construct(), DatasetError);
            }
        }
    }

    SUBCASE("Component info") {
        auto const& dataset_type = test_meta_data_all.datasets.front();
        CAPTURE(dataset_type.name);

        for (auto const batch_size : {-1, 0, 1, 2}) {
            CAPTURE(batch_size);
            auto dataset = create_dataset(true, batch_size, dataset_type);

            SUBCASE("No component added") {
                CHECK(dataset.n_components() == 0);
                CHECK_FALSE(dataset.contains_component(A::name));
                CHECK(dataset.get_description().component_info.empty());
                CHECK_THROWS_AS(dataset.get_component_info(A::name), DatasetError);
            }
        }
    }
}

TEST_CASE("Test writable dataset") {
    constexpr auto create_dataset = [](bool const is_batch, Idx const batch_size, MetaDataset const& dataset_type) {
        return test::create_dataset<WritableDataset>(is_batch, batch_size, dataset_type);
    };
    auto const& dataset_type = test_meta_data_all.datasets.front();
    CAPTURE(dataset_type.name);

    for (auto const batch_size : {-1, 0, 1, 2}) {
        CAPTURE(batch_size);

        SUBCASE("Add homogeneous component info") {
            for (auto const elements_per_scenario : {-1, 0, 1, 2}) {
                CAPTURE(elements_per_scenario);
                auto const total_elements = elements_per_scenario * batch_size;

                auto dataset = create_dataset(true, batch_size, dataset_type);
                dataset.add_component_info(A::name, elements_per_scenario, total_elements);
                CHECK(dataset.n_components() == 1);
                CHECK(dataset.contains_component(A::name));

                auto const& component_info = dataset.get_component_info(A::name);
                CHECK(component_info.component == &dataset_type.get_component(A::name));
                CHECK(component_info.elements_per_scenario == elements_per_scenario);
                CHECK(component_info.total_elements == total_elements);

                auto const& info = dataset.get_description();
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

                auto const& info = dataset.get_description();
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
