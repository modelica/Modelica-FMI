from os import PathLike
from typing import Literal, Iterable

from modelica_fmi._lib import (
    start_value,
    subscripts,
    modelica_type,
    fmi2_type,
    fmi3_type,
    modifiers,
    choices,
    dependencies,
    dependencies2,
    dependencies3,
    numel,
    set_variables,
    name,
)


class FMUImportError(Exception):
    pass


def import_fmu_to_modelica(
    fmu_path: str | PathLike,
    model_path: str | PathLike,
    interface_type: Literal["CoSimulation"] = "CoSimulation",
    variables: Iterable[str] | None = None,
    basic: bool = False,
    hide_connectors=False,
):
    from os import makedirs
    from pathlib import Path
    from . import __version__ as library_version

    import jinja2
    from fmpy import extract, read_model_description
    from fmpy.util import sha256_checksum

    fmu_path = Path(fmu_path)
    model_path = Path(model_path)
    model_name = model_path.stem

    package_dir = Path(model_path).parent

    if not (package_dir / "package.mo").is_file():
        raise FMUImportError(f"{package_dir} is not a package of a Modelica library.")

    model_description = read_model_description(fmu_path)

    if (
        model_description.defaultExperiment is not None
        and model_description.defaultExperiment.stepSize is not None
    ):
        communicationStepSize = model_description.defaultExperiment.stepSize
    else:
        communicationStepSize = "1e-2"

    if interface_type == "CoSimulation":
        if model_description.coSimulation is None:
            raise FMUImportError("The FMU does not support Co-Simulation.")
        model_identifier = model_description.coSimulation.modelIdentifier
        copyPlatformBinary = (
            model_description.coSimulation.canBeInstantiatedOnlyOncePerProcess
        )
        IT = "CS"
    else:
        raise FMUImportError(
            f"interface_type must be 'CoSimulation', but was '{interface_type}'."
        )

    package_root = package_dir

    package = package_root.name

    while (package_root.parent / "package.order").is_file():
        package_root = package_root.parent
        package = package_root.name + "." + package

    hash = sha256_checksum(fmu_path)

    hash = hash[:7]

    unzipdir = package_root / "Resources" / "FMUs" / hash

    makedirs(unzipdir, exist_ok=True)

    extract(filename=fmu_path, unzipdir=unzipdir)

    loader = jinja2.FileSystemLoader(searchpath=Path(__file__).parent / "templates")

    environment = jinja2.Environment(
        loader=loader,
        trim_blocks=True,
        block_start_string="@@",
        block_end_string="@@",
        variable_start_string="@=",
        variable_end_string="=@",
    )

    fmiMajorVersion = int(model_description.fmiVersion[0])

    template = environment.get_template(
        f"FMI{fmiMajorVersion}_{'Basic' if basic else ''}{IT}.mo"
    )

    parameters = []
    structural_parameters = []

    inputs = []
    outputs = []

    for variable in model_description.modelVariables:
        if variables is not None and variable.name not in variables:
            continue

        if variable.type not in {
            "Float32",
            "Float64",
            "Int8",
            "UInt8",
            "Int16",
            "UInt16",
            "Int32",
            "UInt32",
            "Int64",
            "UInt64",
            "Real",
            "Integer",
            "Enumeration",
            "Boolean",
            "String",
        }:
            continue

        if (
            interface_type == "ModelExchange"
            and variable.type in {"Real", "Float32", "Float64"}
            and variable.variability == "discrete"
        ):
            continue

        if variable.causality == "parameter":
            parameters.append(variable)
        elif variable.causality == "structuralParameter":
            structural_parameters.append(variable)
        elif variable.causality == "input":
            inputs.append(variable)
        elif variable.causality == "output":
            outputs.append(variable)

    width = 1200
    height = max(200, 100 * max(len(inputs), len(outputs)))

    x0 = -int(width / 2)
    x1 = int(width / 2)
    y0 = -int(height / 2)
    y1 = int(height / 2)

    annotations = dict()

    if not hide_connectors:
        for i, variable in enumerate(inputs):
            y = y1 - (i + 1) * (height / (1 + len(inputs)))
            annotations[variable.name] = (
                f" annotation(Placement(transformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }}), iconTransformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }})))"
            )

        for i, variable in enumerate(outputs):
            y = y1 - (i + 1) * (height / (1 + len(outputs)))
            annotations[variable.name] = (
                f" annotation(Placement(transformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }}), iconTransformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }})))"
            )

    variables = dict(
        (variable.valueReference, variable)
        for variable in model_description.modelVariables
    )

    template.globals.update(
        {
            "start_value": lambda *args, **kwargs: start_value(
                variables, *args, **kwargs
            ),
            "subscripts": lambda *args, **kwargs: subscripts(
                variables, *args, **kwargs
            ),
            "modelica_type": modelica_type,
            "fmi_type": fmi2_type if fmiMajorVersion == 2 else fmi3_type,
            "modifiers": modifiers,
            "choices": choices,
            "name": name,
            "dependencies": lambda *args, **kwargs: dependencies(
                model_description, *args, **kwargs
            ),
            "dependencies2": lambda *args, **kwargs: dependencies2(
                model_description, *args, **kwargs
            ),
            "dependencies3": lambda *args, **kwargs: dependencies3(
                model_description, variables, *args, **kwargs
            ),
            "numel": lambda *args, **kwargs: numel(variables, *args, **kwargs),
            "set_variables": set_variables,
        }
    )

    stop_time = getattr(model_description.defaultExperiment, "stopTime", 1)

    class_text = template.render(
        hash=hash,
        libraryVersion=library_version,
        rootPackage=package_root.name,
        package=package,
        description=model_description.description,
        fmiMajorVersion=fmiMajorVersion,
        modelName=model_name,
        modelIdentifier=model_identifier,
        interfaceType=0 if interface_type == "ModelExchange" else 1,
        instantiationToken=model_description.guid,
        nx=model_description.numberOfContinuousStates,
        nz=model_description.numberOfEventIndicators,
        parameters=parameters,
        structuralParameters=structural_parameters,
        communicationStepSize=communicationStepSize,
        x0=x0,
        x1=x1,
        y0=y0,
        y1=y1,
        inputs=inputs,
        outputs=outputs,
        continuousOutputs=[v for v in outputs if v.variability == "continuous"],
        discreteOutputs=[v for v in outputs if v.variability == "discrete"],
        annotations=annotations,
        realInputVRs=[str(v.valueReference) for v in inputs if v.type == "Real"],
        realInputs=[v.name for v in inputs if v.type == "Real"],
        integerInputVRs=[str(v.valueReference) for v in inputs if v.type == "Integer"],
        integerInputs=[v.name for v in inputs if v.type == "Integer"],
        booleanInputVRs=[str(v.valueReference) for v in inputs if v.type == "Boolean"],
        booleanInputs=[v.name for v in inputs if v.type == "Boolean"],
        stopTime=stop_time,
        copyPlatformBinary=copyPlatformBinary,
        enumerationTypeDefinitions=[
            t for t in model_description.typeDefinitions if t.type == "Enumeration"
        ],
    )

    with open(model_path, "w") as f:
        f.write(class_text)

    if (package_dir / "package.order").is_file():
        with open(package_dir / "package.order", "r") as f:
            package_order = list(map(lambda line: line.strip(), f.readlines()))
    else:
        package_order = []

    if model_name not in package_order:
        with open(package_dir / "package.order", "a") as f:
            f.write(model_name + "\n")
