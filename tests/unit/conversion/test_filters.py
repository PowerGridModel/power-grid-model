import pytest
from power_grid_model.conversion.filters import get_winding_from, get_winding_to, get_clock
from power_grid_model.enum import WindingType


def test_get_winding_from():
    assert get_winding_from("YNd11") == WindingType.wye_n
    assert get_winding_from("YNyn2") == WindingType.wye_n
    assert get_winding_from("Yyn1") == WindingType.wye
    assert get_winding_from("Dyn5") == WindingType.delta
    with pytest.raises(ValueError):
        get_winding_from("XNd11")


def test_get_winding_to():
    assert get_winding_to("YNd11") == WindingType.delta
    assert get_winding_to("YNyn2") == WindingType.wye_n
    assert get_winding_to("YNy1") == WindingType.wye
    assert get_winding_to("Dyn5") == WindingType.wye_n
    with pytest.raises(ValueError):
        get_winding_to("XNd11")


def test_get_clock():
    assert get_clock("YNd11") == 11
    assert get_clock("YNyn2") == 2
    assert get_clock("YNd1") == 1
    assert get_clock("Dyn5") == 5
    assert get_clock("YNd0") == 0
    assert get_clock("YNd12") == 12
    with pytest.raises(ValueError):
        get_clock("YNd-1")
        get_clock("YNd13")
