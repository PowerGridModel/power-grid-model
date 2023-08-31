# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model raw dataset handler
"""

from typing import Dict, List

from power_grid_model.core.power_grid_core import DatasetInfoPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc


class DatasetInfo:  # pylint: disable=too-few-public-methods
    """
    Raw power grid model dataset info
    """

    name: str
    is_batch: bool
    batch_size: int
    components: List[str]
    elements_per_scenario: Dict[str, int]
    total_elements: Dict[str, int]

    def __init__(self, info: DatasetInfoPtr):
        self._info: DatasetInfoPtr = info

        self.name = pgc.dataset_info_name(self._info)
        self.is_batch = bool(pgc.dataset_info_is_batch(self._info))
        self.batch_size = pgc.dataset_info_batch_size(self._info)

        n_components = pgc.dataset_info_n_components(self._info)
        self.components = [pgc.dataset_info_component_name(self._info, idx) for idx in range(n_components)]
        self.elements_per_scenario = {
            component_name: pgc.dataset_info_elements_per_scenario(self._info, idx)
            for idx, component_name in enumerate(self.components)
        }
        self.total_elements = {
            component_name: pgc.dataset_info_total_elements(self._info, idx)
            for idx, component_name in enumerate(self.components)
        }
