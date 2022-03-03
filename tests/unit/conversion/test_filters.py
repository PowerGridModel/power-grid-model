import pytest
from power_grid_model.conversion.filters import get_winding_from, get_winding_to, get_clock
from power_grid_model.enum import WindingType


def test_get_winding_from():
    assert get_winding_from("Yy1") == WindingType.wye
    assert get_winding_from("Yyn2") == WindingType.wye
    assert get_winding_from("Yd3") == WindingType.wye
    assert get_winding_from("YNy4") == WindingType.wye_n
    assert get_winding_from("YNyn5") == WindingType.wye_n
    assert get_winding_from("YNd6") == WindingType.wye_n
    assert get_winding_from("Dy7") == WindingType.delta
    assert get_winding_from("Dyn8") == WindingType.delta
    assert get_winding_from("Dd9") == WindingType.delta
    with pytest.raises(ValueError):
        get_winding_from("XNd11")


def test_get_winding_to():
    assert get_winding_to("Yy1") == WindingType.wye
    assert get_winding_to("Yyn2") == WindingType.wye_n
    assert get_winding_to("Yd3") == WindingType.delta
    assert get_winding_to("YNy4") == WindingType.wye
    assert get_winding_to("YNyn5") == WindingType.wye_n
    assert get_winding_to("YNd6") == WindingType.delta
    assert get_winding_to("Dy7") == WindingType.wye
    assert get_winding_to("Dyn8") == WindingType.wye_n
    assert get_winding_to("Dd9") == WindingType.delta
    with pytest.raises(ValueError):
        get_winding_to("XNd11")


def test_get_clock():
    assert get_clock("YNd0") == 0
    assert get_clock("YNyn5") == 5
    assert get_clock("YNd12") == 12
    with pytest.raises(ValueError):
        get_clock("YNd-1")
    with pytest.raises(ValueError):
        get_clock("YNd13")
