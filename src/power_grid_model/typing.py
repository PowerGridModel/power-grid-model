# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""
from power_grid_model.data_types import ComponentTypeLike

_ComponentAttributeMappingDict = dict[ComponentTypeLike, set[str] | list[str] | None]

ComponentAttributeMapping = set[ComponentTypeLike] | list[ComponentTypeLike] | None | _ComponentAttributeMappingDict
