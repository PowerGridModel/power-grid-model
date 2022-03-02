import re
from typing import Tuple

from ..enum import WindingType

CONNECTION_PATTERN = re.compile(r'(Y|YN|D)(y|yn|d)(1?[0-9])')

WINDING_TYPES = {
    "Y": WindingType.wye,
    "YN": WindingType.wye_n,
    "D": WindingType.delta,
}


def inverse(val: float):
    return 1 / val if val != 0 else float('inf')


def get_winding_from(conn_str: str) -> WindingType:
    winding_str, _, _ = _split_connection_string(conn_str)
    return WINDING_TYPES[winding_str]


def get_winding_to(conn_str: str) -> WindingType:
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
