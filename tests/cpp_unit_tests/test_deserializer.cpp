// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Issue in msgpack, reported in https://github.com/msgpack/msgpack-c/issues/1098
// May be a Clang Analyzer bug
#ifndef __clang_analyzer__ // TODO(mgovers): re-enable this when issue in msgpack is fixed

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/update.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::meta_data {

using namespace std::string_literals;

// single data
namespace {
constexpr std::string_view json_single = R"(
{
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {
    "node": [
      "id",
      "u_rated"
    ],
    "sym_load": [
      "id",
      "node",
      "status",
      "type",
      "p_specified",
      "q_specified"
    ],
    "source": [
      "id",
      "node",
      "status",
      "u_ref",
      "sk"
    ]
  },
  "data": {
    "node": [
      [
        1,
        10.5e3
      ],
      [
        2,
        10.5e3
      ],
      [
        3,
        10.5e3
      ]
    ],
    "line": [
      {
        "id": 4,
        "from_node": 1,
        "to_node": 2,
        "from_status": 1,
        "to_status": 1,
        "r1": 0.11,
        "x1": 0.12,
        "c1": 4e-05,
        "tan1": 0.1,
        "i_n": 500.0
      },
      {
        "id": 5,
        "from_node": 2,
        "to_node": 3,
        "from_status": 1,
        "to_status": 1,
        "r1": 0.15,
        "x1": 0.16,
        "c1": 5e-05,
        "tan1": 0.12,
        "i_n": 550.0
      }
    ],
    "source": [
      [
        15,
        1,
        1,
        1.03,
        1e20
      ],
      [
        16,
        1,
        1,
        1.04,
        null
      ],
      {
        "id": 17,
        "node": 1,
        "status": 1,
        "u_ref": 1.03,
        "sk": 1e10,
        "rx_ratio": 0.2
      }
    ],
    "sym_load": [
      [
        7,
        2,
        1,
        0,
        1.01e6,
        0.21e6
      ],
      [
        8,
        3,
        1,
        0,
        1.02e6,
        0.22e6
      ],
      [
        36,
        3,
        1,
        0,
        "inf",
        0.22e6
      ],
      [
        37,
        3,
        1,
        0,
        "-inf",
        "+inf"
      ]
    ],
    "asym_load": [
      {
        "id": 100
      }
    ]
  }
}
)";

} // namespace

// batch data
namespace {
constexpr std::string_view json_batch = R"(
{
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {
    "sym_load": [
      "id",
      "p_specified",
      "q_specified"
    ],
    "asym_load": [
      "id",
      "p_specified"
    ]
  },
  "data": [
    {
      "sym_load": [
        [
          7,
          20.0,
          50.0
        ]
      ],
      "asym_load": [
        [
          9,
          [
            100.0,
            null,
            200.0
          ]
        ]
      ]
    },
    {
      "asym_load": [
        [
          9,
          null
        ]
      ]
    },
    {
      "sym_load": [
        [
          7,
          null,
          10.0
        ],
        {
          "id": 8,
          "status": 0
        }
      ],
      "asym_load": [
        {
          "id": 9,
          "q_specified": [
            70.0,
            80.0,
            90.0
          ]
        }
      ]
    },
    {
      "sym_load": [
        [
          37,
          "-inf",
          "inf"
        ]
      ],
      "asym_load": [
        {
          "id": 31,
          "q_specified": [
            "inf",
            80.0,
            "+inf"
          ],
          "p_specified": [
            "-inf",
            75.0,
            "-inf"
          ]
        }
      ]
    }
  ]
}
)";

void check_error(std::string_view json, char const* err_msg) {
    std::vector<NodeInput> node(1);

    auto const run = [&]() {
        Deserializer deserializer{from_json, json, meta_data_gen::meta_data};
        deserializer.get_dataset_info().set_buffer("node", nullptr, node.data());
        deserializer.parse();
    };

    CHECK_THROWS_WITH_AS(run(), doctest::Contains(err_msg), std::exception);
}

} // namespace

TEST_CASE("Deserializer") {
    SUBCASE("Single dataset") {
        Deserializer deserializer{from_json, json_single, meta_data_gen::meta_data};

        SUBCASE("Check meta data") {
            CHECK(deserializer.get_dataset_info().dataset().name == "input"s);
            CHECK(!deserializer.get_dataset_info().is_batch());
            CHECK(deserializer.get_dataset_info().batch_size() == 1);
            CHECK(deserializer.get_dataset_info().n_components() == 5);
        }

        SUBCASE("Check buffer") {
            auto const& info = deserializer.get_dataset_info();

            auto const& node_info = info.get_component_info("node");
            CHECK(node_info.elements_per_scenario == 3);
            CHECK(node_info.total_elements == 3);
            CHECK(node_info.has_attribute_indications == true);
            REQUIRE(node_info.attribute_indications.size() == 2);
            CHECK(node_info.attribute_indications[0]->name == "id"s);
            CHECK(node_info.attribute_indications[1]->name == "u_rated"s);

            auto const& line_info = info.get_component_info("line");
            CHECK(line_info.elements_per_scenario == 2);
            CHECK(line_info.total_elements == 2);
            CHECK(line_info.has_attribute_indications == false);
            CHECK(line_info.attribute_indications.empty());

            auto const& source_info = info.get_component_info("source");
            CHECK(source_info.elements_per_scenario == 3);
            CHECK(source_info.total_elements == 3);
            CHECK(source_info.has_attribute_indications == false);
            CHECK(source_info.attribute_indications.empty());

            auto const& sym_load_info = info.get_component_info("sym_load");
            CHECK(sym_load_info.elements_per_scenario == 4);
            CHECK(sym_load_info.total_elements == 4);
            CHECK(sym_load_info.has_attribute_indications == true);
            REQUIRE(sym_load_info.attribute_indications.size() == 6);
            CHECK(sym_load_info.attribute_indications[0]->name == "id"s);
            CHECK(sym_load_info.attribute_indications[1]->name == "node"s);
            CHECK(sym_load_info.attribute_indications[2]->name == "status"s);
            CHECK(sym_load_info.attribute_indications[3]->name == "type"s);
            CHECK(sym_load_info.attribute_indications[4]->name == "p_specified"s);
            CHECK(sym_load_info.attribute_indications[5]->name == "q_specified"s);
        }

        SUBCASE("Check parse row-based") {
            std::vector<NodeInput> node(3);
            std::vector<LineInput> line(2);
            std::vector<SourceInput> source(3);
            std::vector<SymLoadGenInput> sym_load(4);
            auto& info = deserializer.get_dataset_info();
            info.set_buffer("node", nullptr, node.data());
            info.set_buffer("line", nullptr, line.data());
            info.set_buffer("source", nullptr, source.data());
            info.set_buffer("sym_load", nullptr, sym_load.data());

            deserializer.parse();
            // check node
            CHECK(node[0].id == 1);
            CHECK(node[0].u_rated == doctest::Approx(10.5e3));
            CHECK(node[1].id == 2);
            CHECK(node[1].u_rated == doctest::Approx(10.5e3));
            CHECK(node[2].id == 3);
            CHECK(node[2].u_rated == doctest::Approx(10.5e3));
            // check line
            CHECK(line[0].id == 4);
            CHECK(line[0].r1 == doctest::Approx(0.11));
            CHECK(is_nan(line[0].r0));
            CHECK(line[1].id == 5);
            CHECK(line[1].x1 == doctest::Approx(0.16));
            CHECK(is_nan(line[1].x0));
            // check source
            CHECK(source[0].id == 15);
            CHECK(source[0].u_ref == doctest::Approx(1.03));
            CHECK(source[0].sk == doctest::Approx(1e20));
            CHECK(is_nan(source[0].rx_ratio));
            CHECK(source[1].id == 16);
            CHECK(source[1].u_ref == doctest::Approx(1.04));
            CHECK(is_nan(source[1].sk));
            CHECK(is_nan(source[1].rx_ratio));
            CHECK(source[2].id == 17);
            CHECK(source[2].u_ref == doctest::Approx(1.03));
            CHECK(source[2].sk == doctest::Approx(1e10));
            CHECK(source[2].rx_ratio == doctest::Approx(0.2));
            // check sym_load
            CHECK(sym_load[0].id == 7);
            CHECK(sym_load[0].p_specified == doctest::Approx(1.01e6));
            CHECK(sym_load[1].id == 8);
            CHECK(sym_load[1].q_specified == doctest::Approx(0.22e6));
            CHECK(sym_load[2].id == 36);
            CHECK(sym_load[2].p_specified == std::numeric_limits<double>::infinity());
            CHECK(sym_load[3].id == 37);
            CHECK(sym_load[3].p_specified == -std::numeric_limits<double>::infinity());
            CHECK(sym_load[3].q_specified == std::numeric_limits<double>::infinity());
        }

        SUBCASE("Check parse columnar") {
            std::vector<ID> node_id(3);
            std::vector<double> node_u_rated(3);
            std::vector<ID> line_id(2);
            std::vector<double> line_r1(2);
            std::vector<double> line_r0(2);
            std::vector<double> line_x1(2);
            std::vector<double> line_x0(2);
            std::vector<ID> source_id(3);
            std::vector<double> source_u_ref(3);
            std::vector<double> source_sk(3);
            std::vector<double> source_rx_ratio(3);
            std::vector<ID> sym_load_id(4);
            std::vector<RealValue<symmetric_t>> sym_load_p_specified(4);
            std::vector<RealValue<symmetric_t>> sym_load_q_specified(4);

            auto& info = deserializer.get_dataset_info();
            info.set_buffer("node", nullptr, nullptr);
            info.set_attribute_buffer("node", "id", node_id.data());
            info.set_attribute_buffer("node", "u_rated", node_u_rated.data());
            info.set_buffer("line", nullptr, nullptr);
            info.set_attribute_buffer("line", "id", line_id.data());
            info.set_attribute_buffer("line", "r1", line_r1.data());
            info.set_attribute_buffer("line", "r0", line_r0.data());
            info.set_attribute_buffer("line", "x1", line_x1.data());
            info.set_attribute_buffer("line", "x0", line_x0.data());
            info.set_buffer("source", nullptr, nullptr);
            info.set_attribute_buffer("source", "id", source_id.data());
            info.set_attribute_buffer("source", "u_ref", source_u_ref.data());
            info.set_attribute_buffer("source", "sk", source_sk.data());
            info.set_attribute_buffer("source", "rx_ratio", source_rx_ratio.data());
            info.set_buffer("sym_load", nullptr, nullptr);
            info.set_attribute_buffer("sym_load", "id", sym_load_id.data());
            info.set_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
            info.set_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());

            deserializer.parse();
            // check node
            CHECK(node_id[0] == 1);
            CHECK(node_u_rated[0] == doctest::Approx(10.5e3));
            CHECK(node_id[1] == 2);
            CHECK(node_u_rated[1] == doctest::Approx(10.5e3));
            CHECK(node_id[2] == 3);
            CHECK(node_u_rated[2] == doctest::Approx(10.5e3));
            // check line
            CHECK(line_id[0] == 4);
            CHECK(line_r1[0] == doctest::Approx(0.11));
            CHECK(is_nan(line_r0[0]));
            CHECK(line_id[1] == 5);
            CHECK(line_x1[1] == doctest::Approx(0.16));
            CHECK(is_nan(line_x0[1]));
            // check source
            CHECK(source_id[0] == 15);
            CHECK(source_u_ref[0] == doctest::Approx(1.03));
            CHECK(source_sk[0] == doctest::Approx(1e20));
            CHECK(is_nan(source_rx_ratio[0]));
            CHECK(source_id[1] == 16);
            CHECK(source_u_ref[1] == doctest::Approx(1.04));
            CHECK(is_nan(source_sk[1]));
            CHECK(is_nan(source_rx_ratio[1]));
            CHECK(source_id[2] == 17);
            CHECK(source_u_ref[2] == doctest::Approx(1.03));
            CHECK(source_sk[2] == doctest::Approx(1e10));
            CHECK(source_rx_ratio[2] == doctest::Approx(0.2));
            // check sym_load
            CHECK(sym_load_id[0] == 7);
            CHECK(sym_load_p_specified[0] == doctest::Approx(1.01e6));
            CHECK(sym_load_id[1] == 8);
            CHECK(sym_load_q_specified[1] == doctest::Approx(0.22e6));
            CHECK(sym_load_id[2] == 36);
            CHECK(sym_load_p_specified[2] == std::numeric_limits<double>::infinity());
            CHECK(sym_load_id[3] == 37);
            CHECK(sym_load_p_specified[3] == -std::numeric_limits<double>::infinity());
            CHECK(sym_load_q_specified[3] == std::numeric_limits<double>::infinity());
        }
    }

    SUBCASE("Batch dataset") {
        Deserializer deserializer{from_json, json_batch, meta_data_gen::meta_data};

        SUBCASE("Check meta data") {
            CHECK(deserializer.get_dataset_info().dataset().name == "update"s);
            CHECK(deserializer.get_dataset_info().is_batch());
            CHECK(deserializer.get_dataset_info().batch_size() == 4);
            CHECK(deserializer.get_dataset_info().n_components() == 2);
        }

        SUBCASE("Check buffer") {
            auto const& info = deserializer.get_dataset_info();
            CHECK(info.get_component_info("sym_load").elements_per_scenario == -1);
            CHECK(info.get_component_info("sym_load").total_elements == 4);
            CHECK(info.get_component_info("sym_load").has_attribute_indications == false);
            CHECK(info.get_component_info("sym_load").attribute_indications.empty());
            CHECK(info.get_component_info("asym_load").elements_per_scenario == 1);
            CHECK(info.get_component_info("asym_load").total_elements == 4);
            CHECK(info.get_component_info("asym_load").has_attribute_indications == false);
            CHECK(info.get_component_info("asym_load").attribute_indications.empty());
        }

        SUBCASE("Check parse row-based") {
            std::vector<SymLoadGenUpdate> sym_load(4);
            std::vector<AsymLoadGenUpdate> asym_load(4);
            IdxVector sym_load_indptr(deserializer.get_dataset_info().batch_size() + 1);
            auto& info = deserializer.get_dataset_info();
            info.set_buffer("sym_load", sym_load_indptr.data(), sym_load.data());
            info.set_buffer("asym_load", nullptr, asym_load.data());

            deserializer.parse();

            // sym_load
            CHECK(sym_load_indptr == IdxVector{0, 1, 1, 3, 4});
            CHECK(sym_load[0].id == 7);
            CHECK(sym_load[0].p_specified == doctest::Approx(20.0));
            CHECK(sym_load[0].status == na_IntS);
            CHECK(sym_load[1].id == 7);
            CHECK(is_nan(sym_load[1].p_specified));
            CHECK(sym_load[1].q_specified == doctest::Approx(10.0));
            CHECK(sym_load[1].status == na_IntS);
            CHECK(sym_load[2].id == 8);
            CHECK(is_nan(sym_load[2].p_specified));
            CHECK(is_nan(sym_load[2].q_specified));
            CHECK(sym_load[2].status == 0);
            CHECK(sym_load[3].id == 37);
            CHECK(sym_load[3].p_specified == -std::numeric_limits<double>::infinity());
            CHECK(sym_load[3].q_specified == std::numeric_limits<double>::infinity());

            // asym_load
            CHECK(asym_load[0].id == 9);
            CHECK(asym_load[0].p_specified(0) == doctest::Approx(100.0));
            CHECK(is_nan(asym_load[0].p_specified(1)));
            CHECK(asym_load[0].p_specified(2) == doctest::Approx(200));
            CHECK(is_nan(asym_load[0].q_specified));
            CHECK(asym_load[1].id == 9);
            CHECK(is_nan(asym_load[1].p_specified));
            CHECK(is_nan(asym_load[1].q_specified));
            CHECK(asym_load[2].id == 9);
            CHECK(is_nan(asym_load[2].p_specified));
            CHECK(asym_load[2].q_specified(0) == doctest::Approx(70.0));
            CHECK(asym_load[2].q_specified(1) == doctest::Approx(80.0));
            CHECK(asym_load[2].q_specified(2) == doctest::Approx(90.0));
            CHECK(asym_load[3].id == 31);
            CHECK(asym_load[3].p_specified(0) == -std::numeric_limits<double>::infinity());
            CHECK(asym_load[3].p_specified(1) == doctest::Approx(75.0));
            CHECK(asym_load[3].p_specified(2) == -std::numeric_limits<double>::infinity());
            CHECK(asym_load[3].q_specified(0) == std::numeric_limits<double>::infinity());
            CHECK(asym_load[3].q_specified(1) == doctest::Approx(80.0));
            CHECK(asym_load[3].q_specified(2) == std::numeric_limits<double>::infinity());
        }

        SUBCASE("Check parse columnar") {
            std::vector<ID> sym_load_id(4);
            std::vector<IntS> sym_load_status(4);
            std::vector<RealValue<symmetric_t>> sym_load_p_specified(4);
            std::vector<RealValue<symmetric_t>> sym_load_q_specified(4);
            std::vector<ID> asym_load_id(4);
            std::vector<IntS> asym_load_status(4);
            std::vector<RealValue<asymmetric_t>> asym_load_p_specified(4);
            std::vector<RealValue<asymmetric_t>> asym_load_q_specified(4);
            IdxVector sym_load_indptr(deserializer.get_dataset_info().batch_size() + 1);

            auto& info = deserializer.get_dataset_info();
            info.set_buffer("sym_load", sym_load_indptr.data(), nullptr);
            info.set_attribute_buffer("sym_load", "id", sym_load_id.data());
            info.set_attribute_buffer("sym_load", "status", sym_load_status.data());
            info.set_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());
            info.set_attribute_buffer("sym_load", "q_specified", sym_load_q_specified.data());
            info.set_buffer("asym_load", nullptr, nullptr);
            info.set_attribute_buffer("asym_load", "id", asym_load_id.data());
            info.set_attribute_buffer("asym_load", "status", asym_load_status.data());
            info.set_attribute_buffer("asym_load", "p_specified", asym_load_p_specified.data());
            info.set_attribute_buffer("asym_load", "q_specified", asym_load_q_specified.data());

            deserializer.parse();

            // sym_load
            CHECK(sym_load_indptr == IdxVector{0, 1, 1, 3, 4});
            CHECK(sym_load_id[0] == 7);
            CHECK(sym_load_p_specified[0] == doctest::Approx(20.0));
            CHECK(sym_load_status[0] == na_IntS);
            CHECK(sym_load_id[1] == 7);
            CHECK(is_nan(sym_load_p_specified[1]));
            CHECK(sym_load_q_specified[1] == doctest::Approx(10.0));
            CHECK(sym_load_status[1] == na_IntS);
            CHECK(sym_load_id[2] == 8);
            CHECK(is_nan(sym_load_p_specified[2]));
            CHECK(is_nan(sym_load_q_specified[2]));
            CHECK(sym_load_status[2] == 0);
            CHECK(sym_load_id[3] == 37);
            CHECK(sym_load_p_specified[3] == -std::numeric_limits<double>::infinity());
            CHECK(sym_load_q_specified[3] == std::numeric_limits<double>::infinity());

            // asym_load
            CHECK(asym_load_id[0] == 9);
            CHECK(asym_load_p_specified[0](0) == doctest::Approx(100.0));
            CHECK(is_nan(asym_load_p_specified[0](1)));
            CHECK(asym_load_p_specified[0](2) == doctest::Approx(200));
            CHECK(is_nan(asym_load_q_specified[0]));
            CHECK(asym_load_id[1] == 9);
            CHECK(is_nan(asym_load_p_specified[1]));
            CHECK(is_nan(asym_load_q_specified[1]));
            CHECK(asym_load_id[2] == 9);
            CHECK(is_nan(asym_load_p_specified[2]));
            CHECK(asym_load_q_specified[2](0) == doctest::Approx(70.0));
            CHECK(asym_load_q_specified[2](1) == doctest::Approx(80.0));
            CHECK(asym_load_q_specified[2](2) == doctest::Approx(90.0));
            CHECK(asym_load_id[3] == 31);
            CHECK(asym_load_p_specified[3](0) == -std::numeric_limits<double>::infinity());
            CHECK(asym_load_p_specified[3](1) == doctest::Approx(75.0));
            CHECK(asym_load_p_specified[3](2) == -std::numeric_limits<double>::infinity());
            CHECK(asym_load_q_specified[3](0) == std::numeric_limits<double>::infinity());
            CHECK(asym_load_q_specified[3](1) == doctest::Approx(80.0));
            CHECK(asym_load_q_specified[3](2) == std::numeric_limits<double>::infinity());
        }
    }
}

TEST_CASE("Deserializer with error") {
    SUBCASE("Error in meta data") {
        constexpr std::string_view no_version = R"({})";
        check_error(no_version, "version");
        constexpr std::string_view wrong_dataset =
            R"({"version": "1.0", "attributes": {}, "type": "sym_input", "is_batch": false, "data": {}})";
        check_error(wrong_dataset, "sym_input");
        constexpr std::string_view wrong_is_batch = R"({"version": "1.0", "type": "input", "is_batch": 5})";
        check_error(wrong_is_batch, "is_batch");
    }

    SUBCASE("Error in attributes") {
        constexpr std::string_view unknown_component =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {"node1": []}, "data": {}})";
        check_error(unknown_component, "Position of error: attributes/node1");
        constexpr std::string_view unknown_attribute =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {"node": ["i_from"]}, "data": {}})";
        check_error(unknown_attribute, "Position of error: attributes/node/0");
    }

    SUBCASE("Error in single data") {
        constexpr std::string_view unknown_component =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {}, "data": {"node1": []}})";
        check_error(unknown_component, "Position of error: data/node1");
        constexpr std::string_view unequal_attributes =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {}, "data": {"node": [[5]]}})";
        check_error(unequal_attributes, "Position of error: data/node/0");
        constexpr std::string_view wrong_type_list =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {"node": ["id"]}, "data": {"node":
[[true]]}})";
        check_error(wrong_type_list, "Position of error: data/node/0/0");
        constexpr std::string_view wrong_type_dict =
            R"({"version": "1.0", "type": "input", "is_batch": false, "attributes": {}, "data": {"node": [{"id":
true}]}})";
        check_error(wrong_type_dict, "Position of error: data/node/0/id");
    }

    SUBCASE("Error in batch data") {
        constexpr std::string_view unknown_component =
            R"({"version": "1.0", "type": "input", "is_batch": true, "attributes": {}, "data": [{"node1": []}]})";
        check_error(unknown_component, "Position of error: data/0/node1");
        constexpr std::string_view unequal_attributes =
            R"({"version": "1.0", "type": "input", "is_batch": true, "attributes": {}, "data": [{"node": [[5]]}]})";
        check_error(unequal_attributes, "Position of error: data/0/node/0");
        constexpr std::string_view wrong_type_list =
            R"({"version": "1.0", "type": "input", "is_batch": true, "attributes": {"node": ["id"]}, "data": [{"node":
[[true]]}]})";
        check_error(wrong_type_list, "Position of error: data/0/node/0/0");
        constexpr std::string_view wrong_type_dict =
            R"({"version": "1.0", "type": "input", "is_batch": true, "attributes": {}, "data": [{"node": [{"id":
true}]}]})";
        check_error(wrong_type_dict, "Position of error: data/0/node/0/id");
    }
}

} // namespace power_grid_model::meta_data

#endif // __clang_analyzer__ // issue in msgpack
