<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Python API Reference

This document explains the Python API of `power-grid-model`.
The Python API consists of the following components:

* `power_grid_model.power_grid_meta_data`: containing meta-data information.
  This is inside a C++ extension module.
* `power_grid_model.initialize_array`: helper function to initialize array.
  This is inside a C++ extension module.
* `power_grid_model.enum`: containing enumeration types.
  This is a pure Python module.
* `power_grid_model.PowerGridModel`: the main class.
  This is inside a C++ extension module.
* `power_grid_model.file_io`: containing the functions for load and save test dataset.
  See [Make Test Dataset](../examples/Make%20Test%20Dataset.ipynb) for examples of how to make test datasets.
* `power_grid_model.validation`: optional validation and assertion functions.
  See [Validation Examples](../examples/Validation%20Examples.ipynb) for more information on how to validate input 
  and update datasets.

# Enumerations

Please refer the [Graph Data Model](graph-data-model.md#enumerations) for list of enumerations.

# Meta Data

## Meta Data Dictionary

The dictionary `power_grid_model.power_grid_meta_data` contains all the data types
which are used for exchange between Python and C++ code.
The structure of the dictionary is shown as follows.
The detailed information about component types and attributes is shown in
[Graph Data Model](graph-data-model.md).

```
1st level
key: DATA_TYPE, one of the following: input, update, sym_output, asym_output
value: dictionary of this data type
    2nd level
    key: COMPONENT_TYPE, one of the supported component types
    value: dictionary of this component type
        3rd level, several individual keys
        'dtype': predefined numpy structured dtype for this component for this data type
        'dtype_dict': the internal dictionary to build this numpy.dtype
        'nan_scalar': a single object of this numpy dtype, with all values as null value
```

**NOTE: DO NOT modify the dictionary `power_grid_meta_data`. You will get serious memory leak or crash.**


The code below creates an array which is compatible with transformer input dataset.

```python
from power_grid_model import power_grid_meta_data
import numpy as np

transformer = np.empty(shape=5, dtype=power_grid_meta_data['input']['transformer']['dtype'])
```

## Helper Function

To improve the convenience of initializing an array with given shape and data format,
one can use the helper function `power_grid_model.initialize_array`.
The full signature is as follows:

 ```python
from typing import Union

def initialize_array(data_type: str, component_type: str, shape: Union[tuple, int]):
    """

    Args:
        data_type: input, update, sym_output, or asym_output
        component_type: one component type, e.g. node
        shape: shape of initialization
            for integer, it is 1D array
            for tuple, it is a ND array

    Returns:
        np structured array with all entries as null value
    """
    pass
```

The code below initializes a symmetric load update array with a shape of `(5, 4)`.

```python
from power_grid_model import initialize_array

line_update = initialize_array('update', 'sym_load', (5, 4))
```

# Main Class

The main class of this library is `power_grid_model.PowerGridModel`,
it is defined in cython as a C++ extension type and it wraps the C++ calculation core.

## Initialization

The initialization of a `PowerGridModel` can be done by calling

* The constructor of `PowerGridModel` for a completely new model.
* The `PowerGridModel.copy` to copy an existing model. You can also use the standard `copy.copy` function.

Take a look at [this example](../scripts/quick_example.py) to get a better idea how to initialize.

## Functionality

Below is a Python-friendly description on the functions and their respective signatures.
For implementation details look at [the source](../src/power_grid_model/_power_grid_core.pyx).

```python
import numpy as np
from typing import Dict, Optional, Union
from power_grid_model import CalculationMethod


class PowerGridModel:

    def __init__(self,
            input_data: Dict[str, np.ndarray],
            system_frequency=50.0,
        ):
        """

        Initialize the model from an input data set.
        Args:
            input_data: input data dictionary
                key: component type name
                value: 1D numpy structured array for this component input
            system_frequency: frequency of the power system, default 50 Hz
        """
        pass

    def copy(self) -> 'PowerGridModel':
        """

        Copy the current model

        Returns:
            a copy of PowerGridModel
        """
        pass
    
    @property
    def independent(self) -> bool:
        """
        Get the batch parameter status of the last batch calculation
        If independent is True, all update datasets consists of exactly the same components
        """
        pass
    
    @property
    def cache_topology(self) -> bool:
        """
        Get the batch parameter status of the last batch calculation
        If cache_topology is True, there are no changes in topology (branch, source) in the update datasets
        """
        pass

    @property
    def all_component_count(self) -> Dict[str, int]:
        """
        Get count of number of elements per component type.
        If the count for a component type is zero, it will not be in the returned dictionary.
        Returns:
            a dictionary with
                key: component type name
                value: integer count of elements of this type
        """
        pass

    def update(self, *, update_data: Dict[str, np.ndarray]):
        """
        Update the model with changes.
        Args:
            update_data: update data dictionary
                key: component type name
                value: 1D numpy structured array for this component update
        Returns:
            None
        """
        pass

    def calculate_power_flow(self, *,
            symmetric=True, error_tolerance=1e-8, max_iterations=20,
            calculation_method: Union[CalculationMethod, str] = CalculationMethod.newton_raphson,
            update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
            threading=-1
        ) -> Dict[str, np.ndarray]:
        """
        Calculate power flow once with the current model attributes.
        Or calculate in batch with the given update dataset in batch

        Args:
            symmetric:
                True: three-phase symmetric calculation, even for asymmetric loads/generations
                False: three-phase asymmetric calculation
            error_tolerance:
                error tolerance for voltage in p.u., only applicable when iterative=True
            max_iterations:
                maximum number of iterations, only applicable when iterative=True
            calculation_method: an enumeration or string
                newton_raphson: use Newton-Raphson iterative method (default)
                linear: use linear method
            update_data:
                None: calculate power flow once with the current model attributes
                A dictionary for batch calculation with batch update
                    key: component type name to be updated in batch
                    value:
                        a 2D numpy structured array for homogeneous update batch
                            Dimension 0: each batch
                            Dimension 1: each updated element per batch for this component type
                        **or**
                        a dictionary containing two keys, for inhomogeneous update batch
                            indptr: a 1D integer numpy array with length n_batch + 1
                                given batch number k, the update array for this batch is
                                data[indptr[k]:indptr[k + 1]]
                                This is the concept of compressed sparse structure
                                https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            data: 1D numpy structured array in flat
            threading:
                only applicable for batch calculation
                < 0 sequential
                = 0 parallel, use number of hardware threads
                > 0 specify number of parallel threads

        Returns:
            dictionary of results of all components
                key: component type name to be updated in batch
                value:
                    for single calculation: 1D numpy structured array for the results of this component type
                    for batch calculation: 2D numpy structured array for the results of this component type
                        Dimension 0: each batch
                        Dimension 1: the result of each element for this component type
            Error handling:
                in case an error in the core occurs, an exception will be thrown
        """
        pass

    def calculate_state_estimation(self, *,
            symmetric=True, error_tolerance=1e-8, max_iterations=20,
            calculation_method: Union[CalculationMethod, str] = CalculationMethod.iterative_linear,
            update_data: Optional[Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]] = None,
            threading=-1
        ) -> Dict[str, np.ndarray]:
        """
        Calculate state estimation once with the current model attributes.
        Or calculate in batch with the given update dataset in batch

        Args:
            symmetric:
                True: three-phase symmetric calculation, even for asymmetric loads/generations
                False: three-phase asymmetric calculation
            error_tolerance:
                error tolerance for voltage in p.u., only applicable when iterative=True
            max_iterations:
                maximum number of iterations, only applicable when iterative=True
            calculation_method: an enumeration
                iterative_linear: use iterative linear method
            update_data:
                None: calculate state estimation once with the current model attributes
                A dictionary for batch calculation with batch update
                    key: component type name to be updated in batch
                    value:
                        a 2D numpy structured array for homogeneous update batch
                            Dimension 0: each batch
                            Dimension 1: each updated element per batch for this component type
                        **or**
                        a dictionary containing two keys, for inhomogeneous update batch
                            indptr: a 1D integer numpy array with length n_batch + 1
                                given batch number k, the update array for this batch is
                                data[indptr[k]:indptr[k + 1]]
                                This is the concept of compressed sparse structure
                                https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.csr_matrix.html
                            data: 1D numpy structured array in flat
            threading:
                only applicable for batch calculation
                < 0 sequential
                = 0 parallel, use number of hardware threads
                > 0 specify number of parallel threads

        Returns:
            dictionary of results of all components
                key: component type name to be updated in batch
                value:
                    for single calculation: 1D numpy structured array for the results of this component type
                    for batch calculation: 2D numpy structured array for the results of this component type
                        Dimension 0: each batch
                        Dimension 1: the result of each element for this component type
            Error handling:
                in case an error in the core occurs, an exception will be thrown
        """
        pass
```

# Validation

The main validation functions and classes can be included from `power_grid_model.validation`.

Two helper type definitions are used throughout the validation functions, `InputData` and `UpdateData`. They are not 
special types or classes, but merely type hinting aliases:

```python
InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
```

## Manual Validation
```python
class ValidationError:
    """
    The Validation Error is an abstract base class which should be extended by all validation errors. It supplies
    three public member variables: component, field and ids; storing information about the origin of the validation
    error. Error classes can extend the public members. For example:

        NotBetweenError(ValidationError):
            component = 'vehicle'
            field = 'direction'
            id = [3, 14, 15, 92, 65, 35]
            ref_value = (-3.1416, 3.1416)

    For convenience, a human readable representation of the error is supplied using the str() function.
    I.e. print(str(error)) will print a human readable error message like:

        Field 'direction' is not between -3.1416 and 3.1416 for 6 vehicles

    """

    component: Optional[Union[str, List[str]]] = None
    """
    The component, or components, to which the error applies.
    """

    field: Optional[Union[str, List[str], List[Tuple[str, str]]]] = None
    """
    The field, or fields, to which the error applies. A field can also be a tuple (component, field) when multiple 
    components are being addressed.
    """

    ids: Union[List[int], List[Tuple[str, int]]] = []
    """
    The object identifiers to which the error applies. A field object identifier can also be a tuple (component, id) 
    when multiple components are being addressed.
    """

    def get_context(self) -> Dict[str, Any]:
        """
        Returns a dictionary that supplies (human readable) information about this error. Each member variable is
        included in the dictionary. If a function {field_name}_str() exists, the value is overwritten by that function.
        """
        pass

def validate_input_data(input_data: InputData,
                        calculation_type: Optional[CalculationType] = None,
                        symmetric: bool = True) -> Optional[List[ValidationError]]:
    """
    Validates the entire input dataset:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all required values provided? (checking NaNs)
        3. Are all ID's unique? (checking object identifiers across all components)
        4. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: A power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.

    Returns:
        None if the data is valid, or a list containing all validation errors.
    """

def validate_batch_data(input_data: InputData,
                        update_data: UpdateData,
                        calculation_type: Optional[CalculationType] = None,
                        symmetric: bool = True) -> Optional[Dict[int, List[ValidationError]]]:
    """
    Ihe input dataset is validated:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all input data ID's unique? (checking object identifiers across all components)

    For each batch the update data is validated:
        3. Is the update data structure correct? (checking data types and numpy array shapes)
        4. Are all update ID's valid? (checking object identifiers across update and input data)

    Then (for each batch independently) the input dataset is updated with the batch's update data and validated:
        5. Are all required values provided? (checking NaNs)
        6. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: a power-grid-model input dataset
        update_data: a power-grid-model update dataset (one or more batches)
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.

    Returns:
        None if the data is valid, or a dictionary containing all validation errors,
        where the key is the batch number (0-indexed).
    """
```

## Assertions
```python
InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]

class ValidationException(ValueError):
    """
    An exception storing the name of the validated data, a list/dict of errors and a convenient conversion to string
    to display a summary of all the errors when printing the exception.
    """

def assert_valid_input_data(input_data: InputData,
                            calculation_type: Optional[CalculationType] = None,
                            symmetric: bool = True):
    """
    See validate_input_data()

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.
        ValidationException if the contents are invalid.
    """

def assert_valid_batch_data(input_data: InputData,
                            update_data: UpdateData,
                            calculation_type: Optional[CalculationType] = None,
                            symmetric: bool = True):
    """
    See validate_batch_data()

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.
        ValidationException if the contents are invalid.
    """
```

## Printing validation errors
```python
def errors_to_string(errors: Union[List[ValidationError], Dict[int, List[ValidationError]]],
                     name: str = 'the data', details: bool = False) -> str:
    """
    Convert a set of errors (list or dict) to a human readable string representation.
    Args:
        errors: The error objects. List for input_data only, dict for batch data.
        name: Human understandable name of the dataset, e.g. input_data, or update_data.
        details: Display object ids and error specific information.

    Returns: A human readable string representation of a set of errors.
    """
```