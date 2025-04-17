within FMI.Internal.FMI3;
impure function FMI3GetBooleanMatrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Boolean values[m, n];

  external"C" FMU_FMI3GetBoolean(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetBooleanMatrix;
