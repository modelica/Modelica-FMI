![](https://github.com/user-attachments/assets/dbcd7e72-cdb3-48b5-8323-a1b89c328ea2)

# Modelica FMI

A Modelica library to import [Functional Mock-up Units](https://fmi-standard.org/).

## Highlights

- :white_check_mark: supports FMI 2.0 and FMI 3.0 for Co-Simulation
- :tada: works with all Modelica tools
- :page_with_curl: logs FMI calls to the console or to a file

## Installation

To import FMUs install the `modelica-fmi` and `modelica-fmi-gui` commands with [uv](https://docs.astral.sh/uv/getting-started/installation/):

```bash
uv tool install --reinstall https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/modelica_fmi-0.0.6-py3-none-any.whl
```

Or, with `pip`:

```bash
pip install https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/modelica_fmi-0.0.6-py3-none-any.whl
```

To simulate imported FMUs download the [Modelica-FMI](https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/Modelica-FMI-0.0.6.zip) package and load it in your Modelica tool.

## Usage

To open the import dialog run

```bash
modelica-fmi-gui
```

or

```bash
modelica-fmi /path/to/model.fmu /path/to/package/model.mo
```

to import an FMU directly.

In Dymola you can open the import dialog by opening the `FMI` package and selecting `Simulation > Commands > Import FMU...`.

After the import you have to reload the package.

## Building the binaries from source

To build the binaries in `FMI/Resources/Library` from source install CMake

```
uv tool install cmake
```

clone the repository, and run

```
cmake -S runtime -B runtime/build
```

```
cmake --build runtime/build --config release --target install
```

## Building distributable import commands

To build distributable version of the `modelica-fmi` and `modelica-fmi-gui` commands

- create a new virtual environment
```
uv venv
```

- install PyInstaller and modelica_fmi
```
uv pip install pyinstaller https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/modelica_fmi-0.0.6-py3-none-any.whl
```

- build modelica-fmi-gui
```
uv run pyinstaller --windowed --name modelica-fmi-gui --collect-data modelica_fmi .venv/Lib/site-packages/modelica_fmi/gui/__main__.py
```

- ...or modelica-fmi
```
uv run pyinstaller --name modelica-fmi --collect-data modelica_fmi .venv/Lib/site-packages/modelica_fmi/__main__.py
```

## License

Copyright &copy; 2025 Modelica Association.
The code is released under the [2-Clause BSD license](LICENSE).
