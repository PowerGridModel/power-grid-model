## Component Test Case: Shunt

Test case for validation of shunt component for asymmetrical power flow calculations in pandapower.
- A shunt can be in 2 states: open or closed.

The circuit diagram is as follows:
```
 source_1--node_1--line_3--node_2--shunt_6      (status=1)
                           node_2--shunt_7      (status=0)
```

### Modelling incompatibility with pandapower

- Source impedance is set too low. Result of source component here should be ignored