def generate_examples():

    from fmpy import extract
    from fmpy.util import download_file
    from pathlib import Path
    from modelica_fmi.import_fmu_to_modelica import import_fmu_to_modelica

    url = 'https://github.com/modelica/Reference-FMUs/releases/download/v0.0.23/Reference-FMUs-0.0.23.zip'
    checksum = 'd6ad6fc08e53053fe413540016552387257971261f26f08a855f9a6404ef2991'
    archive = download_file(url=url, checksum=checksum)

    dist = Path(extract(archive))

    modelica = Path(__file__).parent.parent

    for fmi_version in [2, 3]:
        for interface_type in ['CoSimulation', 'ModelExchange']:
            for model in ['BouncingBall', 'Dahlquist', 'Feedthrough', 'Stair', 'Resource', 'VanDerPol']:
                import_fmu_to_modelica(
                    fmu_path=dist / f'{fmi_version}.0' / f'{model}.fmu',
                    model_path=modelica / 'FMI' / 'Examples' / f'FMI{fmi_version}' / interface_type / f'{model}.mo',
                    interface_type=interface_type,
                )


if __name__ == '__main__':
    generate_examples()