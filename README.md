![](https://github.com/user-attachments/assets/dbcd7e72-cdb3-48b5-8323-a1b89c328ea2)

# Modelica FMI

A Modelica library to import [Functional Mock-up Units](https://fmi-standard.org/).

## Highlights

- :white_check_mark: supports FMI 2.0 and FMI 3.0 for Co-Simulation
- :tada: works with all Modelica tools
- :page_with_curl: logs FMI calls to the console or to a file

## Installation

- [install uv](https://docs.astral.sh/uv/getting-started/installation/)

- install the `modelica-fmi` command with
```
uv tool install --reinstall https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/modelica_fmi-0.0.6-py3-none-any.whl
```

- download the [Modelica-FMI](https://github.com/modelica/Modelica-FMI/releases/download/v0.0.6/Modelica-FMI-0.0.6.zip) package and load it in your Modelica tool

## Usage

To import an FMU into an existing package call [FMI.importFMU()](FMI/importFMU.mo) or run

```
modelica-fmi /path/to/model.fmu /path/to/package/model.mo
```

and reload the package.

## License

Copyright &copy; 2025 Modelica Association.
The code is released under the [2-Clause BSD license](LICENSE).
