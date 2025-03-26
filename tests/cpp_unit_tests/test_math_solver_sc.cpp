// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_common.hpp"

#include <power_grid_model/math_solver/short_circuit_solver.hpp>

namespace power_grid_model::math_solver {

namespace {

using FaultType::single_phase_to_ground;
using FaultType::three_phase;
using FaultType::two_phase;
using FaultType::two_phase_to_ground;

template <symmetry_tag sym>
void assert_sc_output(ShortCircuitSolverOutput<sym> const& output, ShortCircuitSolverOutput<sym> const& output_ref,
                      double tolerance = numerical_tolerance) {
    for (size_t i = 0; i != output.u_bus.size(); ++i) {
        check_close<sym>(output.u_bus[i], output_ref.u_bus[i], tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].i_f, output_ref.branch[i].i_f, tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].i_t, output_ref.branch[i].i_t, tolerance);
    }
    for (size_t i = 0; i != output.fault.size(); ++i) {
        check_close<sym>(output.fault[i].i_fault, output_ref.fault[i].i_fault, tolerance);
    }
    for (size_t i = 0; i != output.source.size(); ++i) {
        check_close<sym>(output.source[i].i, output_ref.source[i].i, tolerance);
    }
}

ShortCircuitInput create_sc_test_input(FaultType fault_type, FaultPhase fault_phase, DoubleComplex const& y_fault,
                                       double const vref, grouped_idx_vector_type auto const& fault_buses) {
    ShortCircuitInput sc_input;
    sc_input.fault_buses = fault_buses;
    sc_input.source = {vref};
    sc_input.faults = {{.y_fault = y_fault, .fault_type = fault_type, .fault_phase = fault_phase}};
    return sc_input;
}

template <symmetry_tag sym> constexpr ShortCircuitSolverOutput<sym> blank_sc_output(DoubleComplex vref) {
    ShortCircuitSolverOutput<sym> sc_output;
    sc_output.u_bus = {ComplexValue<sym>(vref), ComplexValue<sym>(vref)};
    sc_output.fault = {{ComplexValue<sym>{}}};
    sc_output.branch = {
        BranchShortCircuitSolverOutput<sym>{.i_f = {ComplexValue<sym>{}}, .i_t = {ComplexValue<sym>{}}}};
    sc_output.source = {{ComplexValue<sym>{}}};
    return sc_output;
}

template <symmetry_tag sym>
constexpr ShortCircuitSolverOutput<sym> create_math_sc_output(ComplexValue<sym> u0, ComplexValue<sym> u1,
                                                              ComplexValue<sym> if_abc) {
    ShortCircuitSolverOutput<sym> sc_output;
    sc_output.u_bus = {std::move(u0), std::move(u1)};
    sc_output.fault = {{if_abc}};
    sc_output.branch = {BranchShortCircuitSolverOutput<sym>{.i_f = if_abc, .i_t = -if_abc}};
    sc_output.source = {{if_abc}};
    return sc_output;
}

template <symmetry_tag sym>
ShortCircuitSolverOutput<sym> create_sc_test_output(FaultType fault_type, DoubleComplex const& z_fault,
                                                    DoubleComplex const& z0, DoubleComplex const& z0_0,
                                                    double const vref, DoubleComplex const& zref) {

    if constexpr (is_symmetric_v<sym>) {
        DoubleComplex const if_abc = vref / (z0 + zref + z_fault);
        DoubleComplex const u0 = vref - if_abc * zref;
        DoubleComplex const u1 = u0 - if_abc * z0;
        return create_math_sc_output<symmetric_t>(u0, u1, if_abc);
    } else {
        ComplexValue<asymmetric_t> if_abc{};
        switch (fault_type) {
        case three_phase: {
            DoubleComplex const if_3ph = vref / (z0 + zref + z_fault);
            if_abc = ComplexValue<asymmetric_t>(if_3ph);
            break;
        }
        case single_phase_to_ground: {
            DoubleComplex const if_1phg = vref / (2.0 * (zref + z0) + (z0_0 + zref) + 3.0 * z_fault);
            if_abc = ComplexValue<asymmetric_t>(3.0 * if_1phg, 0.0, 0.0);
            break;
        }
        case two_phase: {
            DoubleComplex const if_2ph = (-1i * sqrt3) * vref / (2.0 * (zref + z0) + z_fault);
            if_abc = ComplexValue<asymmetric_t>(0.0, if_2ph, -if_2ph);
            break;
        }
        case two_phase_to_ground: {
            DoubleComplex const y2phg_0 = 1.0 / (zref + z0_0 + 3.0 * z_fault);
            DoubleComplex const y2phg_12 = 1.0 / (zref + z0);
            DoubleComplex const y2phg_sum = 2.0 * y2phg_12 + y2phg_0;
            DoubleComplex const i_0 = vref * (-y2phg_0 * y2phg_12 / y2phg_sum);
            DoubleComplex const i_1 = vref * ((-y2phg_12 * y2phg_12 / y2phg_sum) + y2phg_12);
            DoubleComplex const i_2 = vref * (-y2phg_12 * y2phg_12 / y2phg_sum);
            if_abc =
                ComplexValue<asymmetric_t>{i_0 + i_1 + i_2, i_0 + i_1 * a * a + i_2 * a, i_0 + i_1 * a + i_2 * a * a};
            break;
        }
        default:
            throw InvalidShortCircuitType{false, fault_type};
        }
        ComplexValue<asymmetric_t> const vref_asym{vref};
        ComplexValue<asymmetric_t> const u0 = vref_asym - if_abc * zref;
        DoubleComplex const z_self{(2.0 * z0 + z0_0) / 3.0};
        DoubleComplex const z_mutual{(z0_0 - z0) / 3.0};
        ComplexValue<asymmetric_t> const u_drop{
            if_abc(0) * z_self + (if_abc(1) + if_abc(2)) * z_mutual,
            if_abc(1) * z_self + (if_abc(0) + if_abc(2)) * z_mutual,
            if_abc(2) * z_self + (if_abc(0) + if_abc(1)) * z_mutual,
        };
        ComplexValue<asymmetric_t> const u1 = u0 - u_drop;
        return create_math_sc_output<asymmetric_t>(u0, u1, if_abc);
    }
}

} // namespace

TEST_CASE("Short circuit solver") {
    // Test case grid
    // source -- bus --- line -- bus -- fault(type varying as per subcase)

    // Grid for short circuit
    MathModelTopology topo_sc;
    topo_sc.slack_bus = 0;
    topo_sc.phase_shift = {0.0, 0.0};
    topo_sc.branch_bus_idx = {{0, 1}};
    topo_sc.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo_sc.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo_sc.load_gens_per_bus = {from_sparse, {0, 0, 0}};
    DenseGroupedIdxVector const fault_buses{from_sparse, {0, 0, 1}};

    // Impedance / admittances
    // source
    double const vref = 1.1;
    DoubleComplex const yref{10.0 - 50.0i};
    DoubleComplex const zref{1.0 / yref};
    // line
    DoubleComplex const y0{1.0 - 2.0i};
    DoubleComplex const y0_0{0.5 + 0.5i};
    DoubleComplex const z0{1.0 / y0};
    DoubleComplex const z0_0{1.0 / y0_0};
    // fault
    DoubleComplex const z_fault{1.0 + 1.0i};
    DoubleComplex const y_fault{1.0 / z_fault};
    DoubleComplex const z_fault_solid{0.0 + 0.0i};
    DoubleComplex const y_fault_solid{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

    // params sym
    MathModelParam<symmetric_t> param_sc_sym;
    param_sc_sym.branch_param = {{y0, -y0, -y0, y0}};
    param_sc_sym.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};

    // params asym
    MathModelParam<asymmetric_t> param_sc_asym;
    ComplexTensor<asymmetric_t> const y0a{(2.0 * y0 + y0_0) / 3.0, (y0_0 - y0) / 3.0};
    param_sc_asym.branch_param = {{y0a, -y0a, -y0a, y0a}};
    ComplexTensor<asymmetric_t> const yref_asym{yref};
    param_sc_asym.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};

    // topo and param ptr
    auto topo_sc_ptr = std::make_shared<MathModelTopology const>(topo_sc);
    auto param_sym_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param_sc_sym);
    auto param_asym_ptr = std::make_shared<MathModelParam<asymmetric_t> const>(param_sc_asym);

    SUBCASE("Test short circuit solver 3ph") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(three_phase, z_fault, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(three_phase, FaultPhase::default_value, y_fault, vref, fault_buses);
        CHECK_THROWS_AS(solver.run_short_circuit(y_bus_asym, sc_input_default), InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 3ph solid fault") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(three_phase, z_fault_solid, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 3ph sym params") {
        YBus<symmetric_t> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        ShortCircuitSolver<symmetric_t> solver{y_bus_sym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<symmetric_t>(three_phase, z_fault, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_sym, sc_input);
        assert_sc_output<symmetric_t>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(three_phase, FaultPhase::default_value, y_fault, vref, fault_buses);
        CHECK_THROWS_AS(solver.run_short_circuit(y_bus_sym, sc_input_default), InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 3ph sym params solid fault") {
        YBus<symmetric_t> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        ShortCircuitSolver<symmetric_t> solver{y_bus_sym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<symmetric_t>(three_phase, z_fault_solid, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_sym, sc_input);
        assert_sc_output<symmetric_t>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 1phg") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(single_phase_to_ground, z_fault, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(single_phase_to_ground, FaultPhase::default_value, y_fault, vref, fault_buses);
        CHECK_THROWS_AS(solver.run_short_circuit(y_bus_asym, sc_input_default), InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 1phg solid fault") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault_solid, vref, fault_buses);
        auto sc_output_ref =
            create_sc_test_output<asymmetric_t>(single_phase_to_ground, z_fault_solid, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 2ph") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(two_phase, z_fault, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);

        auto sc_input_default = create_sc_test_input(two_phase, FaultPhase::default_value, y_fault, vref, fault_buses);
        CHECK_THROWS_AS(solver.run_short_circuit(y_bus_asym, sc_input_default), InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 2ph solid fault") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault_solid, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(two_phase, z_fault_solid, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 2phg") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault, vref, fault_buses);
        auto sc_output_ref = create_sc_test_output<asymmetric_t>(two_phase_to_ground, z_fault, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(two_phase_to_ground, FaultPhase::default_value, y_fault, vref, fault_buses);
        CHECK_THROWS_AS(solver.run_short_circuit(y_bus_asym, sc_input_default), InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 2phg solid") {
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault_solid, vref, fault_buses);
        auto sc_output_ref =
            create_sc_test_output<asymmetric_t>(two_phase_to_ground, z_fault_solid, z0, z0_0, vref, zref);
        auto output = solver.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver no faults") {
        YBus<symmetric_t> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        YBus<asymmetric_t> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        ShortCircuitSolver<asymmetric_t> solver_asym{y_bus_asym, topo_sc_ptr};
        ShortCircuitInput sc_input;
        sc_input.source = {vref};
        sc_input.fault_buses = {from_dense, {}, topo_sc_ptr->n_bus()};
        auto asym_sc_output_ref = blank_sc_output<asymmetric_t>(vref);
        auto asym_output = solver_asym.run_short_circuit(y_bus_asym, sc_input);
        assert_sc_output<asymmetric_t>(asym_output, asym_sc_output_ref);

        ShortCircuitSolver<symmetric_t> solver_sym{y_bus_sym, topo_sc_ptr};
        auto sym_sc_output_ref = blank_sc_output<symmetric_t>(vref);
        auto sym_output = solver_sym.run_short_circuit(y_bus_sym, sc_input);
        assert_sc_output<symmetric_t>(sym_output, sym_sc_output_ref);
    }

    SUBCASE("Test fault on source bus") {
        // Grid for short circuit
        MathModelTopology topo_comp;
        topo_sc.slack_bus = 0;
        topo_comp.phase_shift = {0.0};
        topo_comp.branch_bus_idx = {};
        topo_comp.sources_per_bus = {from_sparse, {0, 1}};
        topo_comp.shunts_per_bus = {from_sparse, {0, 0}};
        topo_comp.load_gens_per_bus = {from_sparse, {0, 0}};
        DenseGroupedIdxVector const fault_buses_2 = {from_sparse, {0, 1}};
        // params source injection
        MathModelParam<asymmetric_t> asym_param_comp;
        asym_param_comp.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};
        MathModelParam<symmetric_t> sym_param_comp;
        sym_param_comp.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};
        // topo and param ptr
        auto topo_comp_ptr = std::make_shared<MathModelTopology const>(topo_comp);
        auto asym_param_comp_ptr = std::make_shared<MathModelParam<asymmetric_t> const>(asym_param_comp);
        auto sym_param_comp_ptr = std::make_shared<MathModelParam<symmetric_t> const>(sym_param_comp);
        YBus<asymmetric_t> const y_bus_asym{topo_comp_ptr, asym_param_comp_ptr};
        YBus<symmetric_t> const y_bus_sym{topo_comp_ptr, sym_param_comp_ptr};
        ShortCircuitSolver<asymmetric_t> solver{y_bus_asym, topo_comp_ptr};
        ShortCircuitSolver<symmetric_t> sym_solver{y_bus_sym, topo_comp_ptr};

        DoubleComplex const if_comp = vref / (zref + z_fault);
        DoubleComplex const uf_comp = vref - if_comp * zref;
        DoubleComplex const if_comp_solid = vref / (zref + z_fault_solid);
        DoubleComplex const uf_comp_solid = vref - if_comp_solid * zref;

        DoubleComplex const if_b_comp = (vref * (a * a - a)) / (2.0 * zref + z_fault);
        DoubleComplex const uf_b_comp = vref * a * a - if_b_comp * zref;
        DoubleComplex const uf_c_comp = vref * a + if_b_comp * zref;

        DoubleComplex const if_b_comp_solid = (vref * (a * a - a)) / (2.0 * zref + z_fault_solid);
        DoubleComplex const uf_b_comp_solid = vref * a * a - if_b_comp_solid * zref;
        DoubleComplex const uf_c_comp_solid = vref * a + if_b_comp_solid * zref;

        DoubleComplex const uf_b_2phg = (vref * (a * a + a)) * z_fault / (zref + 2.0 * z_fault);
        DoubleComplex const if_b_2phg = (vref * a * a - uf_b_2phg) / zref;
        DoubleComplex const if_c_2phg = (vref * a - uf_b_2phg) / zref;
        DoubleComplex const uf_b_2phg_solid = 0.0 + 0.0i;
        DoubleComplex const if_b_2phg_solid = vref * a * a / zref;
        DoubleComplex const if_c_2phg_solid = vref * a / zref;

        SUBCASE("Source on 3ph sym fault") {
            ShortCircuitSolverOutput<symmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {uf_comp};
            sc_output_ref.fault = {{if_comp}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{if_comp}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_buses_2);
            auto output = sym_solver.run_short_circuit(y_bus_sym, sc_input);
            assert_sc_output<symmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 3ph sym solid fault") {
            ShortCircuitSolverOutput<symmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {DoubleComplex{uf_comp_solid}};
            sc_output_ref.fault = {{if_comp_solid}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{if_comp_solid}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_buses_2);
            auto output = sym_solver.run_short_circuit(y_bus_sym, sc_input);
            assert_sc_output<symmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 3ph fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{uf_comp}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{if_comp}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{if_comp}}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 1phg fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{uf_comp, vref * a * a, vref * a}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{if_comp, 0, 0}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{if_comp, 0, 0}}};

            auto sc_input = create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 1phg solid fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{uf_comp_solid, vref * a * a, vref * a}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{if_comp_solid, 0, 0}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{if_comp_solid, 0, 0}}};

            auto sc_input =
                create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault_solid, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 2ph fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{vref, uf_b_comp, uf_c_comp}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{0.0, if_b_comp, -if_b_comp}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{0.0, if_b_comp, -if_b_comp}}};

            auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 2ph solid fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{vref, uf_b_comp_solid, uf_c_comp_solid}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{0.0, if_b_comp_solid, -if_b_comp_solid}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{0.0, if_b_comp_solid, -if_b_comp_solid}}};

            auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault_solid, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 2phg fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{vref, uf_b_2phg, uf_b_2phg}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{0.0, if_b_2phg, if_c_2phg}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{0.0, if_b_2phg, if_c_2phg}}};

            auto sc_input = create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }

        SUBCASE("Source on 2phg solid fault") {
            ShortCircuitSolverOutput<asymmetric_t> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<asymmetric_t>{vref, uf_b_2phg_solid, uf_b_2phg_solid}};
            sc_output_ref.fault = {{ComplexValue<asymmetric_t>{0.0, if_b_2phg_solid, if_c_2phg_solid}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<asymmetric_t>{0.0, if_b_2phg_solid, if_c_2phg_solid}}};

            auto sc_input =
                create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault_solid, vref, fault_buses_2);
            auto output = solver.run_short_circuit(y_bus_asym, sc_input);
            assert_sc_output<asymmetric_t>(output, sc_output_ref);
        }
    }
}

} // namespace power_grid_model::math_solver
