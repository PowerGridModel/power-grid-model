<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Network

```txt
   Source11
   | Gen12[pq] <-x- VolReg19 Gen21[pq] <- VoltReg29  AsymGen31[pq] <- VoltReg39
   | | Load13[pq]            | Load22[y]             | Load32[pq] <- VoltReg38
   | | | Shunt14             | | Shunt23             | | AsymLoad33[pq]
   | | | |                   | | |                   | | |
  Node10-220 o-----OO-----o Node20-110 o----71----o Node30-110
                   81         o    o                       o
                              |     \                      |
                              |      \                     |
                              72      \-----73-----\       74
                              |                     \      |
                              |                      \     |
                              o                       o    o         82
                            Node40-110 o----75----o Node50-110 o-----OO-----o Node60-380
                             | | |                   | | |                     x | | |
                             | | Shunt43             | | Shunt53               | | | Shunt64
                             | Gen42[i] <- VoltReg48 | Shunt52                 | | Load63[y]
                             Gen41[pq] <- VoltReg49  Load51[pq]                | Gen62[pq] <- VoltReg69
                                                                               Source61
```
