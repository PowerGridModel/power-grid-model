<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Model validation

power-grid-model is validated using multiple cases present in [\tests\data](..\..\tests\data) folder.
There are 2 simple grid test case examples of power-grid-model validated vision and gaia. 
A thorough validation is done using minimal test cases of each component and 2 test networks described in the following sections.

## Minimal test cases in pandapower

Their results are validated against the [pandapower](http://www.pandapower.org/) library.

The cases of a differences in modelling between both the libraries are handled by theoretical workarounds.
For example in power-grid-model, source impedance is included for all component sequences. 
In pandapower, source impedance is present only in positive sequence network whereas it considered in all sequence components in power-grid-model.
Source impedance is then set to a low value to match this modelling difference. 
Hence, the result of source component here should be ignored
All the test cases can be found in [\tests\data\power_flow\pandapower](..\..\tests\data\power_flow\pandapower)

### Node 

A node can have 2 states: energized and non-energized which is presented in the first grid.
The second grid example uses a line to validate node operation for voltages other than 0 or 1 p.u.
The circuit diagram for test cases of the 2 grids is as follows:

```{eval-rst}
.. tikz::
    :libs:  circuitikz
    :align: left
    \begin{circuitikz}
    \ (3,0) node[gridnode, anchor=south]{} to (3,-1);
    \draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
    \draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
    \end{circuitikz}
```

```{eval-rst}
.. tikz:: Node case
    :libs:  circuitikz
    :align: left
    \draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
    \draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
    \draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
```


### Line

A line can be 4 states, closed on both ends, open on both ends and open on any one end.
The circuit diagram for the test case is as follows:

![line](../images/validation/line.PNG)

```{eval-rst}
.. tikz:: Line case
    :libs: circuitikz
    :align: left
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
- Relaxed tolerance parameters are used in asymmetric calculation 
because only 'T' transformer model is available in pandapower while power-grid-model uses 'pi' model.
```
![transformer](../images/validation/transformer.PNG)

```{eval-rst}
.. tikz:: Transformer case
    :align: left
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
![shunt](../images/validation/shunt.PNG)

```{eval-rst}
.. tikz:: Shunt case
    :align: left
        \draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
        \draw (2.5,-1) [black, ultra thick] to ++(1,0);
        \draw (1.5,-3) [black, ultra thick] to ++(3,0);
        \draw (2,-3) to[ncs] ++(0,-1) to[C]  ++(0,-1.5) node[ground]{};
        \draw (4,-3) to[nos] ++(0,-1) to[C]  ++(0,-1.5) node[ground]{};
```

### Source

While source is present in all cases, this case tests two sources being used together.
![source](../images/validation/source.PNG)

```{eval-rst}
.. tikz:: Source case
    :align: left
        \draw (3,0) node[gridnode, anchor=south]{} to (3,-4) node[gridnode, anchor=north]{};
        \draw [black, ultra thick] (2.5,-1) -- (3.5,-1);
        \draw [black, ultra thick] (2.5,-2) -- (3.5,-2);
        \draw [black, ultra thick] (2.5,-3) -- (3.5,-3);
```

### Symmetrical Load

A symmetrical load can be in open or closed state. It can be of 3 types: constant power, constant impedance and constant current.
![sym_load](../images/validation/sym_load.PNG)

```{eval-rst}
.. tikz:: Symmetrical load case
    :align: left
    \draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
    \draw (2.5,-1) [black, ultra thick] to ++(1,0);
    \draw (0.5,-3) [black, ultra thick] to ++(5,0);
    \draw (1,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{P};
    \draw (2,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{Z};
    \draw (4,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{I};
    \draw (5,-3) to[nos] ++(0,-1) [very thick, ->] to ++(0,-1);
```


### Symmetrical generator

A symmetrical generator can be in open or closed state. It can be of 3 types: constant power, constant impedance and constant current.
![sym_gen](../images/validation/sym_gen.PNG)

```{eval-rst}
.. tikz:: Symmetrical generator case
    :align: left
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
![asym_load](../images/validation/asym_load.PNG)

```{eval-rst}
.. tikz:: Asymmetric Load case
    :align: left
    \draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
    \draw (2.5,-1) [black, ultra thick] to ++(1,0);
    \draw (1.5,-3) [black, ultra thick] to ++(3,0);
    \draw (2,-3) to[ncs] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{};
    \draw (4,-3) to[nos] ++(0,-1) [very thick, ->] to ++(0,-1) node[anchor=north]{};
```

### Component Test Case: Asymmetrical generator

An asymmetrical generator can be in open or closed state.
![asym_gen](../images/validation/asym_gen.PNG)

```{eval-rst}
.. tikz:: Asymmetric Generator case
    :align: left
    \draw (3,0) node[gridnode, anchor=south]{} to (3,-3);
    \draw (2.5,-1) [black, ultra thick] to ++(1,0);
    \draw (1.5,-3) [black, ultra thick] to ++(3,0);
    \draw (2,-3) to[ncs] ++(0,-1) to[vsourcesin]  ++(0,-1.5) node[ground]{};
    \draw (4,-3) to[nos] ++(0,-1) to[vsourcesin]  ++(0,-1.5) node[ground]{};
```

## Test case creation

Contribution to power-grid-model is also possible by creating own test cases in similar format. 
A guide for exporting the input or output data is given in [Make Test Dataset](..\examples\Make%20Test%20Dataset.ipynb).