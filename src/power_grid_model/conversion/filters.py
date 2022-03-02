import re
from typing import Tuple

from ..enum import WindingType

CONNECTION_PATTERN = re.compile(r'(Y|YN|D|Z|ZN)(y|yn|d|z|zn)(1?[0-9])')

WINDING_TYPES = {
    "Y": WindingType.wye,
    "YN": WindingType.wye_n,
    "D": WindingType.delta,
    "Z": WindingType.wye,
    "ZN": WindingType.wye_n,
}


def relative_no_load_current(*args):  # TODO
    return 0.0


def neutral_grounding(*args):  # TODO
    return 0.0


def scaled_power(*args):  # TODO
    return 0.0


def complex_inverse_real_part(*args):  # TODO
    return 0.0


def complex_inverse_imaginary_part(*args):  # TODO
    return 0.0


def get_winding_from(conn_str: str, neutral_grounding: bool = True) -> WindingType:  # TODO
    winding_str, _, _ = _split_connection_string(conn_str)
    return WINDING_TYPES[winding_str]


def get_winding_to(conn_str: str, neutral_grounding: bool = True) -> WindingType:  # TODO
    _, winding_str, _ = _split_connection_string(conn_str)
    return WINDING_TYPES[winding_str.upper()]


def get_clock(conn_str: str) -> int:
    _, _, clock_str = _split_connection_string(conn_str)
    clock = int(clock_str)
    if clock > 12:
        raise ValueError
    return clock


def _split_connection_string(conn_str: str) -> Tuple[str, str, str]:
    match = CONNECTION_PATTERN.fullmatch(conn_str)
    if not match:
        raise ValueError(f"Invalid transformer connection string: '{conn_str}'")
    return match.groups()
