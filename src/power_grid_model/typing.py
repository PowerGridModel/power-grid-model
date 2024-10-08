# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""

from power_grid_model.core.dataset_definitions import ComponentType, ComponentTypeVar
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

- A set of `ComponentTypeVar`

- A list of `ComponentTypeVar`

- A `ComponentAttributeFilterOptions` value

- `None`

- A dictionary mapping `ComponentType` to a set, list, `None`, or `ComponentAttributeFilterOptions`
"""
