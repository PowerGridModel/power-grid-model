<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Design of the Power Grid Model core

The Power Grid Model at its core is a header-only C++ interface library, wrapped by a dynamic/shared C API library.
The core itself is an engine called the `MainModel` that provides the C++ interface and contains the logic for the various
aspects that play a role in power grid calculations.
The `MainModel` itself can be decomposed into an API part, a dispatch part, the grid model and the actual
[calculation logic](#calculation-logic-and-data-flow).

## Calculation logic and data flow

The logic involved in power grid calculations in turn can be divided in a number of separate modules.
Coincidentally, those phases also translate to fields of expertise, which enables a reasonably clean architecture.

| Logic/control module                               | Description                                                                                                       | Expertise              |
| -------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- | ---------------------- |
| I/O                                                | Constructing, updating, and outputting components in the power grid                                                | Software Engineering   |
| Electrical parameter construction | Constructing electrical parameters from the power grid components | Electrical Engineering |
| General topology construction                      | Constructing the overall topological layout of the grid, including open connections and disabled components       | Topology               |
| Topology reduction                                 | Splitting the general topological layout into a multi-scale topological representation by merging links on nodes  | Topology               |
| Mathematical topology construction                 | Constructing a graph representation of the reduced topology for efficient matrix solving                      | Topology               |
| $Y_{\text{bus}}$ construction | Constructing the $Y_{\text{bus}}$ from the electrical parameters and the mathematical topology                                         | Electrical Engineering |
| Solver construction/Grid extraction                | Translation from $Y_{\text{bus}}$ to a solvable system of equations and from the solution back to physical values | Physics                |
| Math solving                                       | Abstract solution to the macro-scale system of equations                                                          | Mathematics            |
| Topological node solving                           | Abstract solution to the micro-scale structure using the macro-scale solution                              | Mathematics            |

```{note}
Software Engineering obviously also plays a role in the general design, but that general design does not involve the
logic/control flow and therefore is not listed in this table.
```

The data flow can be visualized as such:

```{mermaid}
graph TD
    ComponentInput(Input/Update data) -->|Input| Components[Power Grid Components]

    Params[Electrical parameters]
    Components -->|Electrical parameter construction| Params
    Components -->|Static topology construction| GeneralTopo["General Topology (including disabled components)"]

    GeneralTopo -->|Topology reduction| ReducedTopo["Reduced Topology (split into topological nodes and substructures)"]

    ReducedTopo -->|Mathematical topology construction| MathTopo[Mathematical topology]

    MathTopo -->|Ybus construction| Ybus(Ybus)
    Params --> Ybus

    Ybus -->|Solver construction| Equations(Solvable system of equations)
    Equations -->|Math solving| Solution(Mathematical solution)

    Solution -->|Grid extraction| MacroGridResult(Macro-grid result)
    Ybus --> MacroGridResult

    MacroGridResult -->|Optional optimization| Params

    MacroGridResult -->|Topological node solving| FullGridResult("Full grid result")
    ReducedTopo --> FullGridResult

    FullGridResult -->|Component extraction| ComponentsOutput(Components result)
    Components --> ComponentsOutput

    ComponentsOutput -->|Output| Output(Output data)
```

## Detailed Power Grid Model core design

The sheer size and complexity of the Power Grid Model core implementation makes it hard to generate an up-to-date and
comprehensive graph of its design.
For a full overview of the core, it is recommended to build and access the Power Grid Model core documentation by
following the steps in the [build guide](./build-guide.md#documentation).
