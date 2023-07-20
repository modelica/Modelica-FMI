within FMI.Internal.FMI3;
impure function FMI3GetInt64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int64 values[m, n];

  external"C" FMU_FMI3GetInt64(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetInt64Matrix;
