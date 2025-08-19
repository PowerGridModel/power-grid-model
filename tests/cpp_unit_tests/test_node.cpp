// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/node.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Test node") {
    Node const node{{.id = 1, .u_rated = 10.0e3}};
    CHECK(node.math_model_type() == ComponentType::node);
    CHECK(node.u_rated() == 10.0e3);

    auto sym_res = node.get_output<symmetric_t>(1.0, 2.0);
    CHECK(sym_res.u == 10.0e3);
    CHECK(sym_res.u_angle == 0.0);
    CHECK(sym_res.u_pu == 1.0);
    CHECK(sym_res.p == 2.0e6);
    CHECK(sym_res.q == 0.0);
    CHECK(sym_res.id == 1);

    ComplexValue<asymmetric_t> u;
    ComplexValue<asymmetric_t> s;
    u << 1.0, a2, a;
    s << 0.0, DoubleComplex(2.1, 2.2), DoubleComplex(3.1, 3.2);
    DoubleComplex const u_sym = 1.0;
    auto asym_res = node.get_output<asymmetric_t>(u, s);
    CHECK(asym_res.u(1) == doctest::Approx(10.0e3 / sqrt3));
    CHECK(asym_res.u_angle(2) == doctest::Approx(-deg_240 + 2 * pi));
    CHECK(asym_res.u_pu(0) == doctest::Approx(1.0));
    CHECK(asym_res.p(1) == doctest::Approx(2.1e6 / 3.0));
    CHECK(asym_res.q(2) == doctest::Approx(3.2e6 / 3.0));

    auto sym_sc_res = node.get_sc_output(u_sym);
    auto asym_sc_res = node.get_sc_output(u);
    CHECK(asym_sc_res.u(1) == doctest::Approx(10.0e3 / sqrt3));
    CHECK(asym_sc_res.u_angle(2) == doctest::Approx(-deg_240 + 2 * pi));
    CHECK(asym_sc_res.u_pu(0) == doctest::Approx(1.0));
    CHECK(sym_sc_res.u(1) == doctest::Approx(asym_sc_res.u(1)));
    CHECK(sym_sc_res.u_angle(2) == doctest::Approx(asym_sc_res.u_angle(2)));
    CHECK(sym_sc_res.u_pu(0) == doctest::Approx(asym_sc_res.u_pu(0)));

    // not energized
    asym_res = node.get_null_output<asymmetric_t>();
    CHECK(asym_res.u(0) == 0.0);
    CHECK(asym_res.p(1) == 0.0);
    CHECK(asym_res.q(2) == 0.0);
    CHECK(!asym_res.energized);

    auto sc_res_null = node.get_null_sc_output();
    CHECK(sc_res_null.u(1) == 0.0);
    CHECK(sc_res_null.u_pu(2) == 0.0);
    CHECK(sc_res_null.u_angle(0) == 0.0);
    CHECK(!sc_res_null.energized);

    SUBCASE("Test energized function") {
        CHECK(node.energized(true));
        CHECK(!node.energized(false));
    }

    SUBCASE("Test node update") {
        BaseUpdate const base_update{};
        UpdateChange const update_change = Node::update(base_update);
        CHECK(update_change.topo == false);
        CHECK(update_change.param == false);
    }

    SUBCASE("Test update inverse") {
        BaseUpdate const base_update{1};
        auto expected = base_update;
        auto const inv = Node::inverse(base_update);
        CHECK(inv.id == expected.id);
    }
}

} // namespace power_grid_model
