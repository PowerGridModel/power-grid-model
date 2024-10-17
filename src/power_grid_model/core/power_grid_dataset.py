# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model raw dataset handler

[deprecated]
"""

from power_grid_model._core.power_grid_dataset import (  # pylint: disable=unused-import
    VALIDATOR_MSG,
    Any,
    AttributeType,
    BufferProperties,
    CAttributeBuffer,
    CBuffer,
    CConstDataset,
    CDatasetInfo,
    CMutableDataset,
    ComponentAttributeFilterOptions,
    ComponentAttributeMapping,
    ComponentData,
    ComponentMetaData,
    ComponentType,
    ConstDatasetPtr,
    CWritableDataset,
    Dataset,
    DatasetInfoPtr,
    DatasetMetaData,
    DatasetType,
    Mapping,
    MutableDatasetPtr,
    Optional,
    WritableDatasetPtr,
    assert_no_error,
    create_buffer,
    get_buffer_properties,
    get_buffer_view,
    get_dataset_type,
    is_columnar,
    is_nan_or_equivalent,
    pgc,
    power_grid_meta_data,
    process_data_filter,
)
