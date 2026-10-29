// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/update.hpp>

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_gen/gen_getters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/component_list.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <vector>

namespace power_grid_model {

namespace {
// a minimal custom update type that only consists of an id, which also makes the buffers easy to fill from ID vectors
struct TestUpdate {
    ID id{na_IntID};
};
static_assert(sizeof(TestUpdate) == sizeof(ID));

struct TestComponent {
    using UpdateType = TestUpdate;

    static constexpr auto name = "test_component";
};
} // namespace

namespace meta_data {
template <> struct get_attributes_list<TestUpdate> {
    static constexpr std::array<MetaAttribute, 1> value{
        meta_data_gen::get_meta_attribute<&TestUpdate::id>(offsetof(TestUpdate, id), "id"),
    };
};
} // namespace meta_data

namespace main_core::update {

namespace {
constexpr meta_data::MetaData test_meta_data =
    meta_data::meta_data_gen::get_meta_data<ComponentList<TestComponent>, meta_data::update_getter_s>::value;
} // namespace

TEST_CASE_TEMPLATE("Test update independence", Columnar, std::false_type, std::true_type) {
    constexpr bool columnar = Columnar::value;
    CAPTURE(columnar);

    SUBCASE("Update independence checks") {
        auto const check_independence = [](std::vector<ID> ids, bool with_id_attribute = true) {
            constexpr Idx elements_per_scenario = 2;
            Idx const total_elements = static_cast<Idx>(ids.size());
            Idx const batch_size = total_elements / elements_per_scenario;

            ConstDataset dataset{true, batch_size, "update", test_meta_data};
            if (columnar) {
                dataset.add_buffer(TestComponent::name, elements_per_scenario, total_elements, nullptr, nullptr);
                if (with_id_attribute) {
                    dataset.add_attribute_buffer(TestComponent::name, "id", ids.data());
                }
            } else {
                dataset.add_buffer(TestComponent::name, elements_per_scenario, total_elements, nullptr, ids.data());
            }
            return independence::check_component_independence<TestComponent>(dataset, elements_per_scenario);
        };

        SUBCASE("Matching ids") {
            auto const properties = check_independence({1, 2, 1, 2});
            CHECK(properties.is_columnar == columnar);
            CHECK_FALSE(properties.ids_all_na);
            CHECK_FALSE(properties.ids_part_na);
            CHECK(properties.update_ids_match);
            CHECK(properties.is_independent());
        }
        SUBCASE("Mismatching ids between scenarios") {
            auto const properties = check_independence({1, 2, 1, 3});
            CHECK_FALSE(properties.ids_all_na);
            CHECK_FALSE(properties.ids_part_na);
            CHECK_FALSE(properties.update_ids_match);
            CHECK_FALSE(properties.is_independent());
        }
        SUBCASE("All ids NA") {
            auto const properties = check_independence({na_IntID, na_IntID, na_IntID, na_IntID});
            CHECK(properties.ids_all_na);
            CHECK_FALSE(properties.ids_part_na);
            CHECK(properties.update_ids_match);
            CHECK(properties.is_independent());
        }
        SUBCASE("Some ids NA") {
            auto const properties = check_independence({1, na_IntID, 1, na_IntID});
            CHECK_FALSE(properties.ids_all_na);
            CHECK(properties.ids_part_na);
            CHECK_FALSE(properties.is_independent());
        }
        SUBCASE("Some scenarios NA, others not") {
            auto const properties = check_independence({na_IntID, na_IntID, 1, 2});
            CHECK_FALSE(properties.ids_all_na);
            CHECK(properties.ids_part_na);
            CHECK_FALSE(properties.is_independent());
        }

        if constexpr (columnar) {
            SUBCASE("Missing id attribute buffer is equivalent to all ids NA") {
                auto const properties = check_independence({0, 0, 0, 0}, false);
                CHECK(properties.ids_all_na);
                CHECK(properties.qualify_for_optional_id());
            }
        }
    }

    SUBCASE("Empty scenarios do not count as missing ids") {
        std::vector<ID> ids{1, 2};
        std::vector<Idx> indptr{0, 0, 2};
        ConstDataset dataset{true, 2, "update", test_meta_data};
        if (columnar) {
            dataset.add_buffer(TestComponent::name, na_Idx, 2, indptr.data(), nullptr);
            dataset.add_attribute_buffer(TestComponent::name, "id", ids.data());
        } else {
            dataset.add_buffer(TestComponent::name, na_Idx, 2, indptr.data(), ids.data());
        }
        auto const properties = independence::check_component_independence<TestComponent>(dataset, std::ssize(ids));
        CHECK_FALSE(properties.ids_all_na);
        CHECK_FALSE(properties.ids_part_na);
    }
}

} // namespace main_core::update

} // namespace power_grid_model
