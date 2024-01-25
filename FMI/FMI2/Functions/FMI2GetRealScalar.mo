within FMI.FMI2.Functions;
impure function FMI2GetRealScalar
  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Real t = 0;
  output Real value;
algorithm
  value := FMI2GetReal(externalFMU, valueReference);
end FMI2GetRealScalar;
