// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/edge.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/auxiliary/update.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/base.hpp>
#include <power_grid_model/component/component.hpp>

#include <doctest/doctest.h>

#include <complex>

namespace power_grid_model {

using namespace std::complex_literals;

namespace {
// Concrete test implementation of Edge for testing purposes
class TestEdge : public Edge {
  public:
    explicit TestEdge(BranchInput const& edge_input, double u_rated, DoubleComplex const& y_series,
                      DoubleComplex const& y_shunt, DoubleComplex const& tap_ratio = 1.0,
                      DoubleComplex const& y0_series = 0.0, DoubleComplex const& y0_shunt = 0.0)
        : Edge{edge_input},
          base_i_{base_power_3p / u_rated / sqrt3},
          y_series_{y_series},
          y_shunt_{y_shunt},
          tap_ratio_{tap_ratio},
          y0_series_{y0_series},
          y0_shunt_{y0_shunt} {}

    ComponentType math_model_type() const override { return ComponentType::branch; }

    double base_i_from() const override { return base_i_; }
    double base_i_to() const override { return base_i_; }
    double loading(double max_s, double /*max_i*/) const override { return max_s / base_power_3p; }
    double phase_shift() const override { return arg(tap_ratio_); }

  private:
    double base_i_;
    DoubleComplex y_series_;
    DoubleComplex y_shunt_;
    DoubleComplex tap_ratio_;
    DoubleComplex y0_series_;
    DoubleComplex y0_shunt_;

    BranchCalcParam<symmetric_t> sym_calc_param() const override {
        return calc_param_y_sym(y_series_, y_shunt_, tap_ratio_);
    }

    BranchCalcParam<asymmetric_t> asym_calc_param() const override {
        return calc_param_y_asym(y_series_, y_shunt_, y0_series_, y0_shunt_, tap_ratio_);
    }
};
} // namespace

TEST_CASE("Test edge") {
    BranchInput const input{.id = 1, .from_node = 2, .to_node = 3, .from_status = 1, .to_status = 1};

    double const u_rated = 10.0e3;
    double const base_i = base_power_3p / u_rated / sqrt3;
    double const base_y = base_i * base_i / base_power_1p;

    DoubleComplex const y_series = (1.0 / (0.3 + 0.4i)) / base_y;
    DoubleComplex const y_shunt = (50.0 * 2 * pi * 2e-4) * (0.1 + 1.0i) / base_y;
    DoubleComplex const tap_ratio = 1.0 + 0.0i;

    // For asymmetric
    DoubleComplex const y0_series = (1.0 / (0.1 + 0.2i)) / base_y;
    DoubleComplex const y0_shunt = (50.0 * 2 * pi * 1e-4) * (0.2 + 1.0i) / base_y;

    TestEdge edge{input, u_rated, y_series, y_shunt, tap_ratio, y0_series, y0_shunt};
    Edge& edge_ref = edge;

    // Expected symmetric values
    DoubleComplex const yff1 = y_series + 0.5 * y_shunt;
    DoubleComplex const yft1 = -y_series;
    DoubleComplex const ys1 = 0.5 * y_shunt + 1.0 / (1.0 / y_series + 2.0 / y_shunt);

    // Expected asymmetric values
    DoubleComplex const yff0 = y0_series + 0.5 * y0_shunt;
    DoubleComplex const yft0 = -y0_series;
    DoubleComplex const ys0 = 0.5 * y0_shunt + 1.0 / (1.0 / y0_series + 2.0 / y0_shunt);
    ComplexTensor<asymmetric_t> const yffa{(2.0 * yff1 + yff0) / 3.0, (yff0 - yff1) / 3.0};
    ComplexTensor<asymmetric_t> const yfta{(2.0 * yft1 + yft0) / 3.0, (yft0 - yft1) / 3.0};
    ComplexTensor<asymmetric_t> const ysa{(2.0 * ys1 + ys0) / 3.0, (ys0 - ys1) / 3.0};

    SUBCASE("Property Mapping") {
        CHECK(edge_ref.id() == 1);
        CHECK(edge_ref.from_node() == 2);
        CHECK(edge_ref.to_node() == 3);
        CHECK(edge_ref.from_status() == 1);
        CHECK(edge_ref.to_status() == 1);
        CHECK(Edge::name == std::string{"edge"});
    }

    SUBCASE("Status Flags") {
        CHECK(edge_ref.from_status() == 1);
        CHECK(edge_ref.to_status() == 1);
        CHECK(edge_ref.edge_status() == 1);

        edge_ref.set_status(1, 0);
        CHECK(edge_ref.edge_status() == 0);

        edge_ref.set_status(0, 1);
        CHECK(edge_ref.edge_status() == 0);

        edge_ref.set_status(0, 0);
        CHECK(edge_ref.edge_status() == 0);

        edge_ref.set_status(1, 1);
    }

    SUBCASE("Status Updates") {
        // Reset to known state
        edge_ref.set_status(1, 1);

        CHECK(!edge_ref.set_status(na_IntS, na_IntS));
        CHECK(edge_ref.from_status() == 1);
        CHECK(edge_ref.to_status() == 1);

        CHECK(edge_ref.set_status(0, na_IntS));
        CHECK(edge_ref.from_status() == 0);

        CHECK(!edge_ref.set_status(0, na_IntS));

        BranchUpdate const update{.id = 1, .from_status = 1, .to_status = na_IntS};
        UpdateChange const change = edge_ref.update(update);
        CHECK(change.topo == 1);
        CHECK(change.param == 1);
    }

    SUBCASE("Inverse State") {
        // Reset to known state
        edge_ref.set_status(1, 1);

        BranchUpdate const update{.id = 1, .from_status = 0, .to_status = 0};
        auto const inv = edge_ref.inverse(update);
        CHECK(inv.from_status == status_to_int(1));
        CHECK(inv.to_status == status_to_int(1));
    }

    SUBCASE("Energization") {
        edge_ref.set_status(1, 1);
        CHECK(edge_ref.energized(0) == 0);
        CHECK(edge_ref.energized(1) == 1);

        edge_ref.set_status(0, 1);
        CHECK(edge_ref.energized(1) == 1);

        edge_ref.set_status(0, 0);
        CHECK(edge_ref.energized(1) == 0);

        edge_ref.set_status(1, 1);
    }

    SUBCASE("Symmetric Parameters") {
        edge_ref.set_status(1, 1);

        // Not energized
        BranchCalcParam<symmetric_t> param = edge_ref.calc_param<symmetric_t>(false);
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);

        // Both disconnected
        edge_ref.set_status(0, 0);
        param = edge_ref.calc_param<symmetric_t>(true);
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);

        // From connected only
        edge_ref.set_status(1, 0);
        param = edge_ref.calc_param<symmetric_t>(true);
        CHECK(cabs(param.yff() - ys1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);

        // To connected only
        edge_ref.set_status(0, 1);
        param = edge_ref.calc_param<symmetric_t>(true);
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - ys1) < numerical_tolerance);

        // Both connected
        edge_ref.set_status(1, 1);
        param = edge_ref.calc_param<symmetric_t>(true);
        CHECK(cabs(param.yff() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - yft1) < numerical_tolerance);
        CHECK(cabs(param.yft() - yft1) < numerical_tolerance);
    }

    SUBCASE("Asymmetric Parameters") {
        edge_ref.set_status(1, 1);
        BranchCalcParam<asymmetric_t> param = edge_ref.calc_param<asymmetric_t>(true);

        CHECK((cabs(param.yff() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - yfta) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - yfta) < numerical_tolerance).all());

        edge_ref.set_status(1, 0);
        param = edge_ref.calc_param<asymmetric_t>(true);
        CHECK((cabs(param.yff() - ysa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
    }

    SUBCASE("Output Generation") {
        edge_ref.set_status(1, 1);
        BranchSolverOutput<symmetric_t> const solver_output{
            .s_f = 1.0 - 1.5i, .s_t = 1.5 - 1.5i, .i_f = 1.0 - 2.0i, .i_t = 2.0 - 1.0i};

        BranchOutput<symmetric_t> const output = edge_ref.get_output<symmetric_t>(solver_output);

        CHECK(output.id == 1);
        CHECK(output.energized == 1);
        CHECK(output.p_from == doctest::Approx(1.0 * base_power<symmetric_t>));
        CHECK(output.q_from == doctest::Approx(-1.5 * base_power<symmetric_t>));
        CHECK(output.i_from == doctest::Approx(cabs(1.0 - 2.0i) * base_i));
        CHECK(output.i_to == doctest::Approx(cabs(2.0 - 1.0i) * base_i));
    }

    SUBCASE("Short Circuit Output") {
        edge_ref.set_status(1, 1);
        DoubleComplex const if_sc{1.0, 1.0};
        DoubleComplex const it_sc{2.0, 2.0 * sqrt3};

        BranchShortCircuitOutput const sc_output = edge_ref.get_sc_output(if_sc, it_sc);
        CHECK(sc_output.id == 1);
        CHECK(sc_output.energized == 1);
        CHECK(sc_output.i_from(0) == doctest::Approx(cabs(if_sc) * base_i));
        CHECK(sc_output.i_to(0) == doctest::Approx(cabs(it_sc) * base_i));
        CHECK(sc_output.i_from_angle(0) == doctest::Approx(pi / 4));
        CHECK(sc_output.i_to_angle(0) == doctest::Approx(pi / 3));
    }

    SUBCASE("Null Outputs") {
        BranchOutput<symmetric_t> const null_output = edge_ref.get_null_output<symmetric_t>();
        CHECK(null_output.id == 1);
        CHECK(null_output.energized == 0);
        CHECK(null_output.loading == 0.0);
        CHECK(null_output.i_from == 0.0);

        BranchShortCircuitOutput const null_sc_output = edge_ref.get_null_sc_output();
        CHECK(null_sc_output.energized == 0);
        CHECK(null_sc_output.i_from(0) == 0.0);
    }

    SUBCASE("Enum Violations") {
        CHECK_THROWS_AS(edge_ref.node(static_cast<BranchSide>(2)), MissingCaseForEnumError);
        CHECK_THROWS_AS(edge_ref.status(static_cast<BranchSide>(2)), MissingCaseForEnumError);
    }
}

} // namespace power_grid_model
