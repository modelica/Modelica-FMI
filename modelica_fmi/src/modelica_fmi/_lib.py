"""Functions for the Modelica code generation"""

from math import prod
from typing import Iterable

import numpy as np
from fmpy.model_description import ModelVariable, SimpleType, ModelDescription, Item


def name(variable: ModelVariable | Item, suffix: str = "") -> str:
    """Convert the FMI variable name to a valid Modelica identifier"""
    import re

    ex = re.compile(r"^[a-zA-Z_$][\w$]*$")
    if ex.match(variable.name):
        return variable.name + suffix
    else:
        return re.sub("[^0-9a-zA-Z]+", "_", f"{variable.name}{suffix}")


def fmi2_type(
    variable: ModelVariable, prefix: bool = False, declared: bool = False
) -> str:
    p = "FMI2" if prefix else ""
    if variable.type == "Enumeration":
        if declared:
            return f"Types.{variable.declaredType.name}"
        else:
            return f"{p}Integer"
    else:
        return f"{p}{variable.type}"


def fmi3_type(
    variable: ModelVariable | SimpleType,
    prefix: bool = False,
    declared: bool = False,
) -> str:
    p = "FMI3" if prefix else ""
    if isinstance(variable, SimpleType):
        return f"{p}Int64"
    elif variable.type == "Enumeration":
        if declared:
            return f"Types.{variable.declaredType.name}"
        else:
            return f"{p}Int64"
    else:
        return f"{p}{variable.type}"


def flatten(variable: ModelVariable, pre: bool = False) -> str:
    """Convert an n-dimensional Modelica variable to a vector"""

    variable_expr = name(variable)

    if pre:
        variable_expr = f"pre({variable_expr})"

    if not variable.dimensions:
        return f"{{{variable_expr}}}"
    elif len(variable.dimensions) == 1:
        return variable_expr
    else:
        s = shape(variable)
        expression = "cat(1"
        for i in np.ndindex(s[:-1]):
            i = map(lambda j: str(j + 1), i)
            expression += f", {variable_expr}[" + ",".join(i) + ",:]"
        return expression + ")"


def get_variables(
    variables: Iterable[ModelVariable], indent: int = 2, prefix: str = ""
) -> str:
    lines = []
    for variable in variables:
        if len(variable.dimensions) > 1:
            raise Exception(
                "Output variables with more than one dimension are not supported."
            )
        line = f"{prefix}{name(variable)} := "
        if not variable.dimensions:
            line += "scalar("
        if variable.type == "Enumeration":
            line += f"Types.Int64To{variable.declaredType.name}("
        line += f"FMI3Get{fmi3_type(variable)}(instance, valueReference={variable.valueReference}, nValues={numel(variable)})"
        if variable.type == "Enumeration":
            line += ")"
        if not variable.dimensions:
            line += ")"
        lines.append(line + ";")
    return " " * indent + ("\n" + " " * indent).join(lines)


def get_initial_variable(model_description: ModelDescription, variable: ModelVariable):
    indent = 4
    if len(variable.dimensions) > 1:
        raise Exception(
            "Output variables with more than one dimension are not supported."
        )
    line = f"{name(variable)} = "
    if not variable.dimensions:
        line += "scalar("
    if variable.type == "Enumeration":
        line += f"Types.Int64To{variable.declaredType.name}("
    deps = dependencies3(
        model_description,
        variable,
        [
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
            "Boolean",
        ],
    )
    line += f"pure(FMI3GetInitial{fmi3_type(variable)}(instance, startTime, {deps}valueReference={variable.valueReference}, nValues={numel(variable)}))"
    if variable.type == "Enumeration":
        line += ")"
    if not variable.dimensions:
        line += ")"
    line += ";"
    return " " * indent + line


def set_variables(
    variables: Iterable[ModelVariable], indent: int = 2, pre: bool = False
) -> str:
    if not variables:
        return ""
    lines = []
    for variable in variables:
        line = " " * indent + f"FMI3Set{fmi3_type(variable)}"
        line += f"(instance, valueReferences={{{variable.valueReference}}}, values="
        if variable.type == "Enumeration":
            line += f"Types.{variable.declaredType.name}ToInt64("
        line += flatten(variable, pre)
        if variable.type == "Enumeration":
            line += ")"
        lines.append(line + ");")
    return "\n" + "\n".join(lines)


def subscripts(variables: dict[int, ModelVariable], variable: ModelVariable):
    if variable.dimensions:
        s = []
        for dimension in variable.dimensions:
            if dimension.start:
                s.append(str(dimension.start))
            else:
                v = variables[dimension.valueReference]
                s.append(v.start)

        return "[" + ",".join(s) + "]"
    else:
        return ""


def start_value(variables: dict[int, ModelVariable], variable: ModelVariable) -> str:
    if variable.dimensions:
        values = variable.start.split(" ")

        if variable.type == "Enumeration":

            def value2enum(value: str) -> str:
                return next(
                    f"Types.{variable.declaredType.name}.{name(item)}"
                    for item in variable.declaredType.items
                    if item.value == value
                )

            values = list(map(value2enum, values))

        if len(variable.dimensions) == 1:
            return "{" + ", ".join(values) + "}"
        elif len(variable.dimensions) == 2:
            if variable.dimensions[0].start:
                m = int(variable.dimensions[0].start)
            else:
                v = variables[variable.dimensions[0].valueReference]
                m = int(v.start)
            if variable.dimensions[1].start:
                n = int(variable.dimensions[1].start)
            else:
                v = variables[variable.dimensions[1].valueReference]
                n = int(v.start)
            s = []
            for i in range(m):
                s.append(", ".join(values[i * n : (i + 1) * n]))
            return "[" + "; ".join(s) + "]"
        else:
            raise Exception("Unsupported number of dimensions.")

    if variable.type == "Boolean":
        return "true" if variable.start in ["true", "1"] else "false"
    elif variable.type == "String":
        return f'"{variable.start}"'
    elif variable.type == "Enumeration":
        item = next(
            item for item in variable.declaredType.items if item.value == variable.start
        )
        return f"Types.{variable.declaredType.name}.{name(item)}"
    else:
        return str(variable.start)


def modelica_type(variable):
    if variable.declaredType is not None and variable.declaredType.name.startswith(
        "Modelica."
    ):
        return variable.declaredType.name
    else:
        return variable.type


def modifiers(variable: ModelVariable, start: bool = False):
    attrs = []

    prefix = "each " if variable.dimensions else ""

    if start:
        attrs.append(f"start={name(variable, suffix='_start')}")

    if variable.unit is not None:
        attrs.append(f'{prefix}unit="{variable.unit}"')
    elif variable.declaredType is not None and variable.declaredType.unit is not None:
        attrs.append(f'{prefix}unit="{variable.declaredType.unit}"')

    if variable.quantity is not None:
        attrs.append(f'{prefix}quantity="{variable.quantity}"')
    elif (
        variable.declaredType is not None and variable.declaredType.quantity is not None
    ):
        attrs.append(f'{prefix}quantity="{variable.declaredType.quantity}"')

    if attrs:
        return "(" + ", ".join(attrs) + ")"

    return ""


def dependencies(
    model_description: ModelDescription, variable: ModelVariable, type, nValues=False
):
    variables = {}

    unknown = next(
        filter(
            lambda u: u.variable.name == variable.name,
            model_description.initialUnknowns,
        )
    )

    for dependency in unknown.dependencies:
        dependencies = variables.get(dependency.type, [])
        dependencies.append((name(dependency), str(dependency.valueReference)))
        variables[dependency.type] = dependencies

    if type in variables:
        names, vrs = zip(*variables[type])
        n = f"{len(names)}, " if nValues else ""
        arguments = "{" + ", ".join(vrs) + "}, " + n + "{" + ", ".join(names) + "}"
    else:
        n = "0, " if nValues else ""
        default = "false" if type == "Boolean" else 0
        arguments = f"fill(0, 0), {n}fill({default}, 0)"

    return arguments


def dependencies2(model_description: ModelDescription, variable):
    unknown = next(
        filter(
            lambda u: u.variable.name == variable.name,
            model_description.initialUnknowns,
        )
    )
    return unknown.dependencies


def numel(variable: ModelVariable) -> int:
    if not variable.dimensions:
        return 1
    else:
        return prod(shape(variable))


def shape(variable: ModelVariable) -> tuple[int, ...]:
    return tuple(
        int(d.start if d.start else d.variable.start) for d in variable.dimensions
    )


def dependencies3(
    model_description: ModelDescription,
    variable: ModelVariable,
    types: list[str],
):
    arguments = []

    for type in types:
        unknowns = list(
            filter(
                lambda u: u.variable.name == variable.name,
                model_description.initialUnknowns,
            )
        )

        if not unknowns:
            continue

        unknown = unknowns[0]

        vrs = []
        values = []

        for dependency in unknown.dependencies:
            if dependency.type != type or dependency.causality != "input":
                continue

            vrs.append(str(dependency.valueReference))
            s = shape(dependency)

            if not s:
                values.append(name(dependency))
            elif len(s) == 1:
                for i in range(1, numel(dependency) + 1):
                    values.append(f"{name(dependency)}[{i}]")
            elif len(s) == 2:
                for i in range(1, s[0] + 1):
                    for j in range(1, s[1] + 1):
                        values.append(f"{name(dependency)}[{i},{j}]")

        if vrs:
            arguments.append(
                f"{type.lower()}InputValueReferences"
                + "={"
                + ", ".join(vrs)
                + "}, "
                + f"{type.lower()}InputValues="
                + "{"
                + ", ".join(values)
                + "}, "
            )

    return "".join(arguments)
