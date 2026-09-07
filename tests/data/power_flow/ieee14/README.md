# Validation case: IEEE 14-bus system

This validation case models the public IEEE 14-bus power-flow test system as a symmetrical power-flow case for Power Grid Model (PGM).

## Source and scope

The source data are the IEEE 14-bus test case from the University of Washington Power Systems Test Case Archive, distributed in IEEE Common Data Format. The tabular values used for this conversion are the CC0 `caseformat/ieee14` representation converted from that archive.

- Original archive: https://labs.ece.uw.edu/pstca/pf14/ieee14cdf.txt
- Tabular conversion: https://github.com/caseformat/ieee14
- System base power: 100 MVA

The expected output validates bus voltage magnitudes. The source data publish voltage magnitudes to three decimal places, so the validation tolerance reflects that source precision.

## Conversion to PGM

The original case does not provide physical nominal bus voltages (`BASE_KV` is zero). A common nominal voltage of 100 kV is therefore used for all PGM nodes. This does not change the per-unit power-flow solution when all per-unit network quantities are converted consistently.

With a 100 MVA source base and 100 kV nominal voltage, the impedance base is 100 ohm. Branch series resistance and reactance are converted from per unit to ohm with this base. MATPOWER/IEEE branch charging susceptance is a total branch value, so half is assigned to each end through `generic_branch.b1`. Off-nominal transformer ratios are represented by `generic_branch.k`.

PQ demands are represented by `sym_load`. The slack bus is represented by a stiff `source` at bus 1. Generators at buses 2, 3, 6, and 8 are represented by `sym_gen` together with active `voltage_regulator` components so that Newton-Raphson treats those buses as voltage-controlled PV nodes. The fixed shunt at bus 9 is represented by a `shunt`.

Only voltage magnitude is included in `sym_output.json`; generator reactive power and branch power-flow values are intentionally outside the scope of this first validation case.
