within FMI.FMI2.Functions;
impure function FMI2GetEventIndicators
  input Internal.ExternalFMU instance;
  input Integer ni;
  output Real eventIndicators[ni];
  external"C" FMU_FMI2GetEventIndicators(instance, eventIndicators, ni) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2GetEventIndicators;
