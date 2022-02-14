import json
from pathlib import Path

import pandapower as pp
from power_grid_model import PowerGridModel, CalculationMethod
from power_grid_model.manual_testing import export_json_data
from power_grid_model.manual_testing import import_json_data
from power_grid_model.validation import assert_valid_input_data

from pgm_pp_converter import pgm2pp_input_converter, pp2pgm_sym_result_converter, pp2pgm_asym_result_converter
from test_cases import *


def pgm_result_exporter(input_data, type_of_output, case_path, rtol=1e-8, algorithm=CalculationMethod.newton_raphson):
    """
    imports input data of grid from json file, runs power flow and exports result
    """
    export_json_data(case_path.absolute() / 'input.json', input_data)
    imported_data = import_json_data(case_path / 'input.json', 'input')
    pgm = PowerGridModel(imported_data)
    result = pgm.calculate_power_flow(symmetric=True if type_of_output == 'sym_output' else False,
                                      error_tolerance=rtol, max_iterations=20,
                                      calculation_method=algorithm)
    export_json_data(case_path / ('pgm_' + type_of_output + '.json'), result)
    return


def pp_result_exporter(input_data, type_of_output, case_path, rtol=1e-8, algorithm='nr'):
    """
    Converts PGM input to pandapower, runs power flow and exports result
    """
    net, additional_indices = pgm2pp_input_converter(input_data, type_of_output=type_of_output)
    if type_of_output == 'sym_output':
        pp.runpp(net, tolerance_mva=rtol, max_iteration=20, algorithm=algorithm,
                 calculate_voltage_angles=True, trafo_model='t',
                 init='auto', voltage_depend_loads=True)
        result = pp2pgm_sym_result_converter(net, additional_indices)
    else:
        pp.runpp_3ph(net, tolerance_mva=rtol, max_iteration=20, algorithm=algorithm,
                     calculate_voltage_angles=False, trafo_model='t')
        result = pp2pgm_asym_result_converter(net, additional_indices)
    export_json_data(case_path / (type_of_output + '.json'), result)
    return


def export_individual_test(case_gen, case_path, params, type_of_output):
    assert_valid_input_data(input_data=case_gen, symmetric=(True if type_of_output == 'sym_output' else False))
    pgm_result_exporter(case_gen, type_of_output=type_of_output, case_path=case_path, rtol=params['rtol'])
    pp_result_exporter(case_gen, type_of_output=type_of_output, case_path=case_path, rtol=params['rtol'])
    with open(case_path / 'params.json', 'w') as file_pointer:
        json.dump(params, file_pointer, indent=2)


def export_all_individual_tests():
    """
    Exports all individual cases
    """
    params = {
        'calculation_method': "newton_raphson",
        'rtol': 1e-8,
        'atol': 0.5
    }
    to_export = {'node': node_case_generator(),
                 'line': line_case_generator(), 'sym_load': sym_load_case_generator(),
                 'sym_gen': sym_gen_case_generator(), 'source': sources_case_generator(),
                 'shunt': shunt_case_generator(), 'transformer': transformer_case_generator(),
                 'asym_load': asym_load_case_generator(), 'asym_gen': asym_gen_case_generator()}

    for case, case_gen in to_export.items():
        print("Test case: " + case)
        if not case in {'asym_load', 'asym_gen'}:
            case_path = Path(".").resolve().parents[1] / "tests" / "data" / "power_flow" /(
                    "pandapower-symmetric-" + case)
            if not case_path.exists():
                case_path.mkdir(parents=True)
            export_individual_test(case_gen, case_path, params, type_of_output='sym_output')

        #if case == 'sym_load':
        #    case_gen = asymcalc_sym_load_case_generator()
        #if case == 'sym_gen':
        #    case_gen = asymcalc_sym_load_case_generator()
        case_gen = asymcalc_sym_gen_case_generator() if case == 'sym_gen' else case_gen
        case_gen = asymcalc_sym_load_case_generator() if case == 'sym_load' else case_gen
        case_gen['source']['sk'] = 1e20
        case_path = Path(".").resolve().parents[1] / "tests" / "data" / "power_flow" /(
                "pandapower-asymmetric-" + case)
        if not case_path.exists():
            case_path.mkdir(parents=True)
        export_individual_test(case_gen, case_path, params, type_of_output='asym_output')


export_all_individual_tests()