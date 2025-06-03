import os.path

from fmpy.util import download_file
from .import_fmu_to_modelica import import_fmu_to_modelica


models = [
    [
        "Dymola",
        [
            (
                "https://github.com/CATIA-Systems/dymola-fmi-compatibility/raw/refs/heads/main/2025x%20Refresh%201,%202025-04-11/CoupledClutches_fmi2_Dassl.fmu",
                "3f2dfb10c7ee6b7a1c16a51e6f2c34796bf47b0e7d62342922170dbee79461eb",
            ),
            (
                "https://github.com/CATIA-Systems/dymola-fmi-compatibility/raw/refs/heads/main/2025x%20Refresh%201,%202025-04-11/CoupledClutches_fmi3_Dassl.fmu",
                "b45f7f2d71ad2f29f1af24de82b518c98feb53e57a0ad9680e079dcfbc758923",
            ),
        ],
    ],
    [
        "Dymola",
        [
            (
                "https://github.com/CATIA-Systems/dymola-fmi-compatibility/raw/refs/heads/main/2025x%20Refresh%201,%202025-04-11/CoupledClutches_fmi2_Dassl.fmu",
                "3f2dfb10c7ee6b7a1c16a51e6f2c34796bf47b0e7d62342922170dbee79461eb",
            ),
            (
                "https://github.com/CATIA-Systems/dymola-fmi-compatibility/raw/refs/heads/main/2025x%20Refresh%201,%202025-04-11/CoupledClutches_fmi3_Dassl.fmu",
                "b45f7f2d71ad2f29f1af24de82b518c98feb53e57a0ad9680e079dcfbc758923",
            ),
        ],
    ],
]


def import_dymola_fmus():
    for url, checksum in models:
        filename = download_file(url, checksum)
        model_name, _ = os.path.splitext(filename)
        model_path = rf"E:\WS\Modelica-FMI\FMI\Tests\Dymola\{model_name}.mo"
        import_fmu_to_modelica(filename, model_path)


if __name__ == "__main__":
    import_dymola_fmus()
