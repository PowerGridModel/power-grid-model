<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Calculations

## Calculation types

With power-grid-model it is possible to perform three different types of calculations:

- [Power flow](#power-flow-algorithms): a "what-if" scenario calculation.
  This calculation can be performed by using the
  {py:class}`calculate_power_flow <power_grid_model.PowerGridModel.calculate_power_flow>` method.
  An example of usage of the power-flow calculation function is given in
  [Power flow Example](../examples/Power%20Flow%20Example.ipynb)
- [State estimation](#state-estimation-algorithms): a statistical method that calculates the most probabilistic state of
  the grid, given sensor values with an uncertainty.
  This calculation can be performed by using the
  {py:class}`calculate_state_estimation <power_grid_model.PowerGridModel.calculate_state_estimation>` method.
  An example of usage of the power-flow calculation function is given in
  [State Estimation Example](../examples/State%20Estimation%20Example.ipynb)
- [Short circuit](#short-circuit-calculation-algorithms): a "what-if" scenario calculation with short circuit entries.
  This calculation can be performed by using the
  {py:class}`calculate_short_circuit <power_grid_model.PowerGridModel.calculate_short_circuit>` method.

### Calculation types explained

#### Power flow

Power flow is a "what-if" based grid calculation that will calculate the node voltages and the power flow through the
branches, based on assumed load/generation profiles.
Some typical use-cases are network planning and contingency analysis.

Input:

- Network data: topology + component attributes
- Assumed load/generation profile

Output:

- Node voltage magnitude and angle
- Power flow through branches

See [Power flow algorithms](../algorithms/pf-algorithms.md#power-flow-algorithms) for detailed documentation on the
calculation methods.

##### Regulated power flow

For most power flow calculations, the grid is fixed as the user dictates.
However, in practice, the grid often contains regulators for certain components.
When including those regulators in the calculations, the grid may be optimized according to the power flow results and
the behaviour of the regulators.

See [Regulated power flow calculations](../algorithms/pf-algorithms.md#regulated-power-flow-calculations) for detailed
documentation on regulated
power flow calculations.

#### State estimation

State estimation is a statistical calculation method that determines the most probable state of the grid, based on
network data and measurements.
Here, measurements can be power flow or voltage values with certain kind of uncertainty, which were either measured,
estimated or forecasted.

Input:

- Network data: topology + component attributes
- Power flow / voltage measurements with uncertainty

Output:

- Node voltage magnitude and angle
- Power flow through branches
- Deviation between measurement values and estimated state

In order to perform a state estimation, the system should be observable.
If the system is not observable, the calculation will raise either a `NotObservableError` or a `SparseMatrixError`.
In short, meeting the requirement of observability indicates that the system is either an overdetermined system (when
the number of independent measurements is larger than the number of unknowns) or an exactly determined system (the
number of independent measurements equals the number of unknowns).
For each node, there are two unknowns, `u` and `u_angle`.
Due to the relative nature of `u_angle` (relevant only in systems with at least two nodes), in total the following
conditions should be met:

$$
n_{measurements} >= n_{unknowns}
$$

Where

$$
n_{unknowns} = 2 \cdot n_{nodes} - 1
$$

The number of measurements can be found by taking the sum of the following:

- number of nodes with a voltage sensor with magnitude only
- two times the number of nodes with a voltage sensor with magnitude and angle
- two times the number of nodes without appliances connected
- two times the number of nodes where all connected appliances are measured by a power sensor
- two times the number of branches with a power sensor and/or a current sensor

```{note}
Having enough measurements does not necessarily mean that the system is observable.
The location of the measurements is also of importance, i.e., the measurements should be topologically independent.
Additionally, there should be at least one voltage measurement.
```

```{note}
Global angle current measurements require at least one voltage angle measurement to make sense.
See also the [current sensor component documentation](./components.md#global-angle-current-sensors).
```

```{note}
It is not possible to mix [power sensors](./components.md#generic-power-sensor) with
[current sensors](./components.md#generic-current-sensor) on the same terminal of the same component.
It is also not possible to mix
[current sensors with global angle measurement type](./components.md#generic-current-sensor) with
[current sensors with local angle measurement type](./components.md#generic-current-sensor) on the same terminal of the
same component.
However, such mixing of sensor types is allowed as long as they are on different terminals.
```

```{warning}
The [iterative linear](../algorithms/se-algorithms.md#iterative-linear-state-estimation) and [Newton-Raphson](../algorithms/se-algorithms.md#newton-raphson-state-estimation) state
estimation algorithms will assume angles to be zero by default (see the details about voltage sensors).
In observable systems this helps better outputting correct results.
On the other hand with unobservable systems, exceptions raised from calculations due to faulty results will be
prevented.
```

##### Necessary observability condition

Based on the requirements of observability mentioned above, users need to satisfy at least the following conditions for
state estimation calculation in `power-grid-model`.

- `n_voltage_sensor >= 1`
- If no voltage phasor sensors are available, then both the following conditions shall be satisfied:
  - There are no global angle current sensors.
  - `n_unique_power_or_current_sensor >= n_bus - 1`.
- Otherwise (if there are voltage phasor sensors available, one will be reserved as reference),
the following condition shall be satisfied:
  - `n_unique_power_or_current_sensor + n_voltage_sensor_with_phasor - 1 >= n_bus - 1`

`n_unique_power_or_current_sensor` can be calculated as sum of following:

- Zero injection or zero power flow constraint if present for all nodes.
- Complete injections for all nodes: All appliances in a node are measured or a node injection sensor is present.
Either of them counts as one.
- Any sensor on a `Branch` for all branches: Parallel branches with either side of measurements count as one.
- All `Branch3` sensors.

##### Sufficient observability condition

The condition check above only checks the necessary condition for observability.
When the measurements are not independent enough, the system may still be unobservable even if the necessary condition
is met.
The `power-grid-model` performs the sufficient condition check on radial networks.
The `power-grid-model` performs the sufficient condition check on meshed networks without voltage phasor sensor.

The sufficient check is done by the topological approach of finding a full spanning tree with the available sensors
 at components in the network.
That is to say, if there exists a tree that visits all nodes in the network, the network is observable.

```{warning}
The handling of voltage phasor sensor in the context of observability check is still work in progress.
When there is more than 1 voltage phasor present in a meshed network, the sufficient check will pass the network
through to the state estimation calculation directly. 
```

In this case, the validation of the independent measurements is rather straightforward.
If the system is not observable, the calculation will raise a `NotObservableError` instead of `SparseMatrixError`.

#### Short circuit calculations

Short circuit calculation is carried out to analyze the worst case scenario when a fault has occurred.
The currents flowing through branches and node voltages are calculated.
Some typical use-cases are selection or design of components like conductors or breakers and power system protection,
e.g. relay co-ordination.

Input:

- Network data: topology + component attributes
- Fault type and impedance.
- In the API call: choose between `minimum` and `maximum` voltage scaling to calculate the minimum or maximum short
- circuit currents (according to IEC 60909).

Output:

- Node voltage magnitude and angle
- Current flowing through branches and fault.

```{note}
Short-circuit calculations are currently implemented in the phase (abc) domain and therefore require a grounded configurations in certain cases, similar to asymmetric power flow calculations.
For details on how floating grids are treated in power-grid-model, please refer to[Floating grid handling](calulations.md#floating-grid-handling).

#### Common calculations

Power flowing through a branch is calculated by voltage and current for any type of calculations in the following way:

$$
\underline{S_{branch-side}} = \sqrt{3} \cdot \underline{U_{LL-side-node}} \cdot \underline{I_{branch-side}}^*
$$

These quantities are in complex form.
Hence, they can be constructed by PGM output attributes in the following way:

- For  $\underline{U}$ of nodes, `u` is the magnitude and `u_angle` is the angle.
  Also the line to neutral voltage can be converted into line to line voltage by $ U_{LN} = U_{LL} / \sqrt{3}$.
  Check [Node Steady State Output](components.md#steady-state-output) to find out which quantity is relevant in your
  calculation.

- For  $\underline{I}$ of branches, `i_side` is the magnitude.
  Its angle can be found from `p_side` and `q_side` by:
  $\arctan(\frac{P_{side} + j \cdot Q_{side}}{\underline{U}})^{*}$.
  The `side` here can be `from`, `to` for [Branch](../user_manual/components.md#branch)es, `1`, `2`, `3` for
  [Branch3](../user_manual/components.md#branch3)s.

#### Symmetric vs asymmetric calculations

Power-grid-model can solve the grid either as a balanced single-phase equivalent (symmetric) or in full three-phase
detail (asymmetric).
The option affects which attributes are required and how results are exposed.

- **Symmetric calculations (`symmetric=True`, default):** Assume a perfectly balanced three-phase system so every phase
  shares the same voltage and current.
  The solver builds a positive-sequence network using `r1`, `x1`, `c1`, … parameters and collapses any asymmetric
  appliance to a single equivalent (asymmetric loads/generators are averaged across phases as described in
  [Component Type Hierarchy and Graph Data Model](./data-model.md#symmetry-of-components-and-calculation)).
  For symmetric calculations voltages are given as line-to-line and the output contains single values for all output
  variables.
- **Asymmetric calculations (`symmetric=False` or any non-three-phase fault):** Builds a full $abc$ nodal admittance
  matrix and solves each phase separately.
  Next to the positive-sequence parameters, the model now also needs the zero-sequence parameters (e.g. `r0`, `x0`,
  `c0`), or per-phase parameters (`r_matrix` & `x_matrix`) for `asym_line`; symmetric components are expanded by evenly
  splitting their totals across the three phases.
  For asymmetric calculations voltages are given as line-to-neutral and output contains arrays with values per phase
  for all output variables.

```{note}
In power-grid model, asymmetric calculations with certain configurations require the network to have a reference to ground. For details on how floating grids are treated in power-grid-model, please refer to[Floating grid handling](calulations.md#floating-grid-handling).
```

```{note}
For short-circuit calculations, a three-phase `fault_type` is calculated with a symmetric calculation, while any other
`fault_type` (e.g. single- or two-phase faults) automatically triggers the asymmetric calculation.
Outputs for short circuit calculations always give asymmetric output, independent of the fault type present.
```

#### Floating grid handling
In power-grid-model, two different concepts should be distinguished:

- Physical grounding of the network
  Whether the electrical system has an explicit path to ground (e.g. via transformer winding connection, shunts, or grounding elements).
- Numerical solvability in PGM
  Whether the formulation provides enough reference to compute a unique solution, even if the physical system is not explicitly grounded.

A floating grid issue only arises in specific configurations where:

- A transformer is present, and
- All involved windings are ungrounded (no star-point grounding, no delta grounding reference, etc.), and
- No other grounding path (shunt, source grounding, etc.) exists in the network.

In this case, the system may lack a reference for certain sequence components (typically zero-sequence), leading to an ill-posed or singular system.

In current power-grid-model, a shunt with small admittance is added only in transformer-related configurations where a grounding reference is missing. This shunt is connected to one side of the transformer. This is intended to ensure numerical stability in cases where the transformer topology introduces an ungrounded subsystem.

### Power flow algorithms

Two types of power flow algorithms are implemented in power-grid-model; iterative algorithms (Newton-Raphson / Iterative
current) and linear algorithms (Linear / Linear current).
Iterative methods converge to accurate solutions through multiple iterations
and should be selected when accurate results are required.
Linear approximation methods perform a single iteration and are significantly faster, but their accuracy depends on grid
conditions and may vary.
The table below summarizes the convergence characteristics and typical use cases for each algorithm to help you pick the
right one.

At the moment, the following power flow algorithms are implemented.

| Algorithm                                                                        | Speed                             | Result                            | Convergence         | Typical Use Cases                                                       | Algorithm call                                                                                             |
|--------------------------------------------------------------------------------- |---------------------------------- |---------------------------------- |-------------------- |------------------------------------------------------------------------ |----------------------------------------------------------------------------------------------------------- |
| [Newton-Raphson](../algorithms/pf-algorithms.md#newton-raphson-power-flow)       | Medium                            | Accurate within `error_tolerance` | Quadratic, robust   | General purpose, any type of grid                                       | {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>`      |
| [Iterative current](../algorithms/pf-algorithms.md#iterative-current-power-flow) | Fast (Radial) Slow (Meshed)       | Accurate within `error_tolerance` | Linear, less robust | Non-topological change batch calculations like timeseries, radial grids | {py:class}`CalculationMethod.iterative_current <power_grid_model.enum.CalculationMethod.iterative_current>`|
| [Linear](../algorithms/pf-algorithms.md#linear-power-flow)                       | Much Faster                       | Approximate                       | Single iteration    | Large number of calculations, troubleshooting iterative methods         | {py:class}`CalculationMethod.linear <power_grid_model.enum.CalculationMethod.linear>`                      |
| [Linear current](../algorithms/pf-algorithms.md#linear-current-power-flow)       | Much Faster                       | Approximate                       | Single iteration    | Large number of calculations                                            | {py:class}`CalculationMethod.linear_current <power_grid_model.enum.CalculationMethod.linear_current>`      |

```{note}
By default, the [Newton-Raphson](../algorithms/pf-algorithms.md#newton-raphson-power-flow) method is used.
```

```{note}
When all the load/generation types are of constant impedance, the [Linear](../algorithms/pf-algorithms.md#linear-power-flow) method will be the
fastest without loss of accuracy.
Therefore power-grid-model will use this method regardless of the input provided by the user in this case.
```

#### Quick decision guide for power flow algorithm

For detailed mathematical descriptions of each algorithm, see
[Power Flow Algorithm Details](../algorithms/pf-algorithms.md).

The choice of algorithm depends on your specific requirements for (non)convergence, accuracy, speed,
and grid configuration: radial or meshed.
Accuracy and convergence should be the first consideration, followed by speed.

Hence if speed is not critical or is a small concern, we recommend using the default
[Newton-Raphson](../algorithms/pf-algorithms.md#newton-raphson-power-flow) method for its robustness across all
scenarios.
If the scenarios are mainly timeseries, you can try
[Iterative current](../algorithms/pf-algorithms.md#iterative-current-power-flow)
, this method can improve speed significantly via
[Matrix prefactorization](performance-guide.md#matrix-prefactorization).
There is a possibility you can face non convergence or lower performance compared to the newton raphson method if the
network is not radial.

When speed becomes a major concern and desired performance is not achieved with the iterative methods, you can try to
explore linear methods.
It is recommended to limit the range of loading conditions when using linear methods to avoid unrealistic scenarios
where the approximations can give highly inaccurate results.

Overall, these methods are recommended only for a range of possible voltage deviations that are close to 1 p.u.
The linear current method will generally give better approximations than the linear method.
However at unrealistically high load levels it can give worse approximations than the linear method.
Check Power Flow Algorithm Comparison demonstration in
[Power Flow Algorithm Comparisons](https://github.com/PowerGridModel/power-grid-model-workshop/blob/main/demonstrations/Power%20Flow%20Algorithm%20Comparison.ipynb)
to know more about this behavior.
A strategy for post calculation verification of results is also provided there.
You can identify applicability of linear methods for your use case by experimenting with the Newton-Raphson method to
find the range of loading conditions that are relevant for your use case and then
only use linear methods within this range for the specific grid configuration.

Non convergence of newton raphson is a good signal of unpractical or unfeasible systems.
This signal can be ignored when using linear methods.
Similarly, having atleast some results from linear methods can aid in finding data errors or the reason
for non convergence of newton raphson method.

### Regulated power flow calculations

Regulated power flow calculations are disabled by default.

At the time of writing, the following regulated power flow calculation types are implemented.
Please refer to their respective sections for detailed documentation.

| Regulation type                                                   | Setting                                                                                 | Enum values                                                                 |
| ----------------------------------------------------------------- | --------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| [Automatic tap changing](#power-flow-with-automatic-tap-changing) | {py:meth}`tap_changing_strategy <power_grid_model.PowerGridModel.calculate_power_flow>` | {py:class}`TapChangingStrategy <power_grid_model.enum.TapChangingStrategy>` |

#### Power flow with automatic tap changing

Some of the most important regulators in the grid affect the tap position of transformers.
These [Transformer Tap Regulator](../user_manual/components.md#transformer-tap-regulator)s try to regulate
a control voltage
$U_{\text{control}}$ such that it is within a specified voltage band.
The $U_{\text{control}}$ may be compensated for the voltage drop during transport.
Power flow calculations that take the behavior of these regulators into account may be toggled by providing one of the
following strategies to the {py:meth}`tap_changing_strategy <power_grid_model.PowerGridModel.calculate_power_flow>`
option.

| Algorithm                                                                   | Default  | Speed    | Algorithm call                                                                                              |
| --------------------------------------------------------------------------- | -------- | -------- | ----------------------------------------------------------------------------------------------------------- |
| No automatic tap changing (regular power flow)                              | &#10004; | &#10004; | {py:class}`TapChangingStrategy.disabled <power_grid_model.enum.TapChangingStrategy.disabled>`               |
| Optimize tap positions for any value in the voltage band                    |          |          | {py:class}`TapChangingStrategy.any_valid_tap <power_grid_model.enum.TapChangingStrategy.any_valid_tap>`     |
| Optimize tap positions for lowest possible voltage in the voltage band      |          |          | {py:class}`TapChangingStrategy.min_voltage_tap <power_grid_model.enum.TapChangingStrategy.min_voltage_tap>` |
| Optimize tap positions for lowest possible voltage in the voltage band      |          |          | {py:class}`TapChangingStrategy.max_voltage_tap <power_grid_model.enum.TapChangingStrategy.max_voltage_tap>` |
| Optimize tap positions for any value in the voltage band with binary search |          | &#10004; | {py:class}`TapChangingStrategy.fast_any_tap <power_grid_model.enum.TapChangingStrategy.fast_any_tap>`       |

For detailed control logic, initialization behavior, search methods, and error handling for automatic tap changing,
see [Automatic Tap Changing Algorithm Details](../algorithms/tap-changing-algorithms.md).

### State estimation algorithms

Weighted least squares (WLS) state estimation can be performed with power-grid-model to evaluate the state variable with
the highest likelihood given (pseudo) measurement input.

At the moment, the following state estimation algorithms are implemented.

| Algorithm | Default | Speed | Accuracy | Algorithm call |
| --------- | ------- | ----- | -------- | -------------- |
| [Iterative linear](../algorithms/se-algorithms.md#iterative-linear-state-estimation) | &#10004; | &#10004; | | {py:class}`CalculationMethod.iterative_linear <power_grid_model.enum.CalculationMethod.iterative_linear>` |
| [Newton-Raphson](../algorithms/se-algorithms.md#newton-raphson-state-estimation) | | | &#10004; | {py:class}`CalculationMethod.newton_raphson <power_grid_model.enum.CalculationMethod.newton_raphson>` |

```{note}
By default, the [Iterative linear](../algorithms/se-algorithms.md#iterative-linear-state-estimation) method is used.
```

For detailed mathematical descriptions including the WLS formulation, measurement aggregation, sensor transformations,
and each algorithm, see [State Estimation Algorithm Details](../algorithms/se-algorithms.md).

### Short circuit calculation algorithms

In the short circuit calculation, specific equations are solved with border conditions of faults added as constraints to
determine the initial symmetrical short circuit current for a fault, which is then used to derive further calculations
of short circuit studies applications.

At the moment, the following short circuit algorithms are implemented.

| Algorithm | Default | Speed | Accuracy | Algorithm call |
| --------- | ------- | ----- | -------- | -------------- |
| [IEC 60909](../algorithms/sc-algorithms.md#iec-60909-short-circuit-calculation) | &#10004; | &#10004; | | {py:class}`CalculationMethod.iec60909 <power_grid_model.enum.CalculationMethod.iec60909>` |

For detailed mathematical descriptions including the short circuit equations and IEC 60909 implementation details,
see [Short Circuit Algorithm Details](../algorithms/sc-algorithms.md).

## Batch Calculations

Usually, a single power-flow or state estimation calculation would not be enough to get insights in the grid.
Any form of multiple calculations can be carried out in power-grid-model using batch calculations.
Batches are not restricted to any particular type of calculations, like timeseries or contingency analysis or their
combination.
They can be used for determining hosting/loading capacity, determining optimal tap positions, estimating system losses,
monte-carlo simulations or any other form of multiple calculations required in a power-flow study.
The framework for creating the batches is the same for all types of calculations.
For every component, the attributes that can be updated in a batch scenario are mentioned in
[Components](components.md).
Examples of batch calculations for timeseries and contingency analysis are given in
[Power Flow Example](../examples/Power%20Flow%20Example.ipynb)

The same method as for single calculations, {py:class}`power_grid_model.PowerGridModel.calculate_power_flow`, can be
used to calculate a number of scenarios in one go.
To do this, you need to supply an `update_data` keyword argument.
This keyword argument contains a dictionary of 2D update arrays (one array per component type).

The performance for different batches vary.
power-grid-model automatically makes efficient calculations whenever possible.
See the [Performance Guide](performance-guide.md#topology-caching) for ways to optimally use the performance
optimizations.

### Batch data set

The parameters of the individual scenarios within a batch can be done by providing deltas compared to the existing state
of the model.
The values of unchanged attributes and components parameters within a scenario may be implicit (like a delta update) or
explicit (similarly to how one would provide a full state).
In the context of the power-grid-model, these are called **dependent** (implicit) and **independent** (explicit) batch
updates, respectively.
In both cases, all scenario updates are relative to the state of the model before the call of the calculation.
See the examples below for usage.

- Dependent batches are useful for a sparse sampling for many different components, e.g. for N-1 checks.
- Independent batches are useful for a dense sampling of a small subset of components, e.g. time series power flow
  calculation.

See the [Performance Guide](performance-guide.md#using-independent-batches) for more suggestions.

#### Example: dependent batch update

```py
# 3 scenarios, 3 objects (lines)
# for each scenario, only one line is specified
line_update = initialize_array(DatasetType.update, ComponentType.line, (3, 1))

# set the mutations for each scenario: disable one of the three lines
for component_update, component_id in zip(line_update, (3, 5, 8)):
    component_update[AttributeType.id] = component_id
    component_update[AttributeType.from_status] = 0
    component_update[AttributeType.to_status] = 0

non_independent_update_data = {ComponentType.line: line_update}
```

#### Example: full batch update data

```py
# 3 scenarios, 3 objects (lines)
# for each scenario, all lines are specified
line_update = initialize_array(DatasetType.update, ComponentType.line, (3, 3))

# use broadcasting to specify the default state
line_update[AttributeType.id] = [[3, 5, 8]]
line_update[AttributeType.from_status] = 1
line_update[AttributeType.to_status] = 1

# set the mutations for each scenario: disable one of the three lines
for component_idx, scenario in enumerate(line_update):
    component = scenario[component_idx]
    component[AttributeType.from_status] = 0
    component[AttributeType.to_status] = 0

independent_update_data = {ComponentType.line: line_update}
```

### Cartesian product of Batch Datasets

Consider an example of running a contingency analysis with a timeseries data.
Or maybe probablistic data along with timeseries data.
In such simulations, it is required to perform a loadflow on a cartesian product of situations.
This is possible to do via providing the `update_data` with a list of multiple batch datasets.
ie. a list[{py:class}`BatchDataset <power_grid_model.data_types.BatchDataset>`]
The datasets can be of row based or columnar format.
The output of such calculation would be flattened with dimension $scenarios * components$.

#### Example: Cartesian product of datasets

```py
# 5 scenarios of timeseries
load_update = initialize_array(DatasetType.update, ComponentType.sym_load, (5, 1))
# (Fill load_update)
line_update = initialize_array(DatasetType.update, ComponentType.line, (3, 1))
# (Fill line_update)

product_update_data = [{ComponentType.line: load_update}, {ComponentType.sym_load: line_udpate }]
```

### Parallel Computing

The batch calculation supports shared memory multi-threading parallel computing.
The common internal states and variables are shared as much as possible to save memory usage and avoid copy.

You can set the `threading` keyword argument in the `calculate_*` functions (like
{py:class}`calculate_power_flow() <power_grid_model.PowerGridModel.calculate_power_flow>`) to enable/disable parallel
computing.

- `threading=-1`, use sequential computing (default)
- `threading=0`, use number of threads available from the machine hardware (recommended)
- `threading>0`, set the number of threads you want to use
