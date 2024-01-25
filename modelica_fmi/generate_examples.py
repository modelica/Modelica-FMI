def generate_examples():

    from fmpy import extract
    from fmpy.util import download_file
    from pathlib import Path
    from modelica_fmi.import_fmu_to_modelica import import_fmu_to_modelica

    url = 'https://github.com/modelica/Reference-FMUs/releases/download/v0.0.29/Reference-FMUs-0.0.29.zip'
    checksum = 'cf17e4e8ca0db0965afc5d4c968ab1a94d6328c8941e20496e69e5c0ee6836f1'
    archive = download_file(url=url, checksum=checksum)

    dist = Path(extract(archive))

    modelica = Path(__file__).parent.parent

    for fmi_version in [2, 3]:
        for interface_type in ['CoSimulation']:  # , 'ModelExchange']:
            for model in ['BouncingBall', 'Dahlquist', 'Feedthrough', 'Stair', 'Resource', 'VanDerPol']:
                import_fmu_to_modelica(
                    fmu_path=dist / f'{fmi_version}.0' / f'{model}.fmu',
                    model_path=modelica / 'FMI' / 'Examples' / f'FMI{fmi_version}' / interface_type / f'{model}.mo',
                    interface_type=interface_type,
                )


if __name__ == '__main__':
    generate_examples()
