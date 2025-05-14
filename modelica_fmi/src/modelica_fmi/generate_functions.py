from itertools import product
from pathlib import Path
import jinja2


loader = jinja2.FileSystemLoader(searchpath=Path(__file__).parent / 'templates')

environment = jinja2.Environment(
    loader=loader,
    trim_blocks=True,
    block_start_string='@@',
    block_end_string='@@',
    variable_start_string='@=',
    variable_end_string='=@'
)

library_dir = Path(__file__).parent.parent.parent.parent / 'FMI'

for variable_type, prefix in product(['Real', 'Integer', 'Boolean', 'String'], ['Get', 'Set']):

    for package, level in [
        (library_dir / 'FMI2' / 'Functions', ''),
        (library_dir / 'Internal' / 'FMI2', 'Internal')
    ]:

        template = environment.get_template(f'FMI2_{level}{prefix}.mo')

        class_text = template.render(
            variable_type=variable_type
        )

        function_name = f'FMI2{prefix}{variable_type}'

        with open(package / f'{function_name}.mo', 'w') as f:
            f.write(class_text)

        package_order_file = package / 'package.order'

        with open(package_order_file, 'r') as f:
            package_order = list(map(lambda l: l.strip(), f.readlines()))

        if function_name not in package_order:
            with open(package_order_file, 'a') as f:
                f.write(function_name + '\n')

for variable_type, (prefix, suffix) in product(
        ['Float32', 'Float64', 'Int8', 'UInt8', 'Int16', 'UInt16', 'Int32', 'UInt32', 'Int64', 'UInt64', 'Boolean', 'String'],
        [('Get', ''), ('Set', ''), ('Get', 'Matrix'), ('Set', 'Matrix')]
):

    for package, level in [
        (library_dir / 'FMI3' / 'Functions', ''),
        (library_dir / 'Internal' / 'FMI3', 'Internal')
    ]:

        template = environment.get_template(f'FMI3_{level}{prefix}{suffix}.mo')

        class_text = template.render(
            variable_type=variable_type
        )

        function_name = f'FMI3{prefix}{variable_type}{suffix}'

        with open(package / f'{function_name}.mo', 'w') as f:
            f.write(class_text)

        package_order_file = package / 'package.order'

        with open(package_order_file, 'r') as f:
            package_order = list(map(lambda l: l.strip(), f.readlines()))

        if function_name not in package_order:
            with open(package_order_file, 'a') as f:
                f.write(function_name + '\n')
