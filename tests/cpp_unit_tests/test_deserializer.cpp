// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/update.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::meta_data {

// single data
namespace {
constexpr char const* json_single = R"(
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
      ]
    ]
  }
}
)";

} // namespace

// batch data
namespace {
constexpr char const* json_batch = R"(
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
    }
  ]
}
)";

} // namespace

namespace {

inline std::map<std::string, Deserializer::Buffer> get_buffer_map(Deserializer const& deserializer) {
    std::map<std::string, Deserializer::Buffer> map;
    for (Idx i = 0; i != deserializer.n_components(); ++i) {
        auto const& buffer = deserializer.get_buffer_info(i);
        map[buffer.component->name] = buffer;
    }
    return map;
}
} // namespace

TEST_CASE("Deserializer") {

    Deserializer deserializer{};

    SUBCASE("Single dataset") {
        deserializer.deserialize_from_json(json_single);

        SUBCASE("Check meta data") {
            CHECK(deserializer.dataset_name() == "input");
            CHECK(!deserializer.is_batch());
            CHECK(deserializer.batch_size() == 1);
            CHECK(deserializer.n_components() == 4);
        }

        SUBCASE("Check buffer") {
            auto const map = get_buffer_map(deserializer);
            CHECK(map.at("node").elements_per_scenario == 3);
            CHECK(map.at("node").total_elements == 3);
            CHECK(map.at("line").elements_per_scenario == 2);
            CHECK(map.at("line").total_elements == 2);
            CHECK(map.at("source").elements_per_scenario == 3);
            CHECK(map.at("source").total_elements == 3);
            CHECK(map.at("sym_load").elements_per_scenario == 2);
            CHECK(map.at("sym_load").total_elements == 2);
        }

        SUBCASE("Check parse") {
            std::vector<NodeInput> node(3);
            std::vector<LineInput> line(2);
            std::vector<SourceInput> source(3);
            std::vector<SymLoadGenInput> sym_load(2);
            std::array<void*, 4> all_data{node.data(), line.data(), source.data(), sym_load.data()};
            std::array all_components{"node", "line", "source", "sym_load"};
            deserializer.set_buffer(all_components.data(), all_data.data(), nullptr);
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
        }
    }

    SUBCASE("Batch dataset") {
        deserializer.deserialize_from_json(json_batch);

        SUBCASE("Check meta data") {
            CHECK(deserializer.dataset_name() == "update");
            CHECK(deserializer.is_batch());
            CHECK(deserializer.batch_size() == 3);
            CHECK(deserializer.n_components() == 2);
        }

        SUBCASE("Check buffer") {
            auto const map = get_buffer_map(deserializer);
            CHECK(map.at("sym_load").elements_per_scenario == -1);
            CHECK(map.at("sym_load").total_elements == 3);
            CHECK(map.at("asym_load").elements_per_scenario == 1);
            CHECK(map.at("asym_load").total_elements == 3);
        }

        SUBCASE("Check parse") {
            std::vector<SymLoadGenUpdate> sym_load(3);
            std::vector<AsymLoadGenUpdate> asym_load(3);
            IdxVector sym_load_indptr(4);
            std::array<void*, 2> all_data{sym_load.data(), asym_load.data()};
            std::array all_components{"sym_load", "asym_load"};
            std::array<Idx*, 2> all_indptrs{sym_load_indptr.data(), nullptr};
            deserializer.set_buffer(all_components.data(), all_data.data(), all_indptrs.data());
            deserializer.parse();

            // sym_load
            CHECK(sym_load_indptr == IdxVector{0, 1, 1, 3});
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
        }
    }
}

} // namespace power_grid_model::meta_data
