def generate_examples():

    from fmpy import extract
    from fmpy.util import download_file
    from pathlib import Path
    from .import_fmu_to_modelica import import_fmu_to_modelica

    url = 'https://github.com/modelica/Reference-FMUs/releases/download/v0.0.29/Reference-FMUs-0.0.29.zip'
    checksum = 'cf17e4e8ca0db0965afc5d4c968ab1a94d6328c8941e20496e69e5c0ee6836f1'
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
