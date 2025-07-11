"""The Command Line Interface (CLI)"""


def main():
    import argparse
    from . import __version__ as version
    from .import_fmu_to_modelica import import_fmu_to_modelica

    parser = argparse.ArgumentParser(
        prog="modelica-fmi",
        description="Import an FMU into a Modelica library",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument(
        "-v",
        "--version",
        action="version",
        version=f"%(prog)s version {version}",
        help="print the program version",
    )
    parser.add_argument(
        "--basic",
        action="store_true",
        help="Use basic co-simulation w/o initialization",
    )
    parser.add_argument(
        "--hide-connectors",
        action="store_true",
        help="Hide the connectors in the graphical representation",
    )
    parser.add_argument(
        "--hide-large-arrays",
        action="store_true",
        help="Hide large parameter arrays in Dymola",
    )
    parser.add_argument("fmu_path", help="Path of the FMU to import")
    parser.add_argument("model_path", help="Path of the Modelica file to generate")

    args = parser.parse_args()

    import_fmu_to_modelica(
        fmu_path=args.fmu_path,
        model_path=args.model_path,
        interface_type="CoSimulation",
        basic=args.basic,
        hide_connectors=args.hide_connectors,
        hide_large_arrays=args.hide_large_arrays,
    )
