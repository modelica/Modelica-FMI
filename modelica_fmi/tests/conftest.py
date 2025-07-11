import shutil
from os import makedirs
import pytest
from pathlib import Path
from fmpy import extract
from fmpy.util import download_file


def pytest_addoption(parser):
    parser.addoption(
        "--executable", default=None, help="Dymola executable to connect to"
    )


@pytest.fixture(scope="session")
def package_dir():
    yield Path(__file__).parent.parent.parent / "FMI"


@pytest.fixture(scope="session")
def work_dir():
    path = Path(__file__).parent / "work"
    shutil.rmtree(path)
    makedirs(path)
    yield path


@pytest.fixture(scope="session")
def resources_dir():
    yield Path(__file__).parent / "resources"


@pytest.fixture(scope="session")
def reference_fmus_dist_dir(resources_dir):
    version = "0.0.39"
    checksum = "6863d55e5818e1ca4e4614c4d4ba4047a921b4495f6336e7002874ed791f6c2a"

    zip_file = download_file(
        url=f"https://github.com/modelica/Reference-FMUs/releases/download/v{version}/Reference-FMUs-{version}.zip",
        checksum=checksum,
    )

    dist_dir = resources_dir / "Reference-FMUs-dist"

    extract(filename=zip_file, unzipdir=dist_dir)

    yield dist_dir


@pytest.fixture(scope="session")
def reference_fmus_repo_dir(resources_dir):
    yield Path(__file__).parent.parent / "thirdparty" / "Reference-FMUs"


@pytest.fixture(scope="session")
def executable(request):
    from pymola import findDymolaExecutables

    executable = request.config.getoption("--executable")

    if executable is None:
        executables = findDymolaExecutables()
        executable = executables[-1]

    yield executable


@pytest.fixture(scope="module")
def dymola(executable, work_dir):
    from pymola import Dymola

    with Dymola(executable=executable, showWindow=True, debug=False) as dymola:
        dymola.cd(work_dir)

        # ensure, that MSL is loaded, as functions like exportSSP do not trigger demand loading
        dymola.openModelFile("Modelica")

        # build 64-bit binaries
        dymola.setVariable("Advanced.CompileWith64", 2)

        yield dymola
