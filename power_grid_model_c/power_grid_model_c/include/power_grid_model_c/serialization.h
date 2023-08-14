// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes serialization functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_SERIALIZATION_H
#define POWER_GRID_MODEL_C_SERIALIZATION_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

PGM_API PGM_Deserializer* PGM_create_deserializer_from_msgpack(PGM_Handle* handle, char const* data, PGM_Idx size);

PGM_API PGM_Deserializer* PGM_create_deserializer_from_json(PGM_Handle* handle, char const* json_string);

PGM_API void PGM_deserializer_set_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer, char const** components,
                                         void** data, PGM_Idx** indptrs);

PGM_API char const* PGM_deserializer_dataset_name(PGM_Handle* handle, PGM_Deserializer* deserializer);

PGM_API PGM_Idx PGM_deserializer_is_batch(PGM_Handle* handle, PGM_Deserializer* deserializer);

PGM_API PGM_Idx PGM_deserializer_batch_size(PGM_Handle* handle, PGM_Deserializer* deserializer);

PGM_API PGM_Idx PGM_deserializer_n_components(PGM_Handle* handle, PGM_Deserializer* deserializer);

PGM_API char const* PGM_deserializer_component_name(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                    PGM_Idx component_idx);

PGM_API PGM_Idx PGM_deserializer_component_elements_per_scenario(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                                 PGM_Idx component_idx);

PGM_API PGM_Idx PGM_deserializer_component_total_elements(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                          PGM_Idx component_idx);

PGM_API void PGM_deserializer_parse(PGM_Handle* handle, PGM_Deserializer* deserializer);

PGM_API void PGM_destroy_deserializer(PGM_Deserializer* deserializer);

#ifdef __cplusplus
}
#endif

#endif
