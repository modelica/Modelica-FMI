from itertools import combinations


def import_fmu_to_modelica(fmu_path, model_path, interface_type, variables=None):

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

    if not (package_dir / 'package.mo').is_file():
        raise Exception(f"{package_dir} is not a package of a Modelica library.")

    model_description = read_model_description(fmu_path)

    if model_description.defaultExperiment is not None and model_description.defaultExperiment.stepSize is not None:
        communicationStepSize = model_description.defaultExperiment.stepSize
    else:
        communicationStepSize = '1e-2'

    if interface_type == 'ModelExchange':
        model_identifier = model_description.modelExchange.modelIdentifier
        copyPlatformBinary = model_description.modelExchange.canBeInstantiatedOnlyOncePerProcess
        IT = 'ME'
    elif interface_type == 'CoSimulation':
        model_identifier = model_description.coSimulation.modelIdentifier
        copyPlatformBinary = model_description.coSimulation.canBeInstantiatedOnlyOncePerProcess
        IT = 'CS'
    else:
        raise Exception(f"interface_type must be 'ModelExchange' or 'CoSimulation', but was '{interface_type}'.")

    package_root = package_dir

    package = package_root.name

    while (package_root.parent / 'package.order').is_file():
        package_root = package_root.parent
        package = package_root.name + '.' + package

    hash = sha256_checksum(fmu_path)

    hash = hash[:7]

    unzipdir = package_root / 'Resources' / 'FMUs' / hash

    makedirs(unzipdir, exist_ok=True)

    extract(filename=fmu_path, unzipdir=unzipdir)

    loader = jinja2.FileSystemLoader(searchpath=Path(__file__).parent / 'templates')

    environment = jinja2.Environment(loader=loader, trim_blocks=True, block_start_string='@@',
                                     block_end_string='@@', variable_start_string='@=', variable_end_string='=@')

    fmiMajorVersion = int(model_description.fmiVersion[0])

    template = environment.get_template(f'FMI{fmiMajorVersion}_{IT}.mo')

    parameters = []
    structural_parameters = []

    inputs = []
    outputs = []

    for variable in model_description.modelVariables:

        if variables is not None and variable.name not in variables:
            continue

        if variable.type not in {'Float32', 'Float64', 'Int32', 'UInt64', 'Real', 'Integer', 'Enumeration', 'Boolean', 'String'}:
            continue

        if (interface_type == 'ModelExchange' and variable.type in {'Real', 'Float32', 'Float64'}
                and variable.variability == 'discrete'):
            continue

        if variable.causality == 'parameter':
            parameters.append(variable)
        elif variable.causality == 'structuralParameter':
            structural_parameters.append(variable)
        elif variable.causality == 'input':
            inputs.append(variable)
        elif variable.causality == 'output':
            outputs.append(variable)

    width = 1200
    height = max(200, 100 * max(len(inputs), len(outputs)))

    x0 = -int(width / 2)
    x1 = int(width / 2)
    y0 = -int(height / 2)
    y1 = int(height / 2)

    annotations = dict()

    for i, variable in enumerate(inputs):
        y = y1 - (i + 1) * (height / (1 + len(inputs)))
        annotations[variable.name] = f'annotation (Placement(transformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }}), iconTransformation(extent={{ {{ {x0 - 40}, {y - 20} }}, {{ {x0}, {y + 20} }} }})))'

    for i, variable in enumerate(outputs):
        y = y1 - (i + 1) * (height / (1 + len(outputs)))
        annotations[variable.name] = f'annotation (Placement(transformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }}), iconTransformation(extent={{ {{ {x1}, {y - 10} }}, {{ {x1 + 20}, {y + 10} }} }})))'

    def as_array(values, default):
        if len(values) > 0:
            return '{ ' + ', '.join(map(str, values)) + ' }'
        else:
            return f'fill({default}, 0)'

    def as_quoted_array(values, default):
        if len(values) > 0:
            return '{ ' + ', '.join(map(lambda v: f"'{v}'", values)) + ' }'
        else:
            return f'fill({default}, 0)'

    variables = dict((variable.valueReference, variable) for variable in model_description.modelVariables)

    def subscripts(variable):
        if variable.dimensions:
            s = []
            for dimension in variable.dimensions:
                if dimension.start:
                    s.append(str(dimension.start))
                else:
                    v = variables[dimension.valueReference]
                    s.append(v.start)
                    # s.append(name(v))

            return '[' + ','.join(s) + ']'
        else:
            return ''

    def start_value(variable):

        if variable.dimensions:
            values = variable.start.split(' ')
            if len(variable.dimensions) == 1:
                return '{' + ', '.join(values) + '}'
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
                # s = []
                # for i in range(m):
                #     s.append('{' + ','.join(values[i*n:(i+1)*n]) + '}')
                # return '{' + ', '.join(s) + '}'
                s = []
                for i in range(m):
                    s.append(', '.join(values[i*n:(i+1)*n]))
                return '[' + '; '.join(s) + ']'


            else:
                raise Exception("Unsupported number of dimensions.")
        if variable.type == 'Boolean':
            return 'true' if variable.start in ['true', '1'] else 'false'
        elif variable.type == 'String':
            return f'"{variable.start}"'
        else:
            return str(variable.start)

    def modelica_type(variable):
        if variable.declaredType is not None and variable.declaredType.name.startswith('Modelica.'):
            return variable.declaredType.name
        else:
            return variable.type

    def fmi2_type(variable, prefix=False):
        p = 'FMI2' if prefix else ''
        if variable.type == 'Enumeration':
            return f'{p}Integer'
        else:
            return f'{p}{variable.type}'

    def fmi3_type(variable, prefix=False):
        p = 'FMI3' if prefix else ''
        if variable.type == 'Enumeration':
            return f'{p}Int64'
        else:
            return f'{p}{variable.type}'

    def attributes(variable, start=False):

        attrs = []

        if start:
            attrs.append(f"start={name(variable, suffix='_start')}")

        if variable.unit is not None:
            attrs.append(f'unit="{variable.unit}"')
        elif variable.declaredType is not None and variable.declaredType.unit is not None:
            attrs.append(f'unit="{variable.declaredType.unit}"')

        if variable.quantity is not None:
            attrs.append(f'quantity="{variable.quantity}"')
        elif variable.declaredType is not None and variable.declaredType.quantity is not None:
            attrs.append(f'quantity="{variable.declaredType.quantity}"')

        if attrs:
            return '(' + ', '.join(attrs) + ')'

        return ''

    def choices(variable):
        if variable.type == 'Enumeration':
            choices = [f'choice={item.value} "{item.name}"' for item in variable.declaredType.items]
            return ' annotation(choices(' + ', '.join(choices) + '))'
        else:
            return ''

    def name(variable, suffix=''):
        import re
        ex = re.compile(r'^[a-zA-Z_$][\w$]*$')
        if ex.match(variable.name):
            return variable.name + suffix
        else:
            # return f"'{variable.name}{suffix}'"
            return re.sub('[^0-9a-zA-Z]+', '_', f"{variable.name}{suffix}")

    def dependencies(variable, type, nValues=False):

        variables = {}

        unknown = next(filter(lambda u: u.variable.name == variable.name, model_description.initialUnknowns))

        for dependency in unknown.dependencies:
            l = variables.get(dependency.type, [])
            l.append((name(dependency), str(dependency.valueReference)))
            variables[dependency.type] = l

        # default_values = {
        #     'Real': '0.0', 'Integer': '0', 'Boolean': 'false',
        #     'Float64': '0.0',
        # }

        if type in variables:
            names, vrs = zip(*variables[type])
            n = f'{len(names)}, ' if nValues else ''
            arguments = '{' + ', '.join(vrs) + '}, ' + n + '{' + ', '.join(names) + '}'
        else:
            n = '0, ' if nValues else ''
            default = 'false' if type == 'Boolean' else 0
            arguments = f'fill(0, 0), {n}fill({default}, 0)'

        return arguments

    def dependencies2(variable):
        unknown = next(filter(lambda u: u.variable.name == variable.name, model_description.initialUnknowns))
        return unknown.dependencies

    def numel(variable):
        n = 1
        for dimension in variable.dimensions:
            n *= int(dimension.start if dimension.start else variables[dimension.valueReference].start)
        return n

    def shape(variable):
        s = []
        for dimension in variable.dimensions:
            s.append(int(dimension.start if dimension.start else variables[dimension.valueReference].start))
        return tuple(s)

    def dependencies3(variable, type):

        if type == 'Boolean':
            default = 'fill(0, 0), fill(false, 0)'
        elif type in {'Float64', 'Float32', 'Real'}:
            default = 'fill(0, 0), fill(0.0, 0)'
        else:
            default = 'fill(0, 0), fill(0, 0)'

        unknowns = list(filter(lambda u: u.variable.name == variable.name, model_description.initialUnknowns))

        if not unknowns:
            return default

        unknown = unknowns[0]

        vrs = []
        values = []
        for dependency in unknown.dependencies:
            if dependency.type == type:
                vrs.append(str(dependency.valueReference))
                s = shape(dependency)
                if not s:
                    values.append(name(dependency))
                elif len(s) == 1:
                    for i in range(1, numel(dependency) + 1):
                        values.append(f'{name(dependency)}[{i}]')
                elif len(s) == 2:
                    for i in range(1, s[0] + 1):
                        for j in range(1, s[1] + 1):
                            values.append(f'{name(dependency)}[{i},{j}]')
                else:
                    pass

        if not vrs:
            return default

        return '{' + ', '.join(vrs) + '}, {' + ', '.join(values) + '}'

    template.globals.update({
        'as_array': as_array,
        'as_quoted_array': as_quoted_array,
        'start_value': start_value,
        'subscripts': subscripts,
        'modelica_type': modelica_type,
        'fmi_type': fmi2_type if fmiMajorVersion == 2 else fmi3_type,
        'attributes': attributes,
        'choices': choices,
        'name': name,
        'dependencies': dependencies,
        'dependencies2': dependencies2,
        'dependencies3': dependencies3,
        'numel': numel
    })

    stopTime = getattr(model_description.defaultExperiment, 'stopTime', 1)

    class_text = template.render(
        hash=hash,
        libraryVersion=library_version,
        rootPackage=package_root.name,
        package=package,
        description=model_description.description,
        fmiMajorVersion=fmiMajorVersion,
        modelName=model_name,
        modelIdentifier=model_identifier,
        interfaceType=0 if interface_type == 'ModelExchange' else 1,
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
        continuousOutputs=[v for v in outputs if v.variability == 'continuous'],
        discreteOutputs=[v for v in outputs if v.variability == 'discrete'],
        annotations=annotations,
        realInputVRs=[str(v.valueReference) for v in inputs if v.type == 'Real'],
        realInputs=[v.name for v in inputs if v.type == 'Real'],
        integerInputVRs=[str(v.valueReference) for v in inputs if v.type == 'Integer'],
        integerInputs=[v.name for v in inputs if v.type == 'Integer'],
        booleanInputVRs=[str(v.valueReference) for v in inputs if v.type == 'Boolean'],
        booleanInputs=[v.name for v in inputs if v.type == 'Boolean'],
        stopTime=stopTime,
        copyPlatformBinary=copyPlatformBinary
    )

    with open(model_path, 'w') as f:
        f.write(class_text)

    if (package_dir / 'package.order').is_file():
        with open(package_dir / 'package.order', 'r') as f:
            package_order = list(map(lambda l: l.strip(), f.readlines()))
    else:
        package_order = []

    if model_name not in package_order:
        with open(package_dir / 'package.order', 'a') as f:
            f.write(model_name + '\n')


if __name__ == '__main__':

    import argparse

    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter)

    parser.add_argument('filename', help="filename of the FMU")
    parser.add_argument('package', help="filename of the package.mo")
    parser.add_argument('interfaceType', help="interface type")

    args = parser.parse_args()

    print(args)

    import_fmu_to_modelica(fmu_path=args.filename, model_path=args.package, interface_type=args.interfaceType)
