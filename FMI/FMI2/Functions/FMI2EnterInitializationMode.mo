within FMI.FMI2.Functions;
impure function FMI2EnterInitializationMode
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI2EnterInitializationMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2EnterInitializationMode;
