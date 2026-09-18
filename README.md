![](FMI/Resources/Images/FMI_package.svg)

# Modelica FMI

Use [Functional Mock-up Units](https://fmi-standard.org/) in Modelica.

## Highlights

- Supports FMI 2.0 and FMI 3.0 for Co-Simulation.
- Works with all Modelica tools.
- Logs FMI calls to the console or to a file.

## Usage

- download the [latest release](https://github.com/modelica/Modelica-FMI/releases/latest/download/Modelica-FMI.zip)
- extract the archive and load the `FMI` library into your Modelica tool
- import an FMU by running `modelica-fmi` from the respective platform directory (e.g. `FMI/Resources/Library/win64` on Windows)

```
modelica-fmi /path/to/BouncingBall.fmu /path/to/BouncingBall.mo
```

## License

Copyright &copy; 2026 Modelica Association.
The code is released under the [2-Clause BSD license](LICENSE).
