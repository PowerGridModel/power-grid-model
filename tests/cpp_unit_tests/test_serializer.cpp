// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

#include <doctest/doctest.h>

#include <limits>

namespace power_grid_model::meta_data {

namespace {
constexpr std::string_view single_dataset_dict =
    R"({"version":"1.0","type":"update","is_batch":false,"attributes":{},"data":{"sym_load":[{"id":9,"p_specified":10},{"id":10},{"id":11,"p_specified":"inf"},{"id":12,"p_specified":"-inf"}],"asym_load":[{"id":5,"p_specified":[10,11,12]},{"id":6,"p_specified":[15,null,16]},{"id":13,"p_specified":["inf",11,17]},{"id":14,"p_specified":[10,"-inf",19]},{"id":7}]}})";
constexpr std::string_view single_dataset_list =
    R"({"version":"1.0","type":"update","is_batch":false,"attributes":{"sym_load":["id","p_specified"],"asym_load":["id","p_specified"]},"data":{"sym_load":[[9,10],[10,null],[11,"inf"],[12,"-inf"]],"asym_load":[[5,[10,11,12]],[6,[15,null,16]],[13,["inf",11,17]],[14,[10,"-inf",19]],[7,null]]}})";
constexpr std::string_view batch_dataset_dict =
    R"({"version":"1.0","type":"update","is_batch":true,"attributes":{},"data":[{"sym_load":[{"id":9,"p_specified":10},{"id":10}],"asym_load":[{"id":5,"p_specified":[10,11,12]},{"id":6,"p_specified":[15,null,16]}]},{"sym_load":[{"id":11,"p_specified":"inf"},{"id":12,"p_specified":"-inf"}],"asym_load":[{"id":13,"p_specified":["inf",11,17]},{"id":14,"p_specified":[10,"-inf",19]}],"asym_gen":[{"id":7}]}]})";
constexpr std::string_view batch_dataset_list =
    R"({"version":"1.0","type":"update","is_batch":true,"attributes":{"asym_gen":["id"],"sym_load":["id","p_specified"],"asym_load":["id","p_specified"]},"data":[{"sym_load":[[9,10],[10,null]],"asym_load":[[5,[10,11,12]],[6,[15,null,16]]]},{"sym_load":[[11,"inf"],[12,"-inf"]],"asym_load":[[13,["inf",11,17]],[14,[10,"-inf",19]]],"asym_gen":[[7]]}]})";

constexpr std::string_view single_dataset_dict_indent =
    R"({
  "version": "1.0",
  "type": "update",
  "is_batch": false,
  "attributes": {},
  "data": {
    "sym_load": [
      {"id": 9, "p_specified": 10},
      {"id": 10},
      {"id": 11, "p_specified": "inf"},
      {"id": 12, "p_specified": "-inf"}
    ],
    "asym_load": [
      {"id": 5, "p_specified": [10, 11, 12]},
      {"id": 6, "p_specified": [15, null, 16]},
      {"id": 13, "p_specified": ["inf", 11, 17]},
      {"id": 14, "p_specified": [10, "-inf", 19]},
      {"id": 7}
    ]
  }
})";

constexpr std::string_view single_dataset_list_indent =
    R"({
  "version": "1.0",
  "type": "update",
  "is_batch": false,
  "attributes": {
    "sym_load": [
      "id",
      "p_specified"
    ],
    "asym_load": [
      "id",
      "p_specified"
    ]
  },
  "data": {
    "sym_load": [
      [9, 10],
      [10, null],
      [11, "inf"],
      [12, "-inf"]
    ],
    "asym_load": [
      [5, [10, 11, 12]],
      [6, [15, null, 16]],
      [13, ["inf", 11, 17]],
      [14, [10, "-inf", 19]],
      [7, null]
    ]
  }
})";

constexpr std::string_view batch_dataset_list_indent =
    R"({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {
    "asym_gen": [
      "id"
    ],
    "sym_load": [
      "id",
      "p_specified"
    ],
    "asym_load": [
      "id",
      "p_specified"
    ]
  },
  "data": [
    {
      "sym_load": [
        [9, 10],
        [10, null]
      ],
      "asym_load": [
        [5, [10, 11, 12]],
        [6, [15, null, 16]]
      ]
    },
    {
      "sym_load": [
        [11, "inf"],
        [12, "-inf"]
      ],
      "asym_load": [
        [13, ["inf", 11, 17]],
        [14, [10, "-inf", 19]]
      ],
      "asym_gen": [
        [7]
      ]
    }
  ]
})";

constexpr std::string_view batch_dataset_dict_indent =
    R"({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "sym_load": [
        {"id": 9, "p_specified": 10},
        {"id": 10}
      ],
      "asym_load": [
        {"id": 5, "p_specified": [10, 11, 12]},
        {"id": 6, "p_specified": [15, null, 16]}
      ]
    },
    {
      "sym_load": [
        {"id": 11, "p_specified": "inf"},
        {"id": 12, "p_specified": "-inf"}
      ],
      "asym_load": [
        {"id": 13, "p_specified": ["inf", 11, 17]},
        {"id": 14, "p_specified": [10, "-inf", 19]}
      ],
      "asym_gen": [
        {"id": 7}
      ]
    }
  ]
})";

} // namespace

TEST_CASE("Serializer") {
    std::vector<SymLoadGenUpdate> sym_load_gen(4);
    meta_data_gen::meta_data.get_dataset("update").get_component("sym_load").set_nan(sym_load_gen.data(), 0, 4);
    sym_load_gen[0].id = 9;
    sym_load_gen[1].id = 10;
    sym_load_gen[2].id = 11;
    sym_load_gen[3].id = 12;
    sym_load_gen[0].p_specified = 10.0;
    sym_load_gen[1].p_specified = nan;
    sym_load_gen[2].p_specified = std::numeric_limits<double>::infinity();
    sym_load_gen[3].p_specified = -std::numeric_limits<double>::infinity();

    std::vector<ID> sym_load_gen_id(sym_load_gen.size());
    RealValueVector<symmetric_t> sym_load_gen_p_specified(sym_load_gen.size());

    std::ranges::transform(sym_load_gen, sym_load_gen_id.begin(), [](auto const& value) { return value.id; });
    std::ranges::transform(sym_load_gen, sym_load_gen_p_specified.begin(),
                           [](auto const& value) { return value.p_specified; });

    std::vector<AsymLoadGenUpdate> asym_load_gen(5);
    meta_data_gen::meta_data.get_dataset("update").get_component("asym_load").set_nan(asym_load_gen.data(), 0, 5);
    asym_load_gen[0].id = 5;
    asym_load_gen[1].id = 6;
    asym_load_gen[2].id = 13;
    asym_load_gen[3].id = 14;
    asym_load_gen[4].id = 7;
    asym_load_gen[0].p_specified = {10.0, 11.0, 12.0};
    asym_load_gen[1].p_specified = {15.0, nan, 16.0};
    asym_load_gen[2].p_specified = {std::numeric_limits<double>::infinity(), 11.0, 17.0};
    asym_load_gen[3].p_specified = {10.0, -std::numeric_limits<double>::infinity(), 19.0};
    // nan for asym_load_gen[4].p_specified

    std::vector<ID> asym_load_gen_id(asym_load_gen.size());
    RealValueVector<asymmetric_t> asym_load_gen_p_specified(asym_load_gen.size());

    std::ranges::transform(asym_load_gen, asym_load_gen_id.begin(), [](auto const& value) { return value.id; });
    std::ranges::transform(asym_load_gen, asym_load_gen_p_specified.begin(),
                           [](auto const& value) { return value.p_specified; });

    SUBCASE("Single row-based dataset") {
        ConstDataset handler{false, 1, "update", meta_data_gen::meta_data};
        handler.add_buffer("sym_load", 4, 4, nullptr, sym_load_gen.data());
        handler.add_buffer("asym_load", 5, 5, nullptr, asym_load_gen.data());

        Serializer serializer{handler, SerializationFormat::json};

        CHECK(serializer.get_string(false, -1) == single_dataset_dict);
        CHECK(serializer.get_string(true, -1) == single_dataset_list);
        CHECK(serializer.get_string(false, 2) == single_dataset_dict_indent);
        CHECK(serializer.get_string(true, 2) == single_dataset_list_indent);
    }

    SUBCASE("Single columnar dataset") {
        ConstDataset handler{false, 1, "update", meta_data_gen::meta_data};
        handler.add_buffer("sym_load", 4, 4, nullptr, nullptr);
        handler.add_attribute_buffer("sym_load", "id", sym_load_gen_id.data());
        handler.add_attribute_buffer("sym_load", "p_specified", sym_load_gen_p_specified.data());
        handler.add_buffer("asym_load", 5, 5, nullptr, nullptr);
        handler.add_attribute_buffer("asym_load", "id", asym_load_gen_id.data());
        handler.add_attribute_buffer("asym_load", "p_specified", asym_load_gen_p_specified.data());

        Serializer serializer{handler, SerializationFormat::json};

        CHECK(serializer.get_string(false, -1) == single_dataset_dict);
        CHECK(serializer.get_string(true, -1) == single_dataset_list);
        CHECK(serializer.get_string(false, 2) == single_dataset_dict_indent);
        CHECK(serializer.get_string(true, 2) == single_dataset_list_indent);
    }

    SUBCASE("Batch row-based dataset") {
        ConstDataset handler{true, 2, "update", meta_data_gen::meta_data};
        std::array<Idx, 3> const indptr_gen{0, 0, 1};
        handler.add_buffer("sym_load", 2, 4, nullptr, sym_load_gen.data());
        handler.add_buffer("asym_load", 2, 4, nullptr, asym_load_gen.data());
        handler.add_buffer("asym_gen", -1, 1, indptr_gen.data(), asym_load_gen.data() + 4);

        Serializer serializer{handler, SerializationFormat::json};

        CHECK(serializer.get_string(false, -1) == batch_dataset_dict);
        CHECK(serializer.get_string(true, -1) == batch_dataset_list);
        CHECK(serializer.get_string(false, 2) == batch_dataset_dict_indent);
        CHECK(serializer.get_string(true, 2) == batch_dataset_list_indent);
    }

    SUBCASE("Batch columnar dataset") {
        ConstDataset handler{true, 2, "update", meta_data_gen::meta_data};
        std::array<Idx, 3> const indptr_gen{0, 0, 1};
        handler.add_buffer("sym_load", 2, 4, nullptr, nullptr);
        handler.add_attribute_buffer("sym_load", "id", sym_load_gen_id.data());
        handler.add_attribute_buffer("sym_load", "p_specified", sym_load_gen_p_specified.data());
        handler.add_buffer("asym_load", 2, 4, nullptr, nullptr);
        handler.add_attribute_buffer("asym_load", "id", asym_load_gen_id.data());
        handler.add_attribute_buffer("asym_load", "p_specified", asym_load_gen_p_specified.data());
        handler.add_buffer("asym_gen", -1, 1, indptr_gen.data(), nullptr);
        handler.add_attribute_buffer("asym_gen", "id", asym_load_gen_id.data() + 4);
        handler.add_attribute_buffer("asym_gen", "p_specified", asym_load_gen_p_specified.data() + 4);

        Serializer serializer{handler, SerializationFormat::json};

        CHECK(serializer.get_string(false, -1) == batch_dataset_dict);
        CHECK(serializer.get_string(true, -1) == batch_dataset_list);
        CHECK(serializer.get_string(false, 2) == batch_dataset_dict_indent);
        CHECK(serializer.get_string(true, 2) == batch_dataset_list_indent);
    }
}

} // namespace power_grid_model::meta_data
