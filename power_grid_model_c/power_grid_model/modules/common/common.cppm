// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/common.hpp>

export module power_grid_model.common.common;

export namespace power_grid_model {
// index types
using power_grid_model::ID;
using power_grid_model::Idx;
using power_grid_model::Idx2D;
using power_grid_model::Idx2DHash;
using power_grid_model::IdxVector;
using power_grid_model::IntS;

// symmetry
using power_grid_model::asymmetric_t;
using power_grid_model::decode_symmetry_v;
using power_grid_model::is_asymmetric_v;
using power_grid_model::is_symmetric_v;
using power_grid_model::other_symmetry_t;
using power_grid_model::symmetric_t;
using power_grid_model::symmetry_tag;
using power_grid_model::underlying_value_t;

// math constants
using power_grid_model::a;
using power_grid_model::a2;
using power_grid_model::deg_120;
using power_grid_model::deg_240;
using power_grid_model::deg_30;
using power_grid_model::DoubleComplex;
using power_grid_model::inv_sqrt3;
using power_grid_model::na_Idx;
using power_grid_model::na_IntID;
using power_grid_model::na_IntS;
using power_grid_model::nan;
using power_grid_model::numerical_tolerance;
using power_grid_model::pi;
using power_grid_model::sqrt3;

// power grid constants
using power_grid_model::base_power;
using power_grid_model::base_power_1p;
using power_grid_model::base_power_3p;
using power_grid_model::default_source_rx_ratio;
using power_grid_model::default_source_sk;
using power_grid_model::default_source_z01_ratio;
using power_grid_model::g_link;
using power_grid_model::ideal_source_sk;
using power_grid_model::transformer_low_susceptance_ratio;
using power_grid_model::u_scale;
using power_grid_model::y_link;

// vectors
using power_grid_model::ComplexVector;
using power_grid_model::DoubleVector;
using power_grid_model::IntSVector;

// concepts and utilities
using power_grid_model::by_const_ref;
using power_grid_model::by_ref;
using power_grid_model::derives_from_any_in_list_c;
using power_grid_model::functor_c;
using power_grid_model::include_all;
using power_grid_model::IncludeAll;
using power_grid_model::is_in_list_c;
using power_grid_model::map_to_cyclic_range;
using power_grid_model::non_owning_view_c;
using power_grid_model::owning_container_c;

namespace capturing {
using power_grid_model::capturing::into_the_void;
} // namespace capturing

namespace detail {
using power_grid_model::detail::is_owning_view;
} // namespace detail

using namespace std::complex_literals;
} // namespace power_grid_model
