// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"

#include <cassert>
#include <format>
#include <string>

namespace power_grid_model::enum_utils {
namespace detail {
using namespace std::string_literals;

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr std::string generic_enum_string(Enum value) {
    return std::format("BusType: #{:d}", std::to_underlying(value));
}

constexpr bool is_valid(BusType value) {
    using enum BusType;
    switch (value) {
    case pq:
    case pv:
    case slack:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(BusType value) {
    using enum BusType;
    switch (value) {
    case pq:
        return "pq"s;
    case pv:
        return "pv"s;
    case slack:
        return "slack"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(LimitViolation value) {
    using enum LimitViolation;
    switch (value) {
    case none:
    case lower:
    case upper:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(LimitViolation value) {
    using enum LimitViolation;
    switch (value) {
    case none:
        return "none"s;
    case lower:
        return "lower"s;
    case upper:
        return "upper"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(LoadGenType value) {
    using enum LoadGenType;
    switch (value) {
    case const_pq:
    case const_y:
    case const_i:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(LoadGenType value) {
    using enum LoadGenType;
    switch (value) {
    case const_pq:
        return "const_pq"s;
    case const_y:
        return "const_y"s;
    case const_i:
        return "const_i"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(WindingType value) {
    using enum WindingType;
    switch (value) {
    case wye:
    case wye_n:
    case delta:
    case zigzag:
    case zigzag_n:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(WindingType value) {
    using enum WindingType;
    switch (value) {
    case wye:
        return "wye"s;
    case wye_n:
        return "wye_n"s;
    case delta:
        return "delta"s;
    case zigzag:
        return "zigzag"s;
    case zigzag_n:
        return "zigzag_n"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(BranchSide value) {
    using enum BranchSide;
    switch (value) {
    case from:
    case to:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(BranchSide value) {
    using enum BranchSide;
    switch (value) {
    case from:
        return "from"s;
    case to:
        return "to"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(Branch3Side value) {
    using enum Branch3Side;
    switch (value) {
    case side_1:
    case side_2:
    case side_3:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(Branch3Side value) {
    using enum Branch3Side;
    switch (value) {
    case side_1:
        return "side_1"s;
    case side_2:
        return "side_2"s;
    case side_3:
        return "side_3"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

enum class ControlSide : IntS { from = 0, to = 1, side_1 = 0, side_2 = 1, side_3 = 2 };
constexpr bool is_valid(ControlSide value) {
    using enum ControlSide;
    switch (value) {
    case from:
        static_assert(from == side_1, "cannot duplicate enum value in switch/case statement");
        [[fallthrough]];
    case to:
        static_assert(from == side_1, "cannot duplicate enum value in switch/case statement");
        [[fallthrough]];
    case side_3:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(ControlSide value) {
    using enum ControlSide;
    switch (value) {
    case from:
        static_assert(from == side_1, "cannot duplicate enum value in switch/case statement");
        return "from/side_1"s;
    case to:
        static_assert(from == side_1, "cannot duplicate enum value in switch/case statement");
        return "to/side_2"s;
    case side_3:
        return "side_3"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(CalculationType value) {
    using enum CalculationType;
    switch (value) {
    case power_flow:
    case state_estimation:
    case short_circuit:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(CalculationType value) {
    using enum CalculationType;
    switch (value) {
    case power_flow:
        return "power_flow"s;
    case state_estimation:
        return "state_estimation"s;
    case short_circuit:
        return "short_circuit"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(CalculationSymmetry value) {
    using enum CalculationSymmetry;
    switch (value) {
    case asymmetric:
    case symmetric:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(CalculationSymmetry value) {
    using enum CalculationSymmetry;
    switch (value) {
    case asymmetric:
        return "asymmetric"s;
    case symmetric:
        return "symmetric"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

enum class CalculationMethod : IntS {
    default_method = -128,
    linear = 0,
    newton_raphson = 1,
    iterative_linear = 2,
    iterative_current = 3,
    linear_current = 4,
    iec60909 = 5,
};

constexpr bool is_valid(CalculationMethod value) {
    using enum CalculationMethod;
    switch (value) {
    case default_method:
    case linear:
    case newton_raphson:
    case iterative_linear:
    case iterative_current:
    case linear_current:
    case iec60909:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(CalculationMethod value) {
    using enum CalculationMethod;
    switch (value) {
    case default_method:
        return "default_method"s;
    case linear:
        return "linear"s;
    case newton_raphson:
        return "newton_raphson"s;
    case iterative_linear:
        return "iterative_linear"s;
    case iterative_current:
        return "iterative_current"s;
    case linear_current:
        return "linear_current"s;
    case iec60909:
        return "iec60909"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(MeasuredTerminalType value) {
    using enum MeasuredTerminalType;
    switch (value) {
    case branch_from:
    case branch_to:
    case source:
    case shunt:
    case load:
    case generator:
    case branch3_1:
    case branch3_2:
    case branch3_3:
    case node:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(MeasuredTerminalType value) {
    using enum MeasuredTerminalType;
    switch (value) {
    case branch_from:
        return "branch_from"s;
    case branch_to:
        return "branch_to"s;
    case source:
        return "source"s;
    case shunt:
        return "shunt"s;
    case load:
        return "load"s;
    case generator:
        return "generator"s;
    case branch3_1:
        return "branch3_1"s;
    case branch3_2:
        return "branch3_2"s;
    case branch3_3:
        return "branch3_3"s;
    case node:
        return "node"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

constexpr bool is_valid(ComponentType value) {
    using enum ComponentType;
    switch (value) {
    case node:
    case branch:
    case appliance:
    case sensor:
    case generic_power_sensor:
    case generic_voltage_sensor:
    case generic_load_gen:
    case shunt:
    case source:
    case branch3:
    case fault:
    case regulator:
    case transformer_tap_regulator:
    case voltage_regulator:
    case link:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(ComponentType value) {
    using enum ComponentType;
    switch (value) {
    case node:
        return "node"s;
    case branch:
        return "branch"s;
    case appliance:
        return "appliance"s;
    case sensor:
        return "sensor"s;
    case generic_power_sensor:
        return "generic_power_sensor"s;
    case generic_voltage_sensor:
        return "generic_voltage_sensor"s;
    case generic_load_gen:
        return "generic_load_gen"s;
    case shunt:
        return "shunt"s;
    case source:
        return "source"s;
    case branch3:
        return "branch3"s;
    case fault:
        return "fault"s;
    case regulator:
        return "regulator"s;
    case transformer_tap_regulator:
        return "transformer_tap_regulator"s;
    case voltage_regulator:
        return "voltage_regulator"s;
    case link:
        return "link"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

enum class FaultType : IntS {
    three_phase = 0,
    single_phase_to_ground = 1,
    two_phase = 2,
    two_phase_to_ground = 3,
    nan = na_IntS
};

constexpr bool is_valid(FaultType value) {
    using enum FaultType;
    switch (value) {
    case three_phase:
    case single_phase_to_ground:
    case two_phase:
    case two_phase_to_ground:
    case nan:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(FaultType value) {
    using enum FaultType;
    switch (value) {
    case three_phase:
        return "three_phase"s;
    case single_phase_to_ground:
        return "single_phase_to_ground"s;
    case two_phase:
        return "two_phase"s;
    case two_phase_to_ground:
        return "two_phase_to_ground"s;
    case nan:
        return "nan"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

enum class FaultPhase : IntS {
    abc = 0,
    a = 1,
    b = 2,
    c = 3,
    ab = 4,
    ac = 5,
    bc = 6,
    default_value = -1,
    nan = na_IntS
};

constexpr bool is_valid(FaultPhase value) {
    using enum FaultPhase;
    switch (value) {
    case abc:
    case a:
    case b:
    case c:
    case ab:
    case ac:
    case bc:
    case default_value:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(FaultPhase value) {
    using enum FaultPhase;
    switch (value) {
    case abc:
        return "abc"s;
    case a:
        return "a"s;
    case b:
        return "b"s;
    case c:
        return "c"s;
    case ab:
        return "ab"s;
    case ac:
        return "ac"s;
    case bc:
        return "bc"s;
    case default_value:
        return "default_value"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

// Initialization of the calculation, only applicable if supported by the calculation type and method.
// For the Newton-Raphson power flow:
// - linear (default): a linear guess with loads and generators as constant admittances
// - flat: every bus at 1 p.u. with the source angle and its topological phase shift, source buses at their
// reference
//   voltage
// - average_source: every bus at the average reference voltage of all sources with its topological phase shift, as
//   used by the iterative current power flow
enum class CalculationInitialization : IntS { default_initialization = 0, flat = 1, linear = 2, average_source = 3 };

enum class CType : IntS { c_int32 = 0, c_int8 = 1, c_double = 2, c_double3 = 3 };

enum class SerializationFormat : IntS { json = 0, msgpack = 1 };

enum class OptimizerType : IntS {
    no_optimization = 0,          // do nothing
    automatic_tap_adjustment = 1, // power flow with automatic tap adjustment
};

enum class OptimizerStrategy : IntS { // Conventions for optimization strategies
    any = 0,                          // any = Any{f(x) \in Range} for x \in Domain
    global_minimum = 1,               // global_minimum = argmin{f(x) \in Range} for x in Domain
    global_maximum = 2,               // global_maximum = argmax{f(x) \in Range} for x in Domain
    local_minimum = 3,                // local_minimum = Any{argmin{f(x) \in Range}} for x in Domain
    local_maximum = 4,                // local_maximum = Any{argmax{f(x) \in Range}} for x in Domain
    fast_any = 5,                     // fast_any = Any{f(x) \in Range} for x \in Domain, but faster
};

enum class SearchMethod : IntS { // Which type of tap search method for finite element optimization process
    linear_search = 0,           // use linear_search method: one step per iteration
    binary_search = 1,           // use binary search: half a tap range at a time
};

enum class AngleMeasurementType : IntS { // The type of the angle measurement for current sensors
    local_angle = 0,                     // local_angle = 0, the angle is relative to the local voltage angle
    global_angle = 1,                    // global_angle = 1, the angle is relative to the global voltage angle
};

constexpr bool is_valid(AngleMeasurementType value) {
    using enum AngleMeasurementType;
    switch (value) {
    case local_angle:
    case global_angle:
        return true;
    default:
        return false;
    };
}
constexpr std::string to_string(AngleMeasurementType value) {
    using enum AngleMeasurementType;
    switch (value) {
    case local_angle:
        return "local_angle"s;
    case global_angle:
        return "global_angle"s;
    default:
        assert(!is_valid(value));
        return generic_enum_string(value);
    };
}

} // namespace detail

using detail::is_valid;
using detail::to_string;
} // namespace power_grid_model::enum_utils
