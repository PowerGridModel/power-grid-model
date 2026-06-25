<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# PV-Node PQ-Switching Validation Test

Tests voltage regulator Q-limit enforcement and PV→PQ bus switching behavior.

## Network Topology

```
Source(10) ──line51── Gen(20) ──line52── Gen(30) ──line53── Load(40)
  u=1.015 pu          Gen 21            Gen 31              Variable Q
                      Reg 29            Gen 32 (asym)
                      q: [-25,27]       Reg 39: [-8,10]
                                        Reg 38: [-13,17]
```

## Test Scenarios

**Scenario 1: Single regulator q_max violation**
- Decrease Reg 29 q_max to 11 MVAr
- Expected: Node 20 switches to PQ, Reg 29 violates upper limit

**Scenario 2: Single regulator q_min violation (capacitive)**
- Increase slack voltage to 1.025 pu and increase Reg 29 q_min to -15 MVAr
- Expected: Node 20 switches to PQ, Reg 29 violates lower limit

**Scenario 3: Combined regulator violation at single bus**
- Decrease both Reg 39 and 38 q_max to 3 MVAr
- Expected: Node 30 switches to PQ, both regulators violate

**Scenario 4: Single regulator violation with compensation**
- Decrease only Reg 38 q_max to 3 MVAr
- Expected: Node 30 stays PV, only Reg 38 violates (Reg 39 compensates)

**Scenario 5: Selective violation (Node 30 only)**
- Decrease Node 30 q_max limits to 5 MVAr and increase load to 25 MVAr
- Expected: Node 30 switches to PQ, Node 20 stays PV

**Scenario 6: Effectively unlimited regulator**
- Set Reg 38 to +/- 100^100 MVAr (unlimited, can't set NaN) + increase load to 25 MVAr
- Expected: Node 30 stays PV, Reg 39 violates but Reg 38 provides capacity

**Scenario 7: One regulator disabled**
- Disable Reg 39
- Expected: Node 30 stays PV (Reg 38 controls alone)

**Scenario 8: Both regulators disabled**
- Disable both Reg 39 and 38
- Expected: Node 30 becomes PQ (no voltage control)

**Scenario 9: Simultaneous violations at multiple buses**
- Decrease all three regulator q_max limits (11, 5, 5 MVAr)
- Expected: Nodes 20 and 30 switch to PQ, all regulators violate