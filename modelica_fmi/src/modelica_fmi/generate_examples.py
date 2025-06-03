import os
import shutil


def generate_examples():
    from fmpy import extract
    from fmpy.util import download_file
    from pathlib import Path
    from .import_fmu_to_modelica import import_fmu_to_modelica

    url = "https://github.com/modelica/Reference-FMUs/releases/download/v0.0.38/Reference-FMUs-0.0.38.zip"
    checksum = "dc3d1467ddea24cf6d171f7eb4d790d7d6f3f7b8856cf76c0cd832c78e8e629a"
    archive = download_file(url=url, checksum=checksum)

    dist = Path(extract(archive))

    examples_dir = Path(__file__).parent.parent.parent.parent / "FMI" / "Examples"

    shutil.rmtree(examples_dir, ignore_errors=True)

    os.makedirs(examples_dir)

    with open(examples_dir / "package.mo", "w") as file:
        file.write("""within FMI;
package Examples
extends Modelica.Icons.ExamplesPackage;
end Examples;
""")

    with open(examples_dir.parent / "package.order", "r") as f:
        package_order = list(map(lambda line: line.strip(), f.readlines()))

    if "Examples" not in package_order:
        with open(examples_dir.parent / "package.order", "a") as file:
            file.write("Examples\n")

    for fmi_version in [2, 3]:
        os.makedirs(examples_dir / f"FMI{fmi_version}")

        with open(examples_dir / f"FMI{fmi_version}" / "package.mo", "w") as file:
            file.write(f"""within FMI.Examples;
package FMI{fmi_version}
extends Modelica.Icons.ExamplesPackage;
end FMI{fmi_version};
""")

        with open(examples_dir / "package.order", "a") as file:
            file.write(f"FMI{fmi_version}\n")

        models = [
            "BouncingBall",
            "Dahlquist",
            "Feedthrough",
            "Stair",
            "Resource",
            "VanDerPol",
        ]

        if fmi_version == 3:
            models.append("StateSpace")

        for model in models:
            import_fmu_to_modelica(
                fmu_path=dist / f"{fmi_version}.0" / f"{model}.fmu",
                model_path=examples_dir / f"FMI{fmi_version}" / f"{model}.mo",
                interface_type="CoSimulation",
            )


if __name__ == "__main__":
    generate_examples()
