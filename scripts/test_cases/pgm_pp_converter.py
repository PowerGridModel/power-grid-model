import math
import numpy as np
import pandapower as pp

from power_grid_model import LoadGenType, WindingType, BranchSide
from power_grid_model import initialize_array


def pgm2pp_input_converter(pgm_data, type_of_output='asym_output'):
    """
    Converts power-grid-model input of arrays into pandapower net
    """
    net = pp.create_empty_network()
    # To track ids of indices used in some results
    pp_indices = {}
    pgm_indices = {}
    if 'node' in pgm_data:
        pp_indices['node'] = _add_pp_buses(pgm_data['node'], net)
    if 'line' in pgm_data:
        pp_indices['line'] = _add_pp_lines(pgm_data['line'], net)
    if 'transformer' in pgm_data:
        pp_indices['transformer'] = _add_pp_transformer(pgm_data['transformer'], net)
        pgm_indices['transformer'] = pgm_data['transformer']['id']
    if 'source' in pgm_data:
        pp_indices['source'] = _add_pp_sources(pgm_data['source'], net, type_of_output)
        pgm_indices['source'] = pgm_data['source']['id']
    if 'sym_load' in pgm_data:
        pp_indices['sym_load'] = _add_pp_loads(pgm_data['sym_load'], net)
    if 'sym_gen' in pgm_data:
        pp_indices['sym_gen'] = _add_pp_sgens(pgm_data['sym_gen'], net)
    if 'shunt' in pgm_data:
        pp_indices['shunt'] = _add_pp_shunt(pgm_data['shunt'], net, type_of_output)
    if 'asym_load' in pgm_data:
        pp_indices['asym_load'] = _add_pp_asym_loads(pgm_data['asym_load'], net)
        pgm_indices['asym_load'] = pgm_data['asym_load']['id']
    if 'asym_gen' in pgm_data:
        pp_indices['asym_gen'] = _add_pp_asym_gens(pgm_data['asym_gen'], net)
        pgm_indices['asym_gen'] = pgm_data['asym_gen']['id']
    additional_indices = {'pgm_indices': pgm_indices, 'pp_indices': pp_indices}
    return net, additional_indices


def _add_pp_asym_gens(comp, net):
    # sgen works as well
    ind = []
    for row in comp:
        ind.append(pp.create_asymmetric_load(
            net, bus=row['node'], in_service=row['status'], type='wye',
            p_a_mw=-row['p_specified'][0] * 1e-6, p_b_mw=-row['p_specified'][1] * 1e-6,
            p_c_mw=-row['p_specified'][2] * 1e-6,
            q_a_mvar=-row['q_specified'][0] * 1e-6, q_b_mvar=-row['q_specified'][1] * 1e-6,
            q_c_mvar=-row['q_specified'][2] * 1e-6))
    return ind


def _add_pp_asym_loads(comp, net):
    ind = []
    for row in comp:
        ind.append(pp.create_asymmetric_load(
            net, bus=row['node'], in_service=row['status'], type='wye',
            p_a_mw=row['p_specified'][0] * 1e-6, p_b_mw=row['p_specified'][1] * 1e-6,
            p_c_mw=row['p_specified'][2] * 1e-6,
            q_a_mvar=row['q_specified'][0] * 1e-6, q_b_mvar=row['q_specified'][1] * 1e-6,
            q_c_mvar=row['q_specified'][2] * 1e-6))
    return ind


def _add_pp_shunt(comp, net, type_of_output):
    ind = []
    # no g0 b0 functionality in pp
    for row in comp:
        ind.append(pp.create_shunt(net, bus=row['node'], in_service=row['status'], index=row['id'],
                                   p_mw=row['g1'] * (net['bus'].loc[row['node'], 'vn_kv']) ** 2 / (
                                       3 if type_of_output == 'asym_output' else 1),
                                   q_mvar=-row['b1'] * (net['bus'].loc[row['node'], 'vn_kv']) ** 2 / (
                                       3 if type_of_output == 'asym_output' else 1)))
    return ind


def _add_pp_sgens(comp, net):
    return pp.create_loads(net, index=comp['id'], buses=comp['node'], p_mw=-comp['p_specified'] * 1e-6,
                           q_mvar=-comp['q_specified'] * 1e-6, in_service=comp['status'], type='wye',
                           const_z_percent=[100 if i == LoadGenType.const_impedance else 0 for i in comp['type']],
                           const_i_percent=[100 if i == LoadGenType.const_current else 0 for i in comp['type']])


def _add_pp_loads(comp, net):
    return pp.create_loads(net, index=comp['id'], buses=comp['node'], p_mw=comp['p_specified'] * 1e-6,
                           q_mvar=comp['q_specified'] * 1e-6, in_service=comp['status'],
                           const_z_percent=[100 if i == 1 else 0 for i in comp['type']],
                           const_i_percent=[100 if i == 2 else 0 for i in comp['type']], type='wye')


def _add_pp_sources(comp, net, type_of_output):
    # Pandapower ext_grid does not contain source impedance element. Adding manually only for symmetric calculations.
    # Not possible for asymmetric calculation. Set sk as ~inf
    # Zs= R+jX where R/X=rx_ratio. Set Zs=R^2+X^2=1 for pu calculations corresponding to base mva 'sk'.

    ind = []
    for row in comp:
        # High negative and zero sequence
        xs_i = 1 / np.sqrt(1 + row['rx_ratio'] ** 2)  # * (net['sn_mva'] / (row['sk'] * 1e-6))
        rs_i = xs_i * row['rx_ratio']

        if type_of_output == 'sym_output':
            bus_id = pp.create_bus(net, vn_kv=net['bus'].loc[row['node'], 'vn_kv'])
            # ind.append(pp.create_line_from_parameters(net, from_bus=bus_id, to_bus=row['node'],
            #                               length_km=1, max_i_ka=1, r_ohm_per_km=rs_i,
            #                               x_ohm_per_km=xs_i, c_nf_per_km=0, r0_ohm_per_km=rs_i * row['z01_ratio'],
            #                               x0_ohm_per_km=xs_i * row['z01_ratio'], c0_nf_per_km=0))
            ind.append(pp.create_impedance(net, from_bus=bus_id, to_bus=row['node'],
                                           rft_pu=rs_i, xft_pu=xs_i, sn_mva=row['sk'] * 1e-6))
            pp.create_ext_grid(net, bus=bus_id, vm_pu=row['u_ref'], in_service=row['status'])
        else:
            ind.append(pp.create_ext_grid(net, bus=row['node'], vm_pu=row['u_ref'], in_service=row['status'],
                                          s_sc_max_mva=row['sk'] * 1e-6, rx_max=row['rx_ratio'],
                                          r0x0_max=row['rx_ratio'],
                                          x0x_max=row['z01_ratio']))
    return ind


def _add_pp_buses(comp, net):
    return pp.create_buses(net, nr_buses=len(comp['id']), index=comp['id'],
                           vn_kv=comp['u_rated'] * 1e-3)


def _add_pp_transformer(comp, net):
    # vkr_percent from pk. r= z* (pk * u2^2/sn^2). vkr= pk/sn
    pp_windings = {WindingType.wye: 'y', WindingType.wye_n: 'yn', WindingType.delta: 'D'}
    vector_group_string = [pp_windings[pri].capitalize() + pp_windings[sec] for pri, sec in
                           zip(comp['winding_from'], comp['winding_to'])]
    # vector_group_string = pp_windings[comp['winding_from']].capitalize() + pp_windings[comp['winding_to']]
    tap_size = comp['tap_size'] / [row['u1'] if row['tap_side'] == BranchSide.from_side else row['u2'] for row in
                                   comp]
    # Check mag0, si0 (All zero sequence)
    ind = pp.create_transformers_from_parameters(
        net, lv_buses=comp['to_node'], hv_buses=comp['from_node'], sn_mva=comp['sn'] * 1e-6,
        vn_lv_kv=comp['u2'] * 1e-3,
        vn_hv_kv=comp['u1'] * 1e-3, vkr_percent=comp['pk'] * 100 / comp['sn'], vk_percent=comp['uk'] * 100,
        pfe_kw=comp['p0'] * 1e-3, i0_percent=comp['i0'] * 100, vector_group=vector_group_string,
        vk0_percent=comp['uk'] * 100, vkr0_percent=comp['pk'] * 100 / comp['sn'], mag0_percent=comp['i0'] * 100,
        mag0_rx=comp['p0'] / comp['sn'] * 100,
        si0_hv_partial=0.9, in_service=True, parallel=1, shift_degree=comp['clock'].astype(float) * 30,
        tap_side=comp['tap_side'], tap_pos=comp['tap_pos'], tap_neutral=comp['tap_nom'],
        tap_max=comp['tap_max'], tap_min=comp['tap_min'], tap_step_percent=tap_size * 100)
    pp.create_switches(net, buses=comp['from_node'], elements=ind, et=['t', ] * len(comp),
                       closed=comp['from_status'])
    pp.create_switches(net, buses=comp['to_node'], elements=ind, et=['t', ] * len(comp),
                       closed=comp['to_status'])
    return ind


def _add_pp_lines(comp, net):
    ind = pp.create_lines_from_parameters(net, index=comp['id'], from_buses=comp['from_node'], to_buses=comp['to_node'],
                                          in_service=True, length_km=1, max_i_ka=comp['i_n'] * 1e-3,
                                          r_ohm_per_km=comp['r1'], g0_us_per_km=0,
                                          x_ohm_per_km=comp['x1'], c_nf_per_km=comp['c1'] * 1e9,
                                          r0_ohm_per_km=comp['r0'],
                                          x0_ohm_per_km=comp['x0'], c0_nf_per_km=comp['c0'] * 1e9)
    pp.create_switches(net, buses=comp['from_node'], elements=comp['id'], et=['l', ] * len(comp),
                       closed=comp['from_status'].astype(bool))
    pp.create_switches(net, buses=comp['to_node'], elements=comp['id'], et=['l', ] * len(comp),
                       closed=comp['to_status'].astype(bool))
    return ind


def pp2pgm_sym_result_converter(net, additional_indices):
    """
    Converts pandapower result to arrays in the format of power-grid-model results for comparison.

    Not implemented properly: energized
    Confirm if filling nan with 0 is proper for nodes
    """
    pgm2pp_input = {'node': 'bus', 'line': 'line', 'transformer': 'trafo',
                    'shunt': 'shunt', 'source': 'ext_grid', 'sym_gen': 'load',
                    'sym_load': 'load'}
    result = {}

    # Neglect additional busses created for source
    comp = (net['res_bus'].loc[additional_indices['pp_indices']['node']]).fillna(0)
    node_result = initialize_array('sym_output', 'node', len(comp))
    node_result['id'] = comp.index
    node_result['u_pu'] = comp['vm_pu']
    node_result['u_angle'] = comp['va_degree'] * (math.pi / 180)
    node_result['u'] = comp['vm_pu'] * net['bus'].loc[additional_indices['pp_indices']['node'], 'vn_kv'] * 1e3
    result['node'] = node_result

    if 'transformer' in additional_indices['pp_indices']:
        comp = net['res_trafo']
        transformer_result = initialize_array('sym_output', 'transformer', len(comp))
        transformer_result['id'] = additional_indices['pgm_indices']['transformer']
        transformer_result['p_from'] = comp['p_hv_mw'] * 1e6
        transformer_result['q_from'] = comp['q_hv_mvar'] * 1e6
        transformer_result['p_to'] = comp['p_lv_mw'] * 1e6
        transformer_result['q_to'] = comp['q_lv_mvar'] * 1e6
        transformer_result['i_from'] = comp['i_hv_ka'] * 1e3
        transformer_result['i_to'] = comp['i_lv_ka'] * 1e3
        transformer_result['s_from'] = np.sqrt(
            comp['p_hv_mw'] ** 2 + comp['q_hv_mvar'] ** 2) * 1e6
        transformer_result['s_to'] = np.sqrt(
            comp['p_lv_mw'] ** 2 + comp['q_lv_mvar'] ** 2) * 1e6
        transformer_result['loading'] = comp['loading_percent'] * 1e-2
        result['transformer'] = transformer_result

    if 'line' in additional_indices['pp_indices']:
        comp = net['res_line']
        line_result = initialize_array('sym_output', 'line', len(comp))
        line_result['id'] = comp.index
        line_result['p_from'] = comp['p_from_mw'] * 1e6
        line_result['q_from'] = comp['q_from_mvar'] * 1e6
        line_result['p_to'] = comp['p_to_mw'] * 1e6
        line_result['q_to'] = comp['q_to_mvar'] * 1e6
        line_result['i_from'] = comp['i_from_ka'] * 1e3
        line_result['i_to'] = comp['i_to_ka'] * 1e3
        line_result['s_from'] = np.sqrt(comp['p_from_mw'] ** 2 + comp['q_from_mvar'] ** 2) * 1e6
        line_result['s_to'] = np.sqrt(comp['p_to_mw'] ** 2 + comp['q_to_mvar'] ** 2) * 1e6
        line_result['loading'] = comp['loading_percent'] * 1e-2
        result['line'] = line_result

    appl_result = {}
    for pgm_appl, pp_appl in {'shunt': 'res_shunt', 'sym_gen': 'res_load', 'sym_load': 'res_load'}.items():
        if pgm_appl in additional_indices['pp_indices']:
            if pgm_appl == 'sym_gen' and 'sym_gen' in additional_indices['pp_indices']:
                comp = net[pp_appl].loc[additional_indices['pp_indices']['sym_gen']]
            elif pgm_appl == 'sym_load' and 'sym_load' in additional_indices['pp_indices']:
                comp = net[pp_appl].loc[additional_indices['pp_indices']['sym_load']]
            else:
                comp = net[pp_appl]
            appl_result[pgm_appl] = initialize_array('sym_output', pgm_appl, len(comp))
            appl_result[pgm_appl]['id'] = comp.index
            appl_result[pgm_appl]['p'] = comp['p_mw'] * 1e6 * (-1 if pgm_appl == 'sym_gen' else 1)
            appl_result[pgm_appl]['q'] = comp['q_mvar'] * 1e6 * (-1 if pgm_appl == 'sym_gen' else 1)
            result[pgm_appl] = appl_result[pgm_appl]

    if not net['res_ext_grid'].empty:
        source_impedance_index = additional_indices['pp_indices']['source']
        source_equivalent_buses = net['impedance'].loc[source_impedance_index, 'to_bus']
        source_result = initialize_array('sym_output', 'source', len(net['ext_grid']))
        source_result['id'] = additional_indices['pgm_indices']['source']
        source_result['p'] = -net['res_impedance'].loc[source_impedance_index, 'p_to_mw'] * 1e6
        source_result['q'] = -net['res_impedance'].loc[source_impedance_index, 'q_to_mvar'] * 1e6

        # repeated code, find alternative later
        mag = np.array(net['res_bus'].loc[source_equivalent_buses, 'vm_pu'])
        ang = np.array(net['res_bus'].loc[source_equivalent_buses, 'va_degree'])
        bus_voltage = np.array(
            (mag * np.cos(ang) + mag * np.sin(ang) * 1j) * net['bus'].loc[source_equivalent_buses, 'vn_kv'] * 1e3)
        s = np.array(source_result['p'] + source_result['q'] * 1j)
        source_result['s'] = np.abs(s)
        source_result['i'] = [np.abs(np.conjugate(s_i) / v) / math.sqrt(3) for s_i, v in zip(s, bus_voltage)]
        # source_result['pf'] = [np.cos(np.angle(i)) if i > 0 else 0 for i in s]
        result['source'] = source_result

    # Sorting for easy comparison
    result = {key: value for key, value in sorted(result.items())}
    return result


def pp2pgm_asym_result_converter(net, additional_indices):
    """
    Converts pandapower result to arrays in the format of power-grid-model results for comparison.

    Not implemented properly: energized
    Confirm if filling nan with 0 is proper for nodes
    """
    result = {}
    # Neglect additional busses created for source
    comp = (net['res_bus_3ph'].loc[additional_indices['pp_indices']['node']]).fillna(0)
    node_result = initialize_array('asym_output', 'node', len(comp))
    node_result['id'] = comp.index
    u_pu = list(zip(comp['vm_a_pu'], comp['vm_b_pu'], comp['vm_c_pu']))
    node_result['u_pu'] = u_pu
    u_angle = [np.multiply([i, j, k], math.pi / 180) for i, j, k in
               zip(comp['va_a_degree'], comp['va_b_degree'], comp['va_c_degree'])]
    node_result['u_angle'] = u_angle
    node_result['u'] = [np.multiply(i, j) * 1e3 / math.sqrt(3) for i, j in
                        zip(u_pu, net['bus'].loc[additional_indices['pp_indices']['node'], 'vn_kv'])]
    result['node'] = node_result

    comp = net['res_trafo_3ph']
    if not comp.empty:
        transformer_result = initialize_array('asym_output', 'transformer', len(net['res_trafo_3ph']))
        transformer_result['id'] = additional_indices['pgm_indices']['transformer']
        transformer_result['p_from'] = np.array(
            list(zip(comp['p_a_hv_mw'], comp['p_b_hv_mw'], comp['p_c_hv_mw']))) * 1e6
        transformer_result['q_from'] = np.array(
            list(zip(comp['q_a_hv_mvar'], comp['q_b_hv_mvar'], comp['q_c_hv_mvar']))) * 1e6
        transformer_result['p_to'] = np.array(list(zip(comp['p_a_lv_mw'], comp['p_b_lv_mw'], comp['p_c_lv_mw']))) * 1e6
        transformer_result['q_to'] = np.array(
            list(zip(comp['q_a_lv_mvar'], comp['q_b_lv_mvar'], comp['q_c_lv_mvar']))) * 1e6
        transformer_result['i_from'] = np.array(
            list(zip(comp['i_a_hv_ka'], comp['i_b_hv_ka'], comp['i_c_hv_ka']))) * 1e3
        transformer_result['i_to'] = np.array(list(zip(comp['i_a_lv_ka'], comp['i_b_lv_ka'], comp['i_c_lv_ka']))) * 1e3
        result['transformer'] = transformer_result

    comp = net['res_line_3ph']
    if not comp.empty:
        # comp = comp.loc[additional_indices['pp_indices']['line']]
        line_result = initialize_array('asym_output', 'line', len(comp))
        line_result['id'] = comp.index
        line_result['p_from'] = np.array(list(zip(comp['p_a_from_mw'], comp['p_b_from_mw'], comp['p_c_from_mw']))) * 1e6
        line_result['q_from'] = np.array(
            list(zip(comp['q_a_from_mvar'], comp['q_b_from_mvar'], comp['q_c_from_mvar']))) * 1e6
        line_result['p_to'] = np.array(list(zip(comp['p_a_to_mw'], comp['p_b_to_mw'], comp['p_c_to_mw']))) * 1e6
        line_result['q_to'] = np.array(list(zip(comp['q_a_to_mvar'], comp['q_b_to_mvar'], comp['q_c_to_mvar']))) * 1e6
        line_result['i_from'] = np.array(list(zip(comp['i_a_from_ka'], comp['i_b_from_ka'], comp['i_c_from_ka']))) * 1e3
        line_result['i_to'] = np.array(list(zip(comp['i_a_to_ka'], comp['i_b_to_ka'], comp['i_c_to_ka']))) * 1e3
        result['line'] = line_result

    appl_result = {}
    for pgm_appl, pp_appl in {'shunt': 'res_shunt_3ph', 'sym_gen': 'res_load_3ph', 'sym_load': 'res_load_3ph'}.items():
        if pgm_appl in additional_indices['pp_indices']:
            if pgm_appl == 'sym_gen' and 'sym_gen' in additional_indices['pp_indices']:
                comp = net[pp_appl].loc[additional_indices['pp_indices']['sym_gen']]
            elif pgm_appl == 'sym_load' and 'sym_load' in additional_indices['pp_indices']:
                comp = net[pp_appl].loc[additional_indices['pp_indices']['sym_load']]
            else:
                comp = net[pp_appl]
            if not comp.empty:
                appl_result[pgm_appl] = initialize_array('asym_output', pgm_appl, len(net[pp_appl]))
                appl_result[pgm_appl]['id'] = comp.index
                appl_result[pgm_appl]['p'] = np.array(
                    list(zip(comp['p_mw'] / 3, comp['p_mw'] / 3, comp['p_mw'] / 3))) * 1e6 * (
                                                 -1 if pgm_appl == 'sym_gen' else 1)
                appl_result[pgm_appl]['q'] = np.array(
                    list(zip(comp['q_mvar'] / 3, comp['q_mvar'] / 3, comp['q_mvar'] / 3))) * 1e6 * (
                                                 -1 if pgm_appl == 'sym_gen' else 1)
                result[pgm_appl] = appl_result[pgm_appl]

    appl_result = {}
    for pgm_appl, pp_appl in {'asym_load': 'res_asymmetric_load_3ph', 'asym_gen': 'res_asymmetric_load_3ph'}.items():
        if pgm_appl in additional_indices['pp_indices']:
            if pgm_appl == 'asym_gen':
                comp = net[pp_appl].loc[additional_indices['pp_indices']['asym_gen']]
            else:
                comp = net[pp_appl].loc[additional_indices['pp_indices']['asym_load']]
            appl_result[pgm_appl] = initialize_array('asym_output', pgm_appl, len(net[pp_appl]))
            appl_result[pgm_appl]['id'] = additional_indices['pgm_indices'][pgm_appl]
            appl_result[pgm_appl]['p'] = np.array(list(zip(comp['p_a_mw'], comp['p_b_mw'], comp['p_c_mw']))) * 1e6 * (
                -1 if pgm_appl == 'asym_gen' else 1)
            appl_result[pgm_appl]['q'] = np.array(
                list(zip(comp['q_a_mvar'], comp['q_b_mvar'], comp['q_c_mvar']))) * 1e6 * (
                                             -1 if pgm_appl == 'asym_gen' else 1)
            result[pgm_appl] = appl_result[pgm_appl]

    # source modelling is not possible in pp for asymmetric calculations
    # if not net['res_ext_grid_3ph'].empty:
    #    comp = net['res_ext_grid_3ph']
    #    source_result = initialize_array('asym_output', 'source', len(net['ext_grid']))
    #    source_result['id'] = additional_indices['pgm_indices']['source']
    #    source_result['p'] = np.array(list(zip(comp['p_a_mw'], comp['p_b_mw'], comp['p_c_mw'] ))) * 1e6
    #    source_result['q'] = np.array(list(zip(comp['q_a_mvar'], comp['q_b_mvar'], comp['q_c_mvar'] ))) * 1e6
    #    result['source'] = source_result

    # Sorting for easy comparison
    result = {key: value for key, value in sorted(result.items())}
    return result


def pp_sandbox_net():
    """
    Testing function for manually creating a net. Can then be used with pp.net_equals(net1, net2) to check if they match
    """
    net1 = pp.create_empty_network()
    b1 = pp.create_bus(net1, 10.5, index=1)
    b2 = pp.create_bus(net1, 10.5, index=2)
    # b3  = pp.create_bus(net, 10.5)
    pp.create_ext_grid(net1, b1, s_sc_max_mva=1e14, rx_max=0.1, x0x_max=1.0,
                       r0x0_max=0.1, in_service=True)

    pp.create_line_from_parameters(net1, b1, b2, index=3, length_km=1, r0_ohm_per_km=0.25,
                                   x0_ohm_per_km=0.2, c0_nf_per_km=1000,
                                   max_i_ka=1, r_ohm_per_km=0.25,
                                   x_ohm_per_km=0.2, c_nf_per_km=1000)
    # pp.create_load(net1, b2, p_mw=-0.01, q_mvar=-0.002, index=6, const_i_percent=0, const_z_percent=0)
    # pp.create_load(net1, b2, p_mw=-0.01, q_mvar=-0.002, in_service=False, index=7, const_i_percent=0,
    #                const_z_percent=0)
    pp.create_asymmetric_load(net1, b2, p_a_mw=0.01, p_b_mw=0.009, p_c_mw=0.0105, q_a_mvar=0.002, q_b_mvar=0.0015,
                              q_c_mvar=0.0025, index=5)
    pp.create_asymmetric_load(net1, b2, p_a_mw=0.01, p_b_mw=0.009, p_c_mw=0.0105, q_a_mvar=0.002, q_b_mvar=0.0015,
                              q_c_mvar=0.0025,
                              index=6, in_service=False)

    return net1
