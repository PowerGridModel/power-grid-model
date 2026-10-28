<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Automatic Tap Changing Algorithm Details

This page provides detailed algorithmic descriptions of the automatic tap changing optimization procedure implemented in
power-grid-model.
Automatic tap changing is an outer-loop optimization procedure that iteratively adjusts transformer tap positions and
runs [power flow calculations](pf-algorithms.md) to achieve desired voltage control objectives.

For an overview of regulated power flow and strategy selection, see
[Regulated power flow with automatic tap changing](
  ../user_manual/calculations.md#power-flow-with-automatic-tap-changing).

## Control logic for power flow with automatic tap changing

The optimizer ranks regulated transformers by their distance from sources, measured by the number of regulated
transformers between them. Transformers with conflicting ranks are placed last. Transformers in the same rank can be
regulated independently.

For each optimization iteration, the optimizer runs a power flow calculation and visits the regulators in rank order.
It processes all regulators in the same rank independently:

- Calculate the controlled voltage $U_{\text{control}}$ using `control_side`.
- If $U_{\text{control}} < u_{\text{set}} - u_{\text{band}} / 2$, adjust the tap by one step to increase the
  controlled voltage. If $U_{\text{control}} > u_{\text{set}} + u_{\text{band}} / 2$, adjust it by one step to
  decrease the controlled voltage.
  The tap-position direction depends on the transformer configuration: increasing the controlled voltage moves the tap
  toward `tap_max` when the control side is the tap side, and toward `tap_min` otherwise. Decreasing the controlled
  voltage reverses that direction.
- Keep the tap position within `tap_min` and `tap_max`.
- If any tap position changes in a rank, stop before processing lower-priority ranks, recalculate power flow, and start
  again from the highest-priority rank. If no tap position changes in a rank, continue to the next rank. Repeat until
  no tap positions change.

For `min_voltage_tap` and `max_voltage_tap`, the optimizer also explores neighboring tap positions after the initial
search, then performs another optimization pass using linear search.

If an iterative power flow calculation diverges or fails with a sparse-matrix error, the optimizer retries the tap
optimization with a linear power flow calculation, then runs it again with the selected calculation method.

The control logic assumes that voltage changes are dominated by the tap position of the regulated transformer rather
than by adjacent transformers. The control side should be farther from a source than the tap side, and line-drop
compensation should be small compared with the voltage change caused by one tap step. A large compensation impedance
combined with generator-like behavior at the control side can violate these assumptions and cause the calculation to
diverge. See [Transformer Tap Regulator](../user_manual/components.md#transformer-tap-regulator) and
[Line drop compensation](../user_manual/components.md#line-drop-compensation) for the input requirements.

## Initialization and exploitation of regulated transformers

The selected strategy and the relationship between the control side and tap side determine the starting tap position
and how the optimizer searches for a suitable position:

|{py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>`|Initial tap position|Search and exploitation|
|---|---|---|
|{py:class}`any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`|Keep the current tap position|Linear search; no neighborhood exploitation|
|{py:class}`fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`|Keep the current tap position|Binary search; no neighborhood exploitation|
|{py:class}`min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>`|Start at the tap position that gives the lowest controlled voltage (`tap_min` if control side is tap side, otherwise `tap_max`)|Binary search, then linear neighborhood exploitation|
|{py:class}`max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>`|Start at the tap position that gives the highest controlled voltage (`tap_max` if control side is tap side, otherwise `tap_min`)|Binary search, then linear neighborhood exploitation|

Some transformer configurations require clamping due to `regulated_object` and `tap_side` pointing to different sides of
the transformer.
For a three-winding transformer the requirements are similar to a regular transformer with the additional consideration
that the primary side tap changer regulates only the secondary or tertiary side voltages:

|Tapping on side|Regulated object|Voltage control side|`tap_side`|`regulated_object`|`control_side`|
|---|---|---|---|---|---|
|HV (0)|LV (1)|LV (1)|`BranchSide.from_side` / `Branch3Side.side_1`|`TransformerTapSide.side_1` / `Branch3Side.side_2`|`BranchSide.to_side` / `Branch3Side.side_2`|
|HV (0)|LV (1)|HV (0)|`BranchSide.from_side` / `Branch3Side.side_1`|`TransformerTapSide.side_1` / `Branch3Side.side_2`|`BranchSide.from_side` / `Branch3Side.side_1`|
|LV (1)|HV (0)|LV (1)|`BranchSide.to_side` / `Branch3Side.side_2`|`TransformerTapSide.side_2` / `Branch3Side.side_1`|`BranchSide.to_side` / `Branch3Side.side_2`|
|LV (1)|HV (0)|HV (0)|`BranchSide.to_side` / `Branch3Side.side_2`|`TransformerTapSide.side_2` / `Branch3Side.side_1`|`BranchSide.from_side` / `Branch3Side.side_1`|

## Search methods used for tap changing optimization

The default search method depends on the selected strategy. Linear search checks adjacent tap positions one at a time;
binary search reduces the remaining tap range by half at each iteration.

|{py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>`|Default search method|Objective|
|---|---|---|
|{py:class}`any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`|Linear search|Find any tap position that puts the controlled voltage within the band|
|{py:class}`fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`|Binary search|Find any tap position that puts the controlled voltage within the band|
|{py:class}`min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>`|Binary search, then linear neighborhood search|Find the lowest controlled voltage within the band|
|{py:class}`max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>`|Binary search, then linear neighborhood search|Find the highest controlled voltage within the band|

## Regulatable voltage range outside `u_band`

Since the regulated transformer has only a limited number of tap positions, it cannot always attain the target voltage
(i.e., the controlled voltage $U_{\text{control}}$ may remain outside the set band).
Because tap direction and controlled-voltage direction depend on whether the control side is the tap side, a tap limit
may be reached while the controlled voltage remains below or above the set band.

## Tap size larger than `u_band`

In cases where the `u_band` is set to be unrealistically small, e.g., smaller than `tap_size`, chances are that the optimizer
can not land a tap position to produces a voltage within this `u_band`. In this case, all search strategies will return with a closest tap position
that it can achieve.

If the voltage band is narrower than one tap step and two adjacent tap positions fall on opposite sides of it, the
optimizer settles at one of those positions instead of alternating between them. The `any_valid_tap` and
`fast_any_tap` strategies may choose either position; the minimum- and maximum-voltage strategies choose the adjacent
position with the lower and higher controlled voltage, respectively.

## Error type `MaxIterationReached`

The optimizer raises `MaxIterationReached` if regulators in a rank continue changing tap positions beyond twice the
largest tap range in that rank. This limit prevents an endless search, for example when discrete tap steps cause
regulators to oscillate without settling. Reaching the limit indicates that the optimizer could not find a stable tap
configuration within the allowed range.
