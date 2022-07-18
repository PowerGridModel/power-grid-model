// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/math_solver/y_bus.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

TEST_CASE("Test y bus") {
    /*
    test Y bus struct
    [
            x, x, 0, 0
            x, x, x, 0
            0, x, x, x
            0, 0, x, x
    ]

     [0]   = Node
--0--> = Branch (from --id--> to)
 -X-   = Open switch / not connected

    Topology:

  --- 4 ---               ----- 3 -----
 |         |             |             |
 |         v             v             |
[0]       [1] --- 1 --> [2] --- 2 --> [3]
 ^         |             |
 |         |             5
  --- 0 ---              |
                             X
    */

    MathModelTopology topo{};
    MathModelParam<true> param_sym;
    topo.phase_shift.resize(4, 0.0);
    topo.branch_bus_idx = {
        {1, 0},  // branch 0 from node 1 to 0
        {1, 2},  // branch 1 from node 1 to 2
        {2, 3},  // branch 2 from node 2 to 3
        {3, 2},  // branch 3 from node 3 to 2
        {0, 1},  // branch 4 from node 0 to 1
        {2, -1}  // branch 5 from node 2 to "not connected"
    };
    param_sym.branch_param = {// ff, ft, tf, tt
                              {1.0i, 2.0i, 3.0i, 4.0i}, {5.0, 6.0, 7.0, 8.0},     {9.0i, 10.0i, 11.0i, 12.0i},
                              {13.0, 14.0, 15.0, 16.0}, {17.0, 18.0, 19.0, 20.0}, {1000i, 0.0, 0.0, 0.0}};
    topo.shunt_bus_indptr = {0, 1, 1, 1, 2};  // 4 buses, 2 shunts -> shunt connected to bus 0 and bus 3
    param_sym.shunt_param = {100.0i, 200.0i};

    // get shared ptr
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);

    // output
    IdxVector row_indptr = {0, 2, 5, 8, 10};

    /* Use col_indices to find the location in Y bus
     *  e.g. col_indices = {0, 1, 0} results in Y bus:
     * [
     *	x, x
     *   x, 0
     * ]
     */
    IdxVector col_indices = {// Culumn col_indices for each non-zero element in Y bus.
                             0, 1, 0, 1, 2, 1, 2, 3, 2, 3};
    Idx nnz = 10;  // Number of non-zero elements in Y bus
    IdxVector bus_entry = {0, 3, 6, 9};
    IdxVector lu_transpose_entry = {// Flip the id's of non-diagonal elements
                                    0, 2, 1, 3, 5, 4, 6, 8, 7, 9};
    IdxVector y_bus_entry_indptr = {0,  3,       // 0, 1, 2 belong to element [0,0] in Ybus /  3,4 to element [0,1]
                                    5,  7,  10,  // 5,6 to [1,0] / 7, 8, 9 to [1,1] / 10 to [1,2]
                                    11, 12, 16,  // 11 to [2,1] / 12, 13, 14, 15 to [2,2] / 16, 17 to [2,3]
                                    18, 20,      // 18, 19 to [3,2] / 20, 21, 22  to [3,3]
                                    23};
    IdxVector map_lu_y_bus = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ComplexTensorVector<true> admittance_sym = {
        17.0 + 104.0i,   // 0, 0 -> {1, 0}tt + {0, 1}ff + shunt(0) = 4.0i + 17.0 + 100.0i
        18.0 + 3.0i,     // 0, 1 -> {0, 1}ft + {1, 0}tf = 18.0 + 3.0i
        19.0 + 2.0i,     // 1, 0 -> {0, 1}tf + {1, 0}ft = 19.0 + 2.0i
        25.0 + 1.0i,     // 1, 1 -> {0, 1}tt + {1, 0}ff + {1,2}ff = 20.0 + 1.0i + 5.0
        6.0,             // 1, 2 -> {1,2}ft = 6.0
        7.0,             // 2, 1 -> {1,2}tf = 7.0
        24.0 + 1009.0i,  // 2, 2 -> {1,2}tt + {2,3}ff + {3, 2}tt + {2,-1}ff = 8.0 + 9.0i + 16.0 + 1000.0i = 24.0 + 1009i
        15.0 + 10.0i,    // 2, 3 -> {2,3}ft + {3,2}tf = 10.0i + 15.0
        14.0 + 11.0i,    // 3, 2 -> {2,3}tf + {3,2}ft = 11.0i + 14.0
        13.0 + 212.0i    // 3, 3 -> {2,3}tt + {3,2}ff + shunt(1) = 12.0i + 13.0 + 200.0i
    };

    // asym input
    // Symmetrical parameters and admittances are converted to asymmetrical tensors,
    // i.e. each parameter/admittance x is converted to:
    //   x 0 0
    //   0 x 0
    //   0 0 x
    MathModelParam<false> param_asym;
    // value
    param_asym.branch_param.resize(param_sym.branch_param.size());
    for (size_t i = 0; i < param_sym.branch_param.size(); i++) {
        for (size_t j = 0; j < 4; j++) {
            param_asym.branch_param[i].value[j] = ComplexTensor<false>{param_sym.branch_param[i].value[j]};
        }
    }
    param_asym.shunt_param.resize(param_sym.shunt_param.size());
    for (size_t i = 0; i < param_sym.shunt_param.size(); i++) {
        param_asym.shunt_param[i] = ComplexTensor<false>{param_sym.shunt_param[i]};
    }
    // admittance_sym
    ComplexTensorVector<false> admittance_asym(admittance_sym.size());
    for (size_t i = 0; i < admittance_sym.size(); i++) {
        admittance_asym[i] = ComplexTensor<false>{admittance_sym[i]};
    }

    SUBCASE("Test y bus construction (symmetrical)") {
        YBus<true> ybus{topo_ptr, std::make_shared<MathModelParam<true> const>(param_sym)};
        CHECK(ybus.size() == 4);
        CHECK(ybus.nnz() == nnz);
        CHECK(row_indptr == ybus.row_indptr());
        CHECK(col_indices == ybus.col_indices());
        CHECK(bus_entry == ybus.bus_entry());
        CHECK(lu_transpose_entry == ybus.lu_transpose_entry());
        CHECK(y_bus_entry_indptr == ybus.y_bus_entry_indptr());
        CHECK(ybus.admittance().size() == admittance_sym.size());
        for (size_t i = 0; i < admittance_sym.size(); i++) {
            CHECK(cabs(ybus.admittance()[i] - admittance_sym[i]) < numerical_tolerance);
        }

        // check lu
        CHECK(*ybus.shared_indptr_lu() == row_indptr);
        CHECK(*ybus.shared_indices_lu() == col_indices);
        CHECK(*ybus.shared_diag_lu() == bus_entry);
        CHECK(ybus.map_lu_y_bus() == map_lu_y_bus);
    }

    SUBCASE("Test y bus construction (asymmetrical)") {
        YBus<true> ybus_sym{topo_ptr, std::make_shared<MathModelParam<true> const>(param_sym)};
        // construct from existing structure
        YBus<false> ybus{topo_ptr, std::make_shared<MathModelParam<false> const>(param_asym),
                         ybus_sym.shared_y_bus_struct()};
        CHECK(ybus.size() == 4);
        CHECK(ybus.nnz() == nnz);
        CHECK(row_indptr == ybus.row_indptr());
        CHECK(col_indices == ybus.col_indices());
        CHECK(bus_entry == ybus.bus_entry());
        CHECK(lu_transpose_entry == ybus.lu_transpose_entry());
        CHECK(y_bus_entry_indptr == ybus.y_bus_entry_indptr());
        CHECK(ybus.admittance().size() == admittance_asym.size());
        for (size_t i = 0; i < admittance_asym.size(); i++) {
            CHECK((cabs(ybus.admittance()[i] - admittance_asym[i]) < numerical_tolerance).all());
        }
    }

    SUBCASE("Test branch flow calculation") {
        YBus<true> ybus{topo_ptr, std::make_shared<MathModelParam<true> const>(param_sym)};
        ComplexVector u{1.0, 2.0, 3.0, 4.0};
        auto branch_flow = ybus.calculate_branch_flow(u);

        // branch 2, bus 2->3
        // if = 3 * 9i + 4 * 10i = 67i
        // it = 3 * 11i + 4 * 12i = 81i
        // sf = 3 * conj(67i) = -201i
        // st = 4 * conj(81i) = -324i
        CHECK(cabs(branch_flow[2].i_f - 67.0i) < numerical_tolerance);
        CHECK(cabs(branch_flow[2].i_t - 81.0i) < numerical_tolerance);
        CHECK(cabs(branch_flow[2].s_f - (-201.0i)) < numerical_tolerance);
        CHECK(cabs(branch_flow[2].s_t - (-324.0i)) < numerical_tolerance);
    }

    SUBCASE("Test shunt flow calculation") {
        YBus<true> ybus{topo_ptr, std::make_shared<MathModelParam<true> const>(param_sym)};
        ComplexVector u{1.0, 2.0, 3.0, 4.0};
        auto shunt_flow = ybus.calculate_shunt_flow(u);

        // shunt 1
        // i = -4 * 200i
        // s = 4 * conj(-800i) = 3200i
        CHECK(cabs(shunt_flow[1].i - (-800.0i)) < numerical_tolerance);
        CHECK(cabs(shunt_flow[1].s - 3200.0i) < numerical_tolerance);
    }
}

TEST_CASE("Test one bus system") {
    MathModelTopology topo{};
    MathModelParam<true> param;

    topo.phase_shift = {0.0};
    topo.shunt_bus_indptr = {0, 0};

    // output
    IdxVector indptr = {0, 1};
    IdxVector col_indices = {0};
    Idx nnz = 1;
    IdxVector bus_entry = {0};
    IdxVector lu_transpose_entry = {0};
    IdxVector y_bus_entry_indptr = {0, 0};

    YBus<true> ybus{std::make_shared<MathModelTopology const>(topo),
                    std::make_shared<MathModelParam<true> const>(param)};

    CHECK(ybus.size() == 1);
    CHECK(ybus.nnz() == nnz);
    CHECK(indptr == ybus.row_indptr());
    CHECK(col_indices == ybus.col_indices());
    CHECK(bus_entry == ybus.bus_entry());
    CHECK(lu_transpose_entry == ybus.lu_transpose_entry());
    CHECK(y_bus_entry_indptr == ybus.y_bus_entry_indptr());
}

TEST_CASE("Test fill-in y bus") {
    /*
     * struct
     * [1] --0--> [0] --1--> [2]
     * extra fill-in: (1, 2) by removing node 0
     *
     * [
     *   0, 1, 2
     *   3, 4, f
     *   5, f, 6
     * ]
     */

    MathModelTopology topo{};
    topo.phase_shift.resize(3, 0.0);
    topo.branch_bus_idx = {
        {1, 0},  // branch 0 from node 1 to 0
        {0, 2},  // branch 1 from node 0 to 2
    };
    topo.shunt_bus_indptr = {0, 0, 0, 0};
    topo.fill_in = {{1, 2}};

    IdxVector row_indptr = {0, 3, 5, 7};
    IdxVector col_indices = {0, 1, 2, 0, 1, 0, 2};
    IdxVector bus_entry = {0, 4, 6};
    IdxVector lu_transpose_entry = {0, 3, 6, 1, 4, 7, 2, 5, 8};
    IdxVector y_bus_entry_indptr = {0, 2,               // 0, 1 belong to element [0,0] in Ybus
                                    3, 4, 5, 6, 7, 8};  // everything else has only one entry
    // lu matrix
    IdxVector row_indptr_lu = {0, 3, 6, 9};
    IdxVector col_indices_lu = {0, 1, 2, 0, 1, 2, 0, 1, 2};
    IdxVector map_lu_y_bus = {0, 1, 2, 3, 4, -1, 5, -1, 6};
    IdxVector diag_lu = {0, 4, 8};

    YBusStructure ybus{topo};

    CHECK(row_indptr == ybus.row_indptr);
    CHECK(col_indices == ybus.col_indices);
    CHECK(bus_entry == ybus.bus_entry);
    CHECK(lu_transpose_entry == ybus.lu_transpose_entry);
    CHECK(y_bus_entry_indptr == ybus.y_bus_entry_indptr);
    // check lu
    CHECK(ybus.row_indptr_lu == row_indptr_lu);
    CHECK(ybus.col_indices_lu == col_indices_lu);
    CHECK(ybus.diag_lu == diag_lu);
    CHECK(ybus.map_lu_y_bus == map_lu_y_bus);
}

/*
TODO:
- test counting_sort_element()
*/

}  // namespace power_grid_model
