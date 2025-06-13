<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Model validation

The implementation of power-grid-model is validated using multiple test cases present in
{{ "[tests/data]({}/tests/data)".format(gh_link_head_tree) }} folder.
There are 2 simple grid test case examples of power-grid-model validated using Vision and GAIA.
A thorough validation is done using minimal test cases of each component and 2 test networks described in the following
sections.

## Minimal test cases in pandapower

Their results are validated against the [pandapower](http://www.pandapower.org/) library.

The cases of a differences in modelling between both the libraries are handled by theoretical workarounds.
For example in power-grid-model, source impedance is included for all component sequences.
In pandapower, source impedance is present only in positive sequence network whereas it considered in all sequence
components in power-grid-model.
Source impedance is then set to a low value to match this modelling difference.
Hence, the result of source component here should be ignored.
The output result attributes of power-grid-model are validated at a tolerance value of $\pm10^{-5}$ of respective unit.
Both the iterative algorithms: Newton Raphson and Iterative current are validated.

All the test cases can be found in
{{ "[/tests/data/power_flow/pandapower]({}/tests/data/power_flow/pandapower)".format(gh_link_head_tree) }}.

### Node

A node can have 2 states: energized and non-energized which is presented in the first grid.
The circuit diagram for the test case is as follows:

```{tikz}
:alt: basic node

\draw (3,0) node[gridnode, anchor=south]{} to (3,-1);
\draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
\draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
```

The second grid example uses a line to validate node operation for voltages other than 0 or 1 p.u.:

```{tikz}
:alt: node

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
\draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
```

### Line

A line can be 4 states, closed on both ends, open on both ends and open on any one end.
The circuit diagram for the test case is as follows:

```{tikz}
:alt: line

\draw (3,0) node[gridnode, anchor=south]{} to (3,-1);
\draw [black, ultra thick] (0.5,-1) -- (5.5,-1);
\draw (1,-1) [ncs] to (1,-2) -- (1,-4) [ncs] to (1,-5);
\draw (2,-1) [ncs] to (2,-2) -- (2,-4) [nos] to (2,-5);
\draw (4,-1) [nos] to (4,-2) -- (4,-4) [ncs] to (4,-5);
\draw (5,-1) [nos] to (5,-2) -- (5,-4) [nos] to (5,-5);
\draw [black, ultra thick] (0.5,-5) -- (5.5,-5);
\draw (3,-5) [very thick, ->] to (3,-6);
```

### Transformer

A transformer can be 4 states, closed on both ends, open on both ends and open on any one end.
The tap changing functionality is tested using a batch calculation for various tap positions.

```{note}
- Asymmetrical calculations are possible only for grounded network transformer in pandapower.
  Hence open cases are not evaluated.
- Relaxed tolerance parameters are used in asymmetric calculation because only 'T' transformer model is available in
  pandapower while power-grid-model uses 'pi' model.
```

```{tikz}
:alt: transformer

\draw (3,0) node[gridnode, anchor=south]{} to (3,-1);
\draw [black, ultra thick] (0.5,-1) -- (5.5,-1);
\draw (1,-1) [ncs] to (1,-2) [oosourcetrans] to (1,-4) [ncs] to (1,-5);
\draw (2,-1) [ncs] to (2,-2) [oosourcetrans] to (2,-4) [nos] to (2,-5);
\draw (4,-1) [nos] to (4,-2) [oosourcetrans] to (4,-4) [ncs] to (4,-5);
\draw (5,-1) [nos] to (5,-2) [oosourcetrans] to (5,-4) [nos] to (5,-5);
\draw [black, ultra thick] (0.5,-5) -- (5.5,-5);
\draw (3,-5) [very thick, ->] to (3,-6);
```

### Shunt

A shunt can be in 2 states: open or closed.

```{tikz}
:alt: shunt

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw (2.5,-1) [black, ultra thick] to ++(1,0);
\draw (1.5,-3) [black, ultra thick] to ++(3,0);
\draw (2,-3) to[ncs] ++(0,-1) to[C]  ++(0,-1.5) node[ground]{};
\draw (4,-3) to[nos] ++(0,-1) to[C]  ++(0,-1.5) node[ground]{};
```

### Source

While source is present in all cases, this case tests two sources being used together.

```{tikz}
:alt: source

\draw (3,0) node[gridnode, anchor=south]{} to (3,-4) node[gridnode, anchor=north]{};
\draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
\draw [black, ultra thick] (2.5,-2) -- (3.5,-2);
\draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
```

### Symmetrical Load

A symmetrical load can be in open or closed state.
It can be of 3 types: constant power, constant impedance and constant current.

```{tikz}
:alt: sym_load

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw (2.5,-1) [black, ultra thick] to ++(1,0);
\draw (0.5,-3) [black, ultra thick] to ++(5,0);
\draw (1,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{P};
\draw (2,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{Z};
\draw (4,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{I};
\draw (5,-3) to[nos] ++(0,-1) [very thick, ->] to ++(0,-1);
```

### Symmetrical generator

A symmetrical generator can be in open or closed state.
It can be of 3 types: constant power, constant impedance and constant current.

```{tikz}
:alt: sym_gen

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw (2.5,-1) [black, ultra thick] to ++(1,0);
\draw (0.5,-3) [black, ultra thick] to ++(5,0);
\draw (1,-3) to[ncs] ++(0,-1) to[vsourcesin, l_=P]  ++(0,-1.5) node[ground]{};
\draw (2,-3) to[ncs] ++(0,-1) to[vsourcesin, l=Z]  ++(0,-1.5) node[ground]{};
\draw (4,-3) to[ncs] ++(0,-1) to[vsourcesin, l_=I]  ++(0,-1.5) node[ground]{};
\draw (5,-3) to[nos] ++(0,-1) to[vsourcesin]  ++(0,-1.5) node[ground]{};
```

```{note}
Only constant power implementation is possible in pandapower for asymmetrical calculations.
All the Z, I and P loads are already validated for symmetrical calculation.
```

### Asymmetrical load

An asymmetrical load can be in open or closed state.

```{tikz}
:alt: asym_load

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw (2.5,-1) [black, ultra thick] to ++(1,0);
\draw (1.5,-3) [black, ultra thick] to ++(3,0);
\draw (2,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{};
\draw (4,-3) to[nos] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{};
```

### Asymmetrical generator

An asymmetrical generator can be in open or closed state.

```{tikz}
:alt: asym_gen

\draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
\draw (2.5,-1) [black, ultra thick] to ++(1,0);
\draw (1.5,-3) [black, ultra thick] to ++(3,0);
\draw (2,-3) to[ncs] ++(0,-1) to[vsourcesin]  ++(0,-1.5) node[ground]{};
\draw (4,-3) to[nos] ++(0,-1) to[vsourcesin]  ++(0,-1.5) node[ground]{};
```

## Distribution network case

This is a minimal case representing a simple distribution grid.
The grid has 2 identical parallel transformers.
They power a series of overhead lines and cables which supply different loads.

The case is validated for ring and radial configuration by open/close position of
one end of Line 13 in asymmetrical batch calculation.

The circuit diagram is as follows:

```txt
                                                        asym_load(22)       sym_load(19)
                                                        |                   |
            |------trafo(17)------|-----cable(9)-----|(3)----cable(11)---|(5)----OHL(13)---(On/off)-|
source(16)--|(1)                  |(2)                                                              |(7)---OHL(15)-|(8)
            |------trafo(18)------|----cable(10)-----|(4)----cable(12)---|(6)----OHL(14)------------|              |
                                                        |                   |                               Load(21)
                                                        asym_load(23)       sym_load(20)
```

## Transmission network case

This is a minimal case representing a simple transmission grid.

The circuit diagram is as follows (The node 6 is same in both lines):

```txt
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

## Vision validation case

There are 2 test grid cases included for validation against vision: A minimal example and a network containing all
components supported by power-grid-model-io.
Their Vision files are included as well.

### Simple example

The `vision-example` is a minimal case with only node, source, cable and load.

### Network case

The Vision files were exported to excel which was then converted to power-grid-model input using
[power-grid-model-io](https://github.com/PowerGridModel/power-grid-model-io).
The `vision-network` case has the following characteristics:

- It contains 26 nodes (plus 20 from transformer load secondary node).
- The voltage level of grid input is at 110kV from which it is stepped down to 10.5kV level.
- On the 10.5kV level, one minimal distribution grid containing transformer loads, wind and PV generation and one
  additional reactance coil.
- All the remaining supported components for which conversion to power-grid-model is supported are also connected to
  this level.
  They include: line, reactance, special transformer, load, synchronous generator, shunt and zig-zag transformer.

The cases are built taking into consideration the modelling differences between Vision and power-grid-model mentioned in
the [power-grid-model-io documentation](https://power-grid-model-io.readthedocs.io/).
The node voltages and branch power flows are validated for symmetrical calculation.
For asymmetrical output only the result attributes being validated are the ones which can be exported to excel. (i.e.,
node voltages and branch currents)
The absolute tolerances here are set to the least count of the Vision result export: i.e., till V and kW level.

## Short Circuit Calculation cases

The short circuit calculations are validated against manual calculations based on electrical engineering theory.
The test grid is designed to encompass most grid configurations of short circuit that might be interesting.

The test grid is as follows:

```{tikz}
:alt: short circuit case

\draw [red, ultra thick] (4,7) -- (4,2);
\draw (0,6) node[gridnode, anchor=east]{} to (1,6);
\draw [black, ultra thick] (1,6.5) -- (1,5.5);
\draw (1,6) [oosourcetrans] to (4,6);

\draw (4,6) [ncs, color=blue] to (5,6); 
\draw (5,6) [oosourcetrans] to (7,6) node[ground] {};

\draw (4,3) to (12,3);
\draw [red, ultra thick] (8,3.5) -- (8,2);

\draw (7,0) node[gridnode, anchor=north]{} to (7,2.5) [ncs, color=green] to (8,2.5);
\draw [red, ultra thick] (12,3.5) -- (12,2.5);
```

There are 4 cases for the 4 types of fault: three_phase, single_phase_ground, two_phase, two_phase_ground.
Each case is tested for `minimum` and `maximum` voltage scaling.
Each case has multiple scenarios.
They are combinations of following situations:

- Valid phase combinations: abc, a, b, c, ab, bc, ac
- Source switched on or off (highlighted in green)
- A shunt with only `b0` value modelled to be a grounding transformer switched on or off.
  (highlighted in blue)
- Fault locations (highlighted in red)
- Fault impedance: hard ground or with impedance.

## Other library validation

There is one example of a simple Gaia network with nodes, lines, source and loads being validated.
A similar example is also recreated for validation with the R state estimation package.

## Test case creation

Contribution to power-grid-model is also possible by creating own test cases in similar format.
A guide for exporting the input or output data is given in [Make Test Dataset](../examples/Make%20Test%20Dataset.ipynb).
