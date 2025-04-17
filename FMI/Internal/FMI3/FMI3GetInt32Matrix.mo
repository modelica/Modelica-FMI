within FMI.Internal.FMI3;
impure function FMI3GetInt32Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int32 values[m, n];

  external"C" FMU_FMI3GetInt32(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetInt32Matrix;
