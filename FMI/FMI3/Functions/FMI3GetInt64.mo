within FMI.FMI3.Functions;
impure function FMI3GetInt64
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int64 values[nValues];

  external"C" FMU_FMI3GetInt64(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3GetInt64;
