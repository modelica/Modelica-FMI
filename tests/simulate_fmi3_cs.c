#include <stdio.h>
#include "ModelicaUtilities.h"
#include "ModelicaFMI.h"


int main() {

    size_t i;
    const double g = -9.81;
    double h;
    const int vr_g = 5;
    const int vr_h = 1;

    void* instance = FMU_load(
        UNZIP_DIR,
        3,
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
    
    FMU_FMI3SetFloat64(instance, &vr_g, 1, &g, 1);

    FMU_FMI3EnterInitializationMode(instance, 0, 0.0, 0.0, 0, 0.0);

    FMU_FMI3ExitInitializationMode(instance);

    for (i = 0; i < 10; i++) {

        FMU_FMI3GetFloat64(instance, 1, &h, 1);
        
        FMU_FMI3DoStep(instance, i * 0.1, 0.1);
    }
    
    FMU_free(instance);

    return 0;
}
