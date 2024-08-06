# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""
from typing import TypeVar

from power_grid_model.core.dataset_definitions import ComponentType

ComponentTypeVar = TypeVar("ComponentTypeVar", bound=ComponentType | str)

_ComponentAttributeMappingDict = dict[ComponentTypeVar, set[str] | list[str] | None]

ComponentAttributeMapping = set[ComponentTypeVar] | list[ComponentTypeVar] | None | _ComponentAttributeMappingDict
