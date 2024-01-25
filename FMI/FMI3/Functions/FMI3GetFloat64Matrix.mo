within FMI.FMI3.Functions;
impure function FMI3GetFloat64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Float64 values[m, n];

  external"C" FMU_FMI3GetFloat64(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3GetFloat64Matrix;
