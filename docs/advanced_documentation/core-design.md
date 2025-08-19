<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Design of the Power Grid Model core

The Power Grid Model at its core is a header-only C++ interface library, wrapped by a dynamic/shared C API library.
The core itself is an engine called the `MainModel` that provides the C++ interface and exhibits logic for the various
aspects that play a role in power grid calculations.
The `MainModel` itself can be deconstructed into an API part, a dispatch part, the grid model and the actual
[calculation logic](#calculation-logic-and-data-flow).

## Calculation logic and data flow

The logic involved in power grid calculations in turn can be devided in a number of separate modules.
Coincidentally, those phases also translate to fields of expertise, which enables a reasonably clean architecture.

| Logic/control module                               | Description                                                                                                       | Expertise              |
| -------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- | ---------------------- |
| I/O                                                | Constructing, updating and outputting components in the power grid                                                | Software Engineering   |
| Topology construction                              | Constructing the topological graph of the grid                                                                    | Topology               |
| $Y_{\text{bus}}$ construction/Component extraction | Constructing $Y_{\text{bus}}$ from the power grid components and topology                                         | Electrical Engineering |
| Solver construction/Grid extraction                | Translation from $Y_{\text{bus}}$ to a solvable system of equations and from the solution back to physical values | Physics                |
| Solving                                            | Abstract solution to the system of equations                                                                      | Mathematics            |

```{note}
Software Engineering obviously also plays a role in the general design, but that general design does not involve the
logic/control flow and therefore is not listed in this table.
```

The data flow can be visualized as such:

```{mermaid}
graph TD
    ComponentInput(Input/Update data) -->|Input| Components[Power Grid Components]
    Components -->|Topology construction| Topo[Topology]

    Topo -->|Ybus construction| Ybus(Ybus)
    Components --> Ybus

    Ybus -->|Solver construction/extraction| Equations(Solvable system of equations)
    Equations -->|Solving| Solution(Solution)

    Solution -->|Grid extraction| GridResult(Grid result)
    Ybus --> GridResult

    GridResult -->|Component extraction| ComponentsOutput(Components result)
    Components --> ComponentsOutput

    ComponentsOutput -->|Output| Output(Output data)
```

## Detailed Power Grid Model core design

The sheer size and complexity of the Power Grid Model core implementation makes it hard to generate an up-to-date and
comprehensive graph of its design.
For a full overview of the core, it is recommended to build and access the Power Grid Model core documentation by
following the steps in the [build guide](./build-guide.md#documentation).
