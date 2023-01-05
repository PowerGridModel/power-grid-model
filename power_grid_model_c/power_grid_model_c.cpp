// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.h"

// thread handle
struct POWER_GRID_MODEL_Handle {
    Idx err_code;
    std::string err_msg;
};

// error handling
Idx err_code(POWER_GRID_MODEL_Handle const* handle) {
    return handle->err_code;
}
char const* err_msg(POWER_GRID_MODEL_Handle const* handle) {
    return handle->err_msg.c_str();
}
