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

## Logger architecture

Logging is opt-in. All three public APIs use the same C API implementation and calculation-core logger hierarchy.
Creating a logger selects one of `NoMultiThreadedLogger`, `MultiThreadedTextLogger`, or
`MultiThreadedCalculationInfo`. Registering it adds a shared pointer to the `MultiThreadedCompositeLogger` owned by a
`PGM_Handle`. At the start of each calculation, the model is pointed at the composite logger of the handle used for
that call.

For batch calculations, the composite creates one child logger per registered logger for each worker. The child
loggers collect without sharing mutable state between workers and merge their results into their parent loggers. The
text logger returns timestamped diagnostic lines; the benchmark logger aggregates numeric events and returns
tab-separated event/value lines.

### C API logger flow

```{mermaid}
    :title: C API logger flow

flowchart LR
    user(["C caller"])
    handle["PGM_Handle<br>MultiThreadedCompositeLogger"]
    wrapper["PGM_Logger<br>shared_ptr"]
    logger["No-op, text, or<br>benchmark logger"]
    model[PGM_PowerGridModel]
    calculation["MainModel::calculate<br>JobDispatch"]
    children["Per-worker<br>child loggers"]
    callback[PGM_LogOutputCallback]

    user -->|PGM_create_logger| wrapper
    wrapper -->|owns| logger
    user -->|PGM_register_logger| handle
    wrapper -->|shared ownership| handle
    user -->|PGM_calculate with handle| model
    handle -->|set_logger before each call| model
    model --> calculation
    calculation -->|create_child| children
    children -->|collect and merge| logger
    user -->|PGM_logger_get_output| wrapper
    logger -->|string_view valid during call| callback
    callback --> user
```

Destroying `PGM_Logger` releases only the caller's wrapper and shared pointer. Each handle registration keeps the
underlying logger alive until it is unregistered, all handle loggers are reset, or the handle is destroyed.

### C++ API logger flow

```{mermaid}
    :title: C++ API logger flow

flowchart LR
    user(["C++ caller"])
    cpp_logger["power_grid_model_cpp::Logger<br>RAII wrapper"]
    cpp_model["power_grid_model_cpp::Model<br>owns Handle"]
    c_logger[PGM_Logger]
    c_handle["PGM_Handle<br>composite logger"]
    core["MainModel calculation<br>and worker loggers"]
    output["std::string"]

    user -->|construct| cpp_logger
    cpp_logger -->|PGM_create_logger| c_logger
    user -->|Model::add_logger| cpp_model
    cpp_model -->|PGM_register_logger| c_handle
    c_logger -->|shared registration| c_handle
    user -->|Model::calculate| cpp_model
    cpp_model -->|PGM_calculate| core
    c_handle -->|fan out events| core
    user -->|Logger::get_output| cpp_logger
    cpp_logger -->|callback copies bytes| output
    output --> user
```

The logger's private `Handle` is used for C API error propagation. Registrations belong to each model's `Handle`, so a
copied model starts without registrations, copy assignment preserves the target model's registrations, and moving a
model transfers them.

### Python API logger flow

```{mermaid}
    :title: Python API logger flow

flowchart LR
    user(["Python caller"])
    context["Logger context manager"]
    pgc["thread-local PowerGridCore<br>owns PGM_Handle"]
    ctypes["ctypes C bindings"]
    c_logger[PGM_Logger]
    composite["Handle composite logger"]
    core["PGM_calculate<br>calculation core"]
    text["Python str"]
    pylog["logging.Logger"]

    user -->|with Logger ctx mgr| context
    context -->|create/register| pgc
    pgc --> ctypes
    ctypes --> c_logger
    c_logger -->|shared registration| composite
    user -->|model.calculate| pgc
    pgc -->|same thread-local handle| core
    composite -->|fan out events| core
    user -->|Logger.output| context
    context -->|PGM_logger_get_output callback| text
    text --> user
    context -->|optional on context exit| pylog
    context -->|unregister and optionally clear| pgc
```

`PowerGridCore` and its native handle are thread-local. The context manager registers on entry and unregisters on exit.
When a Python `logging.Logger` is supplied, each non-empty native output line is emitted as a Python log record and the
native logger is cleared.

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
