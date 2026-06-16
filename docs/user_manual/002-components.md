<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Components

## Fault

* type name: `fault`
* base: [base](001-components.md#base)

`fault` defines a short circuit location in the grid.
A fault can only happen at a `node`.

#### Input

| name           | data type                                                 | unit    | description                                         |                                                required                                                 |  update  |   valid values    |
|----------------|-----------------------------------------------------------|---------|-----------------------------------------------------|:-------------------------------------------------------------------------------------------------------:|:--------:|:-----------------:|
| `status`       | `int8_t`                                                  | -       | whether the fault is active                         |                                                &#10004;                                                 | &#10004; |    `0` or `1`     |
| `fault_type`   | {py:class}`FaultType <power_grid_model.enum.FaultType>`   | -       | the type of the fault                               |                                     &#10024; only for short circuit                                     | &#10004; |                   |
| `fault_phase`  | {py:class}`FaultPhase <power_grid_model.enum.FaultPhase>` | -       | the phase(s) of the fault                           | &#10060; default `FaultPhase.default_value` (see [below](#fault-types-fault-phases-and-default-values)) | &#10004; |                   |
| `fault_object` | `int32_t`                                                 | -       | ID of the component where the short circuit happens |                                                &#10004;                                                 | &#10004; | A valid `node` ID |
| `r_f`          | `double`                                                  | ohm (Ω) | short circuit resistance                            |                                         &#10060; default `0.0`                                          | &#10004; |                   |
| `x_f`          | `double`                                                  | ohm (Ω) | short circuit reactance                             |                                         &#10060; default `0.0`                                          | &#10004; |                   |

```{note}
Multiple faults may exist within one calculation.
Currently, all faults in one scenario are required to have the same `fault_type` and `fault_phase`.
Across scenarios in a batch, the `fault_type` and `fault_phase` may differ.
```

```{note}
If any of the faults in any of the scenarios within a batch are not `three_phase` (i.e. `fault_type` is not
`FaultType.three_phase`), the calculation is treated as asymmetric.
```

#### Steady state output

A `fault` has no steady state output.

#### Short circuit output

| name        | data type         | unit       | description   |
|-------------|-------------------|------------|---------------|
| `i_f`       | `RealValueOutput` | ampere (A) | current       |
| `i_f_angle` | `RealValueOutput` | rad        | current angle |

#### Electric Model

##### Fault types, fault phases and default values

Four types of short circuit fault are included in power-grid-model, each with their own set of supported values for
`fault_phase`.
In case the `fault_phase` is not specified or is equal to `FaultPhase.default_value`, the power-grid-model assumes a
`fault_type`-dependent set of fault phases.
The supported values of `fault_phase`, as well as its default value, are listed in the table below.

| `fault_type`                       | supported values of `fault_phase`                 | `FaultPhase.default_value` | description                                                            |
|------------------------------------|---------------------------------------------------|----------------------------|------------------------------------------------------------------------|
| `FaultType.three_phase`            | `FaultPhase.abc`                                  | `FaultPhase.abc`           | Three phases are connected with fault impedance.                       |
| `FaultType.single_phase_to_ground` | `FaultPhase.a`, `FaultPhase.b`, `FaultPhase.c`    | `FaultPhase.a`             | One phase is grounded with fault impedance, and other phases are open. |
| `FaultType.two_phase`              | `FaultPhase.bc`, `FaultPhase.ac`, `FaultPhase.ab` | `FaultPhase.bc`            | Two phases are connected with fault impedance.                         |
| `FaultType.two_phase_to_ground`    | `FaultPhase.bc`, `FaultPhase.ac`, `FaultPhase.ab` | `FaultPhase.bc`            | Two phases are connected with fault impedance then grounded.           |

## Regulator

* type name: `regulator`
* base: [base](001-components.md#base)

`regulator` is an abstract regulator that is coupled to a given `regulated_object`. For each `regulator`, a switch is
defined between the `regulator` and the `regulated_object`.
Which object types are supported as `regulated_object` is regulator type-dependent.

#### Input

| name               | data type | unit | description                               | required |  update  |        valid values         |
|--------------------|-----------|------|-------------------------------------------|:--------:|:--------:|:---------------------------:|
| `regulated_object` | `int32_t` | -    | ID of the regulated object                | &#10004; | &#10060; | a valid regulated object ID |
| `status`           | `int8_t`  | -    | connection status to the regulated object | &#10004; | &#10004; |         `0` or `1`          |

### Transformer tap regulator

* type name: `transformer_tap_regulator`
* base: [regulator](001-components.md#regulator)

`transformer_tap_regulator` defines a regulator for transformers in the grid.
A transformer tap regulator regulates a component that is either a
[transformer](001-components.md#transformer) or a
[Three-Winding Transformer](001-components.md#three-winding-transformer).

The transformer tap regulator changes the `tap_pos` of the transformer it regulates in the range set by the user via
`tap_min` and `tap_max` (i.e., `(tap_min <= tap_pos <= tap_max)` or `(tap_min >= tap_pos >= tap_max)`).
It regulates the tap position so that the voltage on the control side is in the chosen voltage band.
Other points further into the grid on the control side, away from the transformer, can also be regulated by providing
the cumulative impedance across branches to that point as an additional line drop compensation.
This line drop compensation only affects the controlled voltage and does not have any impact on the actual grid.
It may therefore be treated as a virtual impedance in the grid.

```{note}
The regulator outputs the optimal tap position of the transformer.
The actual grid state is not changed after calculations are done.
```

#### Input

| name                       | data type                                                                                                                                                                                                                                                                                                                                                                                                 | unit      | description                                                                                             | required                      | update   | valid values |
|----------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------|---------------------------------------------------------------------------------------------------------|-------------------------------|----------|-------------|
| `control_side`             | {py:class}`BranchSide <power_grid_model.enum.BranchSide>` if the regulated object is a [transformer](001-components.md#transformer) and {py:class}`Branch3Side <power_grid_model.enum.Branch3Side>` if it is a [Three-Winding Transformer](001-components.md#three-winding-transformer)                                                                                                                | -         | the controlled side of the transformer                                                                  | ✨ only for power flow        | ❌       | `control_side` should be the relatively further side to a source |
| `u_set`                    | `double`                                                                                                                                                                                                                                                                                                                                                                                                  | volt (V)  | the voltage setpoint (at the center of the band)                                                       | ✨ only for power flow        | ✔️       | `>= 0` |
| `u_band`                   | `double`                                                                                                                                                                                                                                                                                                                                                                                                  | volt (V)  | the width of the voltage band ($=2*(\Delta U)_{\text{acceptable}}$)                                     | ✨ only for power flow        | ✔️       | `> 0` (see below) |
| `line_drop_compensation_r` | `double`                                                                                                                                                                                                                                                                                                                                                                                                  | ohm (Ω)   | compensation for voltage drop due to resistance during transport (see [below](#line-drop-compensation)) | ❌ default `0.0`             | ✔️       | `>= 0` |
| `line_drop_compensation_x` | `double`                                                                                                                                                                                                                                                                                                                                                                                                  | ohm (Ω)   | compensation for voltage drop due to reactance during transport (see [below](#line-drop-compensation))  | ❌ default `0.0`             | ✔️       | `>= 0` |

The following additional requirements exist on the input parameters.

* Depending on the resultant voltage being transformed, the voltage band must be sufficiently large: At zero line drop
  compensation if the expected resultant voltages are in the proximity of the rated transformer voltages, it is
  recommended to have the $u_{band} >= \frac{u_2}{1+ u_1 / \text{tap}_{\text{size}}}$
* The line drop compensation is small, in the sense that its product with the typical current through the transformer is
  much smaller (in absolute value) than the smallest change in voltage due to a change in tap position.
* The `control_side` of a transformer regulator should be on the relatively further side to a source in the connected
  grid.

These requirements make sure no edge cases with undefined behavior are encountered.
Typical real-world power grids already satisfy these requirements and they should therefore not cause any problems.

#### Steady state output

| name      | data type | unit | description          |
|-----------|-----------|------|----------------------|
| `tap_pos` | `int8_t`  | -    | optimal tap position |

#### Short circuit output

A `transformer_tap_regulator` has no short circuit output.

#### Electric Model

The transformer tap regulator itself does not have a direct contribution to the grid state.
Instead, it regulates the tap position of the regulated object until the voltage at the control side is in the specified
voltage band:

$$
U_{\text{control}} \in
    \left[U_{\text{set}} - \frac{U_{\text{band}}}{2}, U_{\text{set}} + \frac{U_{\text{band}}}{2}\right]
$$

##### Line drop compensation

The transformer tap regulator tries to regulate the voltage in a specified virtual location in the grid, according to
the folowing model.

```txt
tap_side   control_side                         part of grid where voltage is to be regulated
------^\oo -*---------------------virtual_impedance----*
      |    U_node, I_transformer       Z_comp       U_control
      |                                                |
     regulator <=======================================/
```

The control voltage is the voltage at the node, compensated with the voltage drop corresponding to the specified line
drop compensation.

$$
\begin{aligned}
    Z_{\text{compensation}} &= r_{\text{compensation}} + \mathrm{j} x_{\text{compensation}} \\
    U_{\text{control}} &= \left|\underline{U}_{\text{node}} - \underline{I}_{\text{transformer,out}}
                              \cdot \underline{Z}_{\text{compensation}}\right|
                        = \left|\underline{U}_{\text{node}} + \underline{I}_{\text{transformer}}
                              \cdot \underline{Z}_{\text{compensation}}\right|
\end{aligned}
$$

where $\underline{U}_{\text{node}}$ and $\underline{I}_{\text{transformer}}$ are the calculated voltage and current
phasors at the control side and may be obtained from a regular power flow calculation.
The plus sign in the last equality follows from canceling minus signs from the current direction convention and the
compensation direction.

For example, if we want to regulate the voltage at `load_7` in the following grid, the line drop compensation impedance
is the approximate impedance of `line_5`.

```txt
node_1 --- transformer_4 --- node_2 --- line_5 --- node_3
  |          |                                       |
source_6     |                                    load_7
      transformer_tap_regulator_8
```

### Voltage Regulator

* type name: `voltage_regulator`
* base: {hoverxreftooltip}`user_manual/components:regulator`

`voltage_regulator` defines a regulator for voltage-controlled generators in the grid.
A voltage regulator adjusts the reactive power output of a generator to maintain the voltage at its connection node
at a specified setpoint.

The voltage regulator changes the reactive power output of the generator it regulates to achieve the reference voltage
`u_ref` at the generator's node.
If `q_min` and `q_max` are provided, the reactive power is constrained within this range (i.e., `q_min <= q <= q_max` or
`q_min >= q >= q_max`).
If these limits are not provided, the reactive power can take any value needed to maintain the voltage setpoint.

```{warning}
Voltage regulation is only supported by the [Newton-Raphson power flow](./calculations.md#newton-raphson-power-flow) method.
```

```{note}
The `regulated_object` must reference a generator (`sym_gen` or `asym_gen`) or a load (`sym_load` or `asym_load`).
Each generator or load can have at most one voltage regulator.
When multiple voltage-regulated generators are connected to the same node, they should all specify the same `u_ref`
value to avoid conflicting voltage setpoints.
```

```{warning}
Reactive power limit checking is not yet fully implemented. When `q_min` and `q_max` are specified,
the intended behavior is that if the required reactive power to maintain `u_ref` exceeds these limits,
the voltage regulator should operate at the limit and the voltage may deviate from `u_ref`.
```

#### Input

| name     | data type | unit                       | description                                         |           required           |  update  | valid values |
| -------- | --------- | -------------------------- | --------------------------------------------------- | :--------------------------: | :------: | :----------: |
| `u_ref`  | `double`  | -                          | reference voltage in per-unit at the generator node | &#10024; only for power flow | &#10004; |    `> 0`     |
| `q_min`  | `double`  | volt-ampere-reactive (var) | minimum reactive power limit of the generator       | &#10060;                     | &#10004; |              |
| `q_max`  | `double`  | volt-ampere-reactive (var) | maximum reactive power limit of the generator       | &#10060;                     | &#10004; |              |

#### Steady state output

| name              | data type         | unit                       | description                                                          |
| ----------------- | ----------------- | -------------------------- | -------------------------------------------------------------------- |
| `q`               | `RealValueOutput` | volt-ampere-reactive (var) | reactive power provided by the voltage regulator                     |
| `limit_violated`  | `int8_t`          | -                          | reactive power limit violation indicator (not yet fully implemented) |

#### Short circuit output

A `voltage_regulator` has no short circuit output.

#### Electric Model

The voltage regulator controls the generator to behave as a **PV node** in power flow calculations:

$$
\begin{aligned}
    P_{\text{gen}} &= P_{\text{specified}} \\
    \left|U_{\text{node}}\right| &= U_{\text{ref}} \\
    Q_{\text{gen}} &= \text{calculated to satisfy } U_{\text{ref}}
\end{aligned}
$$

When `q_min` and `q_max` are provided, the reactive power should be constrained:

$$
   Q_{\text{min}} \leq Q_{\text{gen}} \leq Q_{\text{max}}
$$

When fully implemented, if the reactive power constraints are violated, the generator will operate at the limit and the
node becomes a PQ node:

$$
\begin{aligned}
    P_{\text{gen}} &= P_{\text{specified}} \\
    Q_{\text{gen}} &= Q_{\text{min}} \text{ or } Q_{\text{max}} \\
    \left|U_{\text{node}}\right| &= \text{calculated from power flow}
\end{aligned}
$$

In this case, `limit_violated` will indicate which limit was exceeded, and the actual voltage at the node may differ
from `u_ref`.
