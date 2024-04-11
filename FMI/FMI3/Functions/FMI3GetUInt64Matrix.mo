within FMI.FMI3.Functions;
impure function FMI3GetUInt64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt64 values[m, n];

  external"C" FMU_FMI3GetUInt64(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3GetUInt64Matrix;
