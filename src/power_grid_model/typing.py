# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for the power-grid-model library.

This includes all miscellaneous type hints not under dataset or categories.
"""

from power_grid_model._core.dataset_definitions import (  # pylint: disable=unused-import
    ComponentType,
    ComponentTypeVar,
    DatasetType,
    DatasetTypeVar,
)
from power_grid_model._core.power_grid_meta import (  # pylint: disable=unused-import
    ComponentMetaData,
    DatasetMetaData,
    PowerGridMetaData,
)
from power_grid_model.enum import ComponentAttributeFilterOptions

_ComponentAttributeMappingDict = dict[ComponentType, set[str] | list[str] | None | ComponentAttributeFilterOptions]

ComponentAttributeMapping = (
    set[ComponentTypeVar]
    | list[ComponentTypeVar]
    | ComponentAttributeFilterOptions
    | None
    | _ComponentAttributeMappingDict
)
"""
Type hint for mapping component attributes.

`ComponentAttributeMapping` can be one of the following:

- A set of :class:`ComponentType` or `str`

- A list of :class:`ComponentType` or `str`

- A :class:`ComponentAttributeFilterOptions <power_grid_model.enum.ComponentAttributeFilterOptions>` value

- `None`

- A dictionary mapping :class:`ComponentType` to a set, list, `None` or
  :class:`ComponentAttributeFilterOptions <power_grid_model.enum.ComponentAttributeFilterOptions>`
"""
