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


fmi2_functions_dir = Path(__file__).parent.parent / 'FMI' / 'FMI2' / 'Functions'

for variable_type, prefix in product(['Real', 'Integer', 'Boolean', 'String'], ['Get', 'Set']):

    template = environment.get_template(f'FMI2_{prefix}.mo')

    class_text = template.render(
        variable_type=variable_type
    )

    function_name = f'FMI2{prefix}{variable_type}'

    with open(fmi2_functions_dir / f'{function_name}.mo', 'w') as f:
        f.write(class_text)

    package_order_file = Path(fmi2_functions_dir / 'package.order')

    with open(package_order_file, 'r') as f:
        package_order = list(map(lambda l: l.strip(), f.readlines()))

    if function_name not in package_order:
        with open(package_order_file, 'a') as f:
            f.write(function_name + '\n')


fmi3_functions_dir = Path(__file__).parent.parent / 'FMI' / 'FMI3' / 'Functions'

for variable_type, (prefix, suffix) in product(
        ['Float32', 'Float64', 'Int32', 'Int64', 'Boolean', 'String'],
        [('Get', ''), ('Set', ''), ('Get', 'Matrix'), ('Set', 'Matrix')]
):

    template = environment.get_template(f'FMI3_{prefix}{suffix}.mo')

    class_text = template.render(
        variable_type=variable_type
    )

    function_name = f'FMI3{prefix}{variable_type}{suffix}'

    with open(fmi3_functions_dir / f'{function_name}.mo', 'w') as f:
        f.write(class_text)

    package_order_file = Path(fmi3_functions_dir / 'package.order')

    with open(package_order_file, 'r') as f:
        package_order = list(map(lambda l: l.strip(), f.readlines()))

    if function_name not in package_order:
        with open(package_order_file, 'a') as f:
            f.write(function_name + '\n')
