<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# High-level design

The power-grid-model follows a typical dynamic/shared library approach, in which the user interface is separated from
the core implementation using a strict system boundary.
Depending on the use case and programming language used by the user to call the interface, the user can opt to interface
with the C API in different ways.

## Layers of abstraction

For the sake of explanation, we consider the following layers of abstraction, in increasing order of ease-of-use and
abstraction.

* Raw system interface
  * System symbols only
  * Everything is handled by the user, including which symbols are supported
* Exposition-only
  * Exposes the supported symbols in a language supported by the user
  * Memory management and error handling is the responsibility of the user
* Simple wrapper
  * Wraps the interface in a language supported by the user
  * Handles memory management, basic error handling and type conversions
  * Contains no additional features except maybe some basic utility tools
* The feature-rich layer
  * Extensive wrapper around the interface with easy functionality exposure and utility functions
  * Extensive type checks

## Existing library interfaces

The following library interfaces are currently included in the power-grid-model.

| Interface type | Status       | Layer              | Explanation                                                     | Supported by                                        |
| -------------- | ------------ | ------------------ | --------------------------------------------------------------- | --------------------------------------------------- |
| C API          | Stable       | Raw interface      | Shared object / DLL that contains the core implementation       | All programming languages with dynamic load support |
| C headers      | Stable       | Exposition-only    | Exposition-only library using dynamic linking                   | C and C++                                           |
| C++ headers    | Experimental | Wrapper            | Handles memory management and basic error handling              | C++                                                 |
| Python library | Stable       | Feature-rich layer | Library with useful functions, conversions and extensive checks | Python                                              |

Note that the Python library in turn also follows the pattern of a feature-rich library that uses a module-internal
wrapper layer core module, that wraps the exposition-only core module, that exposes the raw interface.

This can be visualized graphically as follows.

```{mermaid}
    :title: Full design

flowchart TD
    classDef user_node fill:#9f6,stroke:#333,stroke-width:2px
    classDef public_interface fill:#69f,stroke:#333,stroke-width:2px
    classDef experimental_interface fill:#99b,stroke:#333,stroke-width:2px
    classDef private_interface fill:#999,stroke:#333,stroke-width:2px
    classDef inclusion_method fill:#ddd,stroke:#fff,stroke-width:2px

    subgraph User
        any_language_user(["Any language user"]):::user_node
        c_user(["C user"]):::user_node
        cpp_user(["C++ user"]):::user_node
        python_user(["Python user"]):::user_node
    end

    dynamic_loading{ }:::inclusion_method
    c_includes{ }:::inclusion_method

    subgraph Raw interface
        power_grid_model_c_dll("power_grid_model_c<br>(shared library)"):::public_interface
    end

    subgraph Exposition
        power_grid_model_c("power_grid_model_c<br>(C library)"):::public_interface
        power_grid_core_python("power_grid_model._core<br>.power_grid_core.py<br>(exposition-only<br>Python module)"):::private_interface
    end

    subgraph Wrapper
        power_grid_model_cpp("power_grid_model_cpp<br>(experimental,<br>C++ library)"):::experimental_interface
        power_grid_model_core_python("power_grid_model._core<br>(Python wrapper library)"):::private_interface
    end

    subgraph Feature-rich library
        power_grid_model_python("power_grid_model<br>(Python library)"):::public_interface
    end

    any_language_user --> dynamic_loading
    c_includes --> dynamic_loading
    dynamic_loading -->|dynamic loading| power_grid_model_c_dll
    c_user --> c_includes
    cpp_user --> c_includes
    c_includes -->|links +<br>includes| power_grid_model_c -->|dynamic linking| power_grid_model_c_dll
    cpp_user -->|experimental<br>links +<br>includes| power_grid_model_cpp -->|links +<br>includes| power_grid_model_c
    python_user -->|import| power_grid_model_python -->|internal import| power_grid_model_core_python -->|internal import| power_grid_core_python -->|"CDLL<br>(dynamic loading)"| power_grid_model_c_dll
```

## Creating a custom library or interface

We seek to provide an optimal user experience, but with the sheer amount of programming languages and features, it would
be impossible to provide a full feature-rich library for every single one.
We, being a {{ "[community-driven]({}/GOVERNANCE.md)".format(pgm_project_contribution) }} project strongly in favor of
modern software engineering practices, therefore encourage people to create their own libraries and interfaces to
improve their own experience.
There are several possible reasons a user may want to create their own library or interface, e.g.:

* Support for a new programming language
* Extending library support for a specific programming language
* A custom wrapper that provides extra features or useful commands for specific custom requirements

In all cases, it is recommended that the user determines their own desired
[layer of abstraction](#layers-of-abstraction) and then creates internal wrappers for all lower-level ones, following
the same pattern as the power-grid-model [uses internally](#existing-library-interfaces) for the custom interfaces.

### Hosting a custom library or interface

The Power Grid Model organization supports people creating and hosting custom libraries and interfaces.
If you are doing so and are willing to notify us, please create an item on our
[discussion board](https://github.com/orgs/PowerGridModel/discussions) on GitHub.
The Power Grid Model organization will review your item and we may decide to mention your custom library on our project
website and documentation.

### Contributing a custom library or interface

When a custom library or interface becomes mature enough and the circumstances allow making it publicly available,
please consider contributing it to the Power Grid Model organization.
If you are considering contributing your custom library or interface, please read and follow our
{{ "[contributing guidelines]({}/CONTRIBUTING.md)".format(pgm_project_contribution) }} and open an item on our
[discussion board](https://github.com/orgs/PowerGridModel/discussions) on GitHub.
The Power Grid Model organization will review your item and contact you accordingly.
