import os
from itertools import product

import numpy as np
import pytest

from fmpy import simulate_fmu, read_model_description
from fmpy.util import validate_signal
from modelica_fmi.generate_examples import generate_examples

models = ["BouncingBall", "Dahlquist", "Stair", "Resource", "VanDerPol"]
fmi_versions = [2, 3]
interface_types = ["CoSimulation"]  # , 'ModelExchange']


def pymola_available():
    try:
        import pymola  # noqa

        return True
    except:  # noqa
        return False


def om_available():
    try:
        import OMPython  # noqa
    except:  # noqa
        return False

    return "OPENMODELICAHOME" in os.environ


def test_import_fmu_to_modelica():
    generate_examples()


@pytest.mark.skipif(
    not om_available() or not pymola_available(),
    reason="OpenModelica and/or Pymola was not found",
)
@pytest.mark.parametrize(
    "fmi_version, interface_type, model", product(fmi_versions, interface_types, models)
)
def test_run_examples_in_om(
    package_dir, reference_fmus_dist_dir, work_dir, fmi_version, interface_type, model
):
    from OMPython import OMCSessionZMQ
    from pymola import Dsres

    omc = OMCSessionZMQ()

    package = package_dir / "package.mo"

    assert omc.sendExpression(f'cd("{work_dir.as_posix()}")')

    assert omc.sendExpression(f'loadFile("{package.as_posix()}")')

    if model == "BouncingBall":
        stop_time = 3
    elif model == "Dahlquist":
        stop_time = 10
    elif model == "Stair":
        stop_time = 8
    elif model == "Resource":
        stop_time = 1
    elif model == "VanDerPol":
        stop_time = 20

    info = omc.sendExpression(
        f"simulate(FMI.Examples.FMI{fmi_version}.{model}, stopTime={stop_time})"
    )

    result = Dsres(info["resultFile"])

    filename = str(reference_fmus_dist_dir / f"{fmi_version}.0" / f"{model}.fmu")

    model_description = read_model_description(filename)

    outputs = [
        v.name for v in model_description.modelVariables if v.causality == "output"
    ]

    reference = simulate_fmu(
        filename=filename,
        # start_values=start_values,
        output=outputs,
        stop_time=stop_time,
        fmi_type=interface_type,
    )

    for name in outputs:
        _, _, _, i_out = validate_signal(
            result["Time"], result[name], reference["time"], reference[name]
        )
        assert not i_out.any()


@pytest.mark.skipif(not pymola_available(), reason="Pymola was not found")
@pytest.mark.parametrize(
    "fmi_version, interface_type, model", product(fmi_versions, interface_types, models)
)
def test_run_examples_in_dymola(
    dymola, package_dir, reference_fmus_dist_dir, fmi_version, interface_type, model
):
    dymola.loadClass(package_dir / "package.mo")

    start_values = {}
    stop_time = None

    if model == "BouncingBall":
        start_values = {"e": 0.8}
    elif model == "Stair":
        stop_time = 8
    elif model == "VanDerPol":
        start_values = {"mu": 2.5}

    filename = str(reference_fmus_dist_dir / f"{fmi_version}.0" / f"{model}.fmu")

    model_description = read_model_description(filename)

    outputs = [
        v.name for v in model_description.modelVariables if v.causality == "output"
    ]

    reference = simulate_fmu(
        filename=filename,
        start_values=start_values,
        output=outputs,
        stop_time=stop_time,
        fmi_type=interface_type,
    )

    intial_values = dict(map(lambda i: (i[0], i[1]), start_values.items()))

    result = dymola.simulate(
        model=f"FMI.Examples.FMI{fmi_version}.{model}",
        initialValues=intial_values,
        stopTime=stop_time,
    )

    for name in outputs:
        _, _, _, i_out = validate_signal(
            result["Time"], result[name], reference["time"], reference[name]
        )
        assert not i_out.any()

    for name, expected in intial_values.items():
        actual = result[name]
        assert np.allclose(actual, expected)
