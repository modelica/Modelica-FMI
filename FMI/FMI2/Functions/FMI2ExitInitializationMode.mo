within FMI.FMI2.Functions;
impure function FMI2ExitInitializationMode
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI2ExitInitializationMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2ExitInitializationMode;
