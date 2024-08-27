# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""

from types import EllipsisType

from power_grid_model.core.dataset_definitions import ComponentType, ComponentTypeVar

_ComponentAttributeMappingDict = dict[ComponentType, set[str] | list[str] | None | EllipsisType]

ComponentAttributeMapping = (
    set[ComponentTypeVar] | list[ComponentTypeVar] | EllipsisType | None | _ComponentAttributeMappingDict
)
