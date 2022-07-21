<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
## Network Test Case: Distribution grid 

Test case representing a typical distribution grid. 
The grid has 2 identical parallel transformers. 
They power a series of overhead lines and cables which supply different loads.

Symmetrical voltage sensors are installed on all nodes and power sensors are installed at both ends of lines and 
transformers.

The case is validated for ring and radial configuration by open/close position of 
one end of Line 13 in symmetrical batch calculation.

The circuit diagram is as follows:
```
                                                        Load(22)            Load(19)
                                                        |                   |
            |------trafo(17)------|-----cable(9)-----|(3)----cable(11)---|(5)----OHL(13)---(On/off)-|
source(16)--|(1)                  |(2)                                                              |(7)---OHL(15)-|(8)
            |------trafo(18)------|----cable(10)-----|(4)----cable(12)---|(6)----OHL(14)------------|              |
                                                        |                   |                               Load(21)
                                                        Load(23)            Load(20)
```
