# Modelica FMI

A Modelica library to import Functional Mock-up Units.

## Installation

To import an FMU

- [install uv](https://docs.astral.sh/uv/getting-started/installation/) 

- download the `modelica_fmi` wheel from the latest release and install it with 
  
```
uvx install --reinstall path/to/modelica_fmimodelica_fmi-<version>-py3-none-any.whl
```

- download the `Modelica-FMI` library from the latest release, load it in your Modelica tool, and execute 

```
FMI.importFMU()
```

## Features

- Supports FMI 2.0 and FMI 3.0 for Co-Simulation
- Works with all Modelica tools
- Logs FMI calls to the console or to a file

## License

Copyright (c) 2025, Modelica Association.
The code is released under the [2-Clause BSD license](LICENSE).
