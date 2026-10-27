<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# PGM-automatic-tap-disabled

This test case captures the bug reported in https://github.com/PowerGridModel/power-grid-model/issues/1653.

It tests the following scenarios:

| base status | updated                      | result                               | remarks                                                                                 |
| ----------- | ---------------------------- | ------------------------------------ | --------------------------------------------------------------------------------------- |
| status 0    |                              | base scenario (regulator disabled)   | baseline                                                                                |
| status 0    | status 0                     | regulator disabled (base scenario)   | related                                                                                 |
| status 0    | u_set set to base value      | regulator disabled (base scenario)   | the failing test case in https://github.com/PowerGridModel/power-grid-model/issues/1653 |
| status 0    | u_set set to different value | regulator disabled (base scenario)   | the failing test case in https://github.com/PowerGridModel/power-grid-model/issues/1653 |
| status 0    | status 1                     | updated scenario (regulator enabled) | the reference "wrong" scenario                                                          |

1. starting from automatic tap regulator status 0, not updating it yields the base scenario (automatic tap regulator disabled)
2. starting from automatic tap regulator status 0, updating the status 0 yields the equivalent result (automatic tap regulator disabled)
3. starting from automatic tap regulator status 0, updating any other field keeps the automatic (automatic tap regulator disabled)