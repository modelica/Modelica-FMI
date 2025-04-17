"""The Command Line Interface (CLI)"""

def main():

    import argparse
    from .import_fmu_to_modelica import import_fmu_to_modelica

    parser = argparse.ArgumentParser(
        description="Import an FMU into a Modelica library",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument('fmu_path', help="Path of the FMU to import")
    parser.add_argument('model_path', help="Path of the Modelica file to generate")

    args = parser.parse_args()

    import_fmu_to_modelica(fmu_path=args.fmu_path, model_path=args.model_path, interface_type='CoSimulation')
