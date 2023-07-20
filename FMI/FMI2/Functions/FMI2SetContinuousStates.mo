within FMI.FMI2.Functions;
impure function FMI2SetContinuousStates
  input Internal.ExternalFMU instance;
    input Real x[nx];
    input Integer nx;
    external"C" FMU_FMI2SetContinuousStates(instance, x, nx) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2SetContinuousStates;
