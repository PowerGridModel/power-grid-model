# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Type hints for PGM. This includes all miscellaneous type hints not under dataset or dataset_definitions categories
"""
from typing import Dict, List, Optional, Set, Union

from power_grid_model.core.dataset_definitions import ComponentType

_OutputComponentTypeDict = Dict[ComponentType, Optional[Union[Set[str], List[str]]]]

OutputComponentNamesType = Optional[Union[Set[ComponentType], List[ComponentType], _OutputComponentTypeDict]]
