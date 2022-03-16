# Definitions 
The definitions of variables, their types are described in this section.

### power_grid_model.hpp
It contains the namespace declaration with math constants used throughout the code. 

### container.hpp
Container class data structure and related methods like get item/index, iterators, etc for the components of grid.

### calculation_parameters.hpp
Data structures used in creating topology are defined here for components types.

### component folder
Characteristics specific to each component and type are defined here.

### three_phase_tensor.hpp
Anything involving 3 phase calculations is mentioned here.

# Calculations

## main_model.hpp
- `add_component`: Adding components to the model. 
The types of components are mentioned in the `container.hpp` document.
- `update_component`: Update components parameters for next iteration of batch flow calculation
- `calculate_`: Internal method for calculating power flow. 
  - It prepares the solver `prepare_solvers`: Builds topology.
  - `prepare_input`:  (Description given, but means what?) 
Converts the input and topology data to a form which is used by calculation method: newton raphson or linear
  - Solve function: Directs to that calculation method
  - Returns output
- `calculate_power_flow` and `calculate_state_estimation`: 
The different variations of this function calls `calculate_` and other internal functions.


## newton_raphson_pf_solver.hpp
Implements newton raphson method. (Properly Commented, expand?)

## topology.hpp

The `calculate_` of main_model uses `build_topology` to build graph topology from input data. 
- It is rebuilt everytime there is a change in topology for batch calculations.
- The different connected topologies get classified in `group` 
while components are given a `pos` for their position in the graph. 
- -1 group indicates its isolated and is hence skipped from all calculations

`build_topology` is the main function which calls other secondary functions in the following order.

- `reset_topology`: Resets the values in topology
- `build_sparse_graph`: Goes into `sparse_mapping`: To explore in detail how reordering happens
- `dfs_search`: 
- `couple_...`: All active components are put together in the topology



## BSR solver
Uses MKL or eigen to do something.
Functions `analyzePattern` or `factorize` are called for eigen. 
For MKL, it is prefactorize, factorie and solve
(To describe them)
