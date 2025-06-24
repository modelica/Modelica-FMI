from os import PathLike
from typing import Literal, Iterable

from modelica_fmi._lib import (
    start_value,
    subscripts,
    modelica_type,
    fmi2_type,
    fmi3_type,
    modifiers,
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
    hide_large_arrays=False,
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
        elif variable.causality == "input" and variable.type != "String":
            inputs.append(variable)
        elif variable.causality == "output" and variable.type != "String":
            outputs.append(variable)

    height = 160

    x0, x1 = -100, 100
    y0, y1 = -80, 80

    annotations = dict()

    if not hide_connectors:
        for i, variable in enumerate(inputs):
            if len(inputs) == 1:
                y = 0
            elif len(inputs) == 2:
                y = -50 if i == 0 else 50
            else:
                y = 0 if len(inputs) == 1 else y1 - i * (height / (len(inputs) - 1))
            annotations[variable.name] = (
                f" annotation(Placement(transformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }}), iconTransformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }})))"
            )

        for i, variable in enumerate(outputs):
            if len(outputs) == 1:
                y = 0
            elif len(outputs) == 2:
                y = -50 if i == 0 else 50
            else:
                y = 0 if len(outputs) == 1 else y1 - i * (height / (len(outputs) - 1))
            annotations[variable.name] = (
                f" annotation(Placement(transformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }}), iconTransformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }})))"
            )

    variables = dict(
        (variable.valueReference, variable)
        for variable in model_description.modelVariables
    )

    if hide_large_arrays:
        for variable in parameters:
            if numel(variables, variable) > 50:
                annotations[variable.name] = (
                    " annotation(Evaluate=true, __Dymola_HideArray=true)"
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

    icon = "modelica://FMI/Resources/Images/FMU_bare.svg"

    if model_description.fmiVersion == "2.0" and (unzipdir / "model.png").is_file():
        icon = f"modelica://FMI/Resources/FMUs/{hash}/model.png"
    elif model_description.fmiVersion.startswith("3."):
        if (unzipdir / "terminalsAndIcons" / "icon.svg").is_file():
            icon = f"modelica://FMI/Resources/FMUs/{hash}/terminalsAndIcons/icon.svg"
        elif (unzipdir / "terminalsAndIcons" / "icon.png").is_file():
            icon = f"modelica://FMI/Resources/FMUs/{hash}/terminalsAndIcons/icon.png"

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
        icon=icon,
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
