# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model raw dataset handler
"""

from typing import Dict

from power_grid_model.core.power_grid_core import DatasetInfoPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc


class DatasetInfo:  # pylint: disable=too-few-public-methods
    """
    Raw power grid model dataset info
    """

    name: str
    is_batch: bool
    batch_size: int
    component_count: Dict[str, int]
    """The amount of components per component type"""

    def __init__(self, info: DatasetInfoPtr):
        self._info: DatasetInfoPtr = info

        self.name = pgc.dataset_info_name(self._info)
        self.is_batch = bool(pgc.dataset_info_is_batch(self._info))
        self.batch_size = pgc.dataset_info_batch_size(self._info)

        n_components = pgc.dataset_info_n_components(self._info)
        self.component_count = {
            pgc.dataset_info_component_name(self._info, idx): pgc.dataset_info_total_elements(self._info, idx)
            for idx in range(n_components)
        }
