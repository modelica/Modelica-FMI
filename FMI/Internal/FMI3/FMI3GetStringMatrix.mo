within FMI.Internal.FMI3;
impure function FMI3GetStringMatrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3String values[m, n];

  external"C" FMU_FMI3GetString(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetStringMatrix;
