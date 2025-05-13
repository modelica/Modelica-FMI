# Modelica FMI

A Modelica library to import Functional Mock-up Units.

## Installation

To import an FMU

- [install uv](https://docs.astral.sh/uv/getting-started/installation/)

- install the `modelica-fmi` command with
```
uv tool install --reinstall https://github.com/modelica/Modelica-FMI/releases/download/v0.0.3/modelica_fmi-0.0.3-py3-none-any.whl
```

- download the [Modelica-FMI](https://github.com/modelica/Modelica-FMI/releases/download/v0.0.3/Modelica-FMI-0.0.3.zip) package, load it in your Modelica tool, and call [FMI.importFMU()](FMI/importFMU.mo)

## Features

- Supports FMI 2.0 and FMI 3.0 for Co-Simulation
- Works with all Modelica tools
- Logs FMI calls to the console or to a file

## License

Copyright &copy; 2025 Modelica Association.
The code is released under the [2-Clause BSD license](LICENSE).
