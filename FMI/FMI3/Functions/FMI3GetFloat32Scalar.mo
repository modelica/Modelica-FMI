within FMI.FMI3.Functions;
impure function FMI3GetFloat32Scalar
  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Real t = 0;
  output Real value;
algorithm
  value :=scalar(Functions.FMI3GetFloat32(
      externalFMU,
      {valueReference},
      1));
end FMI3GetFloat32Scalar;
