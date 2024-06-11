// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "forward_declarations.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>

power_grid_model::meta_data::MetaData const& get_meta_data();
