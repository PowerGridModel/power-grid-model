<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Model validation

power-grid-model is validated using multiple cases present in {{ "[tests/data]({}/tests/data)".format(gh_link_head_tree) }} folder.
There are 2 simple grid test case examples of power-grid-model validated vision and gaia. 
A thorough validation is done using minimal test cases of each component and 2 test networks described in the following sections.

## Minimal test cases in pandapower

Their results are validated against the [pandapower](http://www.pandapower.org/) library.

The cases of a differences in modelling between both the libraries are handled by theoretical workarounds.
For example in power-grid-model, source impedance is included for all component sequences. 
In pandapower, source impedance is present only in positive sequence network whereas it considered in all sequence components in power-grid-model.
Source impedance is then set to a low value to match this modelling difference.
Hence, the result of source component here should be ignored.
The output result attributes of power-grid-model are validated at a tolerance value of $\pm10^{-5}$ of respective unit.
Both the iterative algorithms: Newton Raphson and Iterative current are validated.

All the test cases can be found in {{ "[/tests/data/power_flow/pandapower]({}/tests/data/power_flow/pandapower)".format(gh_link_head_tree) }}. 

### Node 

A node can have 2 states: energized and non-energized which is presented in the first grid.
The second grid example uses a line to validate node operation for voltages other than 0 or 1 p.u.
The circuit diagram for test cases of the 2 grids is as follows:

```{image} ../images/validation/basic_node.svg
:alt: basic node
:width: 100px
:align: center
```
```{image} ../images/validation/node.svg
:alt: node
:width: 100px
:align: center
```

### Line

A line can be 4 states, closed on both ends, open on both ends and open on any one end.
The circuit diagram for the test case is as follows:
```{image} ../images/validation/line.svg
:alt: line
:width: 250px
:align: center
```

### Transformer

A transformer can be 4 states, closed on both ends, open on both ends and open on any one end.
The tap changing functionality is tested using a batch calculation for various tap positions.

```{note}
- Asymmetrical calculations are possible only for grounded network transformer in pandapower. 
Hence open cases are not evaluated.
- Relaxed tolerance parameters are used in asymmetric calculation 
because only 'T' transformer model is available in pandapower while power-grid-model uses 'pi' model.
```
```{image} ../images/validation/transformer.svg
:alt: transformer
:width: 250px
:align: center
```

### Shunt

A shunt can be in 2 states: open or closed.
```{image} ../images/validation/shunt.svg
:alt: shunt
:width: 250px
:align: center
```

### Source

While source is present in all cases, this case tests two sources being used together.
```{image} ../images/validation/source.svg
:alt: Source
:width: 250px
:align: center
```

### Symmetrical Load

A symmetrical load can be in open or closed state. It can be of 3 types: constant power, constant impedance and constant current.
```{image} ../images/validation/sym_load.svg
:alt: sym_load
:width: 250px
:align: center
```

### Symmetrical generator

A symmetrical generator can be in open or closed state. It can be of 3 types: constant power, constant impedance and constant current.
```{image} ../images/validation/sym_gen.svg
:alt: sym_gen
:width: 250px
:align: center
```

```{note}
Only constant power implementation is possible in pandapower for asymmetrical calculations. 
All the Z, I and P loads are already validated for symmetrical calculation.
```

### Asymmetrical load

An asymmetrical load can be in open or closed state. 
```{image} ../images/validation/asym_load.svg
:alt: asym_load
:width: 250px
:align: center
```

### Asymmetrical generator

An asymmetrical generator can be in open or closed state.
```{image} ../images/validation/asym_gen.svg
:alt: asym_gen
:width: 250px
:align: center
```

### Distribution network case

This is a minimal case representing a simple distribution grid. 
The grid has 2 identical parallel transformers. 
They power a series of overhead lines and cables which supply different loads.

The case is validated for ring and radial configuration by open/close position of 
one end of Line 13 in asymmetrical batch calculation.

The circuit diagram is as follows:
```
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


## Vision validation

There are 2 test grid cases included for validation against vision: A minimal example and a network containing all components supported by power-grid-model-io
Their vision files are included as well.

### Simple example

The `vision-example` is a minimal case with only node, source, cable and load.

### Netowork case

The vision files were exported to excel which was then converted to power-grid-model input using [power-grid-model-io](https://github.com/alliander-opensource/power-grid-model-io).
The `vision-network` case has the following characteristics:
- It contains 26 nodes (plus 20 from transformer load secondary node). 
- The voltage level of grid input is at 110kV from which it is stepped down to 10.5kV level. 
- On the 10.5kV level, one minimal distribution grid containing transformer loads, wind and PV generation and one additional reactance coil. 
- All the remaining supported components for which conversion to power-grid-model is supported are also connected to this level.
They include: line, reactance, special transformer, load, synchronous generator, shunt and zig-zag transformer.

The cases are built taking into consideration the modelling differences between vision and power-grid-model mentioned in the power-grid-model-io documentation(https://power-grid-model-io.readthedocs.io/).
The node voltages and branch power flows are validated for symmetrical calculation.
For asymmetrical output only the result attributes being validated are the ones which can be exported to excel. (ie. node voltages and branch currents)
The absolute tolerances here are set to the least count of the vision result export: till ie. till V and kW level.

## Other library validation

There is one example of a simple Gaia network with nodes, lines, source and loads being validated.
A similar example is also recreated for validation with the R state estimation package.

## Test case creation

Contribution to power-grid-model is also possible by creating own test cases in similar format. 
A guide for exporting the input or output data is given in [Make Test Dataset](../examples/Make%20Test%20Dataset.ipynb).