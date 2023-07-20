within FMI.FMI3.Functions;
impure function FMI3EnterEventMode
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3EnterEventMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3EnterEventMode;
