def generate_examples():

    from fmpy import extract
    from fmpy.util import download_file
    from pathlib import Path
    from .import_fmu_to_modelica import import_fmu_to_modelica

    url = 'https://github.com/modelica/Reference-FMUs/releases/download/v0.0.38/Reference-FMUs-0.0.38.zip'
    checksum = 'dc3d1467ddea24cf6d171f7eb4d790d7d6f3f7b8856cf76c0cd832c78e8e629a'
    archive = download_file(url=url, checksum=checksum)

    dist = Path(extract(archive))

    examples_dir = Path(__file__).parent.parent.parent.parent / 'FMI' / 'Examples'

    for fmi_version in [2, 3]:

        models = ['BouncingBall', 'Dahlquist', 'Feedthrough', 'Stair', 'Resource', 'VanDerPol']

        if fmi_version == 3:
            models.append('StateSpace')

        for model in models:
            import_fmu_to_modelica(
                fmu_path=dist / f'{fmi_version}.0' / f'{model}.fmu',
                model_path=examples_dir / f'FMI{fmi_version}' / f'{model}.mo',
                interface_type='CoSimulation',
            )


if __name__ == '__main__':
    generate_examples()
