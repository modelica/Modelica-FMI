#include <stddef.h>
#include "ModelicaFMI.h"
#include "ModelicaFMI2.h"


int main() {

    size_t i;
    const double g = -9.81;
    double h;
    const int vr_g = 5;
    const int vr_h = 1;

    FMUInstance* instance = FMUCreate();
        
    FMULoad(
        instance,
        UNZIP_DIR,
        2,
        "BouncingBall",
        "bouncingBall",
        1,
        INSTANTIATION_TOKEN,
        0,
        0,
        1,
        0,
        NULL,
        0
    );

    FMU_FMI2SetReal(instance, &vr_g, 1, &g);

    FMU_FMI2SetupExperiment(instance, 0, 0.0, 0.0, 0, 0.0);

    FMU_FMI2EnterInitializationMode(instance);

    FMU_FMI2ExitInitializationMode(instance);

    for (i = 0; i < 10; i++) {

        FMU_FMI2GetReal(instance, 1, &h);

        FMU_FMI2DoStep(instance, i * 0.1, 0.1, 1);
    }

    FMUFree(instance);

    return 0;
}
