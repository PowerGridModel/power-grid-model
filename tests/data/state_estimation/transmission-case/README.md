<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

## Network Test Case: Transmission grid.

Test case representing a typical transmission grid.
Symmetrical voltage sensors are installed on all nodes and power sensors are installed at both ends of lines and 
transformers.

The circuit diagram is as follows (The node 6 is same in both lines):
```
    Gen(_21)---|_1--transformer(_30)--|_3--line(_12)-----|              |---line(_14)----------|
                                                         |--line(_13)---|                      |
                       Gen(_22)---|_2--transformer(_31)--|_4            |_5-------line(_15)----|_6
                                                                        |         shunt(_29)---|
                                                           shunt(_28)---|-load(_24)

    
    
    
    
    |----line(_16)-------|               |---line(_19)---|_9---transformer(_32)---|_11---source(_20)
    |                    |---line(_18)---|
    |_6-----line(_17)----|_7             |_8--transformer(_33)---|_10---gen(_23)
    |---load(_25)
```