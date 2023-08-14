// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes meta data functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_META_DATA_H
#define POWER_GRID_MODEL_C_META_DATA_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get number of datasets
 *
 * @param handle
 * @return  The number of datasets
 */
PGM_API PGM_Idx PGM_meta_n_datasets(PGM_Handle* handle);

/**
 * @brief Get pointer of idx-th dataset
 *
 * @param handle
 * @param idx the sequence number, should be between [0, PGM_meta_n_datasets())
 * @return  The pointer to the idx-th dataset. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaDataset const* PGM_meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx);

/**
 * @brief Get pointer of dataset by name
 *
 * @param handle
 * @param dataset name of the dataset
 * @return  The pointer to the dataset with that name. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaDataset const* PGM_meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset);

/**
 * @brief Get name of the dataset
 *
 * @param handle
 * @param dataset pointer to a dataset object
 * @return The name of the dataset in a char const*. The pointer is permanantly valid.
 */
PGM_API char const* PGM_meta_dataset_name(PGM_Handle* handle, PGM_MetaDataset const* dataset);

/**
 * @brief Get the number of components for a dataset
 *
 * @param handle
 * @param dataset pointer to the dataset
 * @return  Number of components
 */
PGM_API PGM_Idx PGM_meta_n_components(PGM_Handle* handle, PGM_MetaDataset const* dataset);

/**
 * @brief Get pointer of idx-th component of a dataset
 *
 * @param handle
 * @param dataset pointer to the dataset
 * @param idx the sequence number, should be between [0, PGM_meta_n_components())
 * @return  The pointer to the idx-th component. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaComponent const* PGM_meta_get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                               PGM_Idx idx);
/**
 * @brief Get pointer of a component by name
 *
 * @param handle
 * @param dataset name of the dataset
 * @param component name of the component
 * @return  The pointer to the component with that name. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaComponent const* PGM_meta_get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                                char const* component);

/**
 * @brief Get name of component
 *
 * @param handle
 * @param component pointer to the component
 * @return  The name of the component in a char const*. The pointer is permanantly valid.
 */
PGM_API char const* PGM_meta_component_name(PGM_Handle* handle, PGM_MetaComponent const* component);

/**
 * @brief Get size of the component
 *
 * @param handle
 * @param component pointer to the component
 * @return  Size of the component.
 */
PGM_API size_t PGM_meta_component_size(PGM_Handle* handle, PGM_MetaComponent const* component);

/**
 * @brief Get alignment of the component
 *
 * @param handle
 * @param component pointer to the component
 * @return  Alignment of the component.
 */
PGM_API size_t PGM_meta_component_alignment(PGM_Handle* handle, PGM_MetaComponent const* component);

/**
 * @brief Get number of attributes of the component
 *
 * @param handle
 * @param component component pointer
 * @return  Number of attributes.
 */
PGM_API PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, PGM_MetaComponent const* component);

/**
 * @brief Get pointer of idx-th attribute of a component
 *
 * @param handle
 * @param component pointer to the component
 * @param idx the sequence number, should be between [0, PGM_meta_n_attributes())
 * @return  The pointer to the idx-th attribute. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaAttribute const* PGM_meta_get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                               PGM_Idx idx);
/**
 * @brief Get pointer of a attribute by name
 *
 * @param handle
 * @param dataset name of the dataset
 * @param component name of the component
 * @param attribute name of the attribute
 * @return  The pointer to the component with that name. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API PGM_MetaAttribute const* PGM_meta_get_attribute_by_name(PGM_Handle* handle, char const* dataset,
                                                                char const* component, char const* attribute);

/**
 * @brief Get attribute name
 *
 * @param handle
 * @param attribute pointer to attribute
 * @return  Name of the attribute in char const*. The pointer is permanantly valid.
 */
PGM_API char const* PGM_meta_attribute_name(PGM_Handle* handle, PGM_MetaAttribute const* attribute);

/**
 * @brief Get the type of an attribute
 *
 * @param handle
 * @param attribute pointer to attribute
 * @return  Type of the attribute as in enum PGM_CType.
 *
 */
PGM_API PGM_Idx PGM_meta_attribute_ctype(PGM_Handle* handle, PGM_MetaAttribute const* attribute);

/**
 * @brief Get the ofsset of an attribute in a component
 *
 * @param handle
 * @param attribute pointer to attribute
 * @return  Offset of this attribute.
 */
PGM_API size_t PGM_meta_attribute_offset(PGM_Handle* handle, PGM_MetaAttribute const* attribute);

/**
 * @brief Get if the system is little endian
 *
 * @param handle
 * @return  One if the system is litten endian. Zero if the system is big endian.
 */
PGM_API int PGM_is_little_endian(PGM_Handle* handle);

#ifdef __cplusplus
}
#endif

#endif
