# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""

from enum import IntEnum

from power_grid_model.core.dataset_definitions import ComponentType, ComponentTypeVar


class ComponentAttributeFilterOptions(IntEnum):
    """Filter option component or attribute"""

    ALL = 0
    """Filter all components/attributes"""
    RELEVANT = 1
    """Filter only non-empty components/attributes that contain non-NaN values"""


_ComponentAttributeMappingDict = dict[ComponentType, set[str] | list[str] | None | ComponentAttributeFilterOptions]

ComponentAttributeMapping = (
    set[ComponentTypeVar]
    | list[ComponentTypeVar]
    | ComponentAttributeFilterOptions
    | None
    | _ComponentAttributeMappingDict
)
