from power_grid_model import LoadGenType, WindingType, BranchSide
from power_grid_model import initialize_array

def pgm_example_case_generator():
    '''
    Case generator for the power grid model example
    '''
    node = initialize_array('input', 'node', 3)
    node['id'] = [1, 2, 6]
    node['u_rated'] = [10.5e3, 10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 3)
    line['id'] = [3, 5, 8]
    line['from_node'] = [1, 2, 1]
    line['to_node'] = [2, 6, 6]
    line['from_status'] = [1, 1, 1]
    line['to_status'] = [1, 1, 1]
    line['r1'] = [0.25, 0.25, 0.25]
    line['x1'] = [0.2, 0.2, 0.2]
    line['c1'] = [10e-6, 10e-6, 10e-6]
    line['tan1'] = [0.0, 0.0, 0.0]
    line['i_n'] = [1000, 1000, 1000]

    sym_load = initialize_array('input', 'sym_load', 2)
    sym_load['id'] = [4, 7]
    sym_load['node'] = [2, 6]
    sym_load['status'] = [1, 1]
    sym_load['type'] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load['p_specified'] = [20e6, 10e6]
    sym_load['q_specified'] = [5e6, 2e6]

    source = initialize_array('input', 'source', 1)
    source['id'] = [10]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_load': sym_load,
        'source': source
    }
    return input_data


def basic_node_case_generator():
    """
    Individual test case for only node and source.

    source--|       |

    """
    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'source': source
    }
    return input_data


def node_case_generator():
    """
    Individual test case for node.

    source--|--line--|
    """
    node = initialize_array('input', 'node', 3)
    node['id'] = [1, 2, 5]
    node['u_rated'] = [10.5e3, 10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    source = initialize_array('input', 'source', 1)
    source['id'] = [10]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'source': source
    }
    return input_data


def line_case_generator():
    """
    Individual test case for line

    source--|--line--|--line--|              (Link from_status=to_status=1)
                     |--line--|--load        (Link from_status=0)
                     |--line--|              (Link to_status=0)
    """
    # node
    node = initialize_array('input', 'node', 3)
    node['id'] = [1, 2, 5]
    node['u_rated'] = [10.5e3, 10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 4)
    line['id'] = [3, 6, 7, 8]
    line['from_node'] = [1, 2, 2, 2]
    line['to_node'] = [2, 5, 5, 5]
    line['from_status'] = [1, 1, 1, 0]
    line['to_status'] = [1, 1, 0, 1]
    line['r1'] = [0.25, 0.25, 0.25, 0.25]
    line['x1'] = [0.2, 0.2, 0.2, 0.2]
    line['c1'] = [10e-6, 10e-6, 10e-6, 10e-6]
    line['tan1'] = [0.0, 0.0, 0.0, 0.0]
    line['i_n'] = [1000, 1000, 1000, 1000]
    line['r0'] = [0.25, 0.25, 0.25, 0.25]
    line['x0'] = [0.2, 0.2, 0.2, 0.2]
    line['c0'] = [10e-6, 10e-6, 10e-6, 10e-6]
    line['tan0'] = [0.0, 0.0, 0.0, 0.0]

    sym_load = initialize_array('input', 'sym_load', 1)
    sym_load['id'] = [9]
    sym_load['node'] = [5]
    sym_load['status'] = [1]
    sym_load['type'] = [LoadGenType.const_power]
    sym_load['p_specified'] = [10e3]
    sym_load['q_specified'] = [2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_load': sym_load,
        'source': source
    }
    return input_data


def transformer_case_generator():
    '''
    Individual test case for transformer

    source--|--transformer--|              (Transformer from_status=to_status=1)
            |--transformer--|--load        (Transformer from_status=0)
            |--transformer--|              (Transformer to_status=0)
    '''
    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 400]

    transformer = initialize_array('input', 'transformer', 3)
    transformer['id'] = [3, 4, 5]
    transformer['from_node'] = [1, 1, 1]
    transformer['to_node'] = [2, 2, 2]
    transformer['from_status'] = [1, 0, 0]
    transformer['to_status'] = [1, 0, 0]
    transformer['u1'] = [10.5e3, 10.5e3, 10.5e3]
    transformer['u2'] = [400, 400, 400]
    transformer['sn'] = [1e6, 1e6, 1e6]
    transformer['uk'] = [0.1, 0.1, 0.1]
    transformer['pk'] = [10000, 10000, 10000]
    transformer['i0'] = [0.001, 0.001, 0.001]
    transformer['p0'] = [1000, 1000, 1000]
    transformer['winding_from'] = [WindingType.delta, WindingType.delta, WindingType.delta]
    transformer['winding_to'] = [WindingType.wye_n, WindingType.wye_n, WindingType.wye_n]
    transformer['clock'] = [11, 11, 11]
    transformer['tap_side'] = [0, 0, 0]
    transformer['tap_pos'] = [0, 0, 0]
    transformer['tap_min'] = [-5, -5, -5]
    transformer['tap_max'] = [5, 5, 5]
    transformer['tap_nom'] = [0, 0, 0]
    transformer['tap_size'] = [105, 105, 105]

    sym_load = initialize_array('input', 'sym_load', 1)
    sym_load['id'] = [6]
    sym_load['node'] = [2]
    sym_load['status'] = [1]
    sym_load['type'] = [LoadGenType.const_power]
    sym_load['p_specified'] = [10e3]
    sym_load['q_specified'] = [2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [7]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    # TODO change sk
    source['sk'] = [1e12]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'transformer': transformer,
        'sym_load': sym_load,
        'source': source
    }
    return input_data

def transformer_case_generator_2():
    '''
    Individual test case for transformer

    source--|--line--|--transformer--|              (Transformer from_status=to_status=1)
                     |--transformer--|--load        (Transformer from_status=0)
                     |--transformer--|              (Transformer to_status=0)
    '''
    node = initialize_array('input', 'node', 3)
    node['id'] = [1, 2, 5]
    node['u_rated'] = [10.5e3, 10.5e3, 400]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [1e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    transformer = initialize_array('input', 'transformer', 3)
    transformer['id'] = [6, 7, 8]
    transformer['from_node'] = [2, 2, 2]
    transformer['to_node'] = [5, 5, 5]
    transformer['from_status'] = [1, 0, 1]
    transformer['to_status'] = [1, 1, 0]
    transformer['u1'] = [10.5e3, 10.5e3, 10.5e3]
    transformer['u2'] = [400, 400, 400]
    transformer['sn'] = [1e6, 1e6, 1e6]
    transformer['uk'] = [0.1, 0.1, 0.1]
    transformer['pk'] = [10000, 10000, 10000]
    transformer['i0'] = [0.001, 0.001, 0.001]
    transformer['p0'] = [1000, 1000, 1000]
    transformer['winding_from'] = [WindingType.delta, WindingType.delta, WindingType.delta]
    transformer['winding_to'] = [WindingType.wye_n, WindingType.wye_n, WindingType.wye_n]
    transformer['clock'] = [11, 11, 11]
    transformer['tap_side'] = [0, 0, 0]
    transformer['tap_pos'] = [0, 0, 0]
    transformer['tap_min'] = [-5, -5, -5]
    transformer['tap_max'] = [5, 5, 5]
    transformer['tap_nom'] = [0, 0, 0]
    transformer['tap_size'] = [105, 105, 105]

    sym_load = initialize_array('input', 'sym_load', 1)
    sym_load['id'] = [9]
    sym_load['node'] = [5]
    sym_load['status'] = [1]
    sym_load['type'] = [LoadGenType.const_power]
    sym_load['p_specified'] = [10e3]
    sym_load['q_specified'] = [2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e12]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'transformer': transformer,
        'sym_load': sym_load,
        'source': source
    }
    return input_data


def sources_case_generator():
    '''
    Individual test case for source

    source--|--line--|--line--|--source
    '''
    node = initialize_array('input', 'node', 3)
    node['id'] = [1, 2, 5]
    node['u_rated'] = [10.5e3, 10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 2)
    line['id'] = [3, 6]
    line['from_node'] = [1, 2]
    line['to_node'] = [2, 5]
    line['from_status'] = [1, 1]
    line['to_status'] = [1, 1]
    line['r1'] = [0.25, 0.25]
    line['x1'] = [0.2, 0.2]
    line['c1'] = [10e-6, 10e-6]
    line['tan1'] = [0.0, 0.0]
    line['i_n'] = [1000, 1000]
    line['r0'] = [0.25, 0.25]
    line['x0'] = [0.2, 0.2]
    line['c0'] = [10e-6, 10e-6]
    line['tan0'] = [0.0, 0.0]

    source = initialize_array('input', 'source', 2)
    source['id'] = [4, 7]
    source['node'] = [1, 5]
    source['status'] = [1, 1]
    source['u_ref'] = [1.0, 1.0]
    source['sk'] = [1e10, 1e10]
    source['rx_ratio'] = [0.1, 0.1]
    source['z01_ratio'] = [1, 1]

    input_data = {
        'node': node,
        'line': line,
        'source': source
    }
    return input_data


def sym_gen_case_generator():
    '''
    Individual test case for generator

    source--|--line--|--sym_gen    (const_power)
                     |--sym_gen    (const_current)
                     |--sym_gen    (const_impedance)
                     |--sym_gen    (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    sym_gen = initialize_array('input', 'sym_gen', 4)
    sym_gen['id'] = [6, 7, 8, 9]
    sym_gen['node'] = [2, 2, 2, 2]
    sym_gen['status'] = [1, 1, 1, 0]
    sym_gen['type'] = [LoadGenType.const_power, LoadGenType.const_current, LoadGenType.const_impedance,
                       LoadGenType.const_current]
    sym_gen['p_specified'] = [10e3, 10e3, 10e3, 10e3]
    sym_gen['q_specified'] = [2e3, 2e3, 2e3, 2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_gen': sym_gen,
        'source': source
    }
    return input_data


def sym_load_case_generator():
    '''
    Individual test case for generator

     source--|--line--|--sym_load (const_power)
                      |--sym_load (const_current)
                      |--sym_load (const_impedance)
                      |--sym_load (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    sym_load = initialize_array('input', 'sym_load', 4)
    sym_load['id'] = [6, 7, 8, 9]
    sym_load['node'] = [2, 2, 2, 2]
    sym_load['status'] = [1, 1, 1, 0]
    sym_load['type'] = [LoadGenType.const_power, LoadGenType.const_current, LoadGenType.const_impedance,
                        LoadGenType.const_current]
    sym_load['p_specified'] = [10e3, 10e3, 10e3, 10e3]
    sym_load['q_specified'] = [2e3, 2e3, 2e3, 2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_load': sym_load,
        'source': source
    }
    return input_data


def shunt_case_generator():
    '''
    Individual test case for shunt

     source--|--line--|--shunt      (status=1)
                      |--shunt      (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    shunt = initialize_array('input', 'shunt', 2)
    shunt['id'] = [6, 7]
    shunt['node'] = [2, 2]
    shunt['status'] = [1, 0]
    shunt['g1'] = [0.01e-3, 0.01e-3]
    shunt['b1'] = [0.08e-3, 0.08e-3]
    shunt['g0'] = [0.01e-3, 0.01e-3]
    shunt['b0'] = [0.08e-3, 0.08e-3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'shunt': shunt,
        'source': source
    }
    return input_data


def asym_load_case_generator():
    '''
    Individual test case for generator
    source -- line ---  line
     source--|--line--|--asym_load (const_power)
                      |--asym_load (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [1e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [1e-6]
    line['tan0'] = [0.0]

    asym_load = initialize_array('input', 'asym_load', 2)
    asym_load['id'] = [5, 6]
    asym_load['node'] = [2, 2]
    asym_load['status'] = [1, 0]
    asym_load['type'] = [LoadGenType.const_power,LoadGenType.const_current]
    asym_load['p_specified'] = [[10e3, 9e3, 10.5e3], [10e3, 9e3, 10.5e3]]
    asym_load['q_specified'] = [[2e3, 1.5e3, 2.5e3], [2e3, 1.5e3, 2.5e3]]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e20]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'asym_load': asym_load,
        'source': source
    }
    return input_data



def asym_gen_case_generator():
    '''
    Individual test case for generator

    source--|--line--|--sym_gen    (const_power)
                     |--sym_gen    (const_current)
                     |--sym_gen    (const_impedance)
                     |--sym_gen    (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    asym_gen = initialize_array('input', 'asym_gen', 2)
    asym_gen['id'] = [6, 7]
    asym_gen['node'] = [2, 2]
    asym_gen['status'] = [1, 0]
    asym_gen['type'] = [LoadGenType.const_power, LoadGenType.const_current]
    asym_gen['p_specified'] = [[10e3, 9e3, 10.5e3], [10e3, 9e3, 10.5e3]]
    asym_gen['q_specified'] = [[2e3, 1.5e3, 2.5e3], [2e3, 1.5e3, 2.5e3]]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'asym_gen': asym_gen,
        'source': source
    }
    return input_data


def asymcalc_sym_gen_case_generator():
    '''
    Individual test case for generator

    source--|--line--|--sym_gen    (const_power)
                     |--sym_gen    (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    sym_gen = initialize_array('input', 'sym_gen', 2)
    sym_gen['id'] = [6, 7]
    sym_gen['node'] = [2, 2]
    sym_gen['status'] = [1, 0]
    sym_gen['type'] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_gen['p_specified'] = [10e3, 10e3]
    sym_gen['q_specified'] = [2e3, 2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_gen': sym_gen,
        'source': source
    }
    return input_data


def asymcalc_sym_load_case_generator():
    '''
    Individual test case for generator

     source--|--line--|--sym_load (const_power)
                      |--sym_load (status=0)
    '''

    node = initialize_array('input', 'node', 2)
    node['id'] = [1, 2]
    node['u_rated'] = [10.5e3, 10.5e3]

    line = initialize_array('input', 'line', 1)
    line['id'] = [3]
    line['from_node'] = [1]
    line['to_node'] = [2]
    line['from_status'] = [1]
    line['to_status'] = [1]
    line['r1'] = [0.25]
    line['x1'] = [0.2]
    line['c1'] = [10e-6]
    line['tan1'] = [0.0]
    line['i_n'] = [1000]
    line['r0'] = [0.25]
    line['x0'] = [0.2]
    line['c0'] = [10e-6]
    line['tan0'] = [0.0]

    sym_load = initialize_array('input', 'sym_load', 2)
    sym_load['id'] = [6, 7]
    sym_load['node'] = [2, 2]
    sym_load['status'] = [1, 0]
    sym_load['type'] = [LoadGenType.const_power, LoadGenType.const_current]
    sym_load['p_specified'] = [10e3, 10e3]
    sym_load['q_specified'] = [2e3, 2e3]

    source = initialize_array('input', 'source', 1)
    source['id'] = [4]
    source['node'] = [1]
    source['status'] = [1]
    source['u_ref'] = [1.0]
    source['sk'] = [1e10]
    source['rx_ratio'] = [0.1]
    source['z01_ratio'] = [1]

    input_data = {
        'node': node,
        'line': line,
        'sym_load': sym_load,
        'source': source
    }
    return input_data