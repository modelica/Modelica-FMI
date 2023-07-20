within FMI.FMI2.Functions;
impure function FMI2EnterContinuousTimeMode
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI2EnterContinuousTimeMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2EnterContinuousTimeMode;
