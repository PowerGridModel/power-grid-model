# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for for library-internal use.
"""

from power_grid_model._core.dataset_definitions import ComponentAttribute, ComponentType, ComponentTypeVar
from power_grid_model._core.enum import ComponentAttributeFilterOptions

ComponentAttributeMappingDict = dict[
    ComponentType, set[ComponentAttribute] | list[ComponentAttribute] | None | ComponentAttributeFilterOptions
]

ComponentAttributeMapping = (
    set[ComponentTypeVar]
    | list[ComponentTypeVar]
    | ComponentAttributeFilterOptions
    | None
    | ComponentAttributeMappingDict
)
