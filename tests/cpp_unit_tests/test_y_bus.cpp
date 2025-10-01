// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

#include <ranges>

namespace power_grid_model {

namespace {
using math_solver::YBusStructure;
} // namespace

TEST_CASE("Test y bus") {
    //     test Y bus struct
    //     [
    //             x, x, 0, 0
    //             x, x, x, 0
    //             0, x, x, x
    //             0, 0, x, x
    //     ]

    //      [0]   = Node
    // --0--> = Branch (from --id--> to)
    //  -X-   = Open switch / not connected

    //     Topology:

    //   --- 4 ---               ----- 3 -----
    //  |         |             |             |
    //  |         v             v             |
    // [0]       [1] --- 1 --> [2] --- 2 --> [3]
    //  ^         |             |
    //  |         |             5
    //   --- 0 ---              |
    //                          X
    MathModelTopology topo{};
    MathModelParam<symmetric_t> param_sym;
    topo.phase_shift.resize(4, 0.0);
    topo.branch_bus_idx = {
        {1, 0}, // branch 0 from node 1 to 0
        {1, 2}, // branch 1 from node 1 to 2
        {2, 3}, // branch 2 from node 2 to 3
        {3, 2}, // branch 3 from node 3 to 2
        {0, 1}, // branch 4 from node 0 to 1
        {2, -1} // branch 5 from node 2 to "not connected"
    };
    param_sym.branch_param = {// ff, ft, tf, tt
                              {1.0i, 2.0i, 3.0i, 4.0i}, {5.0, 6.0, 7.0, 8.0},     {9.0i, 10.0i, 11.0i, 12.0i},
                              {13.0, 14.0, 15.0, 16.0}, {17.0, 18.0, 19.0, 20.0}, {1000i, 0.0, 0.0, 0.0}};
    topo.shunts_per_bus = {from_sparse, {0, 1, 1, 1, 2}}; // 4 buses, 2 shunts -> shunt connected to bus 0 and bus 3
    param_sym.shunt_param = {100.0i, 200.0i};

    // get shared ptr
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);

    // output
    IdxVector row_indptr = {0, 2, 5, 8, 10};

    // Use col_indices to find the location in Y bus
    //  e.g. col_indices = {0, 1, 0} results in Y bus:
    // [
    //	x, x
    //   x, 0
    // ]
    IdxVector col_indices = {// Culumn col_indices for each non-zero element in Y bus.
                             0, 1, 0, 1, 2, 1, 2, 3, 2, 3};
    Idx nnz = 10; // Number of non-zero elements in Y bus
    IdxVector bus_entry = {0, 3, 6, 9};
    IdxVector const lu_transpose_entry = {// Flip the id's of non-diagonal elements
                                          0, 2, 1, 3, 5, 4, 6, 8, 7, 9};
    IdxVector const y_bus_entry_indptr = {0,  3,      // 0, 1, 2 belong to element [0,0] in Ybus /  3,4 to element [0,1]
                                          5,  7,  10, // 5,6 to [1,0] / 7, 8, 9 to [1,1] / 10 to [1,2]
                                          11, 12, 16, // 11 to [2,1] / 12, 13, 14, 15 to [2,2] / 16, 17 to [2,3]
                                          18, 20,     // 18, 19 to [3,2] / 20, 21, 22  to [3,3]
                                          23};
    IdxVector map_lu_y_bus = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ComplexTensorVector<symmetric_t> admittance_sym = {
        17.0 + 104.0i,  // 0, 0 -> {1, 0}tt + {0, 1}ff + shunt(0) = 4.0i + 17.0 + 100.0i
        18.0 + 3.0i,    // 0, 1 -> {0, 1}ft + {1, 0}tf = 18.0 + 3.0i
        19.0 + 2.0i,    // 1, 0 -> {0, 1}tf + {1, 0}ft = 19.0 + 2.0i
        25.0 + 1.0i,    // 1, 1 -> {0, 1}tt + {1, 0}ff + {1,2}ff = 20.0 + 1.0i + 5.0
        6.0,            // 1, 2 -> {1,2}ft = 6.0
        7.0,            // 2, 1 -> {1,2}tf = 7.0
        24.0 + 1009.0i, // 2, 2 -> {1,2}tt + {2,3}ff + {3, 2}tt + {2,-1}ff = 8.0 + 9.0i + 16.0 + 1000.0i = 24.0 + 1009i
        15.0 + 10.0i,   // 2, 3 -> {2,3}ft + {3,2}tf = 10.0i + 15.0
        14.0 + 11.0i,   // 3, 2 -> {2,3}tf + {3,2}ft = 11.0i + 14.0
        13.0 + 212.0i   // 3, 3 -> {2,3}tt + {3,2}ff + shunt(1) = 12.0i + 13.0 + 200.0i
    };

    // asym input
    // Symmetrical parameters and admittances are converted to asymmetrical tensors,
    // i.e. each parameter/admittance x is converted to:
    //   x 0 0
    //   0 x 0
    //   0 0 x
    MathModelParam<asymmetric_t> param_asym;
    // value
    param_asym.branch_param.resize(param_sym.branch_param.size());
    for (size_t i = 0; i < param_sym.branch_param.size(); i++) {
        for (size_t j = 0; j < 4; j++) {
            param_asym.branch_param[i].value[j] = ComplexTensor<asymmetric_t>{param_sym.branch_param[i].value[j]};
        }
    }
    param_asym.shunt_param.resize(param_sym.shunt_param.size());
    for (size_t i = 0; i < param_sym.shunt_param.size(); i++) {
        param_asym.shunt_param[i] = ComplexTensor<asymmetric_t>{param_sym.shunt_param[i]};
    }
    // admittance_sym
    ComplexTensorVector<asymmetric_t> admittance_asym(admittance_sym.size());
    for (size_t i = 0; i < admittance_sym.size(); i++) {
        admittance_asym[i] = ComplexTensor<asymmetric_t>{admittance_sym[i]};
    }

    SUBCASE("Test y bus construction (symmetrical)") {
        YBus<symmetric_t> const ybus{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
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

        SUBCASE("Test y bus structure getter") {
            YBusStructure ybus_struct_ref{topo};
            auto const& ybus_struct = ybus.get_y_bus_structure();
            CHECK(ybus_struct->bus_entry == ybus_struct_ref.bus_entry);
            CHECK(ybus_struct->col_indices == ybus_struct_ref.col_indices);
            CHECK(ybus_struct->col_indices_lu == ybus_struct_ref.col_indices_lu);
            CHECK(ybus_struct->diag_lu == ybus_struct_ref.diag_lu);
            CHECK(ybus_struct->lu_transpose_entry == ybus_struct_ref.lu_transpose_entry);
            CHECK(ybus_struct->map_lu_y_bus == ybus_struct_ref.map_lu_y_bus);
            CHECK(ybus_struct->row_indptr == ybus_struct_ref.row_indptr);
            CHECK(ybus_struct->row_indptr_lu == ybus_struct_ref.row_indptr_lu);
            CHECK(ybus_struct->y_bus_element.size() == ybus_struct_ref.y_bus_element.size());
            CHECK(ybus_struct->y_bus_entry_indptr == ybus_struct_ref.y_bus_entry_indptr);
        }
    }

    SUBCASE("Test y bus construction (asymmetrical)") {
        YBus<symmetric_t> const ybus_sym{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
        // construct from existing structure
        YBus<asymmetric_t> const ybus{topo_ptr, std::make_shared<MathModelParam<asymmetric_t> const>(param_asym),
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
        YBus<symmetric_t> const ybus{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
        ComplexVector const u{1.0, 2.0, 3.0, 4.0};
        auto branch_flow = ybus.calculate_branch_flow<BranchSolverOutput<symmetric_t>>(u);

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
        YBus<symmetric_t> const ybus{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
        ComplexVector const u{1.0, 2.0, 3.0, 4.0};
        auto shunt_flow = ybus.template calculate_shunt_flow<ApplianceSolverOutput<symmetric_t>>(u);

        // shunt 1
        // i = -4 * 200i
        // s = 4 * conj(-800i) = 3200i
        CHECK(cabs(shunt_flow[1].i - (-800.0i)) < numerical_tolerance);
        CHECK(cabs(shunt_flow[1].s - 3200.0i) < numerical_tolerance);
    }
}

TEST_CASE("Test one bus system") {
    MathModelTopology topo{};
    MathModelParam<symmetric_t> const param;

    topo.phase_shift = {0.0};
    topo.shunts_per_bus = {from_sparse, {0, 0}};

    // output
    IdxVector const indptr = {0, 1};
    IdxVector const col_indices = {0};
    Idx nnz = 1;
    IdxVector const bus_entry = {0};
    IdxVector const lu_transpose_entry = {0};
    IdxVector const y_bus_entry_indptr = {0, 0};

    YBus<symmetric_t> const ybus{std::make_shared<MathModelTopology const>(topo),
                                 std::make_shared<MathModelParam<symmetric_t> const>(param)};

    CHECK(ybus.size() == 1);
    CHECK(ybus.nnz() == nnz);
    CHECK(indptr == ybus.row_indptr());
    CHECK(col_indices == ybus.col_indices());
    CHECK(bus_entry == ybus.bus_entry());
    CHECK(lu_transpose_entry == ybus.lu_transpose_entry());
    CHECK(y_bus_entry_indptr == ybus.y_bus_entry_indptr());
}

TEST_CASE("Test fill-in y bus") {
    // [1] --0--> [0] --1--> [2]
    // extra fill-in: (1, 2) by removing node 0
    //
    // [
    //   0, 1, 2
    //   3, 4, f
    //   5, f, 6
    // ]
    MathModelTopology topo{};
    topo.phase_shift.resize(3, 0.0);
    topo.branch_bus_idx = {
        {1, 0}, // branch 0 from node 1 to 0
        {0, 2}, // branch 1 from node 0 to 2
    };
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.fill_in = {{1, 2}};

    IdxVector const row_indptr = {0, 3, 5, 7};
    IdxVector const col_indices = {0, 1, 2, 0, 1, 0, 2};
    IdxVector const bus_entry = {0, 4, 6};
    IdxVector const lu_transpose_entry = {0, 3, 6, 1, 4, 7, 2, 5, 8};
    IdxVector const y_bus_entry_indptr = {0, 2,              // 0, 1 belong to element [0,0] in Ybus
                                          3, 4, 5, 6, 7, 8}; // everything else has only one entry
    // lu matrix
    IdxVector const row_indptr_lu = {0, 3, 6, 9};
    IdxVector const col_indices_lu = {0, 1, 2, 0, 1, 2, 0, 1, 2};
    IdxVector const map_lu_y_bus = {0, 1, 2, 3, 4, -1, 5, -1, 6};
    IdxVector const diag_lu = {0, 4, 8};

    YBusStructure const ybus{topo};

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

TEST_CASE("Incremental update y-bus") {
    // test Y bus struct
    // [
    //         x, x, 0, 0
    //         x, x, x, 0
    //         0, x, x, x
    //         0, 0, x, x
    // ]
    //
    //  [0]   = Node
    // --0--> = Branch (from --id--> to)
    //  -X-   = Open switch / not connected
    //
    //     Topology:
    //
    //   --- 4 ---               ----- 3 -----
    //  |         |             |             |
    //  |         v             v             |
    // [0]       [1] --- 1 --> [2] --- 2 --> [3]
    //  ^         |             |
    //  |         |             5
    //   --- 0 ---              |
    //                          X
    MathModelTopology topo{};
    MathModelParam<symmetric_t> param_sym;
    topo.phase_shift.resize(4, 0.0);
    topo.branch_bus_idx = {
        {1, 0}, // branch 0 from node 1 to 0
        {1, 2}, // branch 1 from node 1 to 2
        {2, 3}, // branch 2 from node 2 to 3
        {3, 2}, // branch 3 from node 3 to 2
        {0, 1}, // branch 4 from node 0 to 1
        {2, -1} // branch 5 from node 2 to "not connected"
    };
    param_sym.branch_param = {
        //  ff,    ft,    tf,   tt
        {1.0i, 2.0i, 3.0i, 4.0i},    // (1, 0)
        {5.0, 6.0, 7.0, 8.0},        // (1, 2)
        {9.0i, 10.0i, 11.0i, 12.0i}, // (2, 3)
        {13.0, 14.0, 15.0, 16.0},    // (3, 2)
        {17.0, 18.0, 19.0, 20.0},    // (0, 1)
        {1000i, 0.0, 0.0, 0.0}       // (2, -1)
    };
    topo.shunts_per_bus = {from_sparse, {0, 1, 1, 1, 2}}; // 4 buses, 2 shunts -> shunt connected to bus 0 and bus 3
    param_sym.shunt_param = {100.0i, 200.0i};

    // get shared ptr
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);

    const ComplexTensorVector<symmetric_t> admittance_sym = {
        17.0 + 104.0i,  // 0, 0 -> {1, 0}tt + {0, 1}ff + shunt(0) = 4.0i + 17.0 + 100.0i
        18.0 + 3.0i,    // 0, 1 -> {0, 1}ft + {1, 0}tf = 18.0 + 3.0i
        19.0 + 2.0i,    // 1, 0 -> {0, 1}tf + {1, 0}ft = 19.0 + 2.0i
        25.0 + 1.0i,    // 1, 1 -> {0, 1}tt + {1, 0}ff + {1,2}ff = 20.0 + 1.0i + 5.0
        6.0,            // 1, 2 -> {1,2}ft = 6.0
        7.0,            // 2, 1 -> {1,2}tf = 7.0
        24.0 + 1009.0i, // 2, 2 -> {1,2}tt + {2,3}ff + {3, 2}tt + {2,-1}ff = 8.0 + 9.0i + 16.0 + 1000.0i = 24.0 + 1009i
        15.0 + 10.0i,   // 2, 3 -> {2,3}ft + {3,2}tf = 10.0i + 15.0
        14.0 + 11.0i,   // 3, 2 -> {2,3}tf + {3,2}ft = 11.0i + 14.0
        13.0 + 212.0i   // 3, 3 -> {2,3}tt + {3,2}ff + shunt(1) = 12.0i + 13.0 + 200.0i
    };

    const ComplexTensorVector<symmetric_t> admittance_sym_state_1 = {
        34.0 + 208.0i, 36.0 + 6.0i,    38.0 + 4.0i,  50.0 + 2.0i,  12.0,
        14.0,          48.0 + 2018.0i, 30.0 + 20.0i, 14.0 + 22.0i, 26.0 + 424.0i};
    topo.branch_bus_idx = {
        {1, 0}, // branch 0 from node 1 to 0
        {1, 2}, // branch 1 from node 1 to 2
        {2, 3}, // branch 2 from node 2 to 3
        {3, 2}, // branch 3 from node 3 to 2
        {0, 1}, // branch 4 from node 0 to 1
        {2, -1} // branch 5 from node 2 to "not connected"
    };

    MathModelParam<symmetric_t> param_sym_update;
    // Swap based params
    param_sym_update.branch_param = {
        //   ff,    ft,   tf,   tt
        {2.0i, 2.0i, 3.0i, 4.0i},    // (1, 0)
        {5.0, 7.0, 7.0, 8.0},        // (1, 2)
        {9.0i, 10.0i, 11.0i, 14.0i}, // (2, 3)
        {13.0, 14.0, 17.0, 16.0},    // (3, 2)
        {17.0, 18.0, 19.0, 20.0},    // (0, 1)
        {1001.0i, 0.0, 0.0, 0.0}     // (2,-1)
    };
    param_sym_update.shunt_param = {101.0i, 200.0i};

    const ComplexTensorVector<symmetric_t> admittance_sym_2 = {
        // 17.0 + 104.0i,                                                                        [v]
        17.0 + 105.0i, // 0, 0 -> += {1, 0}tt + {0, 1}ff + shunt(0) = 0.0 + 0.0 + 1.0i
        // 18.0 + 3.0i,                                                                          [v]
        18.0 + 3.0i, // 0, 1 -> += {0, 1}ft + {1, 0}tf = 0.0 + 0.0
        // 19.0 + 2.0i,                                                                          [v]
        19.0 + 2.0i, // 1, 0 -> += {0, 1}tf + {1, 0}ft = 0.0 + 0.0
        // 25.0 + 1.0i,                                                                          [v]
        25.0 + 2.0i, // 1, 1 -> += {0, 1}tt + {1, 0}ff + {1,2}ff = 0.0 + 1.0i + 0.0
        // 6.0,                                                                                  [v]
        7.0, // 1, 2 -> += {1,2}ft = 1.0
        // 7.0,                                                                                  [v]
        7.0, // 2, 1 -> += {1,2}tf = 0.0
        // 24.0 + 1009.0i,                                                                       [v]
        24.0 + 1010.0i, // 2, 2 -> += {1,2}tt + {2,3}ff + {3, 2}tt + {2,-1}ff = 0.0 + 0.0 + 0.0 + 1.0i
        // 15.0 + 10.0i,                                                                         [v]
        17.0 + 10.0i, // 2, 3 -> += {2,3}ft + {3,2}tf = 0.0 + 2.0
        // 14.0 + 11.0i,                                                                         [v]
        14.0 + 11.0i, // 3, 2 -> += {2,3}tf + {3,2}ft = 0.0 + 0.0
        // 13.0 + 212.0i                                                                         [v]
        13.0 + 214.0i // 3, 3 -> += {2,3}tt + {3,2}ff + shunt(1) = 2.0i + 0.0 + 0.0
    };

    auto verify_admittance = [](ComplexTensorVector<symmetric_t> const& admittance,
                                ComplexTensorVector<symmetric_t> const& admittance_ref) {
        CHECK(admittance.size() == admittance_ref.size());
        for (size_t i = 0; i < admittance.size(); i++) {
            CHECK(cabs(admittance[i] - admittance_ref[i]) < numerical_tolerance);
        }
    };
    SUBCASE("Test whole scale update") {
        YBus<symmetric_t> ybus{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
        verify_admittance(ybus.admittance(), admittance_sym);

        ybus.update_admittance(std::make_shared<MathModelParam<symmetric_t> const>(param_sym));
        verify_admittance(ybus.admittance(), admittance_sym);
    }

    SUBCASE("Test progressive update") {
        YBus<symmetric_t> ybus{topo_ptr, std::make_shared<MathModelParam<symmetric_t> const>(param_sym)};
        verify_admittance(ybus.admittance(), admittance_sym);

        auto branch_param_to_change_views =
            IdxRange{static_cast<int>(param_sym_update.branch_param.size())} |
            std::views::filter([&param_sym_update](Idx i) {
                return param_sym_update.branch_param[i].yff() != ComplexTensor<symmetric_t>{0.0} ||
                       param_sym_update.branch_param[i].yft() != ComplexTensor<symmetric_t>{0.0} ||
                       param_sym_update.branch_param[i].ytf() != ComplexTensor<symmetric_t>{0.0} ||
                       param_sym_update.branch_param[i].ytt() != ComplexTensor<symmetric_t>{0.0};
            });
        auto shunt_param_to_change_views =
            IdxRange{static_cast<int>(param_sym_update.shunt_param.size())} |
            std::views::filter([&param_sym_update](Idx i) {
                return param_sym_update.shunt_param[i] != ComplexTensor<symmetric_t>{0.0};
            });

        MathModelParamIncrement math_model_param_incrmt;
        std::ranges::copy(branch_param_to_change_views,
                          std::back_inserter(math_model_param_incrmt.branch_param_to_change));
        std::ranges::copy(shunt_param_to_change_views,
                          std::back_inserter(math_model_param_incrmt.shunt_param_to_change));

        auto param_update_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param_sym_update);

        ybus.update_admittance_increment(param_update_ptr, math_model_param_incrmt);
        verify_admittance(ybus.admittance(), admittance_sym_2);
    }
}

TEST_CASE("Test counting_sort_element") {
    using math_solver::counting_sort_element;
    using math_solver::YBusElementMap;
    using enum power_grid_model::YBusElementType;

    SUBCASE("Test basic sorting") {
        // Create test data: elements at various matrix positions
        std::vector<YBusElementMap> vec = {
            {{2, 1}, {bft, 5}},   // pos (2,1)
            {{0, 0}, {bff, 0}},   // pos (0,0)
            {{1, 2}, {btf, 3}},   // pos (1,2)
            {{0, 1}, {bft, 1}},   // pos (0,1)
            {{2, 1}, {shunt, 6}}, // pos (2,1) - same position as first
            {{1, 0}, {btf, 2}},   // pos (1,0)
            {{2, 2}, {btt, 7}},   // pos (2,2)
        };

        // Expected sorted order: by row first, then by column
        // (0,0), (0,1), (1,0), (1,2), (2,1), (2,1), (2,2)
        std::vector<std::pair<Idx, Idx>> expected_positions = {{0, 0}, {0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 1}, {2, 2}};

        Idx const n_bus = 3;
        counting_sort_element(vec, n_bus);

        // Verify sorting
        CHECK(vec.size() == 7);
        for (size_t i = 0; i < vec.size(); ++i) {
            CHECK(vec[i].pos.first == expected_positions[i].first);
            CHECK(vec[i].pos.second == expected_positions[i].second);
        }

        // Verify specific elements are preserved correctly
        CHECK(vec[0].element.element_type == bff);
        CHECK(vec[0].element.idx == 0);
        CHECK(vec[1].element.element_type == bft);
        CHECK(vec[1].element.idx == 1);
    }

    SUBCASE("Test with single bus") {
        std::vector<YBusElementMap> vec = {{{0, 0}, {shunt, 10}}};

        counting_sort_element(vec, 1);

        CHECK(vec.size() == 1);
        CHECK(vec[0].pos == std::make_pair(0, 0));
        CHECK(vec[0].element.element_type == shunt);
        CHECK(vec[0].element.idx == 10);
    }

    SUBCASE("Test with empty vector") {
        std::vector<YBusElementMap> vec;
        counting_sort_element(vec, 5);
        CHECK(vec.empty());
    }

    SUBCASE("Test stability - elements with same position maintain relative order") {
        std::vector<YBusElementMap> vec = {
            {{1, 1}, {bff, 100}},
            {{1, 1}, {bft, 200}},
            {{1, 1}, {shunt, 300}},
        };

        counting_sort_element(vec, 2);

        CHECK(vec.size() == 3);
        // All should be at position (1,1)
        for (const auto& element : vec) {
            CHECK(element.pos == std::make_pair(1, 1));
        }
        // Original relative order should be preserved (stable sort)
        CHECK(vec[0].element.idx == 100);
        CHECK(vec[1].element.idx == 200);
        CHECK(vec[2].element.idx == 300);
    }

    SUBCASE("Test large sparse matrix scenario") {
        std::vector<YBusElementMap> vec;
        Idx const n_bus = 10;

        // Add elements in reverse order to test sorting thoroughly
        for (Idx i = n_bus * n_bus - 1; i != static_cast<Idx>(-1); --i) {
            Idx const row = i / n_bus;
            Idx const col = i % n_bus;
            if ((row + col) % 3 == 0) { // Sparse pattern
                vec.push_back({{row, col}, {bff, row * n_bus + col}});
            }
        }

        size_t original_size = vec.size();
        counting_sort_element(vec, n_bus);

        CHECK(vec.size() == original_size);

        // Verify sorted order
        for (size_t i = 1; i < vec.size(); ++i) {
            auto [prev_row, prev_col] = vec[i - 1].pos;
            auto [curr_row, curr_col] = vec[i].pos;

            // Check row-major order
            if (prev_row == curr_row) {
                CHECK(prev_col <= curr_col); // Same row, column should be non-decreasing
            } else {
                CHECK(prev_row < curr_row); // Different row, previous row should be smaller
            }
        }
    }

    SUBCASE("Test all YBusElementType values") {
        std::vector<YBusElementMap> vec = {
            {{1, 1}, {fill_in_tf, 6}}, {{0, 1}, {bft, 1}},   {{1, 0}, {btf, 2}},        {{0, 0}, {bff, 0}},
            {{1, 1}, {btt, 3}},        {{2, 2}, {shunt, 4}}, {{1, 2}, {fill_in_ft, 5}},
        };

        counting_sort_element(vec, 3);

        // Verify positions are sorted correctly
        std::vector<std::pair<Idx, Idx>> expected_positions = {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {1, 1}, {1, 2}, {2, 2}};

        CHECK(vec.size() == 7);
        for (size_t i = 0; i < vec.size(); ++i) {
            CHECK(vec[i].pos == expected_positions[i]);
        }
    }
}

} // namespace power_grid_model
