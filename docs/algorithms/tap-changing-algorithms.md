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

The automatic tap changer iterates over all regulators with automatic tap changing control.
The control logic is described as follows:

- For each regulator, calculate the controlled voltage $U_{\text{control}}$ based on `control_side`.
- If the transformer has a `tap_pos` which is not regulated and within `tap_min` and `tap_max` bounds, skip it and
  consider the next regulator.
  In all other cases, go to the next step.
- If $U_{\text{control}}$ is outside the bounds, the tap position changes:
  - If $U_{\text{control}} < u_{\text{set}} - u_{\text{band}} / 2$, the tap position is incremented by 1 (or according
    to the {py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>` chosen).
  - If $U_{\text{control}} > u_{\text{set}} + u_{\text{band}} / 2$, the tap position is decremented by 1 (or according
    to the {py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>` chosen).
  - If the updated `tap_pos` reaches `tap_min` or `tap_max`, it is clamped to those values.
- Another power flow calculation is run with new tap positions for the regulated transformers.
- If the maximum number of tap changing iterations is reached, the calculation stops and raises an error of type
  `MaxIterationReached`.

## Initialization and exploitation of regulated transformers

The initialization and exploitation of different regulated transformer types are unique and depend on whether the
regulator is automatic.
The following table specifies the behaviour of regulated transformers based on the
{py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>` chosen and whether `tap_pos` is regulated.

|TapChangingStrategy|tap_pos is regulated?|Initialization phase|Exploitation phase|
|---|---|---|---|
|{py:class}`any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`|Yes|Set the tap position to `tap_nom` (default middle tap position)|Binary search (default)|
|{py:class}`any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`|No|Clamp the tap position to stay within `tap_min` and `tap_max`|Do not regulate|
|{py:class}`min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` / {py:class}`max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>`|Yes|Set the tap position to `tap_min` or `tap_max` respectively|Do not regulate|
|{py:class}`min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` / {py:class}`max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>`|No|Clamp the tap position to stay within `tap_min` and `tap_max`|Do not regulate|

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

By default, the algorithm uses binary search to find the optimal tap position to control voltage within the set band.
Users can also choose other search methods via
{py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>`:

|{py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>`|Description|
|---|---|
|{py:class}`fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`|Very fast search (single step per iteration)|
|{py:class}`any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`|Binary search (default)|
|{py:class}`min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` / {py:class}`max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>`|Set to extreme tap and do not regulate|

## Regulatable voltage range outside `u_band`

Since the regulated transformer has only a limited number of tap positions, it cannot always attain the target voltage
(i.e., the controlled voltage $U_{\text{control}}$ may remain outside the set band).
The possible scenarios are:

- For `tap_min`: When increasing the tap position from `tap_min` in successive power flow calculations, the voltage
  might still be below the lower bound of the set band.
- For `tap_max`: When decreasing the tap position from `tap_max` in successive power flow calculations, the voltage
  might still be above the upper bound of the set band.

## Error type `MaxIterationReached`

If the algorithm uses binary search, since the number of tap positions is often limited to tens of taps, binary search
will find the optimal tap positions within a small number of iterations, typically under 10.
When linear search is used, however, we avoid convergence if the search continues to iterate in an oscillatory fashion.
However, the following could be a cause for `MaxIterationReached`:

- **If all `tap_pos` are within `tap_min` and `tap_max` and converged to the targeted band**:
  - The tap changing control has found a control point inside the band.
  - The calculation may be at a saddle point, and further iteration may not result in achieving values which are closer
    to the target voltage.
  - This is often expected behaviour.
- **If no voltage control could find tap positions inside** `u_band`:
  - A transformer is constantly oscillating between two tap positions depending on directions of measurement errors.
    In this case, it's difficult for our algorithm to choose the more optimal tap position for controlled voltage.
- **If a few voltages are out of band**: The algorithm keeps oscillating due to constraint saturation at `tap_max` or
  `tap_min`.
  The regulated transformers at saturation cannot regulate further, although the voltage is outside the band.
  This is often expected behaviour.
- **If voltages are inside the band but `tap_pos` are outside `tap_min` and `tap_max` bounds**: This is a logical
  violation.
  It means the clamping inside the algorithm is not functioning.
  This is a bug and should be reported.
