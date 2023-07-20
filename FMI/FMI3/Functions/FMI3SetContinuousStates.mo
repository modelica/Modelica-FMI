within FMI.FMI3.Functions;
impure function FMI3SetContinuousStates
  input Internal.ExternalFMU instance;
    input Real continuousStates[nContinuousStates];
    input Integer nContinuousStates;
    external"C" FMU_FMI3SetContinuousStates(instance, continuousStates, nContinuousStates) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3SetContinuousStates;
